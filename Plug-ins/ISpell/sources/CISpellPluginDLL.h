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

// CISpellPluginDLL.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 18-Feb-2001
// Author: Cyrus Daboo
// Platforms: Unix
//
// Description:
// This class implements the ISpell spell checking DLL based plug-in for use in Mulberry.
//
// History:
// 18-Feb-2001: Created initial header and implementation.
//

#ifndef __ISPELL_PLUGIN_MULBERRY__
#define __ISPELL_PLUGIN_MULBERRY__

#include "CSpellPluginDLL.h"

#include "cdstring.h"

// Classes
class CISpellPluginDLL : public CSpellPluginDLL
{
public:

	enum ESpellError
	{
		eSpell_NoErr = 0,
		eSpell_UnknownError = -1L
	};

	// Actual plug-in class

	CISpellPluginDLL();
	virtual ~CISpellPluginDLL();
	
	// Entry codes
	virtual bool	CanRun(void);						// Test whether plug-in can run

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:
	size_t mCheckLength;								// Length of checked text

	// These should be returned by specific sub-class
	virtual const char* GetName(void) const;			// Returns the name of the plug-in
	virtual long GetVersion(void) const;				// Returns the version number of the plug-in
	virtual EPluginType GetType(void) const;			// Returns the type of the plug-in
	virtual const char* GetManufacturer(void) const;	// Returns manufacturer of plug-in
	virtual const char* GetDescription(void) const;		// Returns description of plug-in
	virtual long GetDataLength(void) const;				// Returns data of plug-in
	virtual const void* GetData(void) const;			// Returns data of plug-in

	// Entry codes

	// Init/terminate speller
	virtual int SpInitialise(void);
	virtual int SpTerminate(void);

	// Preferences
	virtual void UpdatePreferences(void);

	// Dictionary items
	virtual int NewDictionary(const char* name);
	virtual int OpenDictionary(const char* name);
	virtual int CloseDictionary(int dict);

	virtual int AddWord(const char* word);
	virtual const char* GetAddSuffix(int i) const;
	virtual int RemoveWord(const char* word);
	
	virtual int CountPages(void) const;
	virtual int CurrentPage(void) const;

	virtual int SetPage(int page);
	virtual int GetPage(const char* word, const char** buf, size_t* buflen);
	virtual const char* GetSuggestions(const char* word, long ctr);
	
	virtual bool ContainsWord(const char* word);
	virtual int ClosestWord(const char* word);
	
	// Checking
	virtual int CheckText(const char* txt);
	virtual bool HasErrors(void) const;
	virtual const SpError* CurrentError(void) const;
	virtual SpError* CurrentError(void);
	virtual const SpError* NextError(const char* txt);
	
	virtual int Skip(void);
	virtual int SkipAll(void);
	virtual int Replace(void);
	virtual int ReplaceAll(void);

	virtual void ProcessReplaceString(char* replace) const;
	
	// Error feedback
	virtual bool ErrorIsPunct(void) const;
	virtual bool ErrorIsDoubleWord(void) const;
	
	// Spellex items
	virtual bool CannotDisplayDictionary(void) const;

	virtual int SetMainDictionaryPath(const char* path);
	virtual int SetUserDictionaryPath(const char* path);
	virtual int GetDictionaries(const char** dicts);
	
	virtual int SetDictionary(const char* name);
	virtual int SetDictionaryByLanguage(const char* isolangtag);
	
	virtual bool MoreSuggestions(long ctr2);

	virtual const SpError* CheckWord(const char* word);

private:
	class CErrorDetails
	{
	public:
		SpError mError;

		CErrorDetails() {}
		CErrorDetails(const CErrorDetails& copy);
		~CErrorDetails() {}
	};

	int				mInputfd[2];
	int				mOutputfd[2];
	struct SData
	{
		cdstring	mReadBuffer;
		cdstring	mBuffer;
		cdstrvect	mListBuffer;
	};
	SData*			mData;
	pid_t			mPID;
	int				mSelectionAdjust;
	cdstring		mLastCheck;

	std::vector<CErrorDetails> mErrorList;
	cdstring mErrorWord;
	cdstrmap mSuggestions;
	unsigned long mErrorIndex;
	cdstrset mSkipAll;
	cdstrmap mReplaceAll;

	int GetLine(cdstring& line);
	int SendLine(const char* line);
	int SendLine(const char* line, size_t length);

	void ParseMissGuess(cdstring& line, cdstring& error, int& start, int& end, cdstring& suggestions);
	void ParseNone(cdstring& line, cdstring& error, int& start, int& end, cdstring& suggestions);
};

#endif
