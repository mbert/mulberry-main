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

// CSpellPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 17-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based Spelling plug-in for use in Mulberry.
//
// History:
// CD:	 17-Dec-1997:	Created initial header and implementation.
// CD:	 22-Dec-1997:	Adding UI based spell check.
//

#include "CSpellPlugin.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CBalloonDialog.h"
#endif
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CLocalCommon.h"
#include "CPluginManager.h"
#include "CPreferenceKeys.h"
#include "CPreferenceVersions.h"
#include "CSpellCheckDialog.h"
#include "CSpellChooseDictDialog.h"
#if __dest_os != __linux_os
#include "CSpellEditDialog.h"
#endif
#include "CSpellOptionsDialog.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CTextDisplay.h"
#elif __dest_os == __win32_os
#include "CSDIFrame.h"
#include "CCmdEditView.h"
#elif __dest_os == __linux_os
#include "CTextDisplay.h"
#else
#error __dest_os
#endif
#include "char_stream.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <UStandardDialogs.h>

#endif

#if __dest_os == __linux_os
#include <jXGlobals.h>
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const unsigned char* cDefaultDictionaryName = "\pDictionary";
#elif __dest_os == __win32_os
const char cDefaultDictionaryName[] = "Dictionary.dic";
const char cPluginDirName[] = "Plug-ins";
#elif __dest_os == __linux_os
const char cDefaultDictionaryName[] = "Dictionary.dic";
#else
#error __dest_os
#endif
const char* cDictFileExtension = ".dic";

const char* cMainDictionaryDir = "Main Dictionaries";
const char* cUserDictionaryDir = "Dictionaries";

#pragma mark ____________________________CSpellPreferences

CSpellPreferences::CSpellPreferences()
{
	mIgnoreCapitalised.Value() = false;
	mIgnoreAllCaps.Value() = false;
	mIgnoreWordsNumbers.Value() = false;
	mIgnoreMixedCase.Value() = false;
	mIgnoreDomainNames.Value() = true;
	mReportDoubledWords.Value() = true;
	mCaseSensitive.Value() = true;
	mPhoneticSuggestions.Value() = true;
	mTypoSuggestions.Value() = true;
	mSuggestSplit.Value() = false;
	mAutoCorrect.Value() = true;
	mSuggestLevel.Value() = CSpellPreferencesDLL::eSuggest_Medium;
	
	mAutoPositionDialog.Value() = true;
	mSpellOnSend.Value() = false;
	mSpellAsYouType.Value() = true;
	mSpellColourBackground.Value() = true;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mSpellBackgroundColour.Value().red = 0xFFFF;
	mSpellBackgroundColour.Value().green = 0x9999;
	mSpellBackgroundColour.Value().blue = 0x9999;
#elif __dest_os == __win32_os
	mSpellBackgroundColour.Value() = RGB(0xFF, 0x99, 0x99);
#elif __dest_os == __linux_os
	mSpellBackgroundColour.Value() = JRGB(0xFFFF, 0x9999, 0x9999);
#endif
}

CSpellPreferences::~CSpellPreferences()
{
}

CSpellPreferencesDLL& CSpellPreferencesDLL::operator=(CSpellPreferences& copy)
{
	mIgnoreCapitalised = copy.mIgnoreCapitalised.GetValue();
	mIgnoreAllCaps = copy.mIgnoreAllCaps.GetValue();
	mIgnoreWordsNumbers = copy.mIgnoreWordsNumbers.GetValue();
	mIgnoreMixedCase = copy.mIgnoreMixedCase.GetValue();
	mIgnoreDomainNames = copy.mIgnoreDomainNames.GetValue();
	mReportDoubledWords = copy.mReportDoubledWords.GetValue();
	mCaseSensitive = copy.mCaseSensitive.GetValue();
	mPhoneticSuggestions = copy.mPhoneticSuggestions.GetValue();
	mTypoSuggestions = copy.mTypoSuggestions.GetValue();
	mSuggestSplit = copy.mSuggestSplit.GetValue();
	mAutoCorrect = copy.mAutoCorrect.GetValue();
	::strncpy(mDictionaryName, copy.mDictionaryName.GetValue(), 256);
	mDictionaryName[255] = 0;

	return *this;
}

#pragma mark ____________________________CSpellPlugin

