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

#include <dill_afx.h>

// Define the data structure used by DILL network protocol 

#define DILL_PRIORITY_CONNECTED 247
#define DILL_PRIORITY_DISCONNECTED 248
#define DILL_PRIORITY_REPLY_OK 249
#define DILL_PRIORITY_REPLY_FULL 250
#define DILL_PRIORITY_AVAILCH 251
#define DILL_PRIORITY_SUBSCRIBE 252
#define DILL_PRIORITY_UNSUBSCRIBE 253
#define DILL_PRIORITY_PUBLISH 254
#define DILL_PRIORITY_REGISTER 255

#include <string>

struct DillParcel
{
    unsigned char op;
    unsigned int id;
    unsigned int timestamp1;
    unsigned int timestamp2;
    std::string str1;
    std::string str2;

    /* Build parcel from raw bytes */
    unsigned short inflate(const char * buf)
    {
        op = *buf;
        buf++;

        id = *((unsigned int*)buf);
        buf += sizeof(unsigned int);
        timestamp1 = *((unsigned int*)buf);
        buf += sizeof(unsigned int);
        timestamp2 = *((unsigned int*)buf);
        buf += sizeof(unsigned int);

        unsigned short len1 = *((unsigned short*)buf);
        buf += sizeof(unsigned short);
        str1.clear();
        str1.append(buf, len1);
        buf += len1;

        unsigned short len2 = *((unsigned short*)buf);
        buf += sizeof(unsigned short);
        str2.clear();
        str2.append(buf, len2);

        return 17+len1+len2;
    }

    /* Write parcel to raw bytes */
    unsigned short deflate(char * buf) const
    {
        unsigned short str1len = ((str1.size() > 23766)? 23766: (unsigned short)str1.size());
        unsigned short str2len = ((str2.size() > 23766)? 23766: (unsigned short)str2.size());
        unsigned short totallen = 17 + str1len + str2len;

        if (buf != 0)
        {
            *buf = op;
            buf++;

            *((unsigned int*)buf) = id;
            buf += sizeof(unsigned int);
            *((unsigned int*)buf) = timestamp1;
            buf += sizeof(unsigned int);
            *((unsigned int*)buf) = timestamp2;
            buf += sizeof(unsigned int);

            *((unsigned short*)buf) = str1len;
            buf += sizeof(unsigned short);
            str1.copy(buf, str1len);
            buf += str1len;

            *((unsigned short*)buf) = str2len;
            buf += sizeof(unsigned short);
            str2.copy(buf, str2len);
        }

        return totallen;
    }
};