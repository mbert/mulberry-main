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


// CQuickSearchText.cpp : implementation file
//


#include "CQuickSearchText.h"

#include "CFocusBorder.h"

#include <jASCIIConstants.h>

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText

CQuickSearchText::CQuickSearchText(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CTextInputField(enclosure, hSizing, vSizing, x, y, w, h)
{
}

CQuickSearchText::~CQuickSearchText()
{
}

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText message handlers

bool CQuickSearchText::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// TODO: Add your message handler code here and/or call default

	switch(key)
	{
	case kJReturnKey:
		Broadcast_Message(eBroadcast_Return, this);
		break;
	case kJTabKey:
		Broadcast_Message(eBroadcast_Tab, this);
		break;
	default:
		CTextInputField::HandleChar(key, modifiers);
		Broadcast_Message(eBroadcast_Key, this);
		break;
	}
	
	return true;
}

void CQuickSearchText::HandleFocusEvent()
{
	// Do inherited
	CTextInputField::HandleFocusEvent();
	SetFocus(true);
}

void CQuickSearchText::HandleUnfocusEvent()
{
	// Do inherited
	CTextInputField::HandleUnfocusEvent();
	SetFocus(false);
}

void CQuickSearchText::HandleWindowFocusEvent()
{
	// Do inherited
	CTextInputField::HandleWindowFocusEvent();
	SetFocus(true);
}

void CQuickSearchText::HandleWindowUnfocusEvent()
{
	// Do inherited
	CTextInputField::HandleWindowUnfocusEvent();
	SetFocus(false);
}

void CQuickSearchText::SetFocus(bool focus)
{
	// Focus the edit field
	const CFocusBorder* focusb = dynamic_cast<const CFocusBorder*>(GetEnclosure());
	if (focusb)
		const_cast<CFocusBorder*>(focusb)->SetFocus(focus);

	// If the sorrounding object has one, focus that too
	focusb = dynamic_cast<const CFocusBorder*>(GetEnclosure()->GetEnclosure());
	if (focusb)
		const_cast<CFocusBorder*>(focusb)->SetFocus(focus);
}

