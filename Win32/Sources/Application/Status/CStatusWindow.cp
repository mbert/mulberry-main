/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Source for CStatusWindow class


#include "CStatusWindow.h"
#include "CMailControl.h"
#include "CMulberryApp.h"
#include "CTaskClasses.h"
#include "CXStringResources.h"

#include <stdio.h>

// __________________________________________________________________________________________________
// C L A S S __ C S T A T U S W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CUTF8StatusBar* CStatusWindow::sStatusBar = NULL;
cdstring CStatusWindow::sIdleStatus;
cdstring CStatusWindow::sIMAPStatus;
cdstring CStatusWindow::sSMTPStatus;
bool	CStatusWindow::sStatusLock = false;
bool	CStatusWindow::sChangeOnce = false;

// O T H E R  M E T H O D S ____________________________________________________________________________

#define SBPF_UPDATE 0x0001  // pending update of text

struct AFX_STATUSPANE
{
	UINT    nID;        // IDC of indicator: 0 => normal text area
	int     cxText;     // width of string area in pixels
						//   on both sides there is a 3 pixel gap and
						//   a one pixel border, making a pane 6 pixels wider
	UINT    nStyle;     // style flags (SBPS_*)
	UINT    nFlags;     // state flags (SBPF_*)
	CString strText;    // text in the pane
};

BOOL CUTF8StatusBar::SetPaneTextUTF8(int nIndex, const char* utf8, BOOL bUpdate)
{
#ifdef _UNICODE
	cdustring utf16(utf8);
	return SetPaneText(nIndex, utf16, bUpdate);
#else
	ASSERT_VALID(this);
	ASSERT(::IsWindow(m_hWnd));

	AFX_STATUSPANE* pSBP = _GetPanePtr(nIndex);

	if (!(pSBP->nFlags & SBPF_UPDATE) &&
		((utf8 == NULL && pSBP->strText.IsEmpty()) ||
		 (utf8 != NULL && pSBP->strText.Compare(utf8) == 0)))
	{
		// nothing to change
		return TRUE;
	}

	TRY
	{
		if (utf8 != NULL)
			pSBP->strText = utf8;
		else
			pSBP->strText.Empty();
	}
	CATCH_ALL(e)
	{
		// Note: DELETE_EXCEPTION(e) not required
		return FALSE;
	}
	END_CATCH_ALL

	if (!bUpdate)
	{
		// can't update now, wait until later
		pSBP->nFlags |= SBPF_UPDATE;
		return TRUE;
	}

	pSBP->nFlags &= ~SBPF_UPDATE;
	cdustring utf16(utf8);
	DefWindowProc(SB_SETTEXTW, ((WORD)pSBP->nStyle)|nIndex,
		(pSBP->nStyle & SBPS_DISABLED) ? NULL :
		(LPARAM)(LPCTSTR)utf16.c_str());

	return TRUE;
#endif
}

// Init status
void CStatusWindow::InitStatusBar(CUTF8StatusBar* statusBar)
{
	sStatusBar = statusBar;

	// Set widths
	sStatusBar->SetPaneInfo(0, ID_SEPARATOR, SBPS_NOBORDERS, 350);
	sStatusBar->SetPaneInfo(1, ID_SEPARATOR, SBPS_NOBORDERS, 50);
	sStatusBar->SetPaneInfo(2, ID_SEPARATOR, SBPS_NOBORDERS, 150);
	sStatusBar->SetPaneInfo(2, ID_SEPARATOR, SBPS_STRETCH, 150);

	sIMAPStatus = rsrc::GetString("Status::IMAP");
	sSMTPStatus = rsrc::GetString("Status::SMTP");
	sIdleStatus = rsrc::GetString("Status::IDLE");

	sStatusBar->SetPaneTextUTF8(0, sIdleStatus);
	//sStatusBar->SendMessage(WM_PAINT);
	sStatusBar->RedrawWindow();
}

// Fill status
void CStatusWindow::SetDefaultStatus(void)
{
	// Determine SMTP status
	SetSMTPStatus("Status::IDLE");

	// Determine IMAP status
	SetIMAPStatus("Status::IDLE");
}

// Fill status
void CStatusWindow::SetDefaultProgress(void)
{
	// Empty both progress
	SetSMTPProgress(-1);
	SetIMAPProgress(-1);
}

// Set SMTP status string
void CStatusWindow::SetSMTPStatusStr(const cdstring& str)
{
	sStatusBar->SetPaneTextUTF8(0, str.c_str());
	sStatusBar->RedrawWindow();
}

// Set SMTP status string
void CStatusWindow::SetSMTPStatus(const char* rsrcid)
{
	cdstring s = rsrc::GetString(rsrcid);
	s = sSMTPStatus + s;
	
	// Change status using task in case of call from worker thread
	CSMTPStatusTask* task = new CSMTPStatusTask(s);
	task->Go();
}

