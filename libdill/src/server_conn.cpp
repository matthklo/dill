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

#include <boost/bind.hpp>
#include "server_conn.h"
#include "server_netio.h"
#include "server_channel.h"

void DillServerConnection::asyncReadParcel()
{
    _socket->async_read_some(boost::asio::buffer(_rdbuf, sizeof(unsigned short)),
        boost::bind(&DillServerConnection::onReadParcelLengthFinished, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void DillServerConnection::onReadParcelLengthFinished(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if ((error == boost::system::errc::success) && (bytes_transferred == sizeof(unsigned short)))
    {
        _expecting_size = *((unsigned short*)_rdbuf);
        asyncReadParcelContent(_expecting_size);
    } else {
        _server->scheduleDisconnect(this);
    }
}

void DillServerConnection::asyncReadParcelContent(unsigned short size)
{
    _socket->async_read_some(boost::asio::buffer(_rdbuf, size),
        boost::bind(&DillServerConnection::onReadParcelContentFinished, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

void DillServerConnection::onReadParcelContentFinished(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if ((error == boost::system::errc::success) && (bytes_transferred == _expecting_size))
    {
        std::shared_ptr<DillParcel> p(new DillParcel);
        p->inflate(_rdbuf);

        switch (_mode)
        {
        case MODE_PUBLISHER:
            onPublisherParcel(p.get());
            break;
        case MODE_SUBSCRIBER:
            onSubscribeParcel(p.get());
            break;
        default:
            onParcel(p.get());
            break;
        }

        asyncReadParcel();
    } else {
        _server->scheduleDisconnect(this);
    }
}

void DillServerConnection::onPublisherParcel(DillParcel *p)
{
    switch (p->op)
    {
    case DILL_PRIORITY_VERBOSE:
    case DILL_PRIORITY_DEBUG:
    case DILL_PRIORITY_INFO:
    case DILL_PRIORITY_WARN:
    case DILL_PRIORITY_ERROR:
    case DILL_PRIORITY_REGISTER:
        {
            DillServerChannel *data = _server->getChannelData(p->id);
            if (data)
            {
                data->updateChannel(p);
                _server->callback(p->op, p->id, p->timestamp1, p->timestamp2, p->str1.c_str(), p->str2.c_str());
            }
        }
        break;
    case DILL_PRIORITY_PUBLISH:
        {
            _chname = p->str1;
            DillServerChannel *data = _server->findChannelByName(_chname.c_str());
            if (!data)
            {
                data = _server->createChannelData(_chname.c_str());
            }
            
            p->op = (data != 0)? DILL_PRIORITY_REPLY_OK: DILL_PRIORITY_REPLY_FULL;
            p->id = (data != 0)? data->ID: 0;
            p->timestamp1 = 0;
            p->timestamp2 = 0;
            p->str1.clear();
            p->str2.clear();
            asyncWriteParcel(p);

            _server->callback(DILL_PRIORITY_PUBLISH, (unsigned int)this, 0, 0, _chname.c_str(), 0);
        }
        break;
    default:
        break;
    }
}

void DillServerConnection::onSubscribeParcel(DillParcel *p)
{
    switch (p->op)
    {
    case DILL_PRIORITY_AVAILCH:
        p->timestamp1 = _server->getAvailableChannels();
        p->timestamp2 = 0;
        if (p->timestamp1 > 0)
            p->str1 = _server->getChannelName(p->id);
        asyncWriteParcel(p);
        break;
    case DILL_PRIORITY_SUBSCRIBE:
        {
            DillServerChannel *data = _server->getChannelData(p->id);
            if (data)
            {
                data->attachSubscriber(this);
                _server->callback(DILL_PRIORITY_SUBSCRIBE, (unsigned int)this, 0, 0, data->Name.c_str(), 0);
            }
        }
        break;
    case DILL_PRIORITY_UNSUBSCRIBE:
        {
            DillServerChannel *data = _server->getChannelData(p->id);
            if (data)
            {
                data->dettachSubscriber(this);
                _server->callback(DILL_PRIORITY_UNSUBSCRIBE, (unsigned int)this, 0, 0, data->Name.c_str(), 0);
            }
        }
        break;
    default:
        break;
    }
}

void DillServerConnection::onParcel(DillParcel *p)
{
    switch(p->op)
    {
    case DILL_PRIORITY_VERBOSE:
    case DILL_PRIORITY_DEBUG:
    case DILL_PRIORITY_INFO:
    case DILL_PRIORITY_WARN:
    case DILL_PRIORITY_ERROR:
    case DILL_PRIORITY_PUBLISH:
    case DILL_PRIORITY_REGISTER:
        _mode = MODE_PUBLISHER;
        onPublisherParcel(p);
        break;
    case DILL_PRIORITY_AVAILCH:
    case DILL_PRIORITY_SUBSCRIBE:
    case DILL_PRIORITY_UNSUBSCRIBE:
        _mode = MODE_SUBSCRIBER;
        onSubscribeParcel(p);
        break;
    default:
        break;
    }
}

void DillServerConnection::onWriteParcelFinished(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (error != boost::system::errc::success)
    {
        _server->scheduleDisconnect(this);
    }
}

DillServerConnection::DillServerConnection(DillServerNetIoCallable *server, boost::asio::ip::tcp::socket *socket)
    : _server(server)
    , _socket(socket)
    , _mode(MODE_UNKNOWN)
    , _expecting_size(0)
{
}

DillServerConnection::~DillServerConnection()
{
    _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    _socket->close();
    delete _socket;
}

void DillServerConnection::asyncWriteParcel(DillParcel *p)
{
    unsigned short len = p->deflate(_wtbuf+2);
    *((unsigned short*)_wtbuf) = len;

    boost::asio::async_write(*_socket, boost::asio::buffer(_wtbuf, len + sizeof(unsigned short)), 
        boost::bind(&DillServerConnection::onWriteParcelFinished, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}
