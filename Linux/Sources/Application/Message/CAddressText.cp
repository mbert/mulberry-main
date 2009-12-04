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


// CAddressText.cpp : implementation file
//

#include "CAddressText.h"

#include "CAddressBookManager.h"
#include "CCommands.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressText

CAddressText::CAddressText(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
		: CTextDisplay(text, enclosure, menu, hSizing,  vSizing, x, y, w, h, editorType)
{
}

CAddressText::CAddressText(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
		: CTextDisplay(enclosure, hSizing,  vSizing, x, y, w, h, editorType)
{
}

CAddressText::CAddressText(JXScrollbarSet* sbs, JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
		: CTextDisplay(sbs, enclosure, hSizing,  vSizing, x, y, w, h, editorType)
{
}

CAddressText::~CAddressText()
{
}

/////////////////////////////////////////////////////////////////////////////
// CAddressText message handlers

bool CAddressText::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eAddressesCaptureAddress:
		CaptureAddress();
		return true;
	default:;
	}

	return CTextDisplay::ObeyCommand(cmd, menu);
}

// Handle menus our way
void CAddressText::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eAddressesCaptureAddress:
		cmdui->Enable(HasSelection());
		return;
	default:;
	}

	CTextDisplay::UpdateCommand(cmd, cmdui);
}

void CAddressText::CaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// If empty, select all
	if (selection.empty())
	{
		StPreserveSelection _selection(this);

		SelectAll();
		GetSelectedText(selection);
	}

	// Do capture if address capability available
	if (!selection.empty() && CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}
