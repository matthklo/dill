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

#include "dlv_channelpage.h"
#include <wx/gdicmn.h>

enum _LOGHDRIDX
{
    LOG_HDRIDX_DATE = 0,
    LOG_HDRIDX_PRI,
    LOG_HDRIDX_TAG,
    LOG_HDRIDX_MSG,

    LOG_HDRNUM,
};

struct _LOGHDR
{
    wxString   Label;
    int        Width;
} _LogHeaders[LOG_HDRNUM] = { 
        {DLVSTR_LOGHDR_DATE,      120}, 
        {DLVSTR_LOGHDR_PRIORITY,   30}, 
        {DLVSTR_LOGHDR_TAG,        80}, 
        {DLVSTR_LOGHDR_MESSAGE,   400},
    };

enum _REGHDRIDX
{
    REG_HDRIDX_NAME = 0,
    REG_HDRIDX_CNT,

    REG_HDRNUM,
};

struct _REGHDR
{
    wxString   Label;
    int        Width;
} _RegHeaders[REG_HDRNUM] = { 
        {DLVSTR_REGHDR_NAME,     50}, 
        {DLVSTR_REGHDR_CONTENT, 100}, 
    };

BEGIN_EVENT_TABLE(DlvChannelPage, wxPanel)
    EVT_BUTTON(DLVID_SHOWREGBTN,  DlvChannelPage::OnShowRegViewButtonClicked)
    EVT_BUTTON(DLVID_LOGCLEARBTN, DlvChannelPage::OnLogClear)
    EVT_COMBOBOX(DLVID_PRIORITYCOMBO, DlvChannelPage::OnPriorityFilterChanged)
END_EVENT_TABLE()

