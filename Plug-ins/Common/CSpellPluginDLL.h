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

#ifndef __SPELL_PLUGIN_MULBERRY__
#define __SPELL_PLUGIN_MULBERRY__

#include "CPluginDLL.h"

#include <string.h>

// Classes

class CSpellPreferences
{
public:
	enum ESuggestions
	{
		eSuggest_Fast,
		eSuggest_Medium,
		eSuggest_Slow
	};

	bool	mIgnoreCapitalised;
	bool	mIgnoreAllCaps;
	bool	mIgnoreWordsNumbers;
	bool	mIgnoreMixedCase;
	bool	mIgnoreDomainNames;
	bool	mReportDoubledWords;
	bool	mCaseSensitive;
	bool	mPhoneticSuggestions;
	bool	mTypoSuggestions;
	bool	mSuggestSplit;
	bool	mAutoCorrect;
	ESuggestions 	mSuggestLevel;
	char			mDictionaryName[256];

			CSpellPreferences();
	virtual ~CSpellPreferences();
	
};

class CSpellPluginDLL : public CPluginDLL
{
public:

	enum ESpellPluginCall
	{
		// Derived plug-in calls
		eSpellInitialise = CPluginDLL::ePluginFirstSubCode,	// First code availble for sub-class calls
		eSpellTerminate,
		eSpellGetPrefs,
		eSpellUpdatePrefs,
		eSpellNewDictionary,
		eSpellOpenDictionary,
		eSpellCloseDictionary,
		eSpellAddWord,
		eSpellGetAddSuffix,
		eSpellRemoveWord,
		eSpellSetMaxPerPage,
		eSpellCountPages,
		eSpellCurrentPage,
		eSpellSetPage,
		eSpellGetPage,
		eSpellGetSuggestions,
		eSpellContainsWord,
		eSpellClosestWord,
		eSpellCheckText,
		eSpellHasErrors,
		eSpellCurrentError,
		eSpellNextError,
		eSpellSkip,
		eSpellSkipAll,
		eSpellReplace,
		eSpellReplaceAll,
		eSpellProcessReplaceString,
		eSpellErrorIsPunct,
		eSpellErrorIsDoubleWord,
		
		// New calls for spellex support
		eSpellCannotDisplayDictionary,
		eSpellSetMainDictionaryPath,
		eSpellSetUserDictionaryPath,
		eSpellGetDictionaries,
		eSpellSetDictionary,
		eSpellSetDictionaryByLanguage,
		eSpellMoreSuggestions,
		eSpellCheckWord
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=mac68k
#endif
#endif

	// Spelling error structure
	struct SpError
	{
		bool ask_user;
		bool do_replace;
		const char* word;
		const char* replacement;
		long sel_start;
		long sel_end;
	};

	struct SpGetPageData
	{
		const char* word;
		const char** buf;
		size_t* buf_len;
	};

	struct SpGetSuggestionsData
	{
		const char* word;
		long ctr;
	};

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	// Actual plug-in class

	CSpellPluginDLL();
	virtual ~CSpellPluginDLL();

	// Entry point
	virtual long Entry(long code, void* data, long refCon);	// DLL entry point and dispatch

protected:
	int mMaxPerPage;
	int mErrorCount;
	size_t mCheckLength;
	const char* mCheckTxt;
	short mCurrentError;
	short mMaxError;
	SpError mError;
	CSpellPreferences mPreferences;

	// Entry codes

	// Init/terminate speller
	virtual int SpInitialise(void) = 0;
	virtual int SpTerminate(void) = 0;

	// Preferences
	virtual CSpellPreferences* GetPreferences(void)
		{ return &mPreferences; }
	virtual void UpdatePreferences(void) = 0;

	// Dictionary items
	virtual int NewDictionary(const char* name) = 0;
	virtual int OpenDictionary(const char* name) = 0;
	virtual int CloseDictionary(int dict) = 0;

	virtual int AddWord(const char* word) = 0;
	virtual const char* GetAddSuffix(int i) const = 0;
	virtual int RemoveWord(const char* word) = 0;
	
	virtual void SetMaxPerPage(int max_per_page)
		{ mMaxPerPage = max_per_page; }

	virtual int CountPages(void) const = 0;
	virtual int CurrentPage(void) const = 0;

	virtual int SetPage(int page) = 0;
	virtual int GetPage(const char* word, const char** buf, size_t* buflen) = 0;
	virtual const char* GetSuggestions(const char* word, long ctr) = 0;
	
	virtual bool ContainsWord(const char* word) = 0;
	virtual int ClosestWord(const char* word) = 0;
	
	// Checking
	virtual int CheckText(const char* txt) = 0;
	virtual bool HasErrors(void) const
		{ return mMaxError > 0; }
	virtual const SpError* CurrentError(void) const
		{ return &mError; }
	virtual const SpError* NextError(const char* txt) = 0;
	
	virtual int Skip(void) = 0;
	virtual int SkipAll(void) = 0;
	virtual int Replace(void) = 0;
	virtual int ReplaceAll(void) = 0;

	virtual void ProcessReplaceString(char* replace) const = 0;

	// Error feedback
	virtual bool ErrorIsPunct(void) const = 0;
	virtual bool ErrorIsDoubleWord(void) const = 0;
	
	// Spellex items
	virtual bool CannotDisplayDictionary(void) const = 0;

	virtual int SetMainDictionaryPath(const char* path) = 0;
	virtual int SetUserDictionaryPath(const char* path) = 0;
	virtual int GetDictionaries(const char** dicts) = 0;
	
	virtual int SetDictionary(const char* name) = 0;
	virtual int SetDictionaryByLanguage(const char* isolangtag) = 0;
	
	virtual bool MoreSuggestions(long ctr2) = 0;

	virtual const SpError* CheckWord(const char* word) = 0;
	
};

#endif
