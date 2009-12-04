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

#include "CMessagePaneOptions.h"
#include "CAddressPaneOptions.h"
#include "CAdminLock.h"
#include "CCalendarPaneOptions.h"
#include "CMessageView.h"
#include "CPreferences.h"
#include "CTabController.h"
#include "CUserActionOptions.h"
#include "C3PaneWindow.h"

#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CWindowOptionsDialog::CWindowOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

// Default destructor
CWindowOptionsDialog::~CWindowOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CWindowOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 620,440, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 620,440);
    assert( obj1 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 530,405, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 440,405, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mTabs =
        new CTabController(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 600,385);
    assert( mTabs != NULL );

// end JXLayout

	window->SetTitle("Window Options");
	SetButtons(mOKBtn, mCancelBtn);

	mMessage = new CMessagePaneOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 600, 370);
	mTabs->AppendCard(mMessage, "Email");

	mAddress = new CAddressPaneOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 600, 370);
	mTabs->AppendCard(mAddress, "Contacts");

	mCalendar = new CCalendarPaneOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 600, 370);
	// Don't do if admin locks it out
	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mTabs->AppendCard(mCalendar, "Calendars");

	// Set to intial panel
	mTabs->ShowCard(1);
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

	mMessage->SetData(mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mMailOptions, mIs3Pane);

	mAddress->SetData(mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mAddressOptions, mIs3Pane);

	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mCalendar->SetData(mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mCalendarOptions, mIs3Pane);
}

// Get the details
void CWindowOptionsDialog::GetDetails(C3PaneOptions* options)
{
	// Force update of data from panels
	mMessage->GetData(mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mMailOptions);
	mAddress->GetData(mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mAddressOptions);
	mCalendar->GetData(mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mCalendarOptions);
	
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
}

bool CWindowOptionsDialog::PoseDialog()
{
	bool result = false;

	// Create the dialog
	CWindowOptionsDialog* dlog = new CWindowOptionsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value(), CPreferences::sPrefs->mUse3Pane.GetValue());

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value());
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
		result = true;
		dlog->Close();
	}

	return result;
}
