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


// CLetterTextEditView.cpp : implementation file
//

#include "CLetterTextEditView.h"

#include "CClipboard.h"
#include "CCommands.h"
#include "CFileTable.h"
#include "CMessage.h"
#include "CLetterWindow.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CURL.h"

#include "StValueChanger.h"

#include <JXDisplay.h>
#include <JXDNDManager.h>

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView

CLetterTextEditView::CLetterTextEditView(JXContainer* enclosure,
											 const HSizingOption hSizing, const VSizingOption vSizing,
											 const JCoordinate x, const JCoordinate y,
											 const JCoordinate w, const JCoordinate h)
	: CEditFormattedTextDisplay(enclosure, hSizing, vSizing, x, y, w, h)
{
	mLtrWindow = NULL;
	mPrintMessage = NULL;
	SetReadOnly(false);
	
	// Turn on spell checking if present
	if (CPluginManager::sPluginManager.HasSpelling())
		SpellAutoCheck(CPluginManager::sPluginManager.GetSpelling()->SpellAsYouType());
}

CLetterTextEditView::~CLetterTextEditView()
{
	delete mPrintMessage;
}

/////////////////////////////////////////////////////////////////////////////
// CLetterTextEditView message handlers

#if NOTYET
void CLetterTextEditView::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == this) && message.Is(JTextEditor::kTextChanged))
		mStyleToolbar.OnSelChange();

	// Pass up
	CEditFormattedTextDisplay::Receive(sender, message);
}
#endif

void CLetterTextEditView::OnCreate()
{
	// Do this here to override default context menu
	CreateContextMenu(CMainMenu::eContextLetterText);

	CEditFormattedTextDisplay::OnCreate();

	// Drag and drop
	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAtchList);
	AddDropFlavor(CMulberryApp::sFlavorMsgAtchList);
}

bool CLetterTextEditView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eEditPaste:
		OnEditPaste();
		return true;

	default:;
	}

	return CEditFormattedTextDisplay::ObeyCommand(cmd, menu);
}

void CLetterTextEditView::OnEditPaste()
{
	cdstring txt;
	CClipboard::GetSecondaryClipboard(GetDisplay(), txt);
	bool did_replace = false;
	if (txt.length() && CPreferences::sPrefs->mSmartURLPaste.GetValue())
	{
		const char* p = txt.c_str();
		unsigned long hlen = ::strlen(p);
		unsigned long plen = hlen;

		// Look for URL scheme at start
		unsigned long scheme_len = 0;
		for(cdstrvect::const_iterator iter = CPreferences::sPrefs->mRecognizeURLs.GetValue().begin();
			(scheme_len == 0) && (iter != CPreferences::sPrefs->mRecognizeURLs.GetValue().end()); iter++)
		{
			size_t iter_len = (*iter).length();

			// Look for URL scheme prefix of URL:scheme prefix
			if ((plen > iter_len) &&
				::strncmpnocase(p, (*iter).c_str(), iter_len) == 0)
				scheme_len = iter_len;
			else if ((plen > iter_len + cURLMainSchemeLength) &&
						(::strncmpnocase(p, cURLMainScheme, cURLMainSchemeLength) == 0) &&
					 	(::strncmpnocase(p + cURLMainSchemeLength, (*iter).c_str(), iter_len) == 0))
				scheme_len = cURLMainSchemeLength + iter_len;
		}

		// Check whether a scheme was found
		if (scheme_len != 0)
		{
			// Look for all text contain valid URL characters
			plen -= scheme_len;
			p += scheme_len;
			while(plen-- && (scheme_len != 0))
			{
				// Look for valid URL character
				if (cURLXCharacter[*(unsigned char*)p++] == 0)
					// Set scheme_len to zero to indicate failure
					scheme_len = 0;
			}
			
			// Now create a new string with delimiters
			if (scheme_len != 0)
			{
				// Create new delimited URL
				cdstring new_url = "<";
				new_url += txt;
				new_url += ">";
							
				// Now copy to scrap
				CClipboard::CopyToSecondaryClipboard(GetDisplay(), new_url);
				
				did_replace = true;
			}
		}
		
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Paste();

		// Restore old text on the clipboard
		if (did_replace)
			CClipboard::CopyToSecondaryClipboard(GetDisplay(), txt);
	}
}

