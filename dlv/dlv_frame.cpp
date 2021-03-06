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
#include "dlv_channelpage.h"
#include "dlv_subscribedlg.h"

BEGIN_EVENT_TABLE(DlvFrame, wxFrame)
    EVT_MENU   (wxID_ABOUT, DlvFrame::OnAbout)
    EVT_MENU   (wxID_EXIT,  DlvFrame::OnQuit)
    EVT_COMMAND(DLVEVT_CONNSTAT, DlvDillEvent, DlvFrame::OnUpdateConnStat)
    EVT_COMMAND(DLVEVT_LOGDATA, DlvDillEvent, DlvFrame::OnDillLogData)
    EVT_COMMAND(DLVEVT_REGDATA, DlvDillEvent, DlvFrame::OnDillRegisterUpdate)
    EVT_MENU   (DLVID_TOOLSUBSCRIBE, DlvFrame::OnSubscribe)
    EVT_MENU   (DLVID_TOOLOPENLOG, DlvFrame::OnOpenLog)
    EVT_CLOSE  (DlvFrame::OnClose)
END_EVENT_TABLE()

DlvFrame::DlvFrame()
       : wxFrame(NULL, DLVID_MAINFRAME, DLVSTR_MAINFRAME_TITLE, wxDefaultPosition, wxSize(640, 480))
       , mChannelNotebook(this, DLVID_CHANNELNOTEBOOK)
       , mWaitingFirstChannel(true)
{
    // Set the frame icon
    //SetIcon(wxIcon(mondrian_xpm));

    wxImage::AddHandler(new wxPNGHandler);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();
}

DlvFrame::~DlvFrame()
{
    delete mSubscribeButton; mSubscribeButton = 0;
    delete mOpenLogButton; mOpenLogButton = 0;
}

void DlvFrame::OnAbout(wxCommandEvent& event)
{
    wxMessageBox(DLVSTR_ABOUT_MSG, DLVSTR_ABOUT_TITLE,
                 wxOK | wxICON_INFORMATION, this);
}

void DlvFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void DlvFrame::setupToolBar()
{
    wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_HORIZONTAL, DLVID_MAINTOOLBAR);

    mSubscribeButton = new wxImage(wxT("resource/subscribe-icon.png"), wxBITMAP_TYPE_PNG);
    mOpenLogButton   = new wxImage(wxT("resource/openlog-icon.png"),    wxBITMAP_TYPE_PNG);

    toolBar->AddTool(DLVID_TOOLOPENLOG, DLVSTR_TOOL_OPENLOG_LABEL, 
                     wxBitmap(*mOpenLogButton), DLVSTR_TOOL_OPENLOG_COMMENT);
    toolBar->AddTool(DLVID_TOOLSUBSCRIBE, DLVSTR_TOOL_SUBSCRIBE_LABEL,
                     wxBitmap(*mSubscribeButton), DLVSTR_TOOL_SUBSCRIBE_COMMENT);
    toolBar->Realize();
    toolBar->EnableTool(DLVID_TOOLSUBSCRIBE, false);
}

void DlvFrame::setupMenuBar()
{
    wxMenuBar *menuBar = new wxMenuBar;

    wxMenu *fileMenu = new wxMenu;
    fileMenu->Append(DLVID_TOOLOPENLOG, DLVSTR_TOOL_OPENLOG_LABEL_S,
                     DLVSTR_TOOL_OPENLOG_COMMENT);
    fileMenu->AppendSeparator();
    fileMenu->Append(DLVID_TOOLSUBSCRIBE, DLVSTR_TOOL_SUBSCRIBE_LABEL_S,
                     DLVSTR_TOOL_SUBSCRIBE_COMMENT);
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, DLVSTR_FILEMENU_EXIT_LABEL,
                     DLVSTR_FILEMENU_EXIT_COMMENT);

    fileMenu->Enable(DLVID_TOOLSUBSCRIBE, false);

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, DLVSTR_HELPMENU_ABOUT_LABEL,
                     DLVSTR_HELPMENU_ABOUT_COMMENT);

    menuBar->Append(fileMenu, DLVSTR_FILEMENU_LABEL);
    menuBar->Append(helpMenu, DLVSTR_HELPMENU_LABEL);

    SetMenuBar(menuBar);
}

void DlvFrame::setupStatusBar()
{
    // Create a status bar for showing connection information
    CreateStatusBar(2);
}

void DlvFrame::OnUpdateConnStat(wxCommandEvent &ev)
{
    DlvEvtDataConnStatus *data = (DlvEvtDataConnStatus*) ev.GetClientData();
    if (data)
    {
        wxString statMsg;
        statMsg.Printf(DLVSTR_STATMSG_FORMAT, data->ServerAddr.c_str(),
                       data->ServerPort, data->ChannelNum);
        SetStatusText(statMsg, 1);

        if (mWaitingFirstChannel && (data->ChannelNum > 0))
        {
            DoSubscribeChannel(0);
            mWaitingFirstChannel = false;

            GetMenuBar()->Enable(DLVID_TOOLSUBSCRIBE, true);
            GetToolBar()->EnableTool(DLVID_TOOLSUBSCRIBE, true);
        }
        delete data;
    }
}

void DlvFrame::DoSubscribeChannel(int chidx, bool subscribe)
{
    wxString chname = wxString::FromUTF8(wxGetApp().getChannelName(chidx));
    if (subscribe)
    {
        DlvChannelPage *page = new DlvChannelPage(&mChannelNotebook);
        mChannelNotebook.AddPage(page, chname);
        wxGetApp().subscribeChannel(chidx, (void*)page);
    } else {
        wxGetApp().subscribeChannel(chidx, 0, false);
        for (unsigned int i=0; i<mChannelNotebook.GetPageCount(); ++i)
        {
            if (mChannelNotebook.GetPageText(i) == chname)
            {
                mChannelNotebook.DeletePage(i);
                break;
            }
        }
    }
}

void DlvFrame::OnDillLogData(wxCommandEvent &ev)
{
    DlvEvtDataLog *data = (DlvEvtDataLog*) ev.GetClientData();
    if (data)
    {
        DlvChannelPage *page = (DlvChannelPage*) wxGetApp().getChannelPageByID(data->Channel);
        if (page)
        {
            page->OnAppenLog(data);
        }
    }
}

void DlvFrame::OnDillRegisterUpdate(wxCommandEvent &ev)
{
    DlvEvtDataRegister *data = (DlvEvtDataRegister*) ev.GetClientData();
    if (data)
    {
        DlvChannelPage *page = (DlvChannelPage*) wxGetApp().getChannelPageByID(data->Channel);
        if (page)
        {
            page->OnUpdateRegister(data);
        }
        delete data;
    }
}

void DlvFrame::OnSubscribe(wxCommandEvent &ev)
{
    DlvSubscribeDialog sdlg(this);
    sdlg.ShowModal();
}

void DlvFrame::OnOpenLog(wxCommandEvent &ev)
{
    wxFileDialog fd(this, wxT("Choose Log File(s)"), wxGetCwd(), wxT(""),
                    wxT("Dlv Log Files (*.dlf)|*.dlf"), wxFD_OPEN | wxFD_MULTIPLE);

    if (wxID_OK == fd.ShowModal())
    {
        wxArrayString filenames;
        fd.GetFilenames(filenames);
    }
}

void DlvFrame::OnClose(wxCloseEvent &ev)
{
    wxGetApp().setQuiting();
    Destroy();
}
