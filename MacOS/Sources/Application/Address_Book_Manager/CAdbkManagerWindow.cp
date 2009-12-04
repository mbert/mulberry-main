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
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CAdbkManagerWindow*	CAdbkManagerWindow::sAdbkManager = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerWindow::CAdbkManagerWindow()
{
	sAdbkManager = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Constructor from stream
CAdbkManagerWindow::CAdbkManagerWindow(LStream *inStream)
		: CTableViewWindow(inStream)
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
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create it or bring it to the front
void CAdbkManagerWindow::CreateAdbkManagerWindow()
{
	// Create find & replace window or bring to front
	if (sAdbkManager)
	{
		FRAMEWORK_WINDOW_TO_TOP(sAdbkManager)
	}
	else
	{
		CAdbkManagerWindow* adbkman = (CAdbkManagerWindow*) LWindow::CreateWindow(paneid_AdbkManagerWindow, CMulberryApp::sApp);
		adbkman->ResetState();
		adbkman->Show();
	}
}

void CAdbkManagerWindow::DestroyAdbkManagerWindow()
{
	FRAMEWORK_DELETE_WINDOW(sAdbkManager)
	sAdbkManager = NULL;
}

// Do various bits
void CAdbkManagerWindow::FinishCreateSelf()
{
	// Do inherited
	CTableViewWindow::FinishCreateSelf();

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_AdbkMgrToolbarView);
	mToolbarView->SetSibling(GetAdbkManagerView());

	// Create toolbars for a view we own
	GetAdbkManagerView()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	SetLatentSub(GetAdbkManagerView()->GetTable());

	// Set status
	GetAdbkManagerView()->GetTable()->SetManager(CAddressBookManager::sAddressBookManager);
	GetAdbkManagerView()->SetOpen();
}
