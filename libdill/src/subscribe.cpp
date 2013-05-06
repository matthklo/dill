/*
 * Copyright (c) 2013, hklo.tw@gmail.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <dill_subscriber.h>

#include <boost/lexical_cast.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <string>

#include "protocol.h"
#include "client_netio.h"

class DillSubscriberCallable : public DillClientNetIoCallable
{
    boost::mutex                  *_ctrl_mtx;
    boost::condition_variable     *_rpc_cond;
    bool                           _on_rpc;

    DILL_SUBSCRIBE_CBFN            _evcb;
    unsigned int                   _availch;
    std::string                    _chname;

public:
    DillSubscriberCallable(int port, const char * addr)
        : DillClientNetIoCallable(port, addr)
        , _ctrl_mtx(0)
        , _rpc_cond(0)
        , _evcb(0)
        , _availch(0)
        , _on_rpc(false)
    {
        _ctrl_mtx = new boost::mutex;
        _rpc_cond = new boost::condition_variable;
    }

    ~DillSubscriberCallable()
    {
        stop();
        delete _rpc_cond;
        delete _ctrl_mtx;
    }

    void registerEventCallback(DILL_SUBSCRIBE_CBFN fn)
    {
        _evcb = fn;
    }

    unsigned int subscribe(unsigned int chidx, bool subscribe = true)
    {
        if (isConnected())
        {
            DillParcel p;
            p.op = (subscribe)? DILL_PRIORITY_SUBSCRIBE : DILL_PRIORITY_UNSUBSCRIBE;
            p.id = chidx;
            p.timestamp1 = 0;
            p.timestamp2 = 0;

            asyncWriteParcel(&p);

            return DILL_EC_SUCCEED;
        }
        return DILL_EC_UNKNOWN;
    }

    unsigned int availableChannels()
    {
        if (isConnected())
        {
            DillParcel p;
            p.op = DILL_PRIORITY_AVAILCH;
            p.id = 0;
            p.timestamp1 = 0;
            p.timestamp2 = 0;

            asyncWriteParcel(&p);

            {
                boost::unique_lock<boost::mutex> lock(*_ctrl_mtx);
                _on_rpc = true;
                _rpc_cond->wait(lock);
            }
            return _availch;
        }
        
        return 0;
    }

    const char * channelName(unsigned int chidx)
    {
        if (isConnected())
        {
            DillParcel p;
            p.op = DILL_PRIORITY_AVAILCH;
            p.id = chidx;
            p.timestamp1 = 0;
            p.timestamp2 = 0;

            asyncWriteParcel(&p);

            {
                boost::unique_lock<boost::mutex> lock(*_ctrl_mtx);
                _on_rpc = true;
                _rpc_cond->wait(lock);
            }

            return _chname.c_str();
        }
        return "";
    }

protected:

    void onStarted()
    {
        asyncConnect();
    }

    void onConnected(bool connected)
    {
        if (connected)
        {
            asyncReadParcel();
        } else {
            onIoError();
        }
    }

    void onParcelRead(DillParcel *p)
    {
        switch(p->op)
        {
        case DILL_PRIORITY_VERBOSE:
        case DILL_PRIORITY_DEBUG:
        case DILL_PRIORITY_INFO:
        case DILL_PRIORITY_WARN:
        case DILL_PRIORITY_ERROR:
            {
                if (_evcb)
                {
                    DillEvent ev;
                    ev.Type = DILL_EVENT_LOG;
                    ev.ID = p->id;
                    ev.Priority = p->op;
                    ev.TimeStamp1 = p->timestamp1;
                    ev.TimeStamp2 = p->timestamp2;
                    ev.Tag = p->str1.c_str();
                    ev.Content = p->str2.c_str();

                    _evcb(&ev);
                }
            }
            break;
        case DILL_PRIORITY_REGISTER:
            {
                if (_evcb)
                {
                    DillEvent ev;
                    ev.Type = DILL_EVENT_REG;
                    ev.ID = p->id;
                    ev.Priority = 0;
                    ev.TimeStamp1 = 0;
                    ev.TimeStamp2 = 0;
                    ev.Tag = p->str1.c_str();
                    ev.Content = p->str2.c_str();

                    _evcb(&ev);
                }
            }
            break;
        case DILL_PRIORITY_AVAILCH:
            {
                _availch = p->timestamp1;
                _chname = p->str1;
                _on_rpc = false;
                _rpc_cond->notify_one();
            }
            break;
        }

        asyncReadParcel();
    }

    void onParcelSent(bool succeed)
    {
        if (!succeed)
            onIoError();
    }

    void onIoError()
    {
        if (_on_rpc)
        {
            _availch = 0;
            _chname = "";
            _rpc_cond->notify_one();
        } else {
            boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(3));
        }
        asyncConnect();
    }
};

std::shared_ptr<DillSubscriberCallable> _g_subscriber;

unsigned int dill_subscribe_init()
{
    return ::dill_subscribe_init_to(DILL_DEFAULT_PORT, DILL_DEFAULT_ADDR);
}

unsigned int dill_subscribe_init_to(int port, const char * addr)
{
    if (_g_subscriber.get() == 0)
    {
        _g_subscriber.reset(new DillSubscriberCallable(port, addr));
    }
    return DILL_EC_SUCCEED;
}

void dill_subscribe_deinit()
{
    if (_g_subscriber.get() != 0)
    {
        _g_subscriber.reset();
    }
}

void dill_subscribe_evcb(DILL_SUBSCRIBE_CBFN fn)
{
    if (_g_subscriber.get() != 0)
    {
        _g_subscriber->registerEventCallback(fn);
    }
}

unsigned int dill_available_channels()
{
    if (_g_subscriber.get() != 0)
    {
        return _g_subscriber->availableChannels();
    }
    return 0;
}

const char* dill_channel_name(unsigned int index)
{
    if (_g_subscriber.get() != 0)
    {
        return _g_subscriber->channelName(index);
    }
    return "";
}

unsigned int dill_subscribe_channel(unsigned int index, int subscribe)
{
    if (_g_subscriber.get() != 0)
    {
        return _g_subscriber->subscribe(index, (subscribe == DILL_SUBSCRIBE));
    }
    return DILL_EC_UNKNOWN;
}



namespace dill
{
    unsigned int subscribeInit(int port, const char * addr)
    {
        return ::dill_subscribe_init_to(port, addr);
    }

    void subscribeDeinit()
    {
        ::dill_subscribe_deinit();
    }
    
    unsigned int availableChannels()
    {
        return ::dill_available_channels();
    }

    const char* getChannelName(unsigned int index)
    {
        return ::dill_channel_name(index);
    }

    bool subscribeChannel(unsigned int index, bool subscribe)
    {
        return (::dill_subscribe_channel(index, ((subscribe)? DILL_SUBSCRIBE: DILL_UNSUBSCRIBE)) == DILL_EC_SUCCEED);
    }
    
    void subscribeEventCallback(DILL_SUBSCRIBE_CBFN fn)
    {
        ::dill_subscribe_evcb(fn);
    }
}


