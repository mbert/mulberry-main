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

	// CSpellPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 16-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based Spelling plug-in for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the Spelling DLL being called by Mulberry.
//
// History:
// CD:	 16-Dec-1997:	Created initial header and implementation.
//

#include "CSpellPluginDLL.h"

#pragma mark ____________________________CSpellPreferences

CSpellPreferences::CSpellPreferences()
{
	mIgnoreCapitalised = false;
	mIgnoreAllCaps = false;
	mIgnoreWordsNumbers = false;
	mIgnoreMixedCase = false;
	mIgnoreDomainNames = true;
	mReportDoubledWords = true;
	mCaseSensitive = true;
	mPhoneticSuggestions = true;
	mTypoSuggestions = true;
	mSuggestSplit = false;
	mAutoCorrect = true;
	*mDictionaryName = 0;
}

CSpellPreferences::~CSpellPreferences()
{
}

#pragma mark ____________________________CSpellPluginDLL

// Constructor
CSpellPluginDLL::CSpellPluginDLL()
{
	mMaxPerPage = 1024;
	mErrorCount = 0;
	mCurrentError = 0;
	mMaxError = 0;
}

// Destructor
CSpellPluginDLL::~CSpellPluginDLL()
{
}

// DLL entry point and dispatch
long CSpellPluginDLL::Entry(long code, void* data, long refCon)
{
	switch(code)
	{

	case CSpellPluginDLL::eSpellInitialise:
		return SpInitialise();
	case CSpellPluginDLL::eSpellTerminate:
		return SpTerminate();

	// Preferences
	case CSpellPluginDLL::eSpellGetPrefs:
		*(CSpellPreferences**) data = GetPreferences();
		return 1;
	case CSpellPluginDLL::eSpellUpdatePrefs:
		UpdatePreferences();
		return 1;

	// Dictionary items
	case CSpellPluginDLL::eSpellNewDictionary:
		return NewDictionary((const char*) data);
	case CSpellPluginDLL::eSpellOpenDictionary:
		return OpenDictionary((const char*) data);
	case CSpellPluginDLL::eSpellCloseDictionary:
		return CloseDictionary((int) data);

	case CSpellPluginDLL::eSpellAddWord:
		return AddWord((const char*) data);
	case CSpellPluginDLL::eSpellGetAddSuffix:
		return (long) GetAddSuffix((int) data);
	case CSpellPluginDLL::eSpellRemoveWord:
		return RemoveWord((const char*) data);

	case CSpellPluginDLL::eSpellSetMaxPerPage:
		SetMaxPerPage((int) data);
		return 1;
	case CSpellPluginDLL::eSpellCountPages:
		return CountPages();
	case CSpellPluginDLL::eSpellCurrentPage:
		return CurrentPage();
	case CSpellPluginDLL::eSpellSetPage:
		return SetPage((int) data);
	case CSpellPluginDLL::eSpellGetPage:
		return GetPage(((SpGetPageData*) data)->word, ((SpGetPageData*) data)->buf, ((SpGetPageData*) data)->buf_len);
	case CSpellPluginDLL::eSpellGetSuggestions:
		return (long) GetSuggestions(((SpGetSuggestionsData*) data)->word, ((SpGetSuggestionsData*) data)->ctr);
	case CSpellPluginDLL::eSpellContainsWord:
		return (ContainsWord((const char*) data) ? 1 : 0);
	case CSpellPluginDLL::eSpellClosestWord:
		return ClosestWord((const char*) data);
	case CSpellPluginDLL::eSpellCheckText:
		return CheckText((const char*) data);
	case CSpellPluginDLL::eSpellHasErrors:
		return (HasErrors() ? 1 : 0);
	case CSpellPluginDLL::eSpellCurrentError:
		return (long) CurrentError();
	case CSpellPluginDLL::eSpellNextError:
		return (long) NextError((const char*) data);
	case CSpellPluginDLL::eSpellSkip:
		return Skip();
	case CSpellPluginDLL::eSpellSkipAll:
		return SkipAll();
	case CSpellPluginDLL::eSpellReplace:
		return Replace();
	case CSpellPluginDLL::eSpellReplaceAll:
		return ReplaceAll();
	case CSpellPluginDLL::eSpellProcessReplaceString:
		ProcessReplaceString((char*) data);
		return 1;
	case CSpellPluginDLL::eSpellErrorIsPunct:
		return (ErrorIsPunct() ? 1 : 0);
	case CSpellPluginDLL::eSpellErrorIsDoubleWord:
		return (ErrorIsDoubleWord() ? 1 : 0);

	// Spellex items
	case CSpellPluginDLL::eSpellCannotDisplayDictionary:
		return (CannotDisplayDictionary() ? 1 : 0);

	case CSpellPluginDLL::eSpellSetMainDictionaryPath:
		return SetMainDictionaryPath((const char*) data);

	case CSpellPluginDLL::eSpellSetUserDictionaryPath:
		return SetUserDictionaryPath((const char*) data);

	case CSpellPluginDLL::eSpellGetDictionaries:
		return GetDictionaries((const char**) data);

	case CSpellPluginDLL::eSpellSetDictionary:
		return SetDictionary((const char*) data);

	case CSpellPluginDLL::eSpellSetDictionaryByLanguage:
		return SetDictionaryByLanguage((const char*) data);

	case CSpellPluginDLL::eSpellMoreSuggestions:
		return (MoreSuggestions(*(long*) data) ? 1 : 0);

	case CSpellPluginDLL::eSpellCheckWord:
		return (long) CheckWord((const char*) data);

	default:
		return CPluginDLL::Entry(code, data, refCon);
	}
}
