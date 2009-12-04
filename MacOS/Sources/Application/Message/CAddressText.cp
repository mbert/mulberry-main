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


// Source for CAddressText class

#include "CAddressText.h"

#include "CAddressBookManager.h"
#include "CCommands.h"
#include "CPreferences.h"

// Constructor from stream
CAddressText::CAddressText(LStream *inStream)
		: CTextDisplay(inStream)
{
}

// Default destructor
CAddressText::~CAddressText()
{
}

void CAddressText::FinishCreateSelf()
{
	CTextDisplay::FinishCreateSelf();
	
	const Rect cDefaultMargins = { 0, 1, 0, 1 };  /* t,l,b,r */
    SetMargins(cDefaultMargins);
}

//	Respond to commands
Boolean CAddressText::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_CaptureAddress:
		CaptureAddress();
		break;

	default:
		cmdHandled = CTextDisplay::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAddressText::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_CaptureAddress:
		// Always enabled
		outEnabled = true;
		break;

	// Pass up
	default:
		CTextDisplay::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
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

		SetSelection(0, LONG_MAX);
		GetSelectedText(selection);
	}

	// Do capture if address capability available
	if (!selection.empty() && CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}
