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
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTaskClasses.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "HResourceMap.h"

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXStaticText.h>
#include <JXWindow.h>

#include <cassert>
#include <stdio.h>

// __________________________________________________________________________________________________
// C L A S S __ C S T A T U S W I N D O W
// __________________________________________________________________________________________________

// Static fields
CStatusWindow*	CStatusWindow::sStatusWindow = NULL;
bool		CStatusWindow::sStatusLock = false;
bool		CStatusWindow::sChangeOnce = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CStatusWindow::CStatusWindow(JXDirector* owner)
	: JXWindowDirector(owner)
{
	sStatusWindow = this;
}

// Default destructor
CStatusWindow::~CStatusWindow()
{
	sStatusWindow = NULL;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

CStatusWindow* CStatusWindow::CreateWindow(JXDirector* owner)
{
	new CStatusWindow(owner);
	sStatusWindow->OnCreate();
	sStatusWindow->ResetState();

	return sStatusWindow;
}

// Destroy it
void CStatusWindow::DestroyWindow()
{
	// Update from window
	if (sStatusWindow)
		FRAMEWORK_DELETE_WINDOW(sStatusWindow);
	sStatusWindow = NULL;
}

// Add text after create
void CStatusWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 370,40, "");
    assert( window != NULL );
    SetWindow(window);

    JXStaticText* obj1 =
        new JXStaticText("Network:", window,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 55,20);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    JXStaticText* obj2 =
        new JXStaticText("SMTP:", window,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,20, 55,20);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    mIMAPStatus =
        new CStaticText("Idle", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,0, 240,20);
    assert( mIMAPStatus != NULL );

    mSMTPStatus =
        new CStaticText("Idle", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,20, 240,20);
    assert( mSMTPStatus != NULL );

    mIMAPProgress =
        new CStaticText("", window,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 300,0, 60,20);
    assert( mIMAPProgress != NULL );

    mSMTPProgress =
        new CStaticText("", window,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 300,20, 60,20);
    assert( mSMTPProgress != NULL );

// end JXLayout

	GetWindow()->SetWMClass(cStatusWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_MAINFRAME, GetWindow(), 32, 0x00CCCCCC));
	window->SetTitle("Mulberry Status");
	window->LockCurrentSize();

	mIMAPStatus->SetBreakCROnly(false);
	mSMTPStatus->SetBreakCROnly(false);
	mIMAPProgress->SetBreakCROnly(false);
	mSMTPProgress->SetBreakCROnly(false);

	// Use default status
	SetDefaultStatus();
	SetDefaultProgress();
}

// Hide instead of close
JBoolean CStatusWindow::Close()
{
	if (CMulberryApp::mQuitting || (C3PaneWindow::s3PaneWindow != NULL))
		//We're quitting or we have the 3-pane, so we do want to close
		return JXWindowDirector::Close();
	else
	{
		// Just hide
		GetWindow()->Hide();
		return kFalse;
	}
}

// Fill status
void CStatusWindow::SetDefaultStatus()
{
	SetSMTPStatus("Status::IDLE");
	SetIMAPStatus("Status::IDLE");
}

// Fill status
void CStatusWindow::SetDefaultProgress()
{
	// Empty both progress
	SetSMTPProgress(-1);
	SetIMAPProgress(-1);
}

// Reset window state from prefs
void CStatusWindow::ResetState()
{
	CStatusWindowState& state = CPreferences::sPrefs->mStatusWindowDefault.Value();

	// Get window state
	JRect set_rect = state.GetBestRect(CPreferences::sPrefs->mStatusWindowDefault.GetValue());

	// Use default if empty
	if (set_rect.IsEmpty())
	{
		// Move it to bottom left of main screen
		JRect screen = GetDisplay()->GetBounds();
		set_rect.left = 4;
		set_rect.top = screen.bottom - 36 - GetWindow()->GetFrameHeight();
	}

	// Clip to screen
	::RectOnScreen(set_rect);

	// Change its position
	GetWindow()->Place(set_rect.left, set_rect.top);

	// Set closed state
	Activate();
	if (state.GetClosed())
		GetWindow()->Hide();
	else
		GetWindow()->Show();
}

