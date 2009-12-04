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


// CPreferenceEnum : class to implement a favourite item

#ifndef __CPREFERENCEENUM__MULBERRY__
#define __CPREFERENCEENUM__MULBERRY__

#include "CPreferenceItem.h"
#include "cdstring.h"

template <class T> class CPreferenceEnum : public CPreferenceItem
{
public:
	CPreferenceEnum() { mValue = static_cast<T>(0); }
	CPreferenceEnum(const CPreferenceEnum& copy)
		{ mValue = copy.mValue; }
	CPreferenceEnum(T copy)
		{ mValue = copy; }
	virtual ~CPreferenceEnum() {}
	
	CPreferenceEnum& operator=(const CPreferenceEnum& copy)		// Assignment with same type
		{ mValue = copy.mValue; return *this; }
	CPreferenceEnum& operator=(T copy)							// Assignment with same type
		{ mValue = copy; return *this; }

	int operator==(const CPreferenceEnum& comp) const			// Compare with same type
		{ return mValue == comp.mValue; }
	
	// Get/Set
	T GetValue() const
		{ return mValue; }

	// Read/write prefs
	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	T	mValue;

	const char** GetValues() const;
};

#endif
