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


// Source for CCalendarStoreWindow class

#include "CCalendarStoreWindow.h"

#include "CCalendarStoreTable.h"
#include "CCalendarStoreView.h"
#include "CCalendarWindow.h"
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"

#include "CCalendarStoreManager.h"

#include "HResourceMap.h"

#include <cassert>

// Static

// Static members

CCalendarStoreWindow*	CCalendarStoreWindow::sCalendarStoreManager = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarStoreWindow::CCalendarStoreWindow(JXDirector* owner)
		: CTableViewWindow(owner)
{
	sCalendarStoreManager = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Default destructor
CCalendarStoreWindow::~CCalendarStoreWindow()
{
	sCalendarStoreManager = NULL;

	// Remove from list
	CWindowsMenu::RemoveWindow(this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create it or bring it to the front
void CCalendarStoreWindow::CreateCalendarStoreWindow()
{
	// Create find & replace window or bring to front
	if (sCalendarStoreManager)
	{
		FRAMEWORK_WINDOW_TO_TOP(sCalendarStoreManager)
	}
	else
	{
		CCalendarStoreWindow* calstoreman = new CCalendarStoreWindow(CMulberryApp::sApp);
  		calstoreman->OnCreate();
		calstoreman->ResetState();
		calstoreman->Show();
		
		// Always display subscribed calendars when creating store window
		if (CCalendarWindow::FindWindow(NULL) == NULL)
		{
			// Create new subscribed window
			CCalendarWindow::MakeWindow(NULL);
			
			// Make sure this window is still focussed
			FRAMEWORK_WINDOW_TO_TOP(calstoreman);
		}
	}
}

void CCalendarStoreWindow::DestroyCalendarStoreWindow()
{
	FRAMEWORK_DELETE_WINDOW(sCalendarStoreManager)
	sCalendarStoreManager = NULL;
}

const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cViewTop = cToolbarTop + cToolbarHeight;
const int cViewHeight = 270;

const int cWindowWidth = 340;
const int cWindowHeight = cViewTop + cViewHeight;

// Do various bits
void CCalendarStoreWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,270, "Calendar Store");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Create server view
	CCalendarStoreView* view = new CCalendarStoreView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cViewTop, cWindowWidth, cViewHeight);
	view->OnCreate();
	SetTableView(view);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(GetCalendarStoreView());
	GetCalendarStoreView()->MakeToolbars(GetToolbarView());
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Do inherited
	CTableViewWindow::OnCreate();	// Do inherited

	CreateMainMenu(window,	CMainMenu::fFile |
							CMainMenu::fEdit | 
							CMainMenu::fAddresses |
							CMainMenu::fCalendar |
							CMainMenu::fWindows |
							CMainMenu::fHelp);
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(cCalendarStoreWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_CALENDARSTOREDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	GetCalendarStoreView()->GetTable()->SetManager(calstore::CCalendarStoreManager::sCalendarStoreManager);
	GetCalendarStoreView()->SetOpen();
}