// Set SMTP status string with numbers
void CStatusWindow::SetSMTPStatus2(const char* rsrcid, long num1, long num2)
{
	// Get the string
	cdstring rsrc_str = rsrc::GetString(rsrcid);

	// Write numbers into it
	cdstring status;
	status.reserve(256);
	::snprintf(status.c_str_mod(), 256, rsrc_str, num1, num2);
	status = sSMTPStatus + status;
	
	// Change status using task in case of call from worker thread
	CSMTPStatusTask* task = new CSMTPStatusTask(status);
	task->Go();
}

// Set SMTP status string
void CStatusWindow::SetIMAPStatusStr(const cdstring& str)
{
	sStatusBar->SetPaneTextUTF8(0, str.c_str());
	sStatusBar->RedrawWindow();
}

// Set IMAP status string
void CStatusWindow::SetIMAPStatus(const char* rsrcid)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	cdstring status = rsrc::GetString(rsrcid);
	status = sIMAPStatus + status;

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(status);
	task->Go();
}

// Set IMAP status string with a number
void CStatusWindow::SetIMAPStatus1(const char* rsrcid, long num1)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Get the string
	cdstring rsrc_str = rsrc::GetString(rsrcid);

	// Write numbers into it
	cdstring status;
	status.reserve(256);
	::snprintf(status.c_str_mod(), 256, rsrc_str, num1);

	status = sIMAPStatus + status;

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(status);
	task->Go();
}

// Set IMAP status string with numbers
void CStatusWindow::SetIMAPStatus2(const char* rsrcid, long num1, long num2)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Get the string
	cdstring rsrc_str = rsrc::GetString(rsrcid);

	// Write numbers into it
	cdstring status;
	status.reserve(256);
	::snprintf(status.c_str_mod(), 256, rsrc_str, num1, num2);
	status = sIMAPStatus + status;

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(status);
	task->Go();
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgressStr(const cdstring& str)
{
	// Set the caption's text
	sStatusBar->SetPaneTextUTF8(1, str.c_str());
	sStatusBar->RedrawWindow();
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgress(long progress)
{
	// Set up string
	cdstring status;
	status.reserve(256);

	// Set up the string
	if ((progress >= 0) && (progress <= 100))
		::snprintf(status.c_str_mod(), 256, "%d%%", progress);
	else if (progress > 100)
		::snprintf(status.c_str_mod(), 256, "%d%%", 100);

	// Set the caption's text
	sStatusBar->SetPaneTextUTF8(1, status);
	sStatusBar->RedrawWindow();
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgressStr(const cdstring& str)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Set the caption's text
	sStatusBar->SetPaneTextUTF8(1, str.c_str());
	sStatusBar->RedrawWindow();
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgress(long progress)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Set up string
	// Set up string
	cdstring status;
	status.reserve(256);

	// Set up the string
	if ((progress >= 0) && (progress <= 100))
		::snprintf(status.c_str_mod(), 256, "%d%%", progress);
	else if (progress > 100)
		::snprintf(status.c_str_mod(), 256, "%d%%", 100);


	// Set the caption's text
	sStatusBar->SetPaneTextUTF8(1, status);
	sStatusBar->RedrawWindow();
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C S M T P A T T A C H P R O G R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPAttachProgress::CSMTPAttachProgress()
{
}

// Default destructor
CSMTPAttachProgress::~CSMTPAttachProgress()
{
	// Make sure progress text is removed
	CStatusWindow::SetSMTPProgress(-1);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CSMTPAttachProgress::SetPercentage(unsigned long percentage)
{
	CProgress::SetPercentage(percentage);

	CStatusWindow::SetSMTPProgress(mPercentage);
}

void CSMTPAttachProgress::SetCount(unsigned long count)
{
	CProgress::SetCount(count);

	// Set status with numbers
	CStatusWindow::SetSMTPStatus2("Status::SMTP::AttachSend", mCount, mTotal);
}

#pragma mark -

// __________________________________________________________________________________________________
// C L A S S __ C N E T W O R K A T T A C H P R O G R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNetworkAttachProgress::CNetworkAttachProgress()
{
}

// Default destructor
CNetworkAttachProgress::~CNetworkAttachProgress()
{
	// Make sure progress text is removed
	CStatusWindow::SetIMAPProgress(-1);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CNetworkAttachProgress::SetPercentage(unsigned long percentage)
{
	CProgress::SetPercentage(percentage);

	CStatusWindow::SetIMAPProgress(mPercentage);
}

void CNetworkAttachProgress::SetCount(unsigned long count)
{
	CProgress::SetCount(count);

	// Set status with numbers
	CStatusWindow::SetIMAPStatus2("Status::IMAP::Processing", mCount, mTotal);
}