DlvChannelPage::DlvChannelPage(wxWindow *parent)
    : wxPanel(parent)
    , mMainVBoxSizer(new wxBoxSizer(wxVERTICAL))
    , mButtonHBoxSizer(new wxBoxSizer(wxHORIZONTAL))
    , mPriorityFilterHBoxSizer(new wxBoxSizer(wxHORIZONTAL))
    , mContentHBoxSizer(new wxBoxSizer(wxHORIZONTAL))
    , mLogVBoxSizer(new wxBoxSizer(wxVERTICAL))
    , mFilterHBoxSizer(new wxBoxSizer(wxHORIZONTAL))
{
    // Load icons
    wxImage::AddHandler(new wxPNGHandler);
    mFilterAddImage    = new wxImage(wxT("resource/add-icon.png"),     wxBITMAP_TYPE_PNG);
    mFilterEditImage   = new wxImage(wxT("resource/edit-icon.png"),    wxBITMAP_TYPE_PNG);
    mFilterDeleteImage = new wxImage(wxT("resource/remove-icon.png"),  wxBITMAP_TYPE_PNG);
    mClearLogImage     = new wxImage(wxT("resource/clear-icon.png"),   wxBITMAP_TYPE_PNG);
    mShowRegViewImage  = new wxImage(wxT("resource/logreg-icon.png"),  wxBITMAP_TYPE_PNG);
    mHideRegViewImage  = new wxImage(wxT("resource/logonly-icon.png"), wxBITMAP_TYPE_PNG);

    // Initiate all components
    mLogListCtrl          = new wxListCtrl(this, DLVID_LOGLISTCTRL, wxDefaultPosition, 
                                           wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mRegListCtrl          = new wxListCtrl(this, DLVID_REGLISTCTRL, wxDefaultPosition, 
                                           wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    mShowRegViewButton    = new wxBitmapButton(this, DLVID_SHOWREGBTN, wxBitmap(*mShowRegViewImage),
                                               wxDefaultPosition, wxSize(28, 28));
    mFilterAddButton      = new wxBitmapButton(this, DLVID_FILTERADDBTN, wxBitmap(*mFilterAddImage),
                                               wxDefaultPosition, wxSize(28, 28));
    mFilterEditButton     = new wxBitmapButton(this, DLVID_FILTEREDITBTN, wxBitmap(*mFilterEditImage),
                                               wxDefaultPosition, wxSize(28, 28));
    mFilterDeleteButton   = new wxBitmapButton(this, DLVID_FILTERDELBTN, wxBitmap(*mFilterDeleteImage),
                                               wxDefaultPosition, wxSize(28, 28));
    mClearLogButton       = new wxBitmapButton(this, DLVID_LOGCLEARBTN, wxBitmap(*mClearLogImage),
                                               wxDefaultPosition, wxSize(28, 28));

    wxString priorities[] = { DLVSTR_PRIORITY_VERBOSE, 
                              DLVSTR_PRIORITY_DEBUG, 
                              DLVSTR_PRIORITY_INFO, 
                              DLVSTR_PRIORITY_WARN, 
                              DLVSTR_PRIORITY_ERROR };
    mPriorityComboBox     = new wxComboBox(this, DLVID_PRIORITYCOMBO, DLVSTR_PRIORITY_VERBOSE,
                                           wxDefaultPosition, wxDefaultSize, 5, priorities, wxCB_READONLY);
    mFilterTextCtrl       = new wxTextCtrl(this, DLVID_FILTERTEXTCTRL);

    // Config all layout and size-proportion
    mPriorityFilterHBoxSizer->Add(new wxStaticText(this, wxID_ANY, DLVSTR_CHPAGE_PRIORITY_LABEL),
                                  0, wxALIGN_CENTER_VERTICAL);
    mPriorityFilterHBoxSizer->AddSpacer(5);
    mPriorityFilterHBoxSizer->Add(mPriorityComboBox, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);

    mButtonHBoxSizer->AddStretchSpacer();
    mButtonHBoxSizer->Add(mPriorityFilterHBoxSizer);
    mButtonHBoxSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 28), wxLI_VERTICAL),
                          0, wxLEFT | wxRIGHT, 5);
    mButtonHBoxSizer->Add(mFilterAddButton);
    mButtonHBoxSizer->Add(mFilterEditButton);
    mButtonHBoxSizer->Add(mFilterDeleteButton);
    mButtonHBoxSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 28), wxLI_VERTICAL),
                          0, wxLEFT | wxRIGHT, 5);
    mButtonHBoxSizer->Add(mClearLogButton);
    mButtonHBoxSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(1, 28), wxLI_VERTICAL),
                          0, wxLEFT | wxRIGHT, 5);
    mButtonHBoxSizer->Add(mShowRegViewButton);
    mButtonHBoxSizer->AddSpacer(5);

    mFilterHBoxSizer->Add(new wxStaticText(this, wxID_ANY, DLVSTR_CHPAGE_FILTER_LABEL),
                          0, wxALIGN_CENTER_VERTICAL);
    mFilterHBoxSizer->AddSpacer(5);
    mFilterHBoxSizer->Add(mFilterTextCtrl, 1, wxEXPAND);

    mLogVBoxSizer->Add(mLogListCtrl, 1, wxEXPAND);
    mLogVBoxSizer->AddSpacer(5);
    mLogVBoxSizer->Add(mFilterHBoxSizer, 0, wxEXPAND);
    mLogVBoxSizer->AddSpacer(5);

    mContentHBoxSizer->Add(mLogVBoxSizer, 4, wxEXPAND);
    mContentHBoxSizer->Add(mRegListCtrl, 1, wxLEFT | wxEXPAND, 5);
    mRegListCtrl->Show(false);

    mMainVBoxSizer->AddSpacer(5);
    mMainVBoxSizer->Add(mButtonHBoxSizer, 0, wxEXPAND);
    mMainVBoxSizer->AddSpacer(5);
    mMainVBoxSizer->Add(mContentHBoxSizer, 1, wxEXPAND);

    for (unsigned int i=0; i<LOG_HDRNUM; ++i)
    {
        mLogListCtrl->InsertColumn(i, _LogHeaders[i].Label, wxLIST_FORMAT_LEFT, _LogHeaders[i].Width);
    }

    for (unsigned int i=0; i<REG_HDRNUM; ++i)
    {
        mRegListCtrl->InsertColumn(i, _RegHeaders[i].Label, wxLIST_FORMAT_LEFT, _RegHeaders[i].Width);
    }

    SetSizer(mMainVBoxSizer);

    mPriorityComboBox->SetSelection(0);
}

DlvChannelPage::~DlvChannelPage()
{
    resetContent();

    delete mFilterAddImage;    mFilterAddImage = 0;
    delete mFilterEditImage;   mFilterEditImage = 0;
    delete mFilterDeleteImage; mFilterDeleteImage = 0;
    delete mClearLogImage;     mClearLogImage = 0;
    delete mShowRegViewImage;  mShowRegViewImage = 0;
    delete mHideRegViewImage;  mHideRegViewImage = 0;
}

void DlvChannelPage::OnAppenLog(DlvEvtDataLog *logdata)
{
    mLogData.push_back(logdata);
    if (logdata->Priority >= getCurrentPriorityFilterLevel())
    {
        renderLog(logdata);
    }
}

void DlvChannelPage::OnUpdateRegister(DlvEvtDataRegister *regdata)
{
    std::pair<wxString, long> mapValue(regdata->Register, (long)mRegListCtrl->GetItemCount());
    std::pair<RegisterMap::iterator, bool> result;

    result = mRegisterData.insert(mapValue);
    long pos = result.first->second;
    if ( result.second )
    {
        mRegListCtrl->InsertItem(pos, regdata->Register);
    }
    mRegListCtrl->SetItem(pos, REG_HDRIDX_CNT, regdata->Content);
}

