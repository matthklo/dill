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

#include "dlv.h"
#include <vector>
#include <string>

class  DlvFrame;

class DlvApp : public wxApp
{
public:
    bool OnInit();
    int  OnExit();
    void OnTimeout(wxTimerEvent& e);

    bool isQuiting() const;

    DlvFrame*    getMainFrame();
    const char*  getChannelName(unsigned int id);
    unsigned int getChannelNames(std::vector<std::string> &outCopy);
    void*        getChannelPageByID(unsigned int id);
    bool         subscribeChannel(unsigned int channelId, void* page, bool subscribe = true);

private:
    DECLARE_EVENT_TABLE()

    static void  dillCallback(DillEvent* e);
           void  updateConnectionStatus();
           void  refreshChannelData(bool fullRefresh);

    struct ChannelData
    {
        void*       Page;
        std::string Name;
    };

    std::vector<ChannelData>     mChannelData;
    wxTimer*                     mTimer;
    DlvFrame*                    mFrame;
    wxString                     mServerAddr;
    int                          mServerPort;
    bool                         mQuiting;
};

DECLARE_APP(DlvApp)
