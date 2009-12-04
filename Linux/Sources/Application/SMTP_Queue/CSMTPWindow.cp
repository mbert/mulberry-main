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

#include "CMailboxTitleTable.h"
#include "CMbox.h"
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSMTPTable.h"

#include "HResourceMap.h"

#include <JXMenuBar.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members
CSMTPWindow* CSMTPWindow::sSMTPWindow = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPWindow::CSMTPWindow(JXDirector* owner) :
	CMailboxWindow(owner)
{
	sSMTPWindow = this;
}

// Default destructor
CSMTPWindow::~CSMTPWindow()
{
	GetSMTPView()->DoClose();
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
		CSMTPWindow* pWnd = new CSMTPWindow(CMulberryApp::sApp);
		if (pWnd)
		{
			pWnd->OnCreate();

			// Try activate - note this may have been done if ResetState was called,
			// but ResetState is not called if there are no queuing SMTP accounts
			if (!pWnd->IsActive())
				pWnd->Activate();
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

const int cWindowWidth = 500;
const int cWindowHeight = 200;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cSplitterTop = cToolbarTop + cToolbarHeight;
const int cSplitterHeight = cWindowHeight - cSplitterTop;

// Setup help balloons
void CSMTPWindow::OnCreate(void)
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,200, "");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cSplitterHeight/2);
	heights.InsertElementAtIndex(2, cSplitterHeight/2);
	minHeights.InsertElementAtIndex(1, 16);
	minHeights.InsertElementAtIndex(2, 16);

	mSplitter = mSplitterView = new CSplitterView(heights, 1, minHeights, window,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);

	// Create server view
	mSMTPView = new CSMTPView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
	mSMTPView->OnCreate();
	SetTableView(mSMTPView);

	mMessageView = new CMessageView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 550, 200);
	mMessageView->OnCreate();
	GetMailboxView()->SetPreview(mMessageView);
	mMessageView->SetOwnerView(GetMailboxView());

	// Install the splitter items
	mSplitterView->InstallViews(GetSMTPView(), mMessageView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(mSplitterView);
	GetSMTPView()->MakeToolbars(mToolbarView);
	GetSMTPView()->GetPreview()->MakeToolbars(mToolbarView);
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Do inherited
	CMailboxWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fMailboxes | 
								 CMainMenu::fMessages |
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Set current width and height as minimum
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cSMTPWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_DUMMYDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set first SMTP account
	CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
	for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++)
	{
		// Exit for first enabled
		if (static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
		{
			GetSMTPView()->SetSMTPSender(*iter);
			break;
		}
	}

	// Set status
	GetSMTPView()->SetOpen();
}

void CSMTPWindow::UpdateTitle()
{
	// Set state of enabled button
	bool enable = false;
	enable = CConnectionManager::sConnectionManager.IsConnected() &&
				CSMTPAccountManager::sSMTPAccountManager->GetConnected();

	cdstring title(GetWindow()->GetTitle());
	
	cdstring new_title;
	new_title.FromResource("Alerts::SMTP::Queues");
	if (GetSMTPView()->GetSMTPSender())
	{
		cdstring temp;
		new_title += GetSMTPView()->GetSMTPSender()->GetAccountName();
		new_title += " - ";
		new_title += temp.FromResource(enable ? "Alerts::SMTP::QueuesEnabled" : "Alerts::SMTP::QueuesDisabled");
	}
	
	if (new_title != (const char*) title)
		GetWindow()->SetTitle(new_title);
}
