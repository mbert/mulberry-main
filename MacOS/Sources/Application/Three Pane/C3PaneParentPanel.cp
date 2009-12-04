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


// Source for C3PaneParentPanel class

#include "C3PaneParentPanel.h"

#include "CBaseView.h"
#include "C3PaneWindow.h"

#include <LIconControl.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneParentPanel::C3PaneParentPanel()
{
	mCurrent = NULL;
}

// Constructor from stream
C3PaneParentPanel::C3PaneParentPanel(LStream *inStream)
		: C3PanePanel(inStream)
{
	mCurrent = NULL;
}

// Default destructor
C3PaneParentPanel::~C3PaneParentPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneParentPanel::FinishCreateSelf()
{
	// Do inherited
	C3PanePanel::FinishCreateSelf();

	// Get sub-panes
	mView = (LView*) FindPaneByID(paneid_3PaneView);
}

bool C3PaneParentPanel::HasFocus() const
{
	return mCurrent ? mCurrent->HasFocus() : false;
}

void C3PaneParentPanel::Focus()
{
	if (mCurrent)
		mCurrent->Focus();
}

void C3PaneParentPanel::MakeToolbars(CBaseView* view)
{
	// Create toolbars for a view we own
	view->MakeToolbars(m3PaneWindow->GetToolbarView());
}
