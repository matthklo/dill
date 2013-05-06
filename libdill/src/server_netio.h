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
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <set>
#include <vector>

#include <dill_afx.h>

#include "protocol.h"

class  DillServerConnection;
struct DillServerChannel;

class DillServerNetIoCallable
{
public:
    DillServerNetIoCallable(int port, unsigned int bufSize, unsigned int channels);
    ~DillServerNetIoCallable();

    unsigned int getStatus() const;
    void registerEventCallback(DILL_SUBSCRIBE_CBFN fn);

    DillServerChannel* createChannelData(const char *name);
    DillServerChannel* getChannelData(unsigned int channelId);
    DillServerChannel* findChannelByName(const char *name);
    unsigned int getAvailableChannels();
    const char *getChannelName(unsigned int id);

    void callback(unsigned char op, unsigned int id, unsigned int tsSec, unsigned int tsFrac, const char *str1, const char *str2);

    void scheduleDisconnect(DillServerConnection *conn);

    void operator() ();

private:
    boost::asio::io_service             *_iosvc;
    boost::asio::io_service::work       *_iowork;
    boost::asio::ip::tcp::socket        *_client_socket;
    boost::asio::ip::tcp::socket        *_server_socket;
    boost::asio::ip::tcp::acceptor      *_acceptor;
    boost::thread                       *_thread;
    bool                                 _sig_exit;
    unsigned int                         _status;
    DILL_SUBSCRIBE_CBFN                  _evcb;
    int                                  _port;
    unsigned int                         _id;
    unsigned int                         _bufsize;
    unsigned int                         _maxchannels;
    std::set<DillServerConnection*>      _conn_set;
    std::vector<DillServerChannel*>      _channel_data;

    void asyncAccept();
    void onAcceptFinished(const boost::system::error_code& error);

    void disconnect(DillServerConnection *conn);
};
