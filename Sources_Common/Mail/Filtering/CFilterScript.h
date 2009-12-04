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


// Header for CFilterScript class

#ifndef __CFILTERSCRIPT__MULBERRY__
#define __CFILTERSCRIPT__MULBERRY__

#include "cdstring.h"
#include "prefsvector.h"

#include "CFilterItem.h"

// Classes
class CFilterScript;
typedef prefsptrvector<CFilterScript> CFilterScriptList;

class char_stream;
class CFilterManager;

class CFilterScript
{
public:
	CFilterScript();
	CFilterScript(const CFilterScript& copy)
		{ _copy(copy); }
	virtual ~CFilterScript()
		{ _tidy(); }

	CFilterScript& operator=(const CFilterScript& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }
	
	int operator==(const CFilterScript& comp) const			// Compare with same type
		{ return (this == &comp); }

	bool IsEnabled() const
		{ return mEnabled; }
	void SetEnabled(bool enabled)
		{ mEnabled = enabled; }

	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }

	const CFilterItems& GetFilters() const
		{ return mFilters; }
	CFilterItems& GetFilters()
		{ return mFilters; }

	bool AddFilter(CFilterItem* filter);
	void RemoveFilter(CFilterItem* filter);
	bool ContainsFilter(CFilterItem* filter) const;

	// SIEVE items
	void	GetSIEVEExtensions(CFilterProtocol::EExtension& ext) const;
	void	GetSIEVEScript(cdstring& txt, EEndl line_end) const;

	void	ConvertFilters(CFilterManager* mgr, bool delete_existing = true);

	virtual cdstring GetInfo(void) const;											// Get text expansion for prefs
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);					// Convert text to items

protected:
	bool				mEnabled;
	cdstring			mName;
	CFilterItems		mFilters;

private:
	void _copy(const CFilterScript& copy);
	void _tidy();
};

#endif
