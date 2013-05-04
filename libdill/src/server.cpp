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

#include <dill_server.h>

#include <boost/lexical_cast.hpp>
#include <memory>

#include "server_netio.h"

std::shared_ptr<DillServerNetIoCallable> _g_server;

unsigned int dill_server_init(int port, unsigned int bufSize, unsigned int channels)
{
    if (_g_server.get() == 0)
    {
        _g_server.reset(new DillServerNetIoCallable(port, bufSize, channels));
    }

    return _g_server->getStatus();
}

void dill_server_deinit()
{
    if (_g_server.get() != 0)
    {
        _g_server.reset();
    }
}

void dill_server_evcb(DILL_SUBSCRIBE_CBFN fn)
{
    if (_g_server.get() != 0)
    {
        _g_server->registerEventCallback(fn);
    }
}




namespace dill
{
    unsigned int serverInit(int port, unsigned int bufSize, unsigned int channels)
    {
        return ::dill_server_init(port, bufSize, channels);
    }

    void serverDeinit()
    {
        return ::dill_server_deinit();
    }
    
    void serverEventCallback(DILL_SUBSCRIBE_CBFN fn)
    {
        return ::dill_server_evcb(fn);
    }
}
