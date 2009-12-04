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

#include "CCommands.h"
#include "CTable.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableViewWindow::CTableViewWindow(JXDirector* owner)
	: CWindow(owner)
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
}

void CTableViewWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eWindowsSaveDefault:
	case CCommand::eWindowsResetToDefault:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;
	default:;
	}

	// Call inherited
	CWindow::UpdateCommand(cmd, cmdui);
}

bool CTableViewWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eWindowsSaveDefault:
		SaveDefaultState();
		return true;
	case CCommand::eWindowsResetToDefault:
		ResetState(true);
		return true;
	default:;
	}

	// Call inherited
	return CWindow::ObeyCommand(cmd, menu);
}

CCommander* CTableViewWindow::GetTarget()
{
	// Route commands through table, which will reroute back to us
	if (GetBaseTable())
		return GetBaseTable();
	else
		return this;
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

void CTableViewWindow::ResetStandardSize()
{
}
