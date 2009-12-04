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


// Source for CMailboxInfoWindow class

#include "CMailboxInfoWindow.h"

#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"

#include "HResourceMap.h"

#include <JXDisplay.h>
#include <JXMenuBar.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoWindow::CMailboxInfoWindow(JXDirector* owner)
	: CMailboxWindow(owner)
{
}


// Default destructor
CMailboxInfoWindow::~CMailboxInfoWindow()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

CMailboxInfoWindow* CMailboxInfoWindow::ManualCreate()
{
	CMailboxInfoWindow* pWnd = new CMailboxInfoWindow(CMulberryApp::sApp);
	if (pWnd)
		pWnd->OnCreate();
	return pWnd;
}

const int cWindowWidth = 500;
const int cWindowHeight = 200;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cSplitterTop = cToolbarTop + cToolbarHeight;
const int cSplitterHeight = cWindowHeight - cSplitterTop;

// Setup help balloons
void CMailboxInfoWindow::OnCreate()
{
	// Do this outside the layout so that a compile time error
	// occurs if we forget to change the class after regenerating the layout
    //CMailboxWnd* window = new CMailboxWnd(this, 500,200, "");

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
	mMailboxInfoView = new CMailboxInfoView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
	mMailboxInfoView->OnCreate();
	SetTableView(mMailboxInfoView);

	mMessageView = new CMessageView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 550, 200);
	mMessageView->OnCreate();
	GetMailboxView()->SetPreview(mMessageView);
	mMessageView->SetOwnerView(GetMailboxView());

	// Install the splitter items
	mSplitterView->InstallViews(GetMailboxInfoView(), mMessageView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(mSplitterView);
	GetMailboxInfoView()->MakeToolbars(mToolbarView);
	GetMailboxInfoView()->GetPreview()->MakeToolbars(mToolbarView);
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
	GetWindow()->SetWMClass(cMailboxWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_MAILBOXDOCSDI, GetWindow(), 32, 0x00CCCCCC));
}

// Possibly do expunge
JBoolean CMailboxInfoWindow::Close()
{
	if (GetMailboxInfoView()->IsOpen())
	{
		// Look for dangling messages then process close actions
		if (GetMailboxInfoView()->TestClose())
			// Close the view  - this will close the actual window at idle time
			GetMailboxInfoView()->DoClose();
		return kFalse;
	}
	else
		// Close the window in the normal way
		return CMailboxWindow::Close();
		
}
