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

#ifndef _DILL_SUBSCRIBER_HEADER_
#define _DILL_SUBSCRIBER_HEADER_

#include "dill_afx.h"

#define DILL_SUBSCRIBE 0
#define DILL_UNSUBSCRIBE 1

#ifdef __cplusplus
extern "C" {
#endif

DILLAPI unsigned int dill_subscribe_init();
DILLAPI unsigned int dill_subscribe_init_to(int port, const char * addr);
DILLAPI void         dill_subscribe_deinit();
DILLAPI void         dill_subscribe_evcb(DILL_SUBSCRIBE_CBFN fn);

DILLAPI int          dill_available_channels();
DILLAPI const char*  dill_channel_name(unsigned int index);
DILLAPI unsigned int dill_subscribe_channel(unsigned int index, int subscribe);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
namespace dill
{
    DILLAPI unsigned int subscribeInit(int port = DILL_DEFAULT_PORT, const char * addr = DILL_DEFAULT_ADDR);
    DILLAPI void         subscribeDeinit();
    
    DILLAPI int          availableChannels();
    DILLAPI const char*  getChannelName(unsigned int index);
    DILLAPI bool         subscribeChannel(unsigned int index, bool subscribe = true);
    
    DILLAPI void         subscribeEventCallback(DILL_SUBSCRIBE_CBFN fn);
}
#endif


#endif