// CCocoaSpellerPluginDLL.h
//
// Copyright 2007, Cyrus Daboo.  All Rights Reserved.
//
// Created: 21-Oct-2007
// Author: Cyrus Daboo
// Platforms: Mac OS X
//
// Description:
// This class implements the CocoaSpeller spell checking DLL based plug-in for use in Mulberry.
//

#pragma once

#include "CSpellPluginDLL.h"

#include "cdstring.h"
#include "templs.h"

// Classes
class CCocoaSpellerPluginDLL : public CSpellPluginDLL
{
public:

	// Actual plug-in class

	CCocoaSpellerPluginDLL();
	virtual ~CCocoaSpellerPluginDLL();
	
	// Entry codes
	virtual bool CanRun(void);							// Test whether plug-in can run

	// Registration
	virtual bool UseRegistration(unsigned long* key);	// Does plug-in need to be registered
	virtual bool CanDemo(void);							// Can plug-in run as demo

protected:

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
	virtual const SpError* NextError(const char* txt);
	
	virtual int Skip(void);
	virtual int SkipAll(void);
	virtual int Replace(void);
	virtual int ReplaceAll(void);

	virtual void ProcessReplaceString(char* replace) const;
	
	// Error feedback
	virtual bool ErrorIsPunct(void) const;
	virtual bool ErrorIsDoubleWord(void) const;
	
	// CocoaSpeller items
	virtual bool CannotDisplayDictionary(void) const;

	virtual int SetMainDictionaryPath(const char* path);
	virtual int SetUserDictionaryPath(const char* path);
	virtual int GetDictionaries(const char** dicts);
	
	virtual int SetDictionary(const char* name);
	virtual int SetDictionaryByLanguage(const char* isolangtag);

	virtual bool MoreSuggestions(long ctr);

	virtual const SpError* CheckWord(const char* word);

private:
	int mTag;
	long mCheckLength;
	cdstring mErrorWord;
	cdstring mReplacement;
	cdstrmap mReplaceAll;
	cdstrvect mDictionaries;
	cdstring mBuffer;									// Temp buffer

	int DoSkip(bool skip_all);
	int DoReplace(bool replace_all);
};
