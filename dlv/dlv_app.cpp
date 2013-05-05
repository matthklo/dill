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

#include "dlv_app.h"
#include "dlv_frame.h"

IMPLEMENT_APP(DlvApp)

BEGIN_EVENT_TABLE(DlvApp, wxApp)
    EVT_TIMER(DLVID_CONNSTATTIMER, DlvApp::OnTimeout)
END_EVENT_TABLE()

bool DlvApp::OnInit()
{
    mFrame = 0;
    mTimer = 0;
    mServerAddr = wxT(DILL_DEFAULT_ADDR);
    mServerPort = DILL_DEFAULT_PORT;

    if (argc > 1)
    {
        // An explicit dill server is specified,
        // simply initiate a dill subscriber to it.

        mServerAddr = argv[1];
        wxCharBuffer b = mServerAddr.ToAscii();

        if (argc > 2)
        {
            long port;
            wxString portStr(argv[2]);
            portStr.ToLong(&port);
            mServerPort = (int)port;
        }

        dill::subscribeInit(mServerPort, b.data());
    } else {
        // Try initiate a dill server using default addr/port,
        // and also start up a subscriber connecting to it.

        dill::serverInit(); // TODO: configurable buffer size and channel num.
        dill::subscribeInit();
    }

    dill::subscribeEventCallback(DlvApp::dillCallback);

    mFrame = new DlvFrame;
    mFrame->Show();

    updateConnectionStatus();

    // Kick off a periodical timer for updating
    // server information on status bar
    mTimer = new wxTimer(this, DLVID_CONNSTATTIMER);
    mTimer->Start(3000);

    return true;
}

int DlvApp::OnExit()
{
    mTimer->Stop();

    dill::subscribeDeinit();
    dill::serverDeinit(); // Should be no harm even we never init.

    return wxApp::OnExit();
}

void DlvApp::OnTimeout(wxTimerEvent& e)
{
    updateConnectionStatus();
}

DlvFrame* DlvApp::getMainFrame()
{
    return mFrame;
}

void DlvApp::updateConnectionStatus()
{
    DlvEvtDataConnStatus* stat = new DlvEvtDataConnStatus;
    stat->ServerAddr = mServerAddr;
    stat->ServerPort = mServerPort;
    stat->ChannelNum = dill::availableChannels();

    wxCommandEvent ev(DlvDillEvent, DLVEVT_CONNSTAT);
    ev.SetClientData(stat);
    wxPostEvent(mFrame, ev);
}

/* 
 * Callback from the main thread inside dill library,
 * make sure only communicate with UI via event posting.
 */
void DlvApp::dillCallback(DillEvent *e)
{
    wxASSERT_MSG((e != 0), wxT("Null DillEvent pointer."));

    switch(e->Type)
    {
    case DILL_EVENT_LOG:
        {
            DlvEvtDataLog *data = new DlvEvtDataLog;
            data->Channel = e->ID;
            data->Tag = wxString::FromUTF8(e->Tag);
            data->Message = wxString::FromUTF8(e->Content);
            data->Priority = e->Priority;
            data->TimestampSec = e->TimeStamp1;
            data->TimestampMs = e->TimeStamp2;
            
            wxCommandEvent ev(DlvDillEvent, DLVEVT_LOGDATA);
            ev.SetClientData(data);
            wxPostEvent(wxGetApp().getMainFrame(), ev);
        }
        break;
    case DILL_EVENT_REG:
        {
            DlvEvtDataRegister *data = new DlvEvtDataRegister;
            data->Channel = e->ID;
            data->Register = wxString::FromUTF8(e->Tag);
            data->Content = wxString::FromUTF8(e->Content);

            wxCommandEvent ev(DlvDillEvent, DLVEVT_REGDATA);
            ev.SetClientData(data);
            wxPostEvent(wxGetApp().getMainFrame(), ev);
        }
        break;
    default:
        wxASSERT_MSG(false, wxT("Unexpected DillEvent type."));
        break;
    }
}

unsigned int DlvApp::getChannelNames(std::vector<std::string> &outCopy)
{
    outCopy = mChannelNames;
    return outCopy.size();
}