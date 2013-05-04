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

BEGIN_EVENT_TABLE(DlvFrame, wxFrame)
    EVT_MENU(wxID_ABOUT, DlvFrame::OnAbout)
    EVT_MENU(wxID_EXIT,  DlvFrame::OnQuit)
END_EVENT_TABLE()

DlvFrame::DlvFrame()
       : wxFrame(NULL, wxID_ANY, wxT("Dill Log Viewer"))
{
    // Set the frame icon
    //SetIcon(wxIcon(mondrian_xpm));

    setupMenuBar();
    setupStatusBar();
}

void DlvFrame::OnAbout(wxCommandEvent& event)
{
    wxString msg;
    msg.Printf(wxT("%s"),  
               DLV_VERSION_STRING);

    wxMessageBox(msg, wxT("About Dlv"),
                 wxOK | wxICON_INFORMATION, this);
}

void DlvFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void DlvFrame::setupMenuBar()
{
    wxMenu *fileMenu = new wxMenu;

    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, wxT("&About...\tF1"),
                     wxT("Show about dialog"));

    fileMenu->Append(wxID_EXIT, wxT("E&xit\tAlt-X"),
                     wxT("Quit this program"));

    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(helpMenu, wxT("&Help"));

    SetMenuBar(menuBar);
}

void DlvFrame::setupStatusBar()
{
    // Create a status bar for showing connection information
    CreateStatusBar(2);
    SetStatusText(wxT("Welcome to wxWidgets!"));
}

