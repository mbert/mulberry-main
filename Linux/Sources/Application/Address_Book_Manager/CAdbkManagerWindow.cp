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


// Source for CAdbkManagerWindow class

#include "CAdbkManagerWindow.h"

#include "CAddressBookManager.h"
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CWindowsMenu.h"

#include "HResourceMap.h"

#include <JXMenuBar.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CAdbkManagerWindow*	CAdbkManagerWindow::sAdbkManager = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerWindow::CAdbkManagerWindow(JXDirector* owner)
	: CTableViewWindow(owner)
{
	sAdbkManager = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}


// Default destructor
CAdbkManagerWindow::~CAdbkManagerWindow()
{
	sAdbkManager = NULL;

	// Remove from list
	CWindowsMenu::RemoveWindow(this);
	
	SaveDefaultState();
}

void CAdbkManagerWindow::CreateAdbkManagerWindow()
{
	// Create find & replace window or bring to front
	if (sAdbkManager)
	{
		FRAMEWORK_WINDOW_TO_TOP(sAdbkManager)
	}
	else
	{
		// Create Adbk manager window
		CAdbkManagerWindow* pWnd = CAdbkManagerWindow::ManualCreate();
	}
}

void CAdbkManagerWindow::DestroyAdbkManagerWindow()
{
	FRAMEWORK_DELETE_WINDOW(sAdbkManager)
	sAdbkManager = NULL;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cViewTop = cToolbarTop + cToolbarHeight;
const int cViewHeight = 270;

const int cWindowWidth = 340;
const int cWindowHeight = cViewTop + cViewHeight;

// Do various bits
void CAdbkManagerWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,270, "Address Book Manager");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Create server view
	mAdbkView = new CAdbkManagerView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cViewTop, cWindowWidth, cViewHeight);
	mAdbkView->OnCreate();
	SetTableView(mAdbkView);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(GetAdbkManagerView());
	GetAdbkManagerView()->MakeToolbars(GetToolbarView());
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Do inherited
	CTableViewWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile |
							CMainMenu::fEdit | 
							CMainMenu::fAddresses |
							CMainMenu::fCalendar | 
							CMainMenu::fWindows |
							CMainMenu::fHelp);
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cAdbkManagerWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_ADDRESSBOOKMANAGERDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	GetAdbkManagerView()->GetTable()->SetManager(CAddressBookManager::sAddressBookManager);
	GetAdbkManagerView()->SetOpen();
}

CAdbkManagerWindow* CAdbkManagerWindow::ManualCreate()
{
	CAdbkManagerWindow * pWnd = new CAdbkManagerWindow(CMulberryApp::sApp);
	if (pWnd)
	{
  		pWnd->OnCreate();
		pWnd->ResetState();
	}
	return pWnd;
}