// Exact copy of JXTEBase code except that it uses clipboard that this class determines
JBoolean CLetterTextEditView::TEGetExternalClipboard(JString16* text, JRunArray<Font>* style) const
{
	JBoolean result = CEditFormattedTextDisplay::TEGetExternalClipboard(text, style);
	
	cdustring utxt(text->GetCString());
	cdstring txt(utxt.ToUTF8());
	if (txt.length() && CPreferences::sPrefs->mSmartURLPaste.GetValue())
	{
		const char* p = txt.c_str();
		unsigned long hlen = ::strlen(p);
		unsigned long plen = hlen;

		// Look for URL scheme at start
		unsigned long scheme_len = 0;
		for(cdstrvect::const_iterator iter = CPreferences::sPrefs->mRecognizeURLs.GetValue().begin();
			(scheme_len == 0) && (iter != CPreferences::sPrefs->mRecognizeURLs.GetValue().end()); iter++)
		{
			size_t iter_len = (*iter).length();

			// Look for URL scheme prefix of URL:scheme prefix
			if ((plen > iter_len) &&
				::strncmpnocase(p, (*iter).c_str(), iter_len) == 0)
				scheme_len = iter_len;
			else if ((plen > iter_len + cURLMainSchemeLength) &&
						(::strncmpnocase(p, cURLMainScheme, cURLMainSchemeLength) == 0) &&
					 	(::strncmpnocase(p + cURLMainSchemeLength, (*iter).c_str(), iter_len) == 0))
				scheme_len = cURLMainSchemeLength + iter_len;
		}

		// Check whether a scheme was found
		if (scheme_len != 0)
		{
			// Look for all text contain valid URL characters
			plen -= scheme_len;
			p += scheme_len;
			while(plen-- && (scheme_len != 0))
			{
				// Look for valid URL character
				if (cURLXCharacter[*(unsigned char*)p++] == 0)
					// Set scheme_len to zero to indicate failure
					scheme_len = 0;
			}
			
			// Now create a new string with delimiters
			if (scheme_len != 0)
			{
				// Create new delimited URL
				cdustring new_url = '<';
				new_url += cdustring(txt);
				new_url += '>';
							
				// Now copy to scrap
				*text = new_url.c_str();
			}
		}
	}
	
	return result;
}

#pragma mark ____________________________________Command Updaters

// Common updaters
CMessage* CLetterTextEditView::GetMessage() const
{
	// May need to generate message for current draft
	if (!mPrintMessage)
		mPrintMessage = mLtrWindow->CreateMessage(false);

	return mPrintMessage;
}

void CLetterTextEditView::CleanMessage()
{
	// Delete the message we created
	delete mPrintMessage;
	mPrintMessage = NULL;
}

bool CLetterTextEditView::ChangePrintFont() const
{
	return mLtrWindow->mCurrentPart->GetContent().GetContentSubtype() == eContentSubPlain;
}

// Temporarily add header summary for printing
void CLetterTextEditView::AddPrintSummary()
{
	// Check for summary headers
	mLtrWindow->AddPrintSummary(GetMessage());
}

// Remove temp header summary after printing
void CLetterTextEditView::RemovePrintSummary()
{
	// Check for summary headers
	mLtrWindow->RemovePrintSummary(GetMessage());
}

bool CLetterTextEditView::MessagePrint() const
{
	return false;
}

JBoolean CLetterTextEditView::TEXWillAcceptDrop
	(
	const JArray<Atom>&	typeList,
	const Atom			action,
	const Time			time,
	const JXWidget*		source
	)
{
	// Only drop into attachments when doing D&D move oepration
	JXDNDManager* dndMgr = GetDNDManager();
	Atom move = dndMgr->GetDNDActionMoveXAtom();
	if (action == move)
	{
		std::auto_ptr<JOrderedSetIterator<Atom> > ti(typeList.NewIterator());
		Atom i;
		while (ti->Next(&i))
		{
			CFlavorsList::const_iterator found = std::find(mDropFlavors.begin(), mDropFlavors.end(), i);
			if (found != mDropFlavors.end()) {
				return kTrue;
			}
		}
	}
	return kFalse;
}

JBoolean CLetterTextEditView::TEXConvertDropData
	(
	const JArray<Atom>&	typeList,
	const Atom			action,
	const Time			time,
	JString16*			text,
	JRunArray<Font>*	style
	)
{
	Atom best = 0;

	// Only drop into attachments when doing D&D move oepration
	JXDNDManager* dndMgr = GetDNDManager();
	Atom move = dndMgr->GetDNDActionMoveXAtom();
	if (action != move)
		return kFalse;

	// Check against mDropFlavors in order
	for(CFlavorsList::const_iterator iter = mDropFlavors.begin(); iter != mDropFlavors.end(); iter++)
	{
		for(JIndex index = 1; index <= typeList.GetElementCount(); index++)
		{
			if (typeList.GetElement(index) == *iter)
			{
				best = *iter;
				break;
			}
		}
	}
	
	if (best == 0)
		return kFalse;

	Atom typeReturned;
	unsigned char* dropData;
	JSize dataLen;
	JXSelectionManager::DeleteMethod delMethod;
	GetDisplay()->GetSelectionManager()->
		GetData(GetDNDManager()->GetDNDSelectionName(),
						time,
						best,
						&typeReturned,
						&dropData,
						&dataLen,
						&delMethod);

	try
	{
		mLtrWindow->mPartsTable->DropData(typeReturned, dropData, dataLen);
		mLtrWindow->mPartsTable->ExposePartsList();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	return kFalse;
}
