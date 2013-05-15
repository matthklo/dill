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

#include <dill_afx.h>

#include "trace.h"
#include "server_netio.h"
#include "server_conn.h"
#include "server_channel.h"

void DillServerNetIoCallable::asyncAccept()
{
    _acceptor->async_accept(*_client_socket,
        boost::bind(&DillServerNetIoCallable::onAcceptFinished, this,
                    boost::asio::placeholders::error));
}

void DillServerNetIoCallable::onAcceptFinished(const boost::system::error_code& error)
{
    if (error != boost::system::errc::success)
    {
        TRACEW("!!!! DillServerNetIoCallable::onAcceptFinished(): fail on accepting.");
        _client_socket->close();
        delete _client_socket;
    } else {
        DillServerConnection *w = new DillServerConnection(this, _client_socket);
        _conn_set.insert(w);
        w->asyncReadParcel();

        TRACEI("<=== DillServerNetIoCallable::onAcceptFinished(): accepted a client connection (0x%08x).", DILL_PTRVAL(w));
        callback(DILL_PRIORITY_CONNECTED, DILL_PTRVAL(w), 0, 0, 0, 0);
    }

    // For both case, a new socket object is needed for upcoming connection.
    _client_socket = new boost::asio::ip::tcp::socket(*_iosvc);
    asyncAccept();
}

DillServerNetIoCallable::DillServerNetIoCallable(int port, unsigned int bufSize, unsigned int channels)
    : _status(DILL_EC_UNKNOWN)
    , _sig_exit(false)
    , _evcb(0)
    , _port(port)
    , _bufsize(bufSize)
    , _maxchannels(channels)
    , _iowork(0)
    , _id(0)
    , _thread(0)
{
    _iosvc         = new boost::asio::io_service;
    _server_socket = new boost::asio::ip::tcp::socket(*_iosvc);
    _client_socket = new boost::asio::ip::tcp::socket(*_iosvc);

    try
    {
        _status       = DILL_EC_SUCCEED;
        _acceptor     = new boost::asio::ip::tcp::acceptor(*_iosvc, 
                            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                            _port), false); // Do not reuse addr, i.e. throw error when bind error.
    } catch (boost::system::system_error err) {
        TRACEW("!!!! DillServerNetIoCallable::DillServerNetIoCallable(): fail on binding.");
        _status       = DILL_EC_BIND;
    }

    _thread = new boost::thread(boost::ref(*this));
}

DillServerNetIoCallable::~DillServerNetIoCallable()
{
    _sig_exit = true;
    delete _iowork;
    _iosvc->stop();

    TRACED("     DillServerNetIoCallable::~DillServerNetIoCallable(): shutdowning server...");

    if (_thread)
    {
        _thread->join();
        delete _thread;
        _thread = 0;
    }

    TRACED("     DillServerNetIoCallable::~DillServerNetIoCallable(): thread joined.");

    _client_socket->close();
    delete _client_socket;

    _acceptor->close();
    delete _acceptor;

    _server_socket->close();
    delete _server_socket;

    delete _iosvc;
}

unsigned int DillServerNetIoCallable::getStatus() const
{
    return _status;
}

void DillServerNetIoCallable::scheduleDisconnect(DillServerConnection *conn)
{
    _iosvc->post(boost::bind(&DillServerNetIoCallable::disconnect, this, conn));
}

void DillServerNetIoCallable::registerEventCallback(DILL_SUBSCRIBE_CBFN fn)
{
    _evcb = fn;
}

DillServerChannel* DillServerNetIoCallable::createChannelData(const char *name)
{
    if (name && (_channel_data.size() < _maxchannels))
    {
        TRACEI("     DillServerNetIoCallable::createChannelData(): A new channel [%s] created.", name);
        DillServerChannel *data = new DillServerChannel(_channel_data.size(), _bufsize, name);
        _channel_data.push_back(data);
        return data;
    }
    return 0;
}

DillServerChannel* DillServerNetIoCallable::getChannelData(unsigned int channelId)
{
    if (channelId < _channel_data.size())
        return _channel_data[channelId];
    return 0;
}

