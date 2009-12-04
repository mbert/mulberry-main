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

#include "CMessageView.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoWindow::CMailboxInfoWindow()
{
}

// Constructor from stream
CMailboxInfoWindow::CMailboxInfoWindow(LStream *inStream)
		: CMailboxWindow(inStream)
{
}

// Default destructor
CMailboxInfoWindow::~CMailboxInfoWindow()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CMailboxInfoWindow::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxWindow::FinishCreateSelf();

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_MailboxToolbarView);
	mToolbarView->SetSibling(mSplitter);

	// Create toolbars for a view we own
	GetMailboxInfoView()->MakeToolbars(mToolbarView);
	GetMailboxInfoView()->GetPreview()->MakeToolbars(mToolbarView);
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
}

// Possibly do expunge
void CMailboxInfoWindow::AttemptClose(void)
{
	// Look for dangling messages then process close actions
	if (GetMailboxInfoView()->TestClose())
	{
		// Close the view  - this will close the actual window at idle time
		GetMailboxInfoView()->DoClose();
		GetPreview()->DoClose();
	}
}

