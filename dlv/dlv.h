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

//#include <vld.h>

#include <wx/wx.h>
#include <dill.h>
#include "dlv_dillevt.h"

#define DLVSTR_MAINFRAME_TITLE         wxT("DLV - Dill Log Viewer")
#define DLVSTR_ABOUT_MSG               wxT("DLV 0.1.0")
#define DLVSTR_ABOUT_TITLE             wxT("About DLV")
#define DLVSTR_ERROR_MSG               wxT("Failed to start dill server at %s:%u. (EC:%u)\nTo work with an external dill server, pass its address/port as command arguments.")
#define DLVSTR_ERROR_TITLE             wxT("Error")
#define DLVSTR_HELPMENU_LABEL          wxT("&Help")
#define DLVSTR_HELPMENU_ABOUT_LABEL    wxT("&About...\tF1")
#define DLVSTR_HELPMENU_ABOUT_COMMENT  wxT("Show about dialog")
#define DLVSTR_FILEMENU_LABEL          wxT("&File")
#define DLVSTR_FILEMENU_EXIT_LABEL     wxT("E&xit\tAlt-X")
#define DLVSTR_FILEMENU_EXIT_COMMENT   wxT("Quit this program")
#define DLVSTR_STATMSG_FORMAT          wxT("Server [%s:%d]  Available Channels: %u")
#define DLVSTR_PRIORITY_VERBOSE        wxT("Verbose")
#define DLVSTR_PRIORITY_DEBUG          wxT("Debug")
#define DLVSTR_PRIORITY_INFO           wxT("Info")
#define DLVSTR_PRIORITY_WARN           wxT("Warn")
#define DLVSTR_PRIORITY_ERROR          wxT("Error")
#define DLVSTR_CHPAGE_PRIORITY_LABEL   wxT("Priority:")
#define DLVSTR_CHPAGE_FILTER_LABEL     wxT("Filter:")
#define DLVSTR_LOGHDR_DATE             wxT("Date")
#define DLVSTR_LOGHDR_PRIORITY         wxT("PRI")
#define DLVSTR_LOGHDR_TAG              wxT("Tag")
#define DLVSTR_LOGHDR_MESSAGE          wxT("Message")
#define DLVSTR_REGHDR_NAME             wxT("Name")
#define DLVSTR_REGHDR_CONTENT          wxT("Content")

enum
{
    DLVID_MAINFRAME = 5299,
    DLVID_CHANNELNOTEBOOK,
    DLVID_CONNSTATTIMER,
    DLVID_LOGLISTCTRL,
    DLVID_REGLISTCTRL,
    DLVID_PRIORITYCOMBO,
    DLVID_FILTERTEXTCTRL,
    DLVID_SHOWREGBTN,
    DLVID_FILTERADDBTN,
    DLVID_FILTERDELBTN,
    DLVID_FILTEREDITBTN,
    DLVID_LOGCLEARBTN,
};
