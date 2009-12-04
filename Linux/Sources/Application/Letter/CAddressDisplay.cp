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


// CAddressDisplay.cpp : implementation file
//


#include "CAddressDisplay.h"

#include "CAddressBookManager.h"
#include "CAddressList.h"
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

bool CAddressDisplay::sResolve = true;

CAddressDisplay::CAddressDisplay(JXContainer* enclosure,
									 const HSizingOption hSizing, 
									 const VSizingOption vSizing,
									 const JCoordinate x, const JCoordinate y,
									 const JCoordinate w, const JCoordinate h) : 
	CAddressText(enclosure, hSizing, vSizing, x, y, w, h, kFullEditor)
{
	mTwister = NULL;
	mResolving = false;
}

CAddressDisplay::~CAddressDisplay()
{
}


/////////////////////////////////////////////////////////////////////////////
// CAddressDisplay message handlers

void CAddressDisplay::OnCreate()
{
	// Do this here to override default context menu
	CreateContextMenu(CMainMenu::eContextLetterAddr);

	// Now do inherited
	CAddressText::OnCreate();
}

JBoolean CAddressDisplay::OKToUnfocus()
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

	return super::OKToUnfocus();
}

CAddressList* CAddressDisplay::GetAddresses(bool qualify)
{
	// Always resolve addresses if still active
	if (!mResolving && HasFocus())
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddressList(qualify);
		}
		catch(...)
		{
			// Do not allow this to interrupt the fosuc change
			CLOG_LOGCATCH(...);
		}
	}

	// Now get text data
	cdstring addrs = GetText();
	return new CAddressList(addrs, addrs.length());
}

void CAddressDisplay::ResolveAddressList(bool qualify)
{
	// Resolve addresses
	cdstring str = GetText();
	short text_length = str.length();
	if (text_length && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool qualify = !CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Generate an address list
		std::auto_ptr<CAddressList> list(new CAddressList(str, text_length, 0, resolution));
		
		if (!resolution || !qualify)
		{
			// Look at each address
			for(CAddressList::iterator iter = list->begin(); iter != list->end(); iter++)
			{
				// See if expand required
				if ((*iter)->GetHost().empty() && !(*iter)->GetMailbox().empty())
				{
					// Get current name (which was the text in the address field).
					// Make sure any quotes are removed so the search will work properly.
					cdstring expand = (*iter)->GetMailbox();
					expand.unquote();

					// Expand address usng search into set of results
					cdstrvect results;
					if (ExpandAddressText(expand, results))
					{
						// Replace existing with the first one in the list
						**iter = CAddress(results.front());
						results.erase(results.begin());

						// Insert the rest
						if (results.size())
						{
							// Use array index as iterators are invalidated by insert
							unsigned long pos = iter - list->begin();
							for(cdstrvect::const_iterator iter2 = results.begin(); iter2 != results.end(); iter2++)
								list->insert(++pos + list->begin(), new CAddress(*iter2));
							iter = list->begin() + pos;
						}
					}
				}
			}
			
		}

		// Qualify remainder
		if (qualify)
			list->QualifyAddresses(CPreferences::sPrefs->mMailDomain.GetValue());
		bool twist = (list->size() > 1);

		// Write address list to stream and grab the string
		std::ostrstream new_txt;
		list->WriteToStream(new_txt);
		new_txt << std::ends;
		cdstring total;
		total.steal(new_txt.str());

		// Only change if different
		if (GetText() != total)
			SetText(total);
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

void CAddressDisplay::OnExpandAddress()
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
bool CAddressDisplay::ExpandAddressText(cdstring& expand, cdstrvect& results)
{
	// Do expansion
	cdstrvect addrs;
	CAddressBookManager::sAddressBookManager->ExpandAddresses(expand, addrs);
	
	// Check for multiple
	expand = cdstring::null_str;
	if (addrs.size() == 1)
		results.push_back(addrs.front());
	else if (addrs.size() > 1)
	{
		// Display list to user
		ulvector selection;
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc", NULL, false, false, false, true,
											addrs, expand, selection, 0))
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

bool CAddressDisplay::HandleChar(const int key, const JXKeyModifiers& modifiers)
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
		return CAddressText::HandleChar(key, modifiers);
}

bool CAddressDisplay::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eAddressesExpandAddresses:
		OnExpandAddress();
		return true;
	}

	return super::ObeyCommand(cmd, menu);
}

void CAddressDisplay::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
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

	super::UpdateCommand(cmd, cmdui);
}

void CAddressDisplay::HandleDNDDrop(const JPoint& pt, const JArray<Atom>& typeList, const Atom action, const Time time, const JXWidget* source)
{
	// Only special of drag from outside
	if (source == this)
		CAddressText::HandleDNDDrop(pt, typeList, action, time, source);
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