void DlvChannelPage::OnShowRegViewButtonClicked(wxCommandEvent& ev)
{
    if (mRegListCtrl->IsShown())
    {
        mRegListCtrl->Show(false);
        mShowRegViewButton->SetBitmapLabel(wxBitmap(*mShowRegViewImage));
    } else {
        mRegListCtrl->Show(true);
        mShowRegViewButton->SetBitmapLabel(wxBitmap(*mHideRegViewImage));
    }
    mContentHBoxSizer->Layout();
}

void DlvChannelPage::OnLogClear(wxCommandEvent& ev)
{
    resetContent();
}

void DlvChannelPage::resetContent()
{
    resetLogContent();
    resetRegisterContent();
}

void DlvChannelPage::resetLogContent(bool deleteRaw)
{
    // Save column width settings, use default value for initial reset ( width = 0 )
    int colWidth[LOG_HDRNUM] = {0};
    for (unsigned int i=0; i<LOG_HDRNUM; ++i)
    {
        colWidth[i] = mLogListCtrl->GetColumnWidth(i);
        if (0 == colWidth[i])
            colWidth[i] = _LogHeaders[i].Width;
    }

    mLogListCtrl->ClearAll();
    if (deleteRaw)
    {
        for (LogDataVector::iterator it = mLogData.begin(); it != mLogData.end(); it++)
            delete *it;
        mLogData.clear();
    }

    // setup column headers
    for (unsigned int i=0; i<LOG_HDRNUM; ++i)
    {
        mLogListCtrl->InsertColumn(i, _LogHeaders[i].Label, wxLIST_FORMAT_LEFT, colWidth[i]);
    }
}

void DlvChannelPage::resetRegisterContent()
{
    // Save column width settings, use default value for initial reset ( width = 0 )
    int colWidth[REG_HDRNUM] = {0};
    for (unsigned int i=0; i<REG_HDRNUM; ++i)
    {
        colWidth[i] = mRegListCtrl->GetColumnWidth(i);
        if (0 == colWidth[i])
            colWidth[i] = _RegHeaders[i].Width;
    }

    mRegListCtrl->ClearAll();
    mRegisterData.clear();

    // setup column headers
    for (unsigned int i=0; i<REG_HDRNUM; ++i)
    {
        mRegListCtrl->InsertColumn(i, _RegHeaders[i].Label, wxLIST_FORMAT_LEFT, colWidth[i]);
    }
}

void DlvChannelPage::OnPriorityFilterChanged(wxCommandEvent& ev)
{
    resetLogContent(false);

    for (LogDataVector::iterator it = mLogData.begin();
         it != mLogData.end(); it++)
    {
        if ((*it)->Priority >= getCurrentPriorityFilterLevel())
            renderLog(*it);
    }
}

unsigned char DlvChannelPage::getCurrentPriorityFilterLevel()
{
    return (unsigned char)mPriorityComboBox->GetCurrentSelection();
}

void DlvChannelPage::renderLog(DlvEvtDataLog *logdata)
{
    static wxColourDatabase colorDB;
    static wxColour textColors[] = { colorDB.Find(wxT("BLACK")), 
                                     colorDB.Find(wxT("BLUE")), 
                                     colorDB.Find(wxT("FOREST GREEN")), 
                                     colorDB.Find(wxT("GOLD")),
                                     colorDB.Find(wxT("RED")) };
    static wxString priorityLabels[] = { wxT("V"), wxT("D"), wxT("I"), wxT("W"), wxT("E") };

    int pos = mLogListCtrl->GetItemCount();

    wxDateTime date((time_t)logdata->TimestampSec);
    wxString dateStr;
    dateStr.Printf(wxT("%02d-%02d %02d:%02d:%02d.%03u"), date.GetMonth(), date.GetDay(),
        date.GetHour(), date.GetMinute(), date.GetSecond(), logdata->TimestampMs);
    mLogListCtrl->InsertItem(pos, dateStr);
    mLogListCtrl->SetItem(pos, LOG_HDRIDX_PRI, priorityLabels[logdata->Priority]);
    mLogListCtrl->SetItem(pos, LOG_HDRIDX_TAG, logdata->Tag);
    mLogListCtrl->SetItem(pos, LOG_HDRIDX_MSG, logdata->Message);
    mLogListCtrl->SetItemTextColour(pos, textColors[logdata->Priority]);

    // TODO: scroll to the bottom *conditionally*
    mLogListCtrl->EnsureVisible(pos);
}
