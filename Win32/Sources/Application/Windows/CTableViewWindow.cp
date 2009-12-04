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


// Source for CTableViewWindow class

#include "CTableViewWindow.h"

#include "CTable.h"
#include "CToolbarView.h"

#include "StValueChanger.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

BEGIN_MESSAGE_MAP(CTableViewWindow, CView)
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)
END_MESSAGE_MAP()

// Default constructor
CTableViewWindow::CTableViewWindow()
{
	InitTableViewWindow();
}

// Default destructor
CTableViewWindow::~CTableViewWindow()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CTableViewWindow::InitTableViewWindow()
{
	mTableView = NULL;

	// Preview trigger by selection
	mUserPreviewAction.SetSelection(true);
	
	// Full view triggered by double-click
	mUserFullViewAction.SetDoubleClick(true);
}

// Get child windows
void CTableViewWindow::OnInitialUpdate(void)
{
	// Attach sub-views
	if (GetTableView() && GetTableView()->GetBaseTable())
		GetTableView()->GetBaseTable()->SetFocus();
}

#pragma mark ____________________________Command Updaters

#pragma mark ____________________________Commands

// Resize sub-views
void CTableViewWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its table view to fit
	if (GetTableView())
	{
		CRect rect(0, 0, cx, cy);
		GetTableView()->MoveWindow(rect);
	}
	CView::OnSize(nType, cx, cy);
}

BOOL CTableViewWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route command to current target
	if (CCommander::ProcessCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Tell window to focus on this one
void CTableViewWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to table
	if (GetTableView() && GetTableView()->GetBaseTable())
		GetTableView()->GetBaseTable()->SetFocus();
}

// Reset state from prefs
void CTableViewWindow::ResetState(bool force)
{
	// Pass down to the view object which handles this
	GetTableView()->ResetState(force);
}

// Save current state in prefs
void CTableViewWindow::SaveState()
{
	// Pass down to the view object which handles this
	GetTableView()->SaveState();
}

// Save current state in prefs
void CTableViewWindow::SaveDefaultState()
{
	// Pass down to the view object which handles this
	GetTableView()->SaveDefaultState();
}

// Reset to default state
void CTableViewWindow::OnSaveDefaultState(void)
{
	SaveDefaultState();
}

// Reset to default state
void CTableViewWindow::OnResetDefaultState(void)
{
	ResetState(true);
}