// Constructor
CSpellPlugin::CSpellPlugin(fspec files)
	: CPlugin(files)
{
	mUsePreferences = true;			// Force on to use prefs
	mInitCount = 0;
	mAutoLoaded = false;
}

CSpellPlugin::~CSpellPlugin()
{
	// Terminate plugin if it was permantently loaded
	if (mAutoLoaded)
	{
		try
		{
			SpTerminate();
			UnloadPlugin();
			mAutoLoaded = false;
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}

void CSpellPlugin::AutoLoad()
{
	if (!mAutoLoaded)
	{
		LoadPlugin();
		try
		{
			SpInitialise();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			UnloadPlugin();

			CLOG_LOGRETHROW;
			throw;
		}
		mAutoLoaded = true;
	}
}

int CSpellPlugin::SpInitialise()
{
	// Reference count inits and only do plugin call once
	if (mInitCount++ > 0)
		return 0;

	int result = CallPlugin(eSpellInitialise, NULL);
	if (!result)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	// No dictionary support for ispell right now
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Get path for main dictionaries
	if (CPluginManager::sPluginManager.GetPluginDirs().empty())
		return 0;

	cdstring dir = CPluginManager::sPluginManager.GetPluginDirs().front();
	::addtopath(dir, cMainDictionaryDir);

	// Set the path
	SetMainDictionaryPath(dir);

	// Get and set path for user dictionaries
	dir = CConnectionManager::sConnectionManager.GetCWD();
	::addtopath(dir, cUserDictionaryDir);

	// Check that directory exists (create if not)
	try
	{
		::chkdir(dir);

		// Set the path
		SetUserDictionaryPath(dir);
	}
	catch(...)
	{
	}
#endif

	// Check for dictionary
	CheckDictionary();

	// Make sure proper prefs are loaded
	UpdatePreferences();

	return result;
}

int CSpellPlugin::SpTerminate()
{
	// Reference count terminates and only do plugin call once
	if ((mInitCount != 0) && (--mInitCount == 0))
		return CallPlugin(eSpellTerminate, NULL);
	else
		return 0;
}

void CSpellPlugin::UpdatePreferences()
{
	// Only bother if loaded
	if (mLoaded)
	{
		CSpellPreferencesDLL* prefs;
		CallPlugin(eSpellGetPrefs, &prefs);
		*prefs = mPreferences;
		CallPlugin(eSpellUpdatePrefs, NULL);

		SetDictionary(mPreferences.mDictionaryName.GetValue());
	}
}

bool CSpellPlugin::SpellCheck(CWindowType* wnd, CTextDisplay* text, bool sending, bool hilite_only)
{
	bool inited = false;
	bool result = true;

	try
	{
		// Init spell check module
		LoadPlugin();
		SpInitialise();
		inited = true;

		// Get selected text or all (always get all when sending)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		long selStart;
		long selEnd;
		text->GetSelectionRange(selStart, selEnd);

		cdustring theTxtUTF16;
		if (!sending && (selStart != selEnd))
			text->GetSelectedText(theTxtUTF16);
		else
		{
			selStart = 0;
			text->GetText(theTxtUTF16);
		}
		cdstring theTxt = theTxtUTF16.ToUTF8();
#elif __dest_os == __win32_os
		long selStart;
		long selEnd;
		text->GetSelectionRange(selStart, selEnd);

		cdustring theTxtUTF16;
		if (!sending && (selStart != selEnd))
			text->GetSelectedText(theTxtUTF16);
		else
		{
			selStart = 0;
			text->GetText(theTxtUTF16);
		}

		cdstring theTxt = theTxtUTF16.ToUTF8();
		
		// RichEdit 2.0 CRLF -> LF fix
		theTxt.ConvertEndl(eEndl_LF);
#elif __dest_os == __linux_os
		JIndex selStart;
		JIndex selEnd;
		text->GetSel(selStart, selEnd);

		cdstring theTxt;
		if (!sending && (selStart != selEnd))
			text->GetSelectedText(theTxt);
		else
		{
			selStart = 0;
			theTxt = text->GetText();
		}
#else
#error __dest_os
#endif

		CheckText(theTxt);

		bool show_completion = !sending;

		if (HasErrors())
		{
			if (hilite_only)
			{
				HiliteText(text, selStart, theTxt);
			}
			else
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				// Create the dialog
				CBalloonDialog	theHandler(paneid_SpellCheckDialog, text);
				((CSpellCheckDialog*) theHandler.GetDialog())->SetSpeller(this, wnd, text, selStart, &theTxt, sending);

				// Let DialogHandler process events
				while (!((CSpellCheckDialog*) theHandler.GetDialog())->IsDone())
				{
					MessageT hitMessage = theHandler.DoDialog();

					if (hitMessage == msg_OK)
						break;
					else if (hitMessage == msg_Cancel)
					{
						result = false;
						show_completion = false;
						break;
					}
					else if (hitMessage == msg_SpellCheckSendNow)
					{
						result = true;

						// Don't show completion dialog when asked to send now
						show_completion = false;
						break;
					}
				}
#elif __dest_os == __win32_os
				CSpellCheckDialog dlog(CSDIFrame::GetAppTopWindow());
				dlog.SetSpeller(this, wnd, text, selStart, &theTxt, sending);
				switch(dlog.DoModal())
				{
				case IDCANCEL:
					result = false;
					show_completion = false;
					break;
				case IDC_SPELLCHECK_SENDNOW:
					result = true;

					// Don't show completion dialog when asked to send now
					show_completion = false;
					break;
				default:;
				}
#elif __dest_os == __linux_os
				CSpellCheckDialog* dlog = new CSpellCheckDialog(JXGetApplication());
				dlog->OnCreate();
				dlog->SetSpeller(this, wnd, text, selStart, &theTxt, sending);
				switch(dlog->DoModal(false))
				{
				case CDialogDirector::kDialogClosed_Btn3:
					result = true;
					dlog->Close();
					break;
				case CDialogDirector::kDialogClosed_Cancel:
					result = false;
					break;
				default:
					result = false;
					dlog->Close();
					break;
				}
				if (!result)
					show_completion = false;
#endif
			}
		}

		SpTerminate();
		UnloadPlugin();

		if (show_completion)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			CErrorHandler::PutNoteAlert(STRx_Spell, str_DoneCheck);
#elif __dest_os == __win32_os || __dest_os == __linux_os
			CErrorHandler::PutNoteAlert(IDS_SPELL_DoneCheck);
#else
#error __dest_os
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Terminate if inited
		if (inited)
			SpTerminate();

		// Always unload
		UnloadPlugin();
	}

	return result;
}

void CSpellPlugin::SpellOptions()
{
	bool inited = false;

	try
	{
		// Init spell check module
		LoadPlugin();
		SpInitialise();
		inited = true;

		// Do dialog
		CSpellOptionsDialog::PoseDialog(this);

		SpTerminate();
		UnloadPlugin();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Terminate if inited
		if (inited)
			SpTerminate();

		// Always unload
		UnloadPlugin();
	}
}

void CSpellPlugin::SpellChooseDictionary()
{
	// Do the dialog
	CSpellChooseDictDialog::PoseDialog(this);
}

void CSpellPlugin::SpellEditDictionary()
{
	bool inited = false;

	try
	{
		// Init spell check module
		LoadPlugin();
		SpInitialise();
		inited = true;

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
		// Create the dialog
		CSpellEditDialog::PoseDialog(this);
#elif __dest_os == __linux_os
		// Not implemented for unix
#endif

		SpTerminate();
		UnloadPlugin();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Terminate if inited
		if (inited)
			SpTerminate();

		// Always unload
		UnloadPlugin();
	}
}

void CSpellPlugin::CheckDictionary()
{
	// See if any dictionaries exist
	const char* dicts;
	if (!GetDictionaries(&dicts) || !dicts || !*(const char**)dicts)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CErrorHandler::PutStopAlert(STRx_Spell, str_NoDictionaries);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		CErrorHandler::PutStopAlert(IDS_SPELL_NoMainDictionary);
#else
#error __dest_os
#endif
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	// Check that previously selected dictionary still exists
	if (!mPreferences.mDictionaryName.GetValue().empty())
	{
		const char** name = (const char**)(dicts);
		while(*name)
		{
			// Compare preference with available dicts - done if its found
			if (!::strcmp(mPreferences.mDictionaryName.GetValue(), *name++))
				return;
		}
	}

	// Choose one
	SpellChooseDictionary();
}

int CSpellPlugin::NewDictionary(const char* name)
{
	return CallPlugin(eSpellNewDictionary, (void*) name);
}

int CSpellPlugin::OpenDictionary(const char* name)
{
	return CallPlugin(eSpellOpenDictionary, (void*) name);
}

int CSpellPlugin::CloseDictionary(int dict)
{
	return CallPlugin(eSpellCloseDictionary, (void*) dict);
}

int CSpellPlugin::AddWord(const char* word)
{
	return CallPlugin(eSpellAddWord, (void*) word);
}

const char* CSpellPlugin::GetAddSuffix(int i) const
{
	return (const char*) CallPlugin(eSpellGetAddSuffix, (void*) i);
}

int CSpellPlugin::RemoveWord(const char* word)
{
	return CallPlugin(eSpellRemoveWord, (void*) word);
}

void CSpellPlugin::SetMaxPerPage(int max_per_page)
{
	CallPlugin(eSpellSetMaxPerPage, (void*) max_per_page);
}

int CSpellPlugin::CountPages() const
{
	return CallPlugin(eSpellCountPages, NULL);
}

int CSpellPlugin::CurrentPage() const
{
	return CallPlugin(eSpellCurrentPage, NULL);
}

int CSpellPlugin::SetPage(int page)
{
	return CallPlugin(eSpellSetPage, (void*) page);
}

int CSpellPlugin::GetPage(const char* word, const char** buf, size_t* buflen)
{
	SpGetPageData data;
	data.word = word;
	data.buf = buf;
	data.buf_len = buflen;
	return CallPlugin(eSpellGetPage, (void*) &data);
}

void CSpellPlugin::GetSuggestions(const char* word, long ctr, cdstrvect& suggestions)
{
	SpGetSuggestionsData data;
	data.word = word;
	data.ctr = ctr;
	const char* suggs = (const char*)CallPlugin(eSpellGetSuggestions, (void*) &data);

	// Do for each word
	char* start = (char*) suggs;
	char* end = (char*) suggs;
	while(*end && (*end != lendl1)) end++;
	while(*start)
	{
		cdstring str(start, end - start);
		suggestions.push_back(str);

		if (*end)
		{
			start = ++end;
			while(*end && (*end != lendl1)) end++;
		}
		else
			start = end;
	}
}

void CSpellPlugin::GetQuickSuggestions(const char* word, cdstrvect& suggestions)
{
	GetSuggestions(word, 0, suggestions);
}

bool CSpellPlugin::ContainsWord(const char* word)
{
	return CallPlugin(eSpellContainsWord, (void*) word);
}

int CSpellPlugin::ClosestWord(const char* word)
{
	return CallPlugin(eSpellClosestWord, (void*) word);
}

int CSpellPlugin::CheckText(const char* txt)
{
	return CallPlugin(eSpellCheckText, (void*) txt);
}

void CSpellPlugin::HiliteText(CTextDisplay* text, long start, const cdstring& check)
{
	// Must have text display and text data
	if (!text)
		return;

	// Clear all spell error hiliting in the range being checked
	text->MarkMisspelled(start, start + check.length(), false);

	for(const SpError* sperr = NextError(check.c_str());  sperr != NULL; sperr = NextError(check.c_str()))
	{
		// Do red-wave hilite of text
		text->MarkMisspelled(start + sperr->sel_start, start + sperr->sel_end, true);

		// Skip current error
		Skip();
	}

	// Have to do a redraw here
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	text->Refresh();
#elif __dest_os == __win32_os
	text->RedrawWindow();
#elif __dest_os == __linux_os
#endif
}

bool CSpellPlugin::HasErrors() const
{
	return (CallPlugin(eSpellHasErrors, NULL) == 1);
}

const CSpellPlugin::SpError* CSpellPlugin::CurrentError() const
{
	return (SpError*) CallPlugin(eSpellCurrentError, NULL);
}

const CSpellPlugin::SpError* CSpellPlugin::NextError(const char* txt)
{
	return (SpError*) CallPlugin(eSpellNextError, (void*) txt);
}

bool CSpellPlugin::ErrorIsPunct() const
{
	return (CallPlugin(eSpellErrorIsPunct, NULL) == 1);
}

bool CSpellPlugin::ErrorIsDoubleWord() const
{
	return (CallPlugin(eSpellErrorIsDoubleWord, NULL) == 1);
}

// Skip current error
int CSpellPlugin::Skip()
{
	return CallPlugin(eSpellSkip, NULL);
}

// Skip all current error
int CSpellPlugin::SkipAll()
{
	return CallPlugin(eSpellSkipAll, NULL);
}

// Skip current error
int CSpellPlugin::Replace()
{
	return CallPlugin(eSpellReplace, NULL);
}

// Skip all current error
int CSpellPlugin::ReplaceAll()
{
	return CallPlugin(eSpellReplaceAll, NULL);
}

// Process capitalisation etc
void CSpellPlugin::ProcessReplaceString(char* replace) const
{
	CallPlugin(eSpellProcessReplaceString, (void*) replace);
}

#pragma mark ____________________________Spellex

bool CSpellPlugin::CannotDisplayDictionary()
{
	return CallPlugin(eSpellCannotDisplayDictionary, NULL);
}

void CSpellPlugin::SetMainDictionaryPath(const char* path) const
{
	CallPlugin(eSpellSetMainDictionaryPath, (void*) path);
}

void CSpellPlugin::SetUserDictionaryPath(const char* path) const
{
	CallPlugin(eSpellSetUserDictionaryPath, (void*) path);
}

int CSpellPlugin::GetDictionaries(const char** dicts) const
{
	return CallPlugin(eSpellGetDictionaries, (void*) dicts);
}

int CSpellPlugin::SetDictionary(const char* name) const
{
	return CallPlugin(eSpellSetDictionary, (void*) name);
}

int CSpellPlugin::SetDictionaryByLanguage(const char* isolangtag) const
{
	return CallPlugin(eSpellSetDictionaryByLanguage, (void*) isolangtag);
}

bool CSpellPlugin::MoreSuggestions(long ctr) const
{
	return CallPlugin(eSpellMoreSuggestions, &ctr);
}

const CSpellPlugin::SpError* CSpellPlugin::CheckWord(const char* word)
{
	return (SpError*) CallPlugin(eSpellCheckWord, (void*) word);
}

#pragma mark ____________________________Preferences

// Write data to a stream
void CSpellPlugin::DoWriteToMap(COptionsMap* theMap, bool dirty_only)
{
	// Write all to map
	mPreferences.mIgnoreCapitalised.WriteToMap(cIgnoreCapitalisedKey, theMap, dirty_only);
	mPreferences.mIgnoreAllCaps.WriteToMap(cIgnoreAllCapsKey, theMap, dirty_only);
	mPreferences.mIgnoreWordsNumbers.WriteToMap(cIgnoreWordsNumbersKey, theMap, dirty_only);
	mPreferences.mIgnoreMixedCase.WriteToMap(cIgnoreMixedCaseKey, theMap, dirty_only);
	mPreferences.mIgnoreDomainNames.WriteToMap(cIgnoreDomainNamesKey, theMap, dirty_only);
	mPreferences.mReportDoubledWords.WriteToMap(cReportDoubledWordsKey, theMap, dirty_only);
	mPreferences.mCaseSensitive.WriteToMap(cCaseSensitiveKey, theMap, dirty_only);
	mPreferences.mPhoneticSuggestions.WriteToMap(cPhoneticSuggestionsKey, theMap, dirty_only);
	mPreferences.mTypoSuggestions.WriteToMap(cTypoSuggestionsKey, theMap, dirty_only);
	mPreferences.mSuggestSplit.WriteToMap(cSuggestSplitKey, theMap, dirty_only);
	mPreferences.mAutoCorrect.WriteToMap(cAutoCorrectKey, theMap, dirty_only);
	mPreferences.mSuggestLevel.WriteToMap(cSuggestLevelKey, theMap, dirty_only);
	cdstring dictkey(cDictionaryNameKey);
#if __dest_os == __linux_os
	// Store dictionary names on a per-plugin basis
	dictkey += " ";
	dictkey += GetName();
#endif
	mPreferences.mDictionaryName.WriteToMap(dictkey, theMap, dirty_only);

	mPreferences.mAutoPositionDialog.WriteToMap(cAutoPositionDialogKey, theMap, dirty_only);
	mPreferences.mSpellOnSend.WriteToMap(cSpellOnSendKey, theMap, dirty_only);
	mPreferences.mSpellAsYouType.WriteToMap(cSpellAsYouTypeKey, theMap, dirty_only);
	mPreferences.mSpellColourBackground.WriteToMap(cSpellColourBackground, theMap, dirty_only);
	mPreferences.mSpellBackgroundColour.WriteToMap(cSpellBackgroundColour, theMap, dirty_only);

	// Must create window state if not already done
	if (!CSpellCheckDialog::sWindowState)
		CSpellCheckDialog::sWindowState = new CPreferenceValue<CWindowState>;
	if (!dirty_only || CSpellCheckDialog::sWindowState->IsDirty())
		theMap->WriteValue(cPositionDialogKey, CSpellCheckDialog::sWindowState->GetValue().GetInfo());
}

// Read data from a stream
void CSpellPlugin::DoReadFromMap(COptionsMap* theMap, NumVersion vers_prefs, NumVersion vers_plugin)
{
	// Read all to map
	mPreferences.mIgnoreCapitalised.ReadFromMap(cIgnoreCapitalisedKey, theMap, vers_prefs);
	mPreferences.mIgnoreAllCaps.ReadFromMap(cIgnoreAllCapsKey, theMap, vers_prefs);
	mPreferences.mIgnoreWordsNumbers.ReadFromMap(cIgnoreWordsNumbersKey, theMap, vers_prefs);
	mPreferences.mIgnoreMixedCase.ReadFromMap(cIgnoreMixedCaseKey, theMap, vers_prefs);
	mPreferences.mIgnoreDomainNames.ReadFromMap(cIgnoreDomainNamesKey, theMap, vers_prefs);
	mPreferences.mReportDoubledWords.ReadFromMap(cReportDoubledWordsKey, theMap, vers_prefs);
	mPreferences.mCaseSensitive.ReadFromMap(cCaseSensitiveKey, theMap, vers_prefs);
	mPreferences.mPhoneticSuggestions.ReadFromMap(cPhoneticSuggestionsKey, theMap, vers_prefs);
	mPreferences.mTypoSuggestions.ReadFromMap(cTypoSuggestionsKey, theMap, vers_prefs);
	mPreferences.mSuggestSplit.ReadFromMap(cSuggestSplitKey, theMap, vers_prefs);
	mPreferences.mAutoCorrect.ReadFromMap(cAutoCorrectKey, theMap, vers_prefs);
	mPreferences.mSuggestLevel.ReadFromMap(cSuggestLevelKey, theMap, vers_prefs);
	cdstring dictkey(cDictionaryNameKey);
#if __dest_os == __linux_os
	// Store dictionary names on a per-plugin basis
	dictkey += " ";
	dictkey += GetName();
#endif
	mPreferences.mDictionaryName.ReadFromMap(dictkey, theMap, vers_prefs);

	mPreferences.mAutoPositionDialog.ReadFromMap(cAutoPositionDialogKey, theMap, vers_prefs);
	mPreferences.mSpellOnSend.ReadFromMap(cSpellOnSendKey, theMap, vers_prefs);
	mPreferences.mSpellAsYouType.ReadFromMap(cSpellAsYouTypeKey, theMap, vers_prefs);
	mPreferences.mSpellColourBackground.ReadFromMap(cSpellColourBackground, theMap, vers_prefs);
	mPreferences.mSpellBackgroundColour.ReadFromMap(cSpellBackgroundColour, theMap, vers_prefs);

	// Needed for conversion
	NumVersionVariant temp_1_4_0;
	temp_1_4_0.whole = VERS_1_4_0;

	cdstring info;
	// Must create window state if not already done
	if (!CSpellCheckDialog::sWindowState)
		CSpellCheckDialog::sWindowState = new CPreferenceValue<CWindowState>;
	if (theMap->ReadValue(cPositionDialogKey_2_0, info, vers_prefs))
	{
		char_stream stream(info.c_str_mod());
		((CWindowState&) CSpellCheckDialog::sWindowState->GetValue()).SetInfo(stream, vers_prefs);
	}
	else
	if (theMap->ReadValue(cPositionDialogKey, info, temp_1_4_0.parts))
	{
		char_stream stream(info.c_str_mod());
		((CWindowState&) CSpellCheckDialog::sWindowState->GetValue()).SetInfo(stream, temp_1_4_0.parts);
	}
}
