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


// Source for CLoggingOptionsDialog class

#include "CLoggingOptionsDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include <LCheckBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CLoggingOptionsDialog::CLoggingOptionsDialog()
{
}

// Constructor from stream
CLoggingOptionsDialog::CLoggingOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CLoggingOptionsDialog::~CLoggingOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CLoggingOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mEnabled = (LCheckBox*) FindPaneByID(paneid_LoggingOptionsEnabled);
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		mLogs[i] = (LCheckBox*) FindPaneByID(paneid_LoggingOptions[i]);
	mAuthentication = (LCheckBox*) FindPaneByID(paneid_LoggingOptionsAuthentication);
	mPlayback = (LCheckBox*) FindPaneByID(paneid_LoggingOptionsPlayback);
	mOverwrite = (LCheckBox*) FindPaneByID(paneid_LoggingOptionsOverwrite);
	mApplicationDirectory = (LRadioButton*) FindPaneByID(paneid_LoggingOptionsAppDir);
	mUserDirectory = (LRadioButton*) FindPaneByID(paneid_LoggingOptionsUserDir);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CLoggingOptionsDialog);
}

// Handle buttons
void CLoggingOptionsDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_LoggingOptionsFlush:
			CLog::FlushLogs();
			break;
		case msg_LoggingOptionsClear:
			CLog::ClearLogs();
			break;
	}
}
// Set options in dialog
void CLoggingOptionsDialog::SetOptions(const CLog::SLogOptions& options)
{
	mEnabled->SetValue(options.mActivate);
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		mLogs[i]->SetValue(options.mEnable[i]);
	mAuthentication->SetValue(options.mAuthentication);
	mPlayback->SetValue(options.mPlayback);
	mOverwrite->SetValue(options.mOverwrite);
	if (options.mUserCWD)
		mUserDirectory->SetValue(1);
	else
		mApplicationDirectory->SetValue(1);
}

// Get options from dialog
void CLoggingOptionsDialog::GetOptions(CLog::SLogOptions& options)
{
	options.mActivate = mEnabled->GetValue();
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		options.mEnable[i] = mLogs[i]->GetValue();
	options.mAuthentication = mAuthentication->GetValue();
	options.mPlayback = mPlayback->GetValue();
	options.mOverwrite = mOverwrite->GetValue();
	options.mUserCWD = mUserDirectory->GetValue();
}

bool CLoggingOptionsDialog::PoseDialog(CLog::SLogOptions& options)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_LoggingOptionsDialog, CMulberryApp::sApp);
	((CLoggingOptionsDialog*) theHandler.GetDialog())->SetOptions(options);
	theHandler.GetDialog()->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CLoggingOptionsDialog*) theHandler.GetDialog())->GetOptions(options);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
