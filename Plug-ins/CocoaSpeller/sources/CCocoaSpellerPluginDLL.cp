// CCocoaSpellerPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Dec-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements CocoaSpeller authentication DLL based plug-in for use in Mulberry.
//
// History:
// 13-Dec-1997: CD: Created initial header and implementation.
// 26-Dec-1997: CD: Doing Win32 support.
// 30-Jan-1999: CD: Rebuilt Win32 to remove library bug
// 27-Nov-2001: CD: Modified Spellswell source for CocoaSpeller
// 27-Dec-2002: CD: Added new method for spell-as-you-type
//

#include "CCocoaSpellerPluginDLL.h"

#include "MyCFString.h"
#include "CPluginInfo.h"
#include "CStringUtils.h"
#include "CUStringUtils.h"

#include "CocoaSpell.h"

#include "SysCFArray.h"

#pragma mark ____________________________consts

const char* cPluginName = "CocoaSpeller Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginSpelling;
const char* cPluginDescription = "CocoaSpeller spelling plugin for Mulberry." COPYRIGHT;

#pragma mark ____________________________CCocoaSpellerPluginDLL

// Constructor
CCocoaSpellerPluginDLL::CCocoaSpellerPluginDLL()
{
	mTag = 0;
}

// Destructor
CCocoaSpellerPluginDLL::~CCocoaSpellerPluginDLL()
{
}

// Does plug-in need to be registered
bool CCocoaSpellerPluginDLL::UseRegistration(unsigned long* key)
{
	if (key)
		*key = ('Mlby' | 'COAS');
	return false;
}

// Can plug-in run as demo
bool CCocoaSpellerPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CCocoaSpellerPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CCocoaSpellerPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CCocoaSpellerPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CCocoaSpellerPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CCocoaSpellerPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CCocoaSpellerPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

// Returns manufacturer of plug-in
long CCocoaSpellerPluginDLL::GetDataLength(void) const
{
	return 0;
}

// Returns description of plug-in
const void* CCocoaSpellerPluginDLL::GetData(void) const
{
	return nil;
}

int CCocoaSpellerPluginDLL::SpInitialise(void)
{
	mTag = ::UniqueSpellDocumentTag();
	return (mTag != 0);
}

int CCocoaSpellerPluginDLL::SpTerminate(void)
{
	::CloseSpellDocumentWithTag(mTag);
	return 1;
}

void CCocoaSpellerPluginDLL::UpdatePreferences(void)
{
	// No options for spelling.
}

int CCocoaSpellerPluginDLL::NewDictionary(const char* name)
{
	return 0;
}

int CCocoaSpellerPluginDLL::OpenDictionary(const char* name)
{
	MyCFString cfstr(name, kCFStringEncodingUTF8);
	return SetLanguage(cfstr);
}

int CCocoaSpellerPluginDLL::CloseDictionary(int dict)
{
    return 0;
}

int CCocoaSpellerPluginDLL::AddWord(const char* utf8)
{
	MyCFString cfstr(utf8, kCFStringEncodingUTF8);
	::LearnWord(cfstr);
	return 1;
}

const char* LocStrEnd[] = {" ", "s", "es", "ed", "ies", "ing", "er", "est", "ly", "'s", "al", "less",
							"able", "ness", "like", "ous", "ment", "ible", "lessness", ""};

const char* CCocoaSpellerPluginDLL::GetAddSuffix(int i) const
{
	return LocStrEnd[i];
}

int CCocoaSpellerPluginDLL::RemoveWord(const char* utf8)
{
	return 0;
}

int CCocoaSpellerPluginDLL::CountPages(void) const
{
	return 0;
}

int CCocoaSpellerPluginDLL::CurrentPage(void) const
{
	return 0;
}

int CCocoaSpellerPluginDLL::SetPage(int page)
{
	return 0;
}

int CCocoaSpellerPluginDLL::GetPage(const char* utf8, const char** buf, size_t* buflen)
{
	return 0;
}

