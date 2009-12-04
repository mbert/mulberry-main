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


// Source for CAddressDisplay class

#include "CAddressDisplay.h"

#include "CAddress.h"
#include "CAddressBookManager.h"
#include "CAddressList.h"
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

bool CAddressDisplay::sResolve = true;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressDisplay::CAddressDisplay(LStream *inStream)
		: CAddressText(inStream)
{
	mTwister = NULL;
	mResolving = false;
}

// Default destructor
CAddressDisplay::~CAddressDisplay()
{
}

//	Respond to commands
Boolean CAddressDisplay::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ExpandAddress:
		ExpandAddress();
		break;

	default:
		cmdHandled = CAddressText::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAddressDisplay::FindCommandStatus(
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
		CAddressText::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

// Handle keys our way
Boolean CAddressDisplay::HandleKeyPress(const EventRecord& inKeyEvent)
{
	SInt16 theKey = inKeyEvent.message & charCodeMask;

	// Get key status
	if (!mResolving && !(inKeyEvent.modifiers & cmdKey) && (theKey == char_Return))
	{
		CAddressText::HandleKeyPress(inKeyEvent);

		StValueChanger<bool> _change(mResolving, true);

		// Force resolution
		ResolveAddresses();
		StStopRedraw nodraw(this);
		SetSelectionRange(LONG_MAX, LONG_MAX);
		
		// Check for scroll expand
		if (mTwister && !mTwister->GetValue())
			mTwister->SetValue(1);
	}

	return CAddressText::HandleKeyPress(inKeyEvent);
}

void CAddressDisplay::DontBeTarget(void)
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

	CAddressText::DontBeTarget();
}

CAddressList* CAddressDisplay::GetAddresses(bool qualify)
{
	// Always resolve addresses if still active
	if (!mResolving && IsActive())
	{
		try
		{
			StValueChanger<bool> _change(mResolving, true);
			ResolveAddresses(qualify);
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
	return new CAddressList(txt, txt.length());
}

void CAddressDisplay::ResolveAddresses(bool qualify)
{
	// Resolve addresses
	cdstring orig_text;
	GetText(orig_text);
	if (!orig_text.empty() && sResolve)
	{
		// See if resolution or expand required
		bool resolution = !CPreferences::sPrefs->mExpandNoNicknames.GetValue();
		bool qualify = !CPreferences::sPrefs->mExpandFailedNicknames.GetValue();
		
		// Generate an address list
		std::auto_ptr<CAddressList> list(new CAddressList(orig_text, orig_text.length(), 0, resolution));
		
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

		{
			// Write to a string stream
			std::ostrstream new_txt;
			list->WriteToStream(new_txt);
			new_txt << std::ends;
			cdstring total;
			total.steal(new_txt.str());

			// Only change if different (do not change scroll pos)
			if (orig_text != total)
			{
				SetText(total, false);
				Refresh();
			}
			else
				// Turn off twist change if text is the same as before
				twist = false;
		}

		// If not already twisted (i.e. switching out of address field) force toggle
		if (twist && mTwister && (mTwister->GetValue() == 0))
			mTwister->SetValue(1);
	}
}

// Expand address
void CAddressDisplay::ExpandAddress(void)
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
		if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc",
										NULL, false, false, false, true, addrs, expand, selection, NULL) && selection.size())
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
void CAddressDisplay::DoDragReceive(DragReference inDragRef)
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
	CAddressText::DoDragReceive(inDragRef);
}
