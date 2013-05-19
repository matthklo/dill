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
 
#include "dlv_subscribedlg.h"
#include "dlv_app.h"
#include "dlv_frame.h"
#include <vector>
#include <string>
 
DlvSubscribeDialog::DlvSubscribeDialog(wxWindow* parent)
    : wxDialog(parent, DLVID_SUBSCRIBEDLG, DLVSTR_SUBSCRIBEDLG_TITLE, wxDefaultPosition, wxSize(200,250))
    , mMainSizer(new wxBoxSizer(wxVERTICAL))
{
    // Get all known channel names
    std::vector<std::string> channelNames;
    wxGetApp().getChannelNames(channelNames);

    // Get count and save all names in a wxArrayString
    int chcount = (int) channelNames.size();
    wxArrayString names;
    for (unsigned int i=0; i<channelNames.size(); ++i)
    {
        names.Add(wxString::FromUTF8(channelNames[i].c_str()));
    }

    // Instanciate a wxCheckListBox
    mCheckListBox = new wxCheckListBox(this, DLVID_CHANNELCHECKLISTBOX, wxDefaultPosition,
        wxDefaultSize, names);

    // Check all channels which were already subscribed
    for (unsigned int i=0; i<channelNames.size(); ++i)
    {
        if (wxGetApp().isChannelSubscribed(channelNames[i]))
        {
            mCheckListBox->Check(i);
            mSubscribingChannels.Add(wxString::FromUTF8(channelNames[i].c_str()));
        }
    }

    mMainSizer->Add(mCheckListBox, 1, wxALL | wxEXPAND, 5);

    wxSizer *btnSizer = CreateButtonSizer(wxOK);
    if (btnSizer != 0)
        mMainSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

    SetSizer(mMainSizer);
}

DlvSubscribeDialog::~DlvSubscribeDialog()
{
}

int DlvSubscribeDialog::ShowModal()
{
    int ret = wxDialog::ShowModal();
    if (ret == wxID_OK)
    {
        // Scan check status for all entries in check list box
        // and compare to the subscribing status.
        // Add/delete channel page(s) aaccording to the compare result.
        for (unsigned int i=0; i<mCheckListBox->GetCount(); ++i)
        {
            bool wasInSubscribing = (wxNOT_FOUND != mSubscribingChannels.Index(mCheckListBox->GetString(i)));
            bool wantSubscribing = mCheckListBox->IsChecked(i);
            DlvFrame *frame = dynamic_cast<DlvFrame*>(GetParent());

            wxASSERT((frame != 0));

            if (!wasInSubscribing && wantSubscribing)
            {
                frame->DoSubscribeChannel(i, true);
            } else if (wasInSubscribing && !wantSubscribing) {
                frame->DoSubscribeChannel(i, false);
            }
        }
    }
    return ret;
}