const char* CCocoaSpellerPluginDLL::GetSuggestions(const char* utf8, long ctr)
{
	MyCFString cfstr(utf8, kCFStringEncodingUTF8);
	CFArrayRef results = ::CopyGuessesForWord(cfstr);
	PPx::CFArray<CFStringRef> array(results, false);
	mBuffer.clear();
	for(long count = 0; count < array.GetCount(); count++)
	{
		if (mBuffer.length() != 0)
			mBuffer += "\r";
		MyCFString str(array.GetValueAt(count));
		mBuffer += str.GetString(kCFStringEncodingUTF8);
	}

	return mBuffer.c_str();
}

bool CCocoaSpellerPluginDLL::ContainsWord(const char* utf8)
{
	MyCFString cfstr(utf8, kCFStringEncodingUTF8);

	// Look for word first
	CFRange range = ::CheckSpellingOfString(cfstr, 0, mTag);
	return range.length == 0;
}

int CCocoaSpellerPluginDLL::ClosestWord(const char* utf8)
{
	return 0;
}

int CCocoaSpellerPluginDLL::CheckText(const char* utf8)
{
	// Cache check text
	mCheckLength = 0;

	// Initialise error counters
	mCurrentError = -1;
	mMaxError = 0;
	
	// Run first check and set error count to one
	if (NextError(utf8))
		mMaxError = 1;

	return 1;
}

const CCocoaSpellerPluginDLL::SpError* CCocoaSpellerPluginDLL::NextError(const char* utf8)
{
	MyCFString cfstr(utf8, kCFStringEncodingUTF8);

	// Don't redo the first error
	if (mCurrentError++ == 0)
		return &mError;

	// Check for next error from current pos
	CFRange range = ::CheckSpellingOfString(cfstr, mCheckLength, mTag);

	// Determine what happened
	if (range.length == 0)
		// Check complete
		return NULL;
	else
	{
		// Get error word
		MyCFString errword(::CFStringCreateWithSubstring(NULL, cfstr, range));
		mErrorWord = errword.GetString();

		// See if it is a replacement
		if (mReplaceAll.count(mErrorWord) != 0)
		{
			mReplacement = mReplaceAll.find(mErrorWord)->second;
			mError.ask_user = false;
			mError.do_replace = true;
			mError.word = mErrorWord.c_str();
			mError.replacement = mReplacement.c_str();
			mError.sel_start = range.location; //ISOOffsetToUTF8Offset(iso.c_str(), mCheckLength);
			mError.sel_end = mError.sel_start + mErrorWord.length();
		}
		else
		{
			mError.ask_user = true;
			mError.do_replace = false;
			mError.word = mErrorWord.c_str();
			mError.replacement = NULL;
			mError.sel_start = range.location; //ISOOffsetToUTF8Offset(iso.c_str(), mCheckLength);
			mError.sel_end = mError.sel_start + mErrorWord.length();
		}
		
		mCheckLength = range.location;
	}

	return &mError;
}

bool CCocoaSpellerPluginDLL::ErrorIsPunct(void) const
{
	return false;
}

bool CCocoaSpellerPluginDLL::ErrorIsDoubleWord(void) const
{
	return false;
}

// Skip current error
int CCocoaSpellerPluginDLL::Skip(void)
{
	return DoSkip(false);
}

// Skip all current error
int CCocoaSpellerPluginDLL::SkipAll(void)
{
	return DoSkip(true);
}

// Skip current error
int CCocoaSpellerPluginDLL::Replace(void)
{
	return DoReplace(false);
}

// Skip all current error
int CCocoaSpellerPluginDLL::ReplaceAll(void)
{
	return DoReplace(true);
}

// Skip current error
int CCocoaSpellerPluginDLL::DoSkip(bool skip_all)
{
	// Ignore if skip all, otherwise just ignore it
	if (skip_all)
	{
		MyCFString cfstr(mErrorWord, kCFStringEncodingUTF8);
		::IgnoreWord(cfstr, mTag);
	}

	// Bump text check position over the skip (using iso encoding for error word)
	mCheckLength += mErrorWord.length();

	return 1;
}

