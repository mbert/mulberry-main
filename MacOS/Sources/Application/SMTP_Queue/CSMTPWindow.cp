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


// Source for CSMTPWindow class

#include "CSMTPWindow.h"

#include "CMailboxInfoTable.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

#include "MyCFString.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members
CSMTPWindow* CSMTPWindow::sSMTPWindow = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPWindow::CSMTPWindow()
{
	sSMTPWindow = this;
}

// Constructor from stream
CSMTPWindow::CSMTPWindow(LStream *inStream)
		: CMailboxWindow(inStream)
{
	sSMTPWindow = this;
}

// Default destructor
CSMTPWindow::~CSMTPWindow()
{
	sSMTPWindow = NULL;
}

// Open window
void CSMTPWindow::OpenSMTPWindow()
{

	// Create new or show existing
	if (sSMTPWindow)
		FRAMEWORK_WINDOW_TO_TOP(sSMTPWindow)
	else
	{
		// Create the mailbox list window
		CSMTPWindow* aWindow = (CSMTPWindow*) LWindow::CreateWindow(paneid_SMTPWindow, CMulberryApp::sApp);
		
		// This may throw and delete the window
		try
		{
			// Find first enable SMTP sender and set it
			CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
			for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++)
			{
				// Exit for first enabled
				if (static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
				{
					aWindow->GetSMTPView()->SetSMTPSender(*iter);
					break;
				}
			}
		
			aWindow->Show();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

		}
	}
}

// Open window
void CSMTPWindow::CloseSMTPWindow()
{
	// Delete any existing window
	if (sSMTPWindow)
		FRAMEWORK_DELETE_WINDOW(sSMTPWindow)
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CSMTPWindow::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxWindow::FinishCreateSelf();

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_SMTPToolbarView);
	mToolbarView->SetSibling(mSplitter);

	// Create toolbars for a view we own
	GetSMTPView()->MakeToolbars(mToolbarView);
	GetSMTPView()->GetPreview()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
}

void CSMTPWindow::UpdateTitle()
{
	// Set state of enabled button
	bool enable = false;
	enable = CConnectionManager::sConnectionManager.IsConnected() &&
				CSMTPAccountManager::sSMTPAccountManager->GetConnected();

	MyCFString ptitle(CopyCFDescriptor());
	cdstring title = ptitle.GetString();

	cdstring new_title;
	new_title.FromResource("Alerts::SMTP::Queues");
	if (GetSMTPView()->GetSMTPSender())
	{
		cdstring temp;
		new_title += GetSMTPView()->GetSMTPSender()->GetAccountName();
		new_title += " - ";
		new_title += temp.FromResource(enable ? "Alerts::SMTP::QueuesEnabled" : "Alerts::SMTP::QueuesDisabled");
	}
	
	if (new_title != title)
	{
		MyCFString temp(new_title, kCFStringEncodingUTF8);
		SetCFDescriptor(temp);
	}
}
