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

#ifndef __CSPELLPLUGIN__MULBERRY__
#define __CSPELLPLUGIN__MULBERRY__

#include "CPlugin.h"

#include "CPreferenceValue.h"

#include <string.h>

#if __dest_os == __mac_os || __dest_os == __mac_os_x
// Resourcses
const	ResIDT		STRx_Spell = 20101;
enum
{
	str_CannotAddDuplicate = 1,
	str_NoDictionaries,
	str_ReplaceNotInDictionary,
	str_DoneCheck
};
#endif

// Classes

class CSpellPreferences
{
public:
	CPreferenceValueMap<bool>		mIgnoreCapitalised;
	CPreferenceValueMap<bool>		mIgnoreAllCaps;
	CPreferenceValueMap<bool>		mIgnoreWordsNumbers;
	CPreferenceValueMap<bool>		mIgnoreMixedCase;
	CPreferenceValueMap<bool>		mIgnoreDomainNames;
	CPreferenceValueMap<bool>		mReportDoubledWords;
	CPreferenceValueMap<bool>		mCaseSensitive;
	CPreferenceValueMap<bool>		mPhoneticSuggestions;
	CPreferenceValueMap<bool>		mTypoSuggestions;
	CPreferenceValueMap<bool>		mSuggestSplit;
	CPreferenceValueMap<bool>		mAutoCorrect;
	CPreferenceValueMap<long>		mSuggestLevel;
	CPreferenceValueMap<cdstring>	mDictionaryName;

	CPreferenceValueMap<bool>		mAutoPositionDialog;
	CPreferenceValueMap<bool>		mSpellOnSend;
	CPreferenceValueMap<bool>		mSpellAsYouType;
	CPreferenceValueMap<bool>		mSpellColourBackground;
	CPreferenceValueMap<_RGBColor>	mSpellBackgroundColour;

			CSpellPreferences();
	virtual ~CSpellPreferences();
	
};

class CSpellPreferencesDLL
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
	ESuggestions	mSuggestLevel;
	char	mDictionaryName[256];

			CSpellPreferencesDLL();
	virtual ~CSpellPreferencesDLL();
	CSpellPreferencesDLL& operator=(CSpellPreferences& copy);
};

#if __dest_os == __mac_os || __dest_os == __mac_os_x
class LWindow;
class CTextDisplay;
typedef LWindow	CWindowType;
#elif __dest_os == __win32_os
class CCmdEditView;
typedef CCmdEditView CTextDisplay;
typedef CWnd	CWindowType;
#elif __dest_os == __linux_os
class JXWindowDirector;
class CTextDisplay;
typedef JXWindowDirector	CWindowType;
#endif

class CSpellPlugin : public CPlugin
{
public:

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

#if __dest_os == __mac_os
#if PRAGMA_STRUCT_ALIGN
#pragma options align=reset
#endif
#endif

	class StLoadSpellPlugin
	{
	public:
		StLoadSpellPlugin(CSpellPlugin* aPlugin)
			{ mPlugin = aPlugin; mPlugin->LoadPlugin(); mPlugin->SpInitialise(); }
		~StLoadSpellPlugin()
			{ mPlugin->SpTerminate(); mPlugin->UnloadPlugin(); }
	private:
		CSpellPlugin* mPlugin;
	};

	// Actual plug-in class

	CSpellPlugin(fspec files);
	virtual ~CSpellPlugin();

	// Init/terminate speller
			void	AutoLoad();
	virtual int		SpInitialise();
	virtual int		SpTerminate();

	// Preferences
	virtual CSpellPreferences* GetPreferences()
		{ return &mPreferences; }
	virtual void UpdatePreferences();

	// Main checker
	virtual bool SpellCheck(CWindowType* wnd, CTextDisplay* text, bool sending, bool hilite_only = false);
	virtual bool SpellOnSend() const
		{ return mPreferences.mSpellOnSend.GetValue(); }
	virtual bool SpellAsYouType() const
		{ return mPreferences.mSpellAsYouType.GetValue(); }
	virtual bool SpellColourBackground() const
		{ return mPreferences.mSpellColourBackground.GetValue(); }
	virtual const _RGBColor& SpellBackgroundColour() const
		{ return mPreferences.mSpellBackgroundColour.GetValue(); }
	virtual void SpellOptions();
	virtual void SpellChooseDictionary();
	virtual void SpellEditDictionary();

	// Dictionary items
	virtual void CheckDictionary();					// Get default dictionary if none specified
	virtual int NewDictionary(const char* name);
	virtual int OpenDictionary(const char* name);
	virtual int CloseDictionary(int dict);

	virtual int AddWord(const char* word);
	virtual const char* GetAddSuffix(int i) const;
	virtual int RemoveWord(const char* word);
	
	virtual void SetMaxPerPage(int max_per_page);

	virtual int CountPages() const;
	virtual int CurrentPage() const;

	virtual int SetPage(int page);
	virtual int GetPage(const char* word, const char** buf, size_t* buflen);
	virtual void GetSuggestions(const char* word, long ctr, cdstrvect& suggestions);
	virtual void GetQuickSuggestions(const char* word, cdstrvect& suggestions);
	
	virtual bool ContainsWord(const char* word);
	virtual int ClosestWord(const char* word);
	
	// Checking
	virtual int CheckText(const char* txt);
	virtual bool HasErrors() const;
	virtual const SpError* CurrentError() const;
	virtual const SpError* NextError(const char* txt);
	virtual void HiliteText(CTextDisplay* text, long start, const cdstring& check);
	
	virtual int Skip();
	virtual int SkipAll();
	virtual int Replace();
	virtual int ReplaceAll();

	virtual void ProcessReplaceString(char* replace) const;

	// Error feedback
	virtual bool ErrorIsPunct() const;
	virtual bool ErrorIsDoubleWord() const;

	// Spellex items
	virtual bool CannotDisplayDictionary();

	virtual void SetMainDictionaryPath(const char* path) const;
	virtual void SetUserDictionaryPath(const char* path) const;
	virtual int GetDictionaries(const char** dicts) const;
	
	virtual int SetDictionary(const char* name) const;
	virtual int SetDictionaryByLanguage(const char* isolangtab) const;

	virtual bool MoreSuggestions(long ctr) const;

	virtual const SpError* CheckWord(const char* word);

protected:
	enum ESpellPluginCall
	{
		// Derived plug-in calls
		eSpellInitialise = CPlugin::ePluginFirstSubCode,	// First code availble for sub-class calls
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

	CSpellPreferences	mPreferences;
	CPreferenceValueMap<cdstring> mDictionary;
	unsigned long		mInitCount;
	bool				mAutoLoaded;

	// Preferences
	virtual void 	DoWriteToMap(COptionsMap* theMap, bool dirty_only);	// Write data to a stream
	virtual void	DoReadFromMap(COptionsMap* theMap,
							NumVersion vers_prefs,
							NumVersion vers_plugin);				// Read data from a stream
};

#endif
