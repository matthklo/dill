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

#include <dill_log.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>

unsigned int tokenize(const std::string& line, std::vector<std::string> & out)
{
    char _buf[4096] = {0};

    line.copy(_buf, 4095);
    char *p = std::strtok(_buf, " \t");

    out.clear();
    while(p)
    {
        out.push_back(p);
        p = std::strtok(0, " \t");
    }
    return out.size();
}

int main(int argc, char *argv[])
{
    std::string tag = "test";

    std::srand((unsigned int)std::time(0));

    if (argc > 1)
    {
        dill::logInit(argv[1]);
    } else {
        dill::logInit("matt");
    }

    std::string line;
    std::vector<std::string> params;
    while (true)
    {
        std::getline(std::cin, line);
        unsigned int pnum = tokenize(line, params);

        if (params[0] == "quit")
        {
            break;
        } else if ((params[0] == "tag") && (pnum > 1)) {
            tag = params[1];
            std::cout << " ==> Default tag changed to: " << tag << std::endl;
        } else if ((params[0] == "reg") && (pnum > 2)) {
            dill::regStr(params[1].c_str(), params[2].c_str());
        } else if ((params[0] == "regi") && (pnum > 1)) {
            dill::regInt(params[1].c_str(), rand());
        } else if ((params[0] == "regf") && (pnum > 1)) {
            dill::regReal(params[1].c_str(), ((double)(rand()%1024))/((double)19));
        } else if (pnum > 1) {
            dill::logPrintln((unsigned char)(std::rand()%5), params[0].c_str(), params[1].c_str());
        } else {
            dill::logPrintln((unsigned char)(std::rand()%5), tag.c_str(), params[0].c_str());
        }
    }

    dill::logDeinit();
    return 0;
}
