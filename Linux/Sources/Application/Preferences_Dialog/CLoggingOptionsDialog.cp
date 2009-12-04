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


// CLoggingOptionsDialog.cpp : implementation file
//

#include "CLoggingOptionsDialog.h"

#include <JXWindow.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog dialog


CLoggingOptionsDialog::CLoggingOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CLoggingOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 455,350, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 455,350);
    assert( obj1 != NULL );

    mActivate =
        new JXTextCheckbox("Logging is Enabled", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,15, 180,20);
    assert( mActivate != NULL );

    mIMAP =
        new JXTextCheckbox("Log IMAP Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,40, 180,20);
    assert( mIMAP != NULL );

    mPOP3 =
        new JXTextCheckbox("Log POP3 Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,60, 180,20);
    assert( mPOP3 != NULL );

    mSMTP =
        new JXTextCheckbox("Log SMTP Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,80, 180,20);
    assert( mSMTP != NULL );

    mIMSP =
        new JXTextCheckbox("Log IMSP Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,100, 180,20);
    assert( mIMSP != NULL );

    mACAP =
        new JXTextCheckbox("Log ACAP Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,120, 180,20);
    assert( mACAP != NULL );

    mHTTP =
        new JXTextCheckbox("Log HTTP Connections", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,140, 180,20);
    assert( mHTTP != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 365,315, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 275,315, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mAuthentication =
        new JXTextCheckbox("Show Authentication Details", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,165, 190,20);
    assert( mAuthentication != NULL );

    mPlayback =
        new JXTextCheckbox("Log Disconnected Playback", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,190, 190,20);
    assert( mPlayback != NULL );

    mPlugins =
        new JXTextCheckbox("Log Plugins", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,210, 180,20);
    assert( mPlugins != NULL );

    mFiltering =
        new JXTextCheckbox("Log Filtering", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,230, 180,20);
    assert( mFiltering != NULL );

    mErrors =
        new JXTextCheckbox("Log Errors", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,255, 180,20);
    assert( mErrors != NULL );

    mOverwrite =
        new JXTextCheckbox("Overwrite Logs on Startup", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,280, 180,20);
    assert( mOverwrite != NULL );

    mClearBtn =
        new JXTextButton("Clear Logs", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 140,315, 100,25);
    assert( mClearBtn != NULL );

    mFlushBtn =
        new JXTextButton("Flush Logs", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,315, 100,25);
    assert( mFlushBtn != NULL );

// end JXLayout

	mLogs[CLog::eLogIMAP] = mIMAP;
	mLogs[CLog::eLogPOP3] = mPOP3;
	mLogs[CLog::eLogIMSP] = mIMSP;
	mLogs[CLog::eLogACAP] = mACAP;
	mLogs[CLog::eLogSMTP] = mSMTP;
	mLogs[CLog::eLogHTTP] = mHTTP;
	mLogs[CLog::eLogPlugin] = mPlugins;
	mLogs[CLog::eLogFilters] = mFiltering;
	mLogs[CLog::eLogExceptions] = mErrors;

	window->SetTitle("Logging Options");
	SetButtons(mOKBtn, mCancelBtn);
	ListenTo(mFlushBtn);
	ListenTo(mClearBtn);
}

/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog message handlers

void CLoggingOptionsDialog::Receive(JBroadcaster*	sender, const Message&	message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mFlushBtn)
		{
			CLog::FlushLogs();
			return;
		}
		else if (sender == mClearBtn)
		{
			CLog::ClearLogs();
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

// Set options in dialog
void CLoggingOptionsDialog::SetOptions(const CLog::SLogOptions& options)
{
	mActivate->SetState(JConvertToBoolean(options.mActivate));

	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		mLogs[i]->SetState(JConvertToBoolean(options.mEnable[i]));

	mAuthentication->SetState(JConvertToBoolean(options.mAuthentication));
	mPlayback->SetState(JConvertToBoolean(options.mPlayback));
	mOverwrite->SetState(JConvertToBoolean(options.mOverwrite));
}

// Get options from dialog
void CLoggingOptionsDialog::GetOptions(CLog::SLogOptions& options)
{
	options.mActivate = mActivate->IsChecked();

	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		options.mEnable[i] = mLogs[i]->IsChecked();

	options.mAuthentication = mAuthentication->IsChecked();
	options.mPlayback = mPlayback->IsChecked();
	options.mOverwrite = mOverwrite->IsChecked();
}

bool CLoggingOptionsDialog::PoseDialog(CLog::SLogOptions& options)
{
	bool result = false;

	CLoggingOptionsDialog* dlog = new CLoggingOptionsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetOptions(options);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetOptions(options);
		result = true;
		dlog->Close();
	}

	return result;
}
