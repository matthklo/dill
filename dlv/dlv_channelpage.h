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
#include <wx/listctrl.h>
#include <wx/statline.h>
#include <vector>
#include <map>

class DlvChannelPage : public wxPanel
{
public:
    DlvChannelPage(wxWindow *parent, DLVCHTYPE type = CHTYPE_LIVE);
    ~DlvChannelPage();

    void OnAppenLog(DlvEvtDataLog *logdata);
    void OnUpdateRegister(DlvEvtDataRegister *regdata);

    void OnShowRegViewButtonClicked(wxCommandEvent& ev);
    void OnLogClear(wxCommandEvent& ev);
    void OnPriorityFilterChanged(wxCommandEvent& ev);
    void OnRecordButtonClicked(wxCommandEvent& ev);

    DLVCHTYPE getType() const;

private:
    DECLARE_EVENT_TABLE()

    void renderLog(DlvEvtDataLog *logdata);
    void resetContent();
    void resetLogContent(bool deleteRaw = true);
    void resetRegisterContent();
    unsigned char getCurrentPriorityFilterLevel();

    DLVCHTYPE          mChannelType;
    bool               mRecording;

    wxBoxSizer*        mMainVBoxSizer;
    wxBoxSizer*        mButtonHBoxSizer;
    wxBoxSizer*        mPriorityFilterHBoxSizer;
    wxBoxSizer*        mContentHBoxSizer;
    wxBoxSizer*        mLogVBoxSizer;
    wxBoxSizer*        mFilterHBoxSizer;

    wxListCtrl*        mLogListCtrl;
    wxListCtrl*        mRegListCtrl;
    wxBitmapButton*    mShowRegViewButton;
    wxBitmapButton*    mFilterAddButton;
    wxBitmapButton*    mFilterEditButton;
    wxBitmapButton*    mFilterDeleteButton;
    wxBitmapButton*    mClearLogButton;
    wxBitmapButton*    mRecordButton;
    wxComboBox*        mPriorityComboBox;
    wxTextCtrl*        mFilterTextCtrl;

    wxImage*           mFilterAddImage;
    wxImage*           mFilterEditImage;
    wxImage*           mFilterDeleteImage;
    wxImage*           mClearLogImage;
    wxImage*           mShowRegViewImage;
    wxImage*           mHideRegViewImage;
    wxImage*           mStartRecordingImage;
    wxImage*           mStopRecordingImage;

    typedef std::vector<DlvEvtDataLog*> LogDataVector;
    std::vector<DlvEvtDataLog*> mLogData;

    struct StringLess : std::binary_function<wxString, wxString, bool>
    {
        bool operator() (const wxString& a, const wxString& b) const
        {
            return (a.compare(b) < 0);
        }
    };

    typedef std::map<wxString, long, StringLess> RegisterMap;
    std::map<wxString, long, StringLess> mRegisterData;
};
