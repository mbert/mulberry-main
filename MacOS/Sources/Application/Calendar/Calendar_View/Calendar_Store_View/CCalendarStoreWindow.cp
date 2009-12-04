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
#include "CCalendarWindow.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"

#include "CCalendarStoreManager.h"

// Static

// Static members

CCalendarStoreWindow*	CCalendarStoreWindow::sCalendarStoreManager = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarStoreWindow::CCalendarStoreWindow(LStream *inStream)
		: CTableViewWindow(inStream)
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
		CCalendarStoreWindow* calstoreman = (CCalendarStoreWindow*) LWindow::CreateWindow(paneid_CalendarStoreWindow, CMulberryApp::sApp);
		calstoreman->ResetState();
		calstoreman->Show();
		
		// Always display subscribed calendars when creating store window
		if (CCalendarWindow::FindWindow(NULL) == NULL)
		{
			// Create new subscribed window
			CCalendarWindow::MakeWindow(NULL);
			
			// Make sure this window is still focussed
			calstoreman->Select();
		}
	}
}

void CCalendarStoreWindow::DestroyCalendarStoreWindow()
{
	FRAMEWORK_DELETE_WINDOW(sCalendarStoreManager)
	sCalendarStoreManager = NULL;
}

// Do various bits
void CCalendarStoreWindow::FinishCreateSelf()
{
	// Do inherited
	CTableViewWindow::FinishCreateSelf();

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_CalendarStoreToolbarView);
	mToolbarView->SetSibling(GetCalendarStoreView());

	// Create toolbars for a view we own
	GetCalendarStoreView()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	SetLatentSub(GetCalendarStoreView()->GetTable());

	// Set status
	GetCalendarStoreView()->GetTable()->SetManager(calstore::CCalendarStoreManager::sCalendarStoreManager);
	GetCalendarStoreView()->SetOpen();
}
