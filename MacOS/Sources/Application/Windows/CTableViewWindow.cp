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
#include "CHierarchyTableDrag.h"
#include "CMulberryCommon.h"
#include "CTableDrag.h"
#include "CTitleTable.h"
#include "CToolbarView.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

Point CTableViewWindow::sMouseUp = {0, 0};

// Default constructor
CTableViewWindow::CTableViewWindow() :
	CHelpTagWindow(this)
{
	InitTableViewWindow();
}

// Constructor from stream
CTableViewWindow::CTableViewWindow(LStream *inStream) :
	LWindow(inStream),
	CHelpTagWindow(this)
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
}

// Setup help balloons
void CTableViewWindow::FinishCreateSelf()
{
	// Do inherited
	LWindow::FinishCreateSelf();
	SetupHelpTags();

	// Info
	mTableView = (CTableView*) FindPaneByID(paneid_TableView);
}

// Respond to commands
Boolean CTableViewWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_SetDefaultSize:
		SaveDefaultState();
		break;

	case cmd_ResetDefaultWindow:
		ResetState(true);
		break;

	default:
		cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CTableViewWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_SetDefaultSize:
	case cmd_ResetDefaultWindow:
	case cmd_Toolbar:
		// Always enabled
		outEnabled = true;
		break;

	default:
		LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

void CTableViewWindow::ClickInContent(const EventRecord &inMacEvent)
{
									// Enabled Windows respond to clicks
	Boolean		respondToClick = HasAttribute(windAttr_Enabled);

									// Set up our extended event record
	SMouseDownEvent		theMouseDown;
	theMouseDown.wherePort	 = inMacEvent.where;
	GlobalToPortPoint(theMouseDown.wherePort);
	theMouseDown.whereLocal	 = theMouseDown.wherePort;
	theMouseDown.macEvent	 = inMacEvent;
	theMouseDown.delaySelect = false;

	if (!UDesktop::WindowIsSelected(this)) {
									// Window is not in front, we might
									//   need to select it
		Boolean	doSelect = true;
		if (HasAttribute(windAttr_DelaySelect))
		{
			// Reset mouse up
			sMouseUp.h = 0;
			sMouseUp.v = 0;

									// Delay selection until after handling
									//   the click (called click-through)
			theMouseDown.delaySelect = true;
			Click(theMouseDown);

									// After click-through, we select the
									//   Window if the mouse is still down
									//   or the mouse up occurred inside
									//   this Window.
			if (!::StillDown() && (sMouseUp.h || sMouseUp.v))
			{
									// Check location of mouse up event
				WindowPtr	upWindow;
				::MacFindWindow(sMouseUp, &upWindow);
				doSelect = (upWindow == mMacWindowP);
			}
		}

		if (doSelect) {				// Selecting a Window brings it to the
									//   front of its layer and activates it
			Select();
			respondToClick = HasAttribute(windAttr_GetSelectClick);
		}
	}

	if (respondToClick) {
		if (!theMouseDown.delaySelect) {
			Click(theMouseDown);
		} else {
			sLastPaneClicked = nil;
		}
	}
}

Boolean CTableViewWindow::CalcStandardBounds(Rect &outStdBounds) const
{
	// Redo size calculation
	const_cast<CTableViewWindow*>(this)->ResetStandardSize();
	
	return LWindow::CalcStandardBounds(outStdBounds);
}

// Take column width into account
void CTableViewWindow::ResetStandardSize()
{
	TableIndexT	row_num;
	TableIndexT	col_num;
	SDimension16	std_size = {0, 0};

	GetBaseTable()->GetTableSize(row_num, col_num);

	// Use the table's image width and height and vertical position
	SDimension32 tableSize;
	SPoint32 tablePos;
	GetBaseTable()->GetImageSize(tableSize);
	GetBaseTable()->GetFrameLocation(tablePos);
	std_size.width = tableSize.width;
	std_size.height = tableSize.height + tablePos.v;

	// Adjust for vertical scrollbar
	std_size.width += 14;
	if (std_size.width < mMinMaxSize.left)
		std_size.width = mMinMaxSize.left;

	// Adjust for horizontal scroll bar
	std_size.height += 15;
	if (std_size.height < mMinMaxSize.top)
		std_size.height = mMinMaxSize.top;

	SetStandardSize(std_size);

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