DillServerChannel* DillServerNetIoCallable::findChannelByName(const char *name)
{
    if (!name) return 0;

    for (std::vector<DillServerChannel*>::iterator it = _channel_data.begin();
         it != _channel_data.end(); it++)
    {
        if ((*it)->Name == name)
        {
            return (*it);
        }
    }
    return 0;
}

unsigned int DillServerNetIoCallable::getAvailableChannels()
{
    return _channel_data.size();
}

const char * DillServerNetIoCallable::getChannelName(unsigned int id)
{
    DillServerChannel *data = getChannelData(id);
    if (data)
        return data->Name.c_str();
    return 0;
}

void DillServerNetIoCallable::operator() ()
{
    if (_status != DILL_EC_SUCCEED)
        return;

    _iowork = new boost::asio::io_service::work(*_iosvc);

    asyncAccept();

    while(!_sig_exit)
    {
        _iosvc->run_one();
    }

    for (std::set<DillServerConnection*>::iterator it = _conn_set.begin();
        it != _conn_set.end(); it++)
    {
        delete *it;
    }
    _conn_set.clear();

    for (std::vector<DillServerChannel*>::iterator it = _channel_data.begin();
         it != _channel_data.end(); it++)
    {
        delete (*it);
    }
    _channel_data.clear();
}

void DillServerNetIoCallable::disconnect(DillServerConnection *conn)
{
    if (!conn) return;

    if (conn->isSubscriber())
    {
        for (std::vector<DillServerChannel*>::iterator it = _channel_data.begin();
             it != _channel_data.end(); it++)
        {
            (*it)->detachSubscriber(conn);
        }
    }

    _conn_set.erase(conn);

    TRACEI("     DillServerNetIoCallable::disconnect(): closed a client connection (0x%08x)", DILL_PTRVAL(conn));
    callback(DILL_PRIORITY_DISCONNECTED, DILL_PTRVAL(conn), ((conn->isPublisher())?1:0), 0, conn->getChannelName(), 0);
    delete conn;
}

void DillServerNetIoCallable::callback(unsigned char op, unsigned int id, unsigned int tsSec, unsigned int tsFrac, const char *str1, const char *str2)
{
    DillEvent ev;
    ev.ID = id;
    ev.TimeStamp1 = tsSec;
    ev.TimeStamp2 = tsFrac;

    if (_evcb)
    {
        switch (op)
        {
        case DILL_PRIORITY_VERBOSE:
        case DILL_PRIORITY_DEBUG:
        case DILL_PRIORITY_INFO:
        case DILL_PRIORITY_WARN:
        case DILL_PRIORITY_ERROR:
            ev.Type = DILL_EVENT_LOG;
            ev.Priority = op;
            ev.Tag = str1;
            ev.Content = str2;
            break;
        case DILL_PRIORITY_REGISTER:
            ev.Type = DILL_EVENT_REG;
            ev.Priority = 0;
            ev.Tag = str1;
            ev.Content = str2;
            break;
        case DILL_PRIORITY_PUBLISH:
            ev.Type = DILL_EVENT_PUBLISHER;
            ev.Priority = 0;
            ev.Tag = str1;
            ev.Content = 0;
            break;
        case DILL_PRIORITY_SUBSCRIBE:
            ev.Type = DILL_EVENT_SUBSCRIBER;
            ev.Priority = 1;
            ev.Tag = str1;
            ev.Content = 0;
            break;
        case DILL_PRIORITY_UNSUBSCRIBE:
            ev.Type = DILL_EVENT_SUBSCRIBER;
            ev.Priority = 0;
            ev.Tag = str1;
            ev.Content = 0;
            break;
        case DILL_PRIORITY_CONNECTED:
            ev.Type = DILL_EVENT_CONNECTED;
            ev.Priority = 0;
            ev.Tag = 0;
            ev.Content = 0;
            break;
        case DILL_PRIORITY_DISCONNECTED:
            ev.Type = DILL_EVENT_DISCONNECTED;
            ev.Priority = 0;
            ev.Tag = str1;
            ev.Content = 0;
            break;
        default:
            break;
        }

        _evcb(&ev);
    }
}

