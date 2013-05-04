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


#include "client_netio.h"

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <string>

#include "protocol.h"

class DillClientNetIoCallableImpl
{
    boost::thread                 *_thread;
    boost::asio::io_service       *_iosvc;
    boost::asio::ip::tcp::socket  *_socket;
    boost::asio::io_service::work *_iowork;
    bool                           _sig_exit;
    bool                           _sig_connected;
    const std::string              _addr;
    const int                      _port;
    char                           _rd_buf[65538]; // 64Kb + 2
    unsigned int                   _expecting_rdlen;
    char                           _wt_buf[65538];
    unsigned int                   _expecting_wtlen;

    DillClientNetIoCallable       *_host;

public:
    DillClientNetIoCallableImpl(DillClientNetIoCallable *host, int port, const char * addr)
        : _thread(0)
        , _sig_exit(false)
        , _addr(addr)
        , _port(port)
        , _expecting_rdlen(0)
        , _expecting_wtlen(0)
        , _host(host)
        , _sig_connected(false)
    {
        _iosvc = new boost::asio::io_service;
        _socket = new boost::asio::ip::tcp::socket(*_iosvc);
    }

    ~DillClientNetIoCallableImpl()
    {
        stop();
        resetSocket();
        delete _socket;
        delete _iosvc;
    }

    void start()
    {
        _sig_exit = false;
        _thread = new boost::thread(boost::ref(*this));
    }

    void stop()
    {
        if (_thread)
        {
            boost::system::error_code ec;

            _sig_exit = true;
            delete _iowork;

            _socket->cancel(ec);

            _thread->join();
            delete _thread;
            _thread = 0;
        }
    }

    bool isConnected() const
    {
        return _sig_connected;
    }

    void asyncConnect()
    {
        std::string portStr = boost::lexical_cast<std::string>(_port);
        boost::asio::ip::tcp::resolver           resolver(*_iosvc);
        boost::asio::ip::tcp::resolver::query    query(boost::asio::ip::tcp::v4(), _addr, portStr);

        boost::asio::async_connect(*_socket, resolver.resolve(query), 
            boost::bind(&DillClientNetIoCallableImpl::onConnectFinished, this,
            boost::asio::placeholders::error, boost::asio::placeholders::iterator));
    }

    void asyncReadParcel()
    {
        _expecting_rdlen = sizeof(unsigned short);
        boost::asio::async_read(*_socket, boost::asio::buffer(_rd_buf, _expecting_rdlen),
            boost::bind(&DillClientNetIoCallableImpl::onReadParcelLengthFinished, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void asyncWriteParcel(DillParcel *p)
    {
        unsigned short len = p->deflate(_wt_buf + sizeof(unsigned short));
        *((unsigned short*)_wt_buf) = len;
        _expecting_wtlen = len + sizeof(unsigned short);
        boost::asio::async_write(*_socket, boost::asio::buffer(_wt_buf, _expecting_wtlen),
            boost::bind(&DillClientNetIoCallableImpl::onWriteParcelFinished, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    // boost thread body
    void operator() ()
    {
        _iowork = new boost::asio::io_service::work(*_iosvc);

        _host->onStarted();

        while (!_sig_exit)
        {
            _iosvc->run_one();
        }
    }

private:
    void asyncReadParcelContent(unsigned short len)
    {
        _expecting_rdlen = len;
        boost::asio::async_read(*_socket, boost::asio::buffer(_rd_buf, _expecting_rdlen),
            boost::bind(&DillClientNetIoCallableImpl::onReadParcelContentFinished, this,
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }

    void onConnectFinished(const boost::system::error_code &ec,
                           boost::asio::ip::tcp::resolver::iterator i)
    {
        _sig_connected = (ec == boost::system::errc::success);

        if (!_sig_exit)
        {
            if (!_sig_connected)
                resetSocket();
            _host->onConnected(_sig_connected);
        }
    }

    void onReadParcelLengthFinished(const boost::system::error_code &ec,
                                    size_t bytes_transferred)
    {
        if ((ec == boost::system::errc::success) && (bytes_transferred == _expecting_rdlen))
        {
            unsigned short len = *((unsigned short*)_rd_buf);
            asyncReadParcelContent(len);
        } else if (!_sig_exit) {
            resetSocket();
            _host->onIoError();
        }
    }

    void onReadParcelContentFinished(const boost::system::error_code &ec,
                                     size_t bytes_transferred)
    {
        if ((ec == boost::system::errc::success) && (bytes_transferred == _expecting_rdlen))
        {
            DillParcel p;
            p.inflate(_rd_buf);
            _host->onParcelRead(&p);
        } else if (!_sig_exit) {
            resetSocket();
            _host->onIoError();
        }
    }

    void onWriteParcelFinished(const boost::system::error_code &ec,
                               size_t bytes_transferred)
    {
        bool succeed = ((ec == boost::system::errc::success) && (bytes_transferred == _expecting_wtlen));

        if (!_sig_exit)
        {
            if (!succeed)
                resetSocket();
            _host->onParcelSent(succeed);
        }
    }

    void resetSocket()
    {
        _sig_connected = false;

        boost::system::error_code ec;
        _socket->cancel(ec);
        _socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        _socket->close();
    }
};

DillClientNetIoCallable::DillClientNetIoCallable(int port, const char * addr)
{
    _impl = new DillClientNetIoCallableImpl(this, port, addr);

    _impl->start();
}

DillClientNetIoCallable::~DillClientNetIoCallable()
{
    delete _impl;
}

void DillClientNetIoCallable::asyncConnect()
{
    _impl->asyncConnect();
}

void DillClientNetIoCallable::asyncReadParcel()
{
    _impl->asyncReadParcel();
}

void DillClientNetIoCallable::asyncWriteParcel(DillParcel *p)
{
    _impl->asyncWriteParcel(p);
}

bool DillClientNetIoCallable::isConnected() const
{
    return _impl->isConnected();
}

void DillClientNetIoCallable::start()
{
    _impl->start();
}

void DillClientNetIoCallable::stop()
{
    _impl->stop();
}

