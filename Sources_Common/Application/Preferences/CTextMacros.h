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


// Text macros object

// Stores and looksup text macros

#ifndef __CTEXTMACROS__MULBERRY__
#define __CTEXTMACROS__MULBERRY__

#include "CPreferenceItem.h"

#include "cdstring.h"
#include "CKeyAction.h"

class CTextMacros : public CPreferenceItem
{
public:
	CTextMacros();
	CTextMacros(const CTextMacros& copy)
		{ _copy(copy); }
	virtual ~CTextMacros();

	CTextMacros& operator=(const CTextMacros& copy)					// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }
	int operator==(const CTextMacros& other) const;					// Compare with same type
	
	bool Add(const cdstring& name, const cdstring& macro);
	bool Change(const cdstring& oldname, const cdstring& newname, const cdstring& macro);
	bool Delete(const cdstring& name);

	bool Contains(const cdstring& name) const;
	const cdstring&	Lookup(const char* name) const;
	
	const cdstrmap&	GetMacros() const
		{ return mMacros; }
	cdstrmap&	GetMacros()
		{ return mMacros; }

	const CKeyAction& GetKeyAction() const
		{ return mKey; }
	CKeyAction& GetKeyAction()
		{ return mKey; }

	// Read/write prefs
	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:
	cdstrmap	mMacros;	
	CKeyAction	mKey;
	
	void _copy(const CTextMacros& copy);
};

#endif
