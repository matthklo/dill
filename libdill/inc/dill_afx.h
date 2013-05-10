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

#ifndef _DILL_AFX_HEADER_
#define _DILL_AFX_HEADER_


#if defined(_MSC_VER)
#  define DILLEXPORT __declspec(dllexport)
#  define DILLIMPORT __declspec(dllimport)
#elif defined(__GNUC__)
#  define DILLEXPORT __attribute__ ((visibility("default")))
#  define DILLIMPORT
#endif

#ifdef DILL_STATIC_LIBRARY
#  define DILLAPI
#else
#  ifdef DILL_EXPORTS
#    define DILLAPI DILLEXPORT
#  else
#    define DILLAPI DILLIMPORT
#  endif
#endif




#define DILL_PRIORITY_VERBOSE 0
#define DILL_PRIORITY_DEBUG 1
#define DILL_PRIORITY_INFO 2
#define DILL_PRIORITY_WARN 3
#define DILL_PRIORITY_ERROR 4

#define DILL_EVENT_LOG 0          // subscriber, server
#define DILL_EVENT_REG 1          // subscriber, server
#define DILL_EVENT_PUBLISHER 3    // server
#define DILL_EVENT_SUBSCRIBER 4   // server
#define DILL_EVENT_CONNECTED 5    // server 
#define DILL_EVENT_DISCONNECTED 6 // server

struct DillEvent
{   
    unsigned char  Type;        // one of DILL_EVENT_*

    unsigned char  Priority;    // LOG: log priority; 
                                // REG,PUBLISHER,CONNECTED,DISCONNECTED: not used; 
                                // SUBSCRIBER: 1 - subscribe, 0 - unsubscribe

    unsigned int   ID;          // LOG,REG: Channel ID
                                // PUBLISHER,SUBSCRIBER,CONNECTED,DISCONNECTED: ID of peer

    const char *   Tag;         // LOG: log tag; 
                                // REG: register name; 
                                // PUBLISHER,SUBSCRIBER,CONNECTED,DISCONNECTED: not used

    const char *   Content;     // LOG,REG: content; 
                                // PUBLISHER,SUBSCRIBER,CONNECTED,DISCONNECTED: not used

    unsigned int   TimeStamp1;  // LOG,REG: seconds since 1970-01-01 00:00:00 UTC;
                                // PUBLISHER,SUBSCRIBER,CONNECTED: not used
                                // DISCONNECTED: 1 - a publisher disconnected, 0 - a subscriber disconnected

    unsigned int   TimeStamp2;  // LOG,REG: microsecond;
                                // PUBLISHER,SUBSCRIBER,CONNECTED,DISCONNECTED: not used
};

typedef void (*DILL_SUBSCRIBE_CBFN)(struct DillEvent *);





#define DILL_DEFAULT_PORT 5299
#define DILL_DEFAULT_ADDR "127.0.0.1"
#define DILL_DEFAULT_CHANNEL_BUFFER 1048576
#define DILL_DEFAULT_MAX_CHANNELS 10



// Error codes
#define DILL_EC_SUCCEED 0
#define DILL_EC_UNKNOWN 1
#define DILL_EC_BIND    2




#if defined(DILL_EXPORTS) && defined(_MSC_VER)
#pragma warning ( disable : 4819 4996)
#endif

#define DILL_PTRVAL(x) (unsigned int)(unsigned long long)(x)

#endif
