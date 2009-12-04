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


// CSpacebarEdit.cpp : implementation file
//


#include "CSpacebarEdit.h"

#include "CCommands.h"
#include "CMessageWindow.h"
#include "CSpacebarEditView.h"

#include <jASCIIConstants.h>
#include <JXScrollbar.h>
#include <jXKeysym.h>

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEdit

CSpacebarEdit::CSpacebarEdit(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
		: CAddressText(enclosure, hSizing,  vSizing, x, y, w, h)
{
	mMsgWindow = NULL;
}

CSpacebarEdit::~CSpacebarEdit()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEdit message handlers

bool CSpacebarEdit::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch (key)
	{
	case ' ':
	case kJDeleteKey:
	case kJForwardDeleteKey:
		// Pass to text display
		mMsgWindow->GetText()->HandleChar(key, modifiers);
		return true;

	default:
		return CAddressText::HandleChar(key, modifiers);
	}
}

void CSpacebarEdit::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eAddressesCaptureAddress:
		cmdui->Enable(true);
		return;
	default:;
	}

	CAddressText::UpdateCommand(cmd, cmdui);
}

