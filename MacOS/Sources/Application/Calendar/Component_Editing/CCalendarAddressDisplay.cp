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


// Source for CCalendarAddressDisplay class

#include "CCalendarAddressDisplay.h"

#include "CAddressBookManager.h"
#include "CCommands.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CTextListChoice.h"

#include "cdustring.h"

#include <LDisclosureTriangle.h>

#ifndef __TEXTSERVICES__
#include <TextServices.h>
#endif

#include <strstream>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S D I S P L A Y
// __________________________________________________________________________________________________

bool CCalendarAddressDisplay::sResolve = true;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCalendarAddressDisplay::CCalendarAddressDisplay(LStream *inStream)
		: CTextDisplay(inStream)
{
	mTwister = NULL;
	mResolving = false;
}

// Default destructor
CCalendarAddressDisplay::~CCalendarAddressDisplay()
{
}

//	Respond to commands
Boolean CCalendarAddressDisplay::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ExpandAddress:
		ExpandAddress();
		break;

	default:
		cmdHandled = CTextDisplay::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarAddressDisplay::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_ExpandAddress:
		outEnabled = GetTextLength() > 0;
		break;

	default:
		CTextDisplay::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

// Handle keys our way
Boolean CCalendarAddressDisplay::HandleKeyPress(const EventRecord& inKeyEvent)
{
	SInt16 theKey = inKeyEvent.message & charCodeMask;

	// Get key status
	if (!mResolving && !(inKeyEvent.modifiers & cmdKey) && (theKey == char_Return))
	{
		CTextDisplay::HandleKeyPress(inKeyEvent);

		StValueChanger<bool> _change(mResolving, true);

		// Force resolution
		ResolveAddresses();
		StStopRedraw nodraw(this);
		SetSelectionRange(LONG_MAX, LONG_MAX);
		
		// Check for scroll expand
		if (mTwister && !mTwister->GetValue())
			mTwister->SetValue(1);
	}

	return CTextDisplay::HandleKeyPress(inKeyEvent);
}

void CCalendarAddressDisplay::DontBeTarget(void)
{
	// Resolve addresses only if still active
	if (!mResolving && IsActive())
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddresses();
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}
	}

	CTextDisplay::DontBeTarget();
}

CCalendarAddressList* CCalendarAddressDisplay::GetAddresses()
{
	// Always resolve addresses if still active
	if (!mResolving && IsActive())
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddresses();
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}
	}

	// Now get text data
	cdstring txt;
	GetText(txt);
	CCalendarAddressList* list = new CCalendarAddressList;
	cdstrvect tokens;
	txt.split("\r\n", tokens);
	for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		list->push_back(new CCalendarAddress(*iter));
	}
	return list;
}

void CCalendarAddressDisplay::ResolveAddresses()
{
	// Resolve addresses
	cdstring orig_text;
	GetText(orig_text);
	if (!orig_text.empty() && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool expand = CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Tokenize
		cdstrvect tokens;
		orig_text.split("\r\n", tokens);
		cdstrvect results;
		for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
		{
			ResolveExpandAddress(*iter, results, resolution, expand);
		}

		cdstring result;
		result.join(results, "\r");

		// Only change if different (do not change scroll pos)
		bool twist = true;
		if (orig_text != result)
		{
			SetText(result, false);
			Refresh();
		}
		else
			// Turn off twist change if text is the same as before
			twist = false;

		// If not already twisted (i.e. switching out of address field) force toggle
		if (twist && mTwister && (mTwister->GetValue() == 0))
			mTwister->SetValue(1);
	}
}

void CCalendarAddressDisplay::ResolveExpandAddress(const cdstring& text, cdstrvect& results, bool resolve, bool expand)
{
	// Check for valid address first
	CCalendarAddress addr(text);
	if (addr.IsValid())
	{
		results.push_back(text);
		return;
	}
	
	// See whether we need to resolve
	if (resolve)
	{
		CAddressList list;
		if (CAddressBookManager::sAddressBookManager->ResolveNickName(text, &list) ||
			CAddressBookManager::sAddressBookManager->ResolveGroupName(text, &list))
		{
			list.AddCalendarAddressToList(results, true);
			return;
		}
	}
	
	if (expand)
	{
		ExpandAddressText(text, results);
		return;
	}
}

// Expand address
void CCalendarAddressDisplay::ExpandAddress(void)
{
	cdustring txt;
	GetText(txt);

	// Look for selection
	SInt32 selStart;
	SInt32 selEnd;
	GetSelectionRange(selStart, selEnd);
	if (selStart == selEnd)
	{
		cdustring unitxt;
		GetText(unitxt);
		const unichar_t* p1 = ::unistrrchr(unitxt.c_str(), ',');
		const unichar_t* p2 = ::unistrrchr(unitxt.c_str(), '\r');
		if (p1 || p2)
		{
			const unichar_t* p = (p1 > p2) ? p1 : p2;
			p++;
			while(*p == ' ') p++;
			SetSelectionRange(p - unitxt.c_str(), selStart);
		}
		else
			SetSelectionRange(0, selStart);
	}

	// Only bother of something present
	GetSelectionRange(selStart, selEnd);
	unsigned long length = selEnd - selStart;
	if (!length)
		return;

	// Now copy selection
	cdstring expand;
	GetSelectedText(expand);
	expand.trimspace();
	
	// Do expansion
	cdstrvect results;
	if (ExpandAddressText(expand, results))
	{
		// Now insert new items
		for(cdstrvect::const_iterator iter = results.begin(); iter != results.end(); iter++)
		{
			if (iter != results.begin())
				InsertUTF8("\r", 1);
			InsertUTF8(*iter, (*iter).length());
		}
		Refresh();

		// If not already twisted (i.e. switching out of address field) force toggle
		if ((results.size() > 1) && mTwister && (mTwister->GetValue() == 0))
			mTwister->SetValue(1);
	}
}

// Expand address
bool CCalendarAddressDisplay::ExpandAddressText(const cdstring& expand, cdstrvect& results)
{
	// Do expansion
	cdstrvect addrs;
	CAddressBookManager::sAddressBookManager->ExpandCalendarAddresses(expand, addrs);
	
	// Check for multiple
	cdstring temp;
	if (addrs.size() == 1)
		results.push_back(addrs.front());
	else if (addrs.size() > 1)
	{
		// Display list to user
		ulvector selection;
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc",
										NULL, false, false, false, true, addrs, temp, selection, NULL) && selection.size())
		{
			for(ulvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++)
				results.push_back(addrs.at(*iter));
		}
	}
	else
		::SysBeep(1);
	
	// Now return result
	return results.size();
}

// Handle multiple text items
void CCalendarAddressDisplay::DoDragReceive(DragReference inDragRef)
{
	// Do default behaviour when dragging inside of view
	if (!CheckIfViewIsAlsoSender(inDragRef))
	{
		// Text from outside always goes at the end
		SetSelection(GetTextLength(), GetTextLength());
		mLastDragOffset = GetTextLength();
		
		// See if last character is a line end - if not insert one
		if (GetTextLength() && (GetText()[GetTextLength() - 1] != '\r'))
		{
			InsertUTF8("\r");
			mLastDragOffset++;
		}
	}

	// Now do default
	CTextDisplay::DoDragReceive(inDragRef);
}
