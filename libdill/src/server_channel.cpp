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
#include <boost/shared_ptr.hpp>

#include "trace.h"
#include "server_channel.h"
#include "server_conn.h"

DillServerChannel::DillServerChannel(unsigned int id, unsigned int capacity, const char *name)
    : ID(id)
    , Name(name)
    , Capacity(capacity)
    , Used(0)
{
}

DillServerChannel::~DillServerChannel()
{
    Subscribers.clear();
    for (std::deque<DillParcel*>::iterator it = Buffers.begin();
        it != Buffers.end(); it++)
    {
        delete *it;
    }
    Buffers.clear();
}

void DillServerChannel::updateChannel(DillParcel *p)
{
    bool broadcast = false;

    if (p->op == DILL_PRIORITY_REGISTER)
    {
        Registers[p->str1] = p->str2;
        broadcast = (!Subscribers.empty());
    } else {
        DillParcel *archive = new DillParcel(*p); // clone parcel
        bool capable = false;
        const unsigned int psize = archive->deflate(0);
        unsigned int purgedSize = 0;

        while ((Used + psize > Capacity) && (!Buffers.empty()))
        {
            DillParcel *oldp = Buffers.front();
            const unsigned int oldpsize = oldp->deflate(0);
            Used -= oldpsize;
            delete oldp;
            Buffers.pop_front();
            purgedSize += oldpsize;
        }

        capable = (Used + psize <= Capacity);

        if (capable)
        {
            Buffers.push_back(archive);
            Used += psize;

            TRACED("     DillServerChannel::updateChannel(): channel [%s] take: %u bytes, purge: %u bytes, total: %u bytes",
                Name.c_str(), psize, purgedSize, Used);

            broadcast = (!Subscribers.empty());
        } else {
            TRACEE("!!!! DillServerChannel::updateChannel(): channel buffer can not hold a single log !");
        }
    }

    if (broadcast)
    {
        TRACED("     DillServerChannel::updateChannel(): channel [%s] brodcasting update to %u subscribers",
            Name.c_str(), Subscribers.size());
        for (std::set<DillServerConnection*>::iterator it = Subscribers.begin();
            it != Subscribers.end(); it++)
        {
            (*it)->asyncWriteParcel(p);
        }
    }
}

void DillServerChannel::attachSubscriber(DillServerConnection *conn)
{
    TRACEI("     DillServerChannel::attachSubscriber(): subscriber (0x%08x) attached. sending %u logs, %u regctx.", 
        DILL_PTRVAL(conn), Buffers.size(), Registers.size());

    Subscribers.insert(conn);

    for (std::deque<DillParcel*>::iterator it = Buffers.begin();
         it != Buffers.end(); it++)
    {
        conn->asyncWriteParcel(*it);
    }

    boost::shared_ptr<DillParcel> p(new DillParcel);
    p->op = DILL_PRIORITY_REGISTER;
    p->id = ID;
    p->timestamp1 = 0;
    p->timestamp2 = 0;
    for (boost::unordered_map<std::string, std::string>::iterator it = Registers.begin();
         it != Registers.end(); it++)
    {
        p->str1 = it->first;
        p->str2 = it->second;
        conn->asyncWriteParcel(p.get());
    }
}

void DillServerChannel::detachSubscriber(DillServerConnection *conn)
{
    TRACEI("     DillServerChannel::detachSubscriber(): subscriber (0x%08x) detached.", DILL_PTRVAL(conn));
    Subscribers.erase(conn);
}