// Replace current error
int CCocoaSpellerPluginDLL::DoReplace(bool replace_all)
{
	// Add to temp lexicon if skip all, otherwise just ignore it
	if (replace_all)
	{
		mReplaceAll.insert(cdstrmap::value_type(mErrorWord, mReplacement));
	}

	// Bump text check position over the replaced word
	mCheckLength += mReplacement.length();

	return 1;
}

// Process capitalisation etc
void CCocoaSpellerPluginDLL::ProcessReplaceString(char* replace) const
{
	// No need to do this as the suggestions come back properly capitalised

	// Convert first char to upper if in error word
	//if (needCaps)
	//	*replace = ::toupper(*replace);
}

bool CCocoaSpellerPluginDLL::CannotDisplayDictionary(void) const
{
	// CocoaSpeller cannot display contents of (compressed) dictionary
	return true;
}

int CCocoaSpellerPluginDLL::SetMainDictionaryPath(const char* path)
{
	return 1;
}

int CCocoaSpellerPluginDLL::SetUserDictionaryPath(const char* path)
{
	return 1;
}

int CCocoaSpellerPluginDLL::GetDictionaries(const char** dicts)
{
	mDictionaries.clear();
	CFArrayRef results = ::CopyAvailableLanguages();
	PPx::CFArray<CFStringRef> array(results, false);
	for(long count = 0; count < array.GetCount(); count++)
	{
		MyCFString str(array.GetValueAt(count));
		mDictionaries.push_back(str.GetString(kCFStringEncodingUTF8));
	}
	
	// Count number of matches and create char** buffer size
	mBuffer.clear();
	mBuffer.reserve(sizeof(char*) * (mDictionaries.size() + 1));
	char** p = reinterpret_cast<char**>(mBuffer.c_str_mod());
	for(cdstrvect::const_iterator iter = mDictionaries.begin(); iter != mDictionaries.end(); iter++)
	{
		*p++ = (char*)(*iter).c_str();
	}
	*p++ = NULL;

	*dicts = mBuffer.c_str();

	return 1;
}

int CCocoaSpellerPluginDLL::SetDictionary(const char* name)
{
	MyCFString cfstr(name, kCFStringEncodingUTF8);
	return ::SetLanguage(cfstr);
}

int CCocoaSpellerPluginDLL::SetDictionaryByLanguage(const char* isolangtag)
{
	// Not implmented right now
	// Will eventually use when language tagging text
	return 0;
}

bool CCocoaSpellerPluginDLL::MoreSuggestions(long ctr)
{
	return false;
}

const CCocoaSpellerPluginDLL::SpError* CCocoaSpellerPluginDLL::CheckWord(const char* utf8)
{
	MyCFString cfstr(utf8, kCFStringEncodingUTF8);

	// Check for next error from current pos
	CFRange range = ::CheckSpellingOfString(cfstr, 0, mTag);

	// Determine what happened
	if (range.length == 0)
		// Check complete
		return NULL;
	else
	{
		// Get error word
		MyCFString errword(::CFStringCreateWithSubstring(NULL, cfstr, range));
		mErrorWord = errword.GetString();

		// See if it is a replacement
		if (mReplaceAll.count(mErrorWord) != 0)
		{
			mReplacement = mReplaceAll.find(mErrorWord)->second;
			mError.ask_user = false;
			mError.do_replace = true;
			mError.word = mErrorWord.c_str();
			mError.replacement = mReplacement.c_str();
			mError.sel_start = range.location; //ISOOffsetToUTF8Offset(iso.c_str(), mCheckLength);
			mError.sel_end = mError.sel_start + mErrorWord.length();
		}
		else
		{
			mError.ask_user = true;
			mError.do_replace = false;
			mError.word = mErrorWord.c_str();
			mError.replacement = NULL;
			mError.sel_start = range.location; //ISOOffsetToUTF8Offset(iso.c_str(), mCheckLength);
			mError.sel_end = mError.sel_start + mErrorWord.length();
		}
	}

	return &mError;
}
