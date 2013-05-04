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

#ifndef _DILL_LOG_HEADER_
#define _DILL_LOG_HEADER_

#include "dill_afx.h"

#ifdef __cplusplus
extern "C" {
#endif

DILLAPI unsigned int dill_log_init(const char * channel);
DILLAPI unsigned int dill_log_init_to(const char * channel, int port, const char * addr);
DILLAPI void         dill_log_deinit();

DILLAPI int          dill_logv(const char * tag, const char * msg);
DILLAPI int          dill_logd(const char * tag, const char * msg);
DILLAPI int          dill_logi(const char * tag, const char * msg);
DILLAPI int          dill_logw(const char * tag, const char * msg);
DILLAPI int          dill_loge(const char * tag, const char * msg);
DILLAPI int          dill_println(unsigned char pri, const char * tag, const char * msg);

DILLAPI int          dill_reg(const char * name, const char * content);
DILLAPI int          dill_regi(const char * name, int content);
DILLAPI int          dill_regd(const char * name, double content);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
namespace dill
{
    enum PRIORITY
    {
        VERBOSE = DILL_PRIORITY_VERBOSE,
        DEBUG   = DILL_PRIORITY_DEBUG,
        INFO    = DILL_PRIORITY_INFO,
        WARN    = DILL_PRIORITY_WARN,
        ERR     = DILL_PRIORITY_ERROR,
    };

    DILLAPI unsigned int logInit(const char * channel, int port = DILL_DEFAULT_PORT, const char * addr = DILL_DEFAULT_ADDR);
    DILLAPI void         logDeinit();

    // Log texts
    DILLAPI bool logv(const char * tag, const char * msg);
    DILLAPI bool logd(const char * tag, const char * msg);
    DILLAPI bool logi(const char * tag, const char * msg);
    DILLAPI bool logw(const char * tag, const char * msg);
    DILLAPI bool loge(const char * tag, const char * msg);
    DILLAPI bool logPrintln(unsigned char pri, const char * tag, const char * msg);

    // Registers
    DILLAPI bool regStr (const char * name, const char * content);
    DILLAPI bool regInt (const char * name, int content);
    DILLAPI bool regReal(const char * name, double content);
}
#endif


#endif