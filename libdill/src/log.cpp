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

#include <dill_log.h>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include <cstring>
#include <string>
#include <deque>
#include <algorithm>
#include <memory>

#include "protocol.h"
#include "client_netio.h"

class DillLogAgentCallable : public DillClientNetIoCallable
{
    boost::mutex                 *_data_mtx;

    // members protected by _data_mtx
    std::deque< std::shared_ptr<DillParcel> > 
                                  _queue;
    bool                          _pending_logwrites;
    // --


    bool                          _loggable;
    unsigned int                  _channel_id;
    const std::string             _channel;

public:
    DillLogAgentCallable(const char * channel, int port, const char * addr)
        : DillClientNetIoCallable(port, addr)
        , _channel(channel)
        , _channel_id(0)
        , _pending_logwrites(false)
        , _loggable(false)
    {
        _data_mtx = new boost::mutex;
    }

    ~DillLogAgentCallable()
    {
        stop();
        delete _data_mtx;
    }

    bool post(unsigned char pri, const char * tag, const char * msg)
    {
        if (!_loggable)
            return false;

        unsigned int len1 = std::strlen(tag);
        unsigned int len2 = std::strlen(msg);

        boost::posix_time::ptime curtime = boost::posix_time::microsec_clock::universal_time();
        static boost::posix_time::ptime epochtime(boost::gregorian::date(1970,1,1)); 
        boost::posix_time::time_duration td = curtime - epochtime;

        std::shared_ptr<DillParcel> parcel(new DillParcel);
        parcel->op = pri;
        parcel->id = _channel_id;
        parcel->timestamp1 = (unsigned int) td.total_seconds();
        parcel->timestamp2 = (unsigned int) td.fractional_seconds();
        parcel->str1.append(tag);
        parcel->str2.append(msg);

        {
            boost::lock_guard<boost::mutex> guard(*_data_mtx);
            _queue.push_back(parcel);
            
            if (!_pending_logwrites)
            {
                _pending_logwrites = true;
                asyncWriteParcel(_queue.front().get());
            }
        }

        return true;
    }

protected:
    void onStarted()
    {
        _loggable = false;
        _pending_logwrites = false;
        asyncConnect();
    }

    void onConnected(bool connected)
    {
        if (connected)
        {
            DillParcel req;
            req.op = DILL_PRIORITY_PUBLISH;
            req.str1 = _channel;
            asyncWriteParcel(&req);

            asyncReadParcel(); // Expecting a reply
        } else {
            onIoError();
        }
    }

    void onParcelRead(DillParcel *p)
    {
        if (p->op == DILL_PRIORITY_REPLY_OK)
        {
            _loggable = true;
            _channel_id = p->id;
        } else {
            onIoError();
        }
    }

    void onParcelSent(bool succeed)
    {
        if (succeed)
        {
            boost::lock_guard<boost::mutex> guard(*_data_mtx);

            if (_pending_logwrites)
            {
                _queue.pop_front();
                if (_queue.empty())
                {
                    _pending_logwrites = false;
                } else {
                    asyncWriteParcel(_queue.front().get());
                }
            }
        } else {
            onIoError();
        }
    }

    void onIoError()
    {
        _loggable = false;

        {
            boost::lock_guard<boost::mutex> guard(*_data_mtx);
            _queue.clear();
            _pending_logwrites = false;
        }

        boost::thread::sleep(boost::get_system_time() + boost::posix_time::seconds(3));
        asyncConnect();
    }
};


//======------ Global Sigletons ------======//

std::shared_ptr<DillLogAgentCallable> _g_agent;

//======------ C Funcs ------======//

unsigned int dill_log_init(const char * channel)
{
    return dill_log_init_to(channel, DILL_DEFAULT_PORT, DILL_DEFAULT_ADDR);
}

unsigned int dill_log_init_to(const char * channel, int port, const char * addr)
{
    if (_g_agent.get() == 0)
    {
        _g_agent.reset(new DillLogAgentCallable(channel, port, addr));
    }
    return DILL_EC_SUCCEED;
}

void dill_log_deinit()
{
    if (_g_agent.get() != 0)
    {
        _g_agent.reset();
    }
}

int dill_logv(const char * tag, const char * msg)
{
    return dill_println((unsigned char)DILL_PRIORITY_VERBOSE, tag, msg);
}

int dill_logd(const char * tag, const char * msg)
{
    return dill_println((unsigned char)DILL_PRIORITY_DEBUG, tag, msg);
}

int dill_logi(const char * tag, const char * msg)
{
    return dill_println((unsigned char)DILL_PRIORITY_INFO, tag, msg);
}

int dill_logw(const char * tag, const char * msg)
{
    return dill_println((unsigned char)DILL_PRIORITY_WARN, tag, msg);
}

int dill_loge(const char * tag, const char * msg)
{
    return dill_println((unsigned char)DILL_PRIORITY_ERROR, tag, msg);
}

int dill_println(unsigned char pri, const char * tag, const char * msg)
{
    int ret;
    if (_g_agent.get())
    {
        ret = (_g_agent->post(pri, tag, msg))
                ? DILL_EC_SUCCEED
                : DILL_EC_UNKNOWN;
    }
    return ret;
}

int dill_reg(const char * name, const char * content)
{
    return dill_println((unsigned char)DILL_PRIORITY_REGISTER, name, content);
}

int dill_regi(const char * name, int content)
{
    return dill_reg(name, 
            boost::lexical_cast<std::string>(content).c_str());
}

int dill_regd(const char * name, double content)
{
    return dill_reg(name, 
            boost::lexical_cast<std::string>(content).c_str());
}

//======------ C Funcs ------======//







//======------ C++ Funcs ------======//

namespace dill
{
    unsigned int logInit(const char * channel, int port /*= DILL_DEFAULT_PORT*/, const char * addr /*= DILL_DEFAULT_ADDR*/)
    {
        return ::dill_log_init_to(channel, port, addr);
    }

    void logDeinit()
    {
        ::dill_log_deinit();
    }

    bool logv(const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(dill::VERBOSE, tag, msg));
    }

    bool logd(const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(dill::DEBUG, tag, msg));
    }

    bool logi(const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(dill::INFO, tag, msg));
    }

    bool logw(const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(dill::WARN, tag, msg));
    }

    bool loge(const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(dill::ERR, tag, msg));
    }

    bool logPrintln(unsigned char pri, const char * tag, const char * msg)
    {
        return (DILL_EC_SUCCEED == ::dill_println(pri, tag, msg));
    }

    bool regStr (const char * name, const char * content)
    {
        return (DILL_EC_SUCCEED == ::dill_println((unsigned char)DILL_PRIORITY_REGISTER, name, content));
    }

    bool regInt (const char * name, int content)
    {
        return (DILL_EC_SUCCEED == ::dill_println((unsigned char)DILL_PRIORITY_REGISTER, name, 
            boost::lexical_cast<std::string>(content).c_str()));
    }

    bool regReal(const char * name, double content)
    {
        return (DILL_EC_SUCCEED == ::dill_println((unsigned char)DILL_PRIORITY_REGISTER, name, 
            boost::lexical_cast<std::string>(content).c_str()));
    }
}

//======------ C++ Funcs ------======//