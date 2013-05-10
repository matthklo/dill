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

#include <dill_subscriber.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdio>
#include <ctime>

void subscriberEventCallback(struct DillEvent *ev)
{
    switch (ev->Type)
    {
    case DILL_EVENT_LOG:
        {
            static char pri[] = { 'V', 'D', 'I', 'W', 'E' };
            std::time_t t = (std::time_t) ev->TimeStamp1;
            struct std::tm *tm = std::localtime(&t);

            std::cout << "CH #" << ev->ID << " {" 
                      << std::setfill('0') 
                      << std::setw(2) << (tm->tm_mon + 1) << "-"
                      << std::setw(2) << tm->tm_mday << " " << std::setw(2) << tm->tm_hour
                      << ":" << std::setw(2) << tm->tm_min << ":" << std::setw(2) << tm->tm_sec
                      << "." << std::setw(3) << (ev->TimeStamp2/1000);

            std::cout << "} " << pri[ev->Priority - DILL_PRIORITY_VERBOSE] << " ["
                      << ev->Tag << "] " << ev->Content << std::endl;
        }
        break;
    case DILL_EVENT_REG:
        {
            std::cout << std::setfill('0') << "Channel #" << std::setw(2) << ev->ID 
                      << " Register [" << ev->Tag << "]: " << ev->Content << std::endl;
        }
        break;
    default:
        std::cout << "Fatal event error." << std::endl;
        break;
    }
}

int main(int argc, char *argv[])
{
    dill::subscribeInit();
    dill::subscribeEventCallback(subscriberEventCallback);

    std::string line;
    while (true)
    {
        std::getline(std::cin, line);
        if (line == "quit")
        {
            break;
        } else if (line == "ch") {
            unsigned int chnum = dill::availableChannels();
            std::cout << "Available Channels:" << chnum << std::endl;
            for (unsigned int i=0; i<chnum; ++i)
            {
                std::cout << "  CH:" << i << "    Name: " << dill::getChannelName(i) << std::endl;
            }
        } else if (line.substr(0, 2) == "s ") {
            std::string chstr = line.substr(2);
            if (!chstr.empty())
            {
                unsigned int id;
                std::sscanf(chstr.c_str(), "%u", &id);
                dill::subscribeChannel(id);
            }
        } else if (line.substr(0, 2) == "u ") {
            std::string chstr = line.substr(2);
            if (!chstr.empty())
            {
                unsigned int id;
                std::sscanf(chstr.c_str(), "%u", &id);
                dill::subscribeChannel(id, false);
            }
        }
    }

    dill::subscribeDeinit();

    return 0;
}
