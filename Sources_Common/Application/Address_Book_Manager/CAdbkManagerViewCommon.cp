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


// Source for CAdbkManagerView class

#include "CAdbkManagerView.h"

#include "CAdbkManagerWindow.h"
#include "CPreferences.h"
#include "CTableViewWindow.h"
#include "C3PaneWindow.h"

CAdbkManagerWindow* CAdbkManagerView::GetAdbkManagerWindow(void) const
{
	return !Is3Pane() ? static_cast<CAdbkManagerWindow*>(GetOwningWindow()) : NULL;
}

// Return user action data
const CUserAction& CAdbkManagerView::GetPreviewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview);
}

// Return user action data
const CUserAction& CAdbkManagerView::GetFullViewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView);
}

// Check that close is allowed
bool CAdbkManagerView::TestClose()
{
	// Always allowed to close
	return true;
}

// Close it
void CAdbkManagerView::DoClose()
{
	// Set status
	SetClosing();

	// Clear out data and stop listening
	ClearTable();

	// Save state
	SaveDefaultState();

	// Set status
	SetClosed();
}

// Reset the table
void CAdbkManagerView::ResetTable(void)
{
	// reset table contents
	GetTable()->ResetTable();
	
	// Reset zoom size if 1-pane
	if (!Is3Pane())
		GetAdbkManagerWindow()->ResetStandardSize();
}