// Save state
void CStatusWindow::SaveState()
{
	// Get name as cstr
	char name = '\0';

	// Get bounds - convert to position only
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	CStatusWindowState state(&name, &bounds, eWindowStateNormal, !GetWindow()->IsVisible());
	if (CPreferences::sPrefs->mStatusWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mStatusWindowDefault.SetDirty();

}

// Set SMTP status string
void CStatusWindow::SetSMTPStatusStr(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
	{
		sStatusWindow->mSMTPStatus->SetText(str);
		sStatusWindow->mSMTPStatus->Redraw();
	}
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(str);
}

// Set SMTP status string
void CStatusWindow::SetSMTPStatus(const char* rsrcid)
{
	// Get the string
	cdstring s = rsrc::GetString(rsrcid);
	
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
	cdstring s;
	s.reserve(256);
	::snprintf(s.c_str_mod(), 256, rsrc_str, num1, num2);

	// Change status using task in case of call from worker thread
	CSMTPStatusTask* task = new CSMTPStatusTask(s);
	task->Go();
}

// Set SMTP status string
void CStatusWindow::SetIMAPStatusStr(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
	{
		sStatusWindow->mIMAPStatus->SetText(str);
		sStatusWindow->mIMAPStatus->Redraw();
	}
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetStatus(str);
}

// Set IMAP status string
void CStatusWindow::SetIMAPStatus(const char* rsrcid)
{
	if (sStatusLock && !sChangeOnce)
		return;
	sChangeOnce = false;

	// Get the string
	cdstring s = rsrc::GetString(rsrcid);

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(s);
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
	cdstring s;
	s.reserve(256);
	::snprintf(s.c_str_mod(), 256, rsrc_str, num1);

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(s);
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
	cdstring s;
	s.reserve(256);
	::snprintf(s.c_str_mod(), 256, rsrc_str, num1, num2);

	// Change status using task in case of call from worker thread
	CIMAPStatusTask* task = new CIMAPStatusTask(s);
	task->Go();
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgressStr(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
	{
		sStatusWindow->mSMTPProgress->SetText(str);
		sStatusWindow->mSMTPProgress->Redraw();
	}
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(str);
}

// Set SMTP progress string
void CStatusWindow::SetSMTPProgress(long progress)
{
	// Set up the string
	cdstring s;
	s.reserve(32);
	if ((progress >= 0) && (progress <= 100))
		::snprintf(s.c_str_mod(), 32, "%3ld%%", progress);
	else if (progress > 100)
		::snprintf(s.c_str_mod(), 32, "%3ld%%", 100L);

	CSMTPProgressTask* task = new CSMTPProgressTask(s);
	task->Go();
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgressStr(const cdstring& str)
{
	// Set the caption's text
	if (sStatusWindow)
	{
		sStatusWindow->mIMAPProgress->SetText(str);
		sStatusWindow->mIMAPProgress->Redraw();
	}
	else if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->SetProgress(str);
}

// Set IMAP progress string
void CStatusWindow::SetIMAPProgress(long progress)
{
	// Set up the string
	cdstring s;
	s.reserve(32);
	if ((progress >= 0) && (progress <= 100))
		::snprintf(s.c_str_mod(), 32, "%3ld%%", progress);
	else if (progress > 100)
		::snprintf(s.c_str_mod(), 32, "%3ld%%", 100L);

	CIMAPProgressTask* task = new CIMAPProgressTask(s);
	task->Go();
}

// Hide instead of close
JBoolean CStatusWindow::OKToDeactivate()
{
	// Just hide it - do not close
	GetWindow()->Hide();
	return kFalse;
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

		CStatusWindow::SetSMTPProgress(mPercentage);
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

		CStatusWindow::SetIMAPProgress(mPercentage);
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
