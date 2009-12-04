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


// CCalendarAddressDisplay.cpp : implementation file
//


#include "CCalendarAddressDisplay.h"

#include "CAddressBookManager.h"
#include "CCommands.h"
#include "CLetterWindow.h"
#include "CMainMenu.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTextListChoice.h"
#include "CTwister.h"

#include "StValueChanger.h"

#include <JXDNDManager.h>

#include <strstream>

bool CCalendarAddressDisplay::sResolve = true;

CCalendarAddressDisplay::CCalendarAddressDisplay(JXContainer* enclosure,
									 const HSizingOption hSizing, 
									 const VSizingOption vSizing,
									 const JCoordinate x, const JCoordinate y,
									 const JCoordinate w, const JCoordinate h) : 
	CTextDisplay(enclosure, hSizing, vSizing, x, y, w, h, kFullEditor)
{
	mTwister = NULL;
	mResolving = false;
}

CCalendarAddressDisplay::~CCalendarAddressDisplay()
{
}


/////////////////////////////////////////////////////////////////////////////
// CCalendarAddressDisplay message handlers

void CCalendarAddressDisplay::OnCreate()
{
	// Do this here to override default context menu
	CreateContextMenu(CMainMenu::eContextLetterAddr);

	// Now do inherited
	CTextDisplay::OnCreate();
}

JBoolean CCalendarAddressDisplay::OKToUnfocus()
{
	if (!mResolving)
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddressList();
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}
	}

	return CTextDisplay::OKToUnfocus();
}

CCalendarAddressList* CCalendarAddressDisplay::GetAddresses()
{
	// Always resolve addresses if still active
	if (!mResolving && HasFocus())
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddressList();
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
	txt.split("\n", tokens);
	for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		list->push_back(new CCalendarAddress(*iter));
	}
	return list;
}

void CCalendarAddressDisplay::ResolveAddressList()
{
	// Resolve addresses
	cdstring str = GetText();
	short text_length = str.length();
	if (text_length && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool expand = !CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Tokenize
		cdstrvect tokens;
		str.split("\n", tokens);
		cdstrvect results;
		for(cdstrvect::const_iterator iter = tokens.begin(); iter != tokens.end(); iter++)
		{
			ResolveExpandAddress(*iter, results, resolution, expand);
		}

		cdstring result;
		result.join(results, "\r");

		// Only change if different (do not change scroll pos)
		bool twist = true;
		if (str != result)
		{
			SetText(result);
			Refresh();
		}
		else
			// Turn off twist change if text is the same as before
			twist = false;

		// If not already twisted (i.e. switching out of address field) force toggle
		if (twist && mTwister && !mTwister->IsChecked())
		{
			sResolve = false;
			mTwister->SetState(kTrue);
			sResolve = true;
		}
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

void CCalendarAddressDisplay::OnExpandAddress()
{
	// Must not resolve during this process
	StValueChanger<bool> change(sResolve, false);

	cdstring expand;

	// Look for selection
	JIndex selStart, selEnd;
	GetSel(selStart, selEnd);
	if (selStart == selEnd)
	{
		cdstring str = GetText();
		expand.assign(str, selStart);
		const char* p1 = ::strrchr(expand.c_str(), ',');
		const char* p2 = ::strrchr(expand.c_str(), '\n');
		if (p1 || p2)
		{
			const char* p = (p1 > p2) ? p1 : p2;
			p++;
			while(*p == ' ') p++;
			SetSel(p - expand.c_str(), selStart);
		}
		else
			SetSel(0, selStart);
	}

	// Only bother of something present
	GetSel(selStart, selEnd);
	unsigned long length = selEnd - selStart;
	if (!length)
		return;

	// Now copy selection
	GetSelectedText(expand);
	expand.trimspace();
	
	// Now insert new item
	cdstrvect results;
	if (ExpandAddressText(expand, results))
	{
		cdstring replace;
		for(cdstrvect::const_iterator iter = results.begin(); iter != results.end(); iter++)
		{
			if (iter != results.begin())
				replace += os_endl;
			replace += *iter;
		}
		InsertUTF8(replace);
	}
		
	// Reset focus back to field
	Focus();
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
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc", NULL, false, false, false, true,
											addrs, temp, selection, 0))
		{
			for(ulvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++)
				results.push_back(addrs.at(*iter));
		}
	}
	else
		::MessageBeep(-1);
	
	// Now return result
	return results.size();
}

bool CCalendarAddressDisplay::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Force resolution
	if (!mResolving && (key == '\r'))
	{
		StValueChanger<bool> _change(mResolving, true);

		ResolveAddressList();
		SetSel(GetTextLength(), GetTextLength());
		InsertUTF8(os_endl);

		// Do twist
		if (mTwister && !mTwister->IsChecked())
		{
			mTwister->SetState(kTrue);
			switch(GetLineCount())
			{
			case 0:
				SetSel(0, 0);
				break;

			case 1:
				break;

			default:
				SetSel(GetLineStart(2), GetLineStart(2));
			}
		}
		
		return true;
	}
	else
		return CTextDisplay::HandleChar(key, modifiers);
}

bool CCalendarAddressDisplay::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eAddressesExpandAddresses:
		OnExpandAddress();
		return true;
	}

	return CTextDisplay::ObeyCommand(cmd, menu);
}

void CCalendarAddressDisplay::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eAddressesExpandAddresses:
		cmdui->Enable(GetTextLength() > 0);
		return;
	case CCommand::eAddressesCaptureAddress:
		cmdui->Enable(true);
		return;
	default:;
	}

	CTextDisplay::UpdateCommand(cmd, cmdui);
}

void CCalendarAddressDisplay::HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList, const Atom action, const Time time, const JXWidget* source)
{
	// Only special of drag from outside
	if (source == this)
		CTextDisplay::HandleDNDDrop(pt, typeList, action, time, source);
	else
	{
		// Get text from DND
		JString16 text;
		JRunArray<Font> style;
		const Atom selectionName = GetDNDManager()->GetDNDSelectionName();
		if (GetSelectionData(selectionName, typeList, time, &text, &style) != kJNoError)
			return;

		// Set selection to end
		SetSel(GetTextLength(), GetTextLength());

		// Check last character in selection and prepend LF if not
		if (!GetText().empty() && (GetText()[GetTextLength() - 1] != '\n'))
			text.Prepend('\n');

		// Must have LF at end of DND'd text if something present before
		if (!GetText().empty() &&
			(text.IsEmpty() || (text.GetLastCharacter() != '\n')))
			text.Append('\n');

		// Add it to the control
		JRunArray<Font>* s = (style.IsEmpty() ? NULL : &style);
		Paste(text, s);
	}
}
