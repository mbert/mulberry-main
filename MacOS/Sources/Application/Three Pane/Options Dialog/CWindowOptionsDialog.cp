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


// Source for CWindowOptionsDialog class

#include "CWindowOptionsDialog.h"

#include "CAdminLock.h"
#include "CBalloonDialog.h"
#include "CMessagePaneOptions.h"
#include "CAddressPaneOptions.h"
#include "CCalendarPaneOptions.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTabController.h"
#include "CUserActionOptions.h"
#include "C3PaneWindow.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CWindowOptionsDialog::CWindowOptionsDialog()
{
	mCurrentView = N3Pane::eView_Empty;
}

// Constructor from stream
CWindowOptionsDialog::CWindowOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mCurrentView = N3Pane::eView_Empty;
}

// Default destructor
CWindowOptionsDialog::~CWindowOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CWindowOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mTabs = (CTabController*) FindPaneByID(paneid_WindowOptionsTabs);
	
	// Remove calendar if disabled by admin
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		// Reset maximum to remove button
		mTabs->SetMaxValue(2);
	}

	mTabs->AddPanel(paneid_MailViewOptions);
	mMessage = static_cast<CMessagePaneOptions*>(mTabs->GetPanel(1));
	mTabs->AddPanel(paneid_AddressViewOptions);
	mAddress = static_cast<CAddressPaneOptions*>(mTabs->GetPanel(2));
	mTabs->AddPanel(paneid_CalendarViewOptions);
	mCalendar = static_cast<CCalendarPaneOptions*>(mTabs->GetPanel(3));

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CWindowOptionsDialogBtns);
}

// Handle OK button
void CWindowOptionsDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_WindowOptionsTab:
		SetViewType((N3Pane::EViewType) *((long*) ioParam));
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CWindowOptionsDialog::SetDetails(C3PaneOptions* options, bool is3pane)
{
	mIs3Pane = is3pane;

	// Copy details into local cache of user actions
	for(int i = 0; i < N3Pane::eView_Total; i++)
	{
		for(int j = 0; j < C3PaneOptions::C3PaneViewOptions::eUserAction_Total; j++)
			mUserActions[i][j] = options->GetViewOptions((N3Pane::EViewType) i).GetUserAction((C3PaneOptions::C3PaneViewOptions::EUserAction) j);
	}
	mMailOptions = options->GetMailViewOptions();
	mAddressOptions = options->GetAddressViewOptions();
	mCalendarOptions = options->GetCalendarViewOptions();

	// Set intiial view
	SetViewType(N3Pane::eView_Mailbox);
}

// Get the details
void CWindowOptionsDialog::GetDetails(C3PaneOptions* options)
{
	// Force update of data from current panel
	SetViewType(N3Pane::eView_Empty);
	
	// Copy local cached data into details
	for(int i = 0; i < N3Pane::eView_Total; i++)
	{
		for(int j = 0; j < C3PaneOptions::C3PaneViewOptions::eUserAction_Total; j++)
			options->GetViewOptions((N3Pane::EViewType) i).GetUserAction((C3PaneOptions::C3PaneViewOptions::EUserAction) j) = mUserActions[i][j];
	}
	
	// Update message views if mail view options have changed
	if (!(options->GetMailViewOptions() == mMailOptions))
	{
		options->GetMailViewOptions() = mMailOptions;

		// Reset all message views belonging to server - in reverse
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin();
					iter != CMessageView::sMsgViews->end(); iter++)
			{
				(*iter)->ResetOptions();
			}
		}
		
		// Reset use of substitute
		if (mIs3Pane && C3PaneWindow::s3PaneWindow)
			C3PaneWindow::s3PaneWindow->SetUseSubstitute(mMailOptions.GetUseTabs());
	}

	options->GetAddressViewOptions() = mAddressOptions;
	options->GetCalendarViewOptions() = mCalendarOptions;
}

void CWindowOptionsDialog::SetViewType(N3Pane::EViewType view)
{
	// Get data from existing panel
	switch(mCurrentView)
	{
	case N3Pane::eView_Mailbox:
		mMessage->GetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mMailOptions);
		mMessage->Hide();
		break;
	case N3Pane::eView_Contacts:
		mAddress->GetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mAddressOptions);
		mAddress->Hide();
		break;
	case N3Pane::eView_Calendar:
		mCalendar->GetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mCalendarOptions);
		mCalendar->Hide();
		break;
	default:;
	}
	
	mCurrentView = view;

	// Set data in new panel
	switch(mCurrentView)
	{
	case N3Pane::eView_Mailbox:
		mMessage->SetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mMailOptions, mIs3Pane);
		mMessage->Show();
		break;
	case N3Pane::eView_Contacts:
		mAddress->SetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mAddressOptions, mIs3Pane);
		mAddress->Show();
		break;
	case N3Pane::eView_Calendar:
		mCalendar->SetData(mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[mCurrentView][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mCalendarOptions, mIs3Pane);
		mCalendar->Show();
		break;
	default:;
	}
}

bool CWindowOptionsDialog::PoseDialog()
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_WindowOptionsDialog, CMulberryApp::sApp);
	((CWindowOptionsDialog*) theHandler.GetDialog())->SetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value(), CPreferences::sPrefs->mUse3Pane.GetValue());
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CWindowOptionsDialog*) theHandler.GetDialog())->GetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value());
			CPreferences::sPrefs->Get3PaneOptions().SetDirty();
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
