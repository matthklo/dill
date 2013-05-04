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

#pragma once

#include <boost/asio.hpp>

#include <dill_afx.h>

#include "protocol.h"

class DillServerNetIoCallable;

class DillServerConnection
{
public:
    DillServerConnection(DillServerNetIoCallable *server, boost::asio::ip::tcp::socket *socket);
    ~DillServerConnection();

    void asyncReadParcel();
    void asyncWriteParcel(DillParcel *p);

    const char * getChannelName() { return _chname.c_str(); }
    bool isSubscriber() { return (_mode == MODE_SUBSCRIBER); }
    bool isPublisher()  { return (_mode == MODE_PUBLISHER); }

private:
    enum WorkingMode
    {
        MODE_UNKNOWN,
        MODE_PUBLISHER,
        MODE_SUBSCRIBER,
    };

    boost::asio::ip::tcp::socket *_socket;
    DillServerNetIoCallable      *_server;
    WorkingMode                   _mode;
    std::string                   _chname;
    unsigned short                _expecting_size;
    char                          _rdbuf[65538]; // 64Kb + 2
    char                          _wtbuf[65538];

    void asyncReadParcelContent(unsigned short size);

    void onReadParcelLengthFinished(const boost::system::error_code& error, size_t bytes_transferred);
    void onReadParcelContentFinished(const boost::system::error_code& error, size_t bytes_transferred);
    void onPublisherParcel(DillParcel *p);
    void onSubscribeParcel(DillParcel *p);
    void onParcel(DillParcel *p);
    void onWriteParcelFinished(const boost::system::error_code& error, size_t bytes_transferred);
};