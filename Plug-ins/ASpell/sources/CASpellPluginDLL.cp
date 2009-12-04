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

// CASpellPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements ASpell spell checker DLL based plug-in for use in Mulberry.
//
// History:
// 18-Feb-2001: Created initial header and implementation.
// 14-Jan-2003: Switched to ASpell from ISpell
//

#include "CASpellPluginDLL.h"
#include "CPluginInfo.h"
#include "CStringUtils.h"

#include <algorithm>
#include <memory>

#include <string.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#define ASPELLERROR(xx_errno, xx_msg) do { \
	char err_buf[256]; \
		::strcpy(err_buf, "ASpell Plugin Error: "); \
		::strncat(err_buf, xx_msg, 200); \
		LogEntry(err_buf); \
	} while (0)

//#define DEBUG_OUTPUT

#pragma mark ____________________________consts

const char* cPluginName = "ASpell Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSpelling;
const char* cPluginDescription = "ASpell spelling plugin for Mulberry." OS_ENDL OS_ENDL COPYRIGHT;

#pragma mark ____________________________CASpellPluginDLL

// Constructor
CASpellPluginDLL::CASpellPluginDLL()
{
	mSelectionAdjust = 0;
	mData = new SData;
	mASpellConfig = NULL;
	mASpellSpeller = NULL;
	mASpellChecker = NULL;
	mData->mDictLang = "en";
	mData->mDictChanged = false;
}

// Destructor
CASpellPluginDLL::~CASpellPluginDLL()
{
	if (mASpellChecker)
		::delete_aspell_document_checker(mASpellChecker);
	mASpellChecker = NULL;
	if (mASpellSpeller)
		::delete_aspell_speller(mASpellSpeller);
	mASpellSpeller = NULL;
	if (mASpellConfig)
		::delete_aspell_config(mASpellConfig);
	mASpellConfig = NULL;
	
	delete mData;
	mData = NULL;
}

// Does plug-in need to be registered
bool CASpellPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'ASPL');
	return false;
}

// Can plug-in run as demo
bool CASpellPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CASpellPluginDLL::CanRun(void)
{
	// If the plugin loads (we are dynamically linked to libaspell) then it can run
	return true;
}

// Returns the name of the plug-in
const char* CASpellPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CASpellPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CASpellPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CASpellPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CASpellPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Returns manufacturer of plug-in
long CASpellPluginDLL::GetDataLength(void) const
{
	return 0;
}

// Returns description of plug-in
const void* CASpellPluginDLL::GetData(void) const
{
	return nil;
}

int CASpellPluginDLL::SpInitialise(void)
{
	AspellCanHaveError* ret = 0;

	// Start an aspell session here
	mASpellConfig = ::new_aspell_config();
	if (mASpellConfig == NULL)
	{
		ASPELLERROR(1, "SpInitialise::new_aspell_config");
		return 0;
	}
	::aspell_config_replace(mASpellConfig, "lang", mData->mDictLang.c_str());

	// Create a speller
	ret = ::new_aspell_speller(mASpellConfig);
	if (::aspell_error(ret) != 0)
	{
		ASPELLERROR(::aspell_error_number(ret), ::aspell_error_message(ret));
    	::delete_aspell_can_have_error(ret);
		return 0;
	}
	mASpellSpeller = ::to_aspell_speller(ret);

	// Create a checker
	ret = ::new_aspell_document_checker(mASpellSpeller);
	if (::aspell_error(ret) != 0)
	{
		ASPELLERROR(::aspell_error_number(ret), ::aspell_error_message(ret));
    	::delete_aspell_can_have_error(ret);
		return 0;
	}
	mASpellChecker = ::to_aspell_document_checker(ret);

	return 1;
}

int CASpellPluginDLL::SpTerminate(void)
{
	// Check for dictionary change and write to disk
	if (mData->mDictChanged)
	{
		if (mASpellSpeller)
		{
			::aspell_speller_save_all_word_lists(mASpellSpeller);
			if (::aspell_speller_error(mASpellSpeller) != 0)
				ASPELLERROR(::aspell_speller_error_number(mASpellSpeller), "Failed to Save Word Lists");
		}
		mData->mDictChanged = false;
	}

	// Clear up each allocated object
	if (mASpellChecker)
		::delete_aspell_document_checker(mASpellChecker);
	mASpellChecker = NULL;
	if (mASpellSpeller)
		::delete_aspell_speller(mASpellSpeller);
	mASpellSpeller = NULL;
	if (mASpellConfig)
		::delete_aspell_config(mASpellConfig);
	mASpellConfig = NULL;
	return 1;
}

void CASpellPluginDLL::UpdatePreferences(void)
{
	// ASpell has no user options right now
}

int CASpellPluginDLL::NewDictionary(const char* name)
{
	return 0;
}

int CASpellPluginDLL::OpenDictionary(const char* name)
{
	return 1;
}

int CASpellPluginDLL::CloseDictionary(int dict)
{
	return 1;
}

int CASpellPluginDLL::AddWord(const char* word)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to AddWord - no mASpellSpeller");
		return 0;
	}

	// Add a word to personal dictionary
	::aspell_speller_add_to_personal(mASpellSpeller, word, -1);
	if (::aspell_speller_error(mASpellSpeller) != 0)
	{
		ASPELLERROR(::aspell_speller_error_number(mASpellSpeller), "Failed to AddWord");
		return 0;
	}
	mData->mDictChanged = true;

	// Also do implicit SkipAll on this word in case it's
	// already flagged as an error 
	mSkipAll.insert(word);

	return 1;
}

const char* CASpellPluginDLL::GetAddSuffix(int i) const
{
	return NULL;
}

int CASpellPluginDLL::RemoveWord(const char* word)
{
	// Not supported by ASpell
	return 1;
}

int CASpellPluginDLL::CountPages(void) const
{
	// Not supported by ASpell
	return 0;
}

int CASpellPluginDLL::CurrentPage(void) const
{
	// Not supported by ASpell
	return 0;
}

int CASpellPluginDLL::SetPage(int page)
{
	// Not supported by ASpell
	return 0;
}

int CASpellPluginDLL::GetPage(const char* word, const char** buf, size_t* buflen)
{
	// Not supported by ASpell

	// If word exists then try to cache its page and get closest pos
	int result = -1;

	// Return page details
	*buf = NULL;
	*buflen = 0;
	
	return result;
}

const char* CASpellPluginDLL::GetSuggestions(const char* word, long ctr)
{
	// Clear existing buffer
	mData->mBuffer = cdstring::null_str;

	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to GetSuggestions - no mASpellSpeller");
		return mData->mBuffer.c_str();
	}

	// Look up error word
	const struct AspellWordList* words = ::aspell_speller_suggest(mASpellSpeller, word, -1);
	if (words)
	{
		// Get suggestions into array
	    AspellStringEnumeration* els = ::aspell_word_list_elements(words);
	    cdstrvect suggs;
	    const char* item = NULL;
		while((item = ::aspell_string_enumeration_next(els)) != 0)
			suggs.push_back(item);
		::delete_aspell_string_enumeration(els);

		// Create suggestions buffer - '\n' terminated list of items
		for(cdstrvect::const_iterator iter = suggs.begin(); iter != suggs.end(); iter++)
		{
			if (mData->mBuffer.length() != 0)
				mData->mBuffer += '\n';
			mData->mBuffer += *iter;
		}
	}
	
	return mData->mBuffer.c_str();
}

// ContainsWord: check a word for correct spelling
// word:    the word to check
//
// returns: true if word is in dictionary
//          false if not in dictionary
//
bool CASpellPluginDLL::ContainsWord(const char* word)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to ContainsWord - no mASpellSpeller");
		return true;
	}

	// Check a single word
	return ::aspell_speller_check(mASpellSpeller, word, -1) == 1;
}

int CASpellPluginDLL::ClosestWord(const char* word)
{
	// Not supported by ASpell
	return 0;
}

int CASpellPluginDLL::CheckText(const char* txt)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to CheckText - no mASpellSpeller");
		return 0;
	}

	// Reset document checker
	mSelectionAdjust = 0;
	::aspell_document_checker_reset(mASpellChecker);

	// Do spell check
	::aspell_document_checker_process(mASpellChecker, txt, -1);

	// Set error count to zero at start
	mMaxError = 0;

	// Do initial check to indicate maximum number of errors for HasErrors
	NextError(txt);

    return 1;
}

const CASpellPluginDLL::SpError* CASpellPluginDLL::NextError(const char* txt)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to NextError - no mASpellSpeller");
		return NULL;
	}

	// Special case mMaxError == 1 - we've already got the error - just return it
	if (mMaxError == 1)
	{
		mMaxError++;
		return &mError;
	}
	else
	{
		// Get error token
		struct AspellToken token = ::aspell_document_checker_next_misspelling(mASpellChecker);
		
		// Check for no more errors
		if (token.len == 0)
			return NULL;

		// Bump error marker after we know we have an error
		mMaxError++;
		
		// Setup selection with adjustment first
		mError.sel_start = token.offset + mSelectionAdjust;
		mError.sel_end = mError.sel_start + token.len;

		// Get current error word
		cdstring word(txt + mError.sel_start, token.len);
		mErrorWord = word;
		mError.word = mErrorWord;

		// Now check to see if error is a skip all
		cdstrset::iterator found_skip = mSkipAll.find(mError.word);
		bool do_skip = (found_skip != mSkipAll.end());

		// Now check to see if error is a replace all
		cdstrmap::iterator found_replace = mReplaceAll.find(mError.word);
		bool do_replace = (found_replace != mReplaceAll.end());

		mError.ask_user = !do_skip;
		mError.do_replace = do_replace;
		mError.replacement = (do_replace ? found_replace->second.c_str() : NULL);

		return &mError;
	}
}

bool CASpellPluginDLL::ErrorIsPunct(void) const
{
	return false;
}

bool CASpellPluginDLL::ErrorIsDoubleWord(void) const
{
	return false;
}

// Skip current error
int CASpellPluginDLL::Skip(void)
{
	// Nothing to do for a one-word skip
	return 1;
}

// Skip all current error
int CASpellPluginDLL::SkipAll(void)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to SkipAll - no mASpellSpeller");
		return 0;
	}

	// Add a word to session dictionary
	::aspell_speller_add_to_session(mASpellSpeller, CurrentError()->word, -1);
	if (::aspell_speller_error(mASpellSpeller) != 0)
	{
		ASPELLERROR(::aspell_speller_error_number(mASpellSpeller), "Failed to SkipAll");
		return 0;
	}

	// Add current error to skip all set
	mSkipAll.insert(CurrentError()->word);

	return 1;
}

// Replace current error
int CASpellPluginDLL::Replace(void)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to Replace - no mASpellSpeller");
		return 0;
	}

	::aspell_speller_store_replacement(mASpellSpeller, CurrentError()->word, -1, CurrentError()->replacement, -1);

	// Caller is doing the replacement in the original text
	// We must adjust future selection offsets to accomodate
	// a change in length of the word being replaced
	if (CurrentError()->replacement)
		mSelectionAdjust += ::strlen(CurrentError()->replacement) - ::strlen(CurrentError()->word);
	return 1;
}

// Replace all current error
int CASpellPluginDLL::ReplaceAll(void)
{
	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to ReplaceAll - no mASpellSpeller");
		return 0;
	}

	::aspell_speller_store_replacement(mASpellSpeller, CurrentError()->word, -1, CurrentError()->replacement, -1);

	// Add current error to replace all map
	if (CurrentError()->replacement)
		mReplaceAll.insert(cdstrmap::value_type(CurrentError()->word, CurrentError()->replacement));

	// Also do replace behaviour
	return Replace();
}

// Process capitalisation etc
void CASpellPluginDLL::ProcessReplaceString(char* replace) const
{
	// Convert first char to upper if in error word
	if (replace && isupper(CurrentError()->word[0]))
		replace[0] = toupper(replace[0]);
}

bool CASpellPluginDLL::CannotDisplayDictionary(void) const
{
	// ASpell cannot display contents of (compressed) dictionary
	return true;
}

int CASpellPluginDLL::SetMainDictionaryPath(const char* path)
{
	return 1;
}

int CASpellPluginDLL::SetUserDictionaryPath(const char* path)
{
	return 1;
}

int CASpellPluginDLL::GetDictionaries(const char** dicts)
{
	*dicts = NULL;

	// Get dictionary list
	AspellDictInfoList* dlist = ::get_aspell_dict_info_list(mASpellConfig);

	// Copy dictionary names into list buffer
	mData->mListBuffer.clear();
	const AspellDictInfo* entry = NULL;
	AspellDictInfoEnumeration* dels = ::aspell_dict_info_list_elements(dlist);
	while((entry = ::aspell_dict_info_enumeration_next(dels)) != 0)
		mData->mListBuffer.push_back(entry->code);
	::delete_aspell_dict_info_enumeration(dels);

	// Sort then make list unique
	std::sort(mData->mListBuffer.begin(), mData->mListBuffer.end());
	mData->mListBuffer.erase(std::unique(mData->mListBuffer.begin(), mData->mListBuffer.end()), mData->mListBuffer.end());

	// Allocate space for array and arrays
	mData->mBuffer.clear();
	mData->mBuffer.reserve(sizeof(char*) * (mData->mListBuffer.size() + 1));

	// Assign list buffer items to buffer
	char** p = reinterpret_cast<char**>(mData->mBuffer.c_str_mod());
	for(cdstrvect::iterator iter = mData->mListBuffer.begin(); iter != mData->mListBuffer.end(); iter++)
		*p++ = (*iter).c_str_mod();
	*p++ = NULL;

	*dicts = mData->mBuffer.c_str();
	
	return 1;
}


int CASpellPluginDLL::SetDictionary(const char* name)
{
	// Kill current spell checker items
	SpTerminate();
	
	// Set new name
	mData->mDictLang = name;
	
	// Re-initialise to get new dictionary
	SpInitialise();

	return 1;
}

int CASpellPluginDLL::SetDictionaryByLanguage(const char* isolangtag)
{
	return 0;
}

bool CASpellPluginDLL::MoreSuggestions(long ctr)
{
	return false;
}

const CASpellPluginDLL::SpError* CASpellPluginDLL::CheckWord(const char* word)
{
	mErrorWord = word;

	if (mASpellSpeller == NULL)
	{
		ASPELLERROR(1, "Failed to CheckWord - no mASpellSpeller");
		return NULL;
	}

	// Check a single word
	bool result = ::aspell_speller_check(mASpellSpeller, word, -1) == 1;

	if (result)
		return NULL;
	else
	{
		// Now check to see if error is a replace all
		cdstrmap::iterator found_replace = mReplaceAll.find(word);
		bool do_replace = (found_replace != mReplaceAll.end());
		const char* replacement = (do_replace ? found_replace->second.c_str() : NULL);

		mError.ask_user = false;
		mError.do_replace = do_replace;
		mError.word = mErrorWord;
		mError.replacement = replacement;
		mError.sel_start = 0;
		mError.sel_end = 0;

	  	return &mError;
	}
}
