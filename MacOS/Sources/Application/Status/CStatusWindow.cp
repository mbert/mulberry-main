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
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include <stdio.h>

// __________________________________________________________________________________________________
// C L A S S __ C S T A T U S W I N D O W
// __________________________________________________________________________________________________

// Static fields
CStatusWindow*	CStatusWindow::sStatusWindow = NULL;
bool			CStatusWindow::sStatusLock = false;
bool			CStatusWindow::sChangeOnce = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CStatusWindow::CStatusWindow()
{
	sStatusWindow = this;
}

// Constructor from stream
CStatusWindow::CStatusWindow(LStream *inStream) : LWindow(inStream)
{
	sStatusWindow = this;
}

// Default destructor
CStatusWindow::~CStatusWindow()
{
	sStatusWindow = NULL;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Add text after create
void CStatusWindow::FinishCreateSelf(void)
{
	mSMTPStatus = (CStaticText*) FindPaneByID(paneid_SMTPStatus);
	mSMTPStatus->AddAttachment(new LEraseAttachment(true));

	mIMAPStatus = (CStaticText*) FindPaneByID(paneid_IMAPStatus);
	mIMAPStatus->AddAttachment(new LEraseAttachment(true));

	mSMTPProgress = (CStaticText*) FindPaneByID(paneid_SMTPProgress);
	mSMTPProgress->AddAttachment(new LEraseAttachment(true));

	mIMAPProgress = (CStaticText*) FindPaneByID(paneid_IMAPProgress);
	mIMAPProgress->AddAttachment(new LEraseAttachment(true));

	// Use default status
	SetDefaultStatus();
	SetDefaultProgress();

	// Get window state
	ResetState();
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

// Reset window state from prefs
void CStatusWindow::ResetState(void)
{
	CStatusWindowState& state = CPreferences::sPrefs->mStatusWindowDefault.Value();

	// Set closed state
	if (state.GetClosed())
		Hide();
	else
		Show();

	// Get window state
	Rect set_rect = state.GetBestRect(CPreferences::sPrefs->mStatusWindowDefault.GetValue());

	// Use default if empty
	if (::EmptyRect(&set_rect))
	{
		// Move it to bottom left of main screen
		Rect screen = (**::GetMainDevice()).gdRect;
		set_rect.left = 4;
		set_rect.top = screen.bottom - 4 - mFrameSize.height;
	}

	// Clip to screen
	::RectOnScreen(set_rect, this);

	// Change its position
	DoSetPosition(topLeft(set_rect));
}

// Save state
void CStatusWindow::SaveState(void)
{
	// Get name as cstr
	char name = '\0';

	// Get bounds
	Rect bounds;
	CalcPortFrameRect(bounds);
	PortToGlobalPoint(topLeft(bounds));
	PortToGlobalPoint(botRight(bounds));

	// Add info to prefs
	CStatusWindowState state(&name, &bounds, eWindowStateNormal, mVisible == triState_Off);
	if (CPreferences::sPrefs->mStatusWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mStatusWindowDefault.SetDirty();

}

void CStatusWindow::SetSMTPStatusStr(const cdstring& status)
{
	if (sStatusWindow)
		sStatusWindow->SetSMTPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
}

void CStatusWindow::SetSMTPStatusPrivate(const cdstring& status)
{
	StGrafPortSaver graf_port_save;
	mSMTPStatus->SetText(status);
	mSMTPStatus->DontRefresh();
	mSMTPStatus->Draw(NULL);
	OutOfFocus(NULL);
}

void CStatusWindow::SetSMTPProgressStr(const cdstring& status)
{
	if (sStatusWindow)
		sStatusWindow->SetSMTPProgressPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(status);
}

void CStatusWindow::SetSMTPProgressPrivate(const cdstring& status)
{
	StGrafPortSaver graf_port_save;
	mSMTPProgress->SetText(status);
	mSMTPProgress->DontRefresh();
	mSMTPProgress->Draw(NULL);
	OutOfFocus(NULL);
}

void CStatusWindow::SetIMAPStatusStr(const cdstring& status)
{
	if (sStatusWindow)
		sStatusWindow->SetIMAPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
}

void CStatusWindow::SetIMAPStatusPrivate(const cdstring& status)
{
	StGrafPortSaver graf_port_save;
	mIMAPStatus->SetText(status);
	mIMAPStatus->DontRefresh();
	mIMAPStatus->Draw(NULL);
	OutOfFocus(NULL);
}

void CStatusWindow::SetIMAPProgressStr(const cdstring& status)
{
	if (sStatusWindow)
		sStatusWindow->SetIMAPProgressPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(status);
}

void CStatusWindow::SetIMAPProgressPrivate(const cdstring& status)
{
	StGrafPortSaver graf_port_save;
	mIMAPProgress->SetText(status);
	mIMAPProgress->DontRefresh();
	mIMAPProgress->Draw(NULL);
	OutOfFocus(NULL);
}

// Set SMTP status string
void CStatusWindow::SetSMTPStatus(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetSMTPStatusPrivate(str);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(str);
}

// Set SMTP status string
void CStatusWindow::SetSMTPStatus(const char* rsrcid)
{
	// Get the string
	cdstring status = rsrc::GetString(rsrcid);

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetSMTPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
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

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetSMTPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
}

// Set IMAP status string
void CStatusWindow::SetIMAPStatus(const cdstring& str)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPStatusPrivate(str);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(str);
}

// Set IMAP status string
void CStatusWindow::SetIMAPStatus(const char* rsrcid)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Set up the string
	cdstring status = rsrc::GetString(rsrcid);

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
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

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
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

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPStatusPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(status);
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgress(const cdstring& str)
{
	LStr255 status(str);

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetSMTPProgressPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(status);
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgress(long progress)
{
#if 0
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;
#endif

	cdstring status;
	status.reserve(256);

	// Set up the string
	if ((progress >= 0) && (progress <= 100))
		::snprintf(status.c_str_mod(), 256, "%d%%", progress);
	else if (progress > 100)
		::snprintf(status.c_str_mod(), 256, "%d%%", 100);

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetSMTPProgressPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(status);
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgress(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPProgressPrivate(str);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(str);
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgress(long progress)
{
#if 0
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;
#endif

	cdstring status;
	status.reserve(256);

	// Set up the string
	if ((progress >= 0) && (progress <= 100))
		::snprintf(status.c_str_mod(), 256, "%d%%", progress);
	else if (progress > 100)
		::snprintf(status.c_str_mod(), 256, "%d%%", 100);

	// Set the caption's text
	if (sStatusWindow)
		sStatusWindow->SetIMAPProgressPrivate(status);
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(status);
}

// Hide instead of close
void CStatusWindow::AttemptClose()
{
	// Just hide it - do not close
	Hide();

	// Force update of menu
	SetUpdateCommandStatus(true);
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
	if (GetPercentage() != percentage)
	{
		CProgress::SetPercentage(percentage);

		CStatusWindow::SetSMTPProgress(GetPercentage());
	}
}

void CSMTPAttachProgress::SetCount(unsigned long count)
{
	if (GetCount() != count)
	{
		CProgress::SetCount(count);

		// Set status with numbers
		CStatusWindow::SetSMTPStatus2("Status::SMTP::AttachSend", mCount, mTotal);
	}
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
	if (GetPercentage() != percentage)
	{
		CProgress::SetPercentage(percentage);

		CStatusWindow::SetIMAPProgress(GetPercentage());
	}
}

void CNetworkAttachProgress::SetCount(unsigned long count)
{
	if (GetCount() != count)
	{
		CProgress::SetCount(count);

		// Set status with numbers
		CStatusWindow::SetIMAPStatus2("Status::IMAP::Processing", mCount, mTotal);
	}
}

#pragma mark ____________________________StStatusWindowHide

StStatusWindowHide::StStatusWindowHide()
{
	mWasVisible = (CStatusWindow::sStatusWindow ? CStatusWindow::sStatusWindow->IsVisible() : false);
	if (mWasVisible) CStatusWindow::sStatusWindow->Hide();
}

StStatusWindowHide::~StStatusWindowHide()
{
	if (mWasVisible) CStatusWindow::sStatusWindow->Show();
}
