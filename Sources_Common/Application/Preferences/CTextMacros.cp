/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CTextMacros.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

CTextMacros::CTextMacros()
{
	// Use shift-return by default
	CKeyModifiers mods;
	mods.Set(true, CKeyModifiers::eShift);
	mKey = CKeyAction('\r', mods);
}


CTextMacros::~CTextMacros()
{
}

int CTextMacros::operator==(const CTextMacros& comp) const
{
	return (mMacros == comp.mMacros) && (mKey == comp.mKey);
}

// Assignment with same type
void CTextMacros::_copy(const CTextMacros& copy)
{
	mMacros = copy.mMacros;
	mKey = copy.mKey;
}

// Add an item if not duplicate
bool CTextMacros::Add(const cdstring& name, const cdstring& macro)
{
	// Try to find it - fail if it already exists
	cdstrmap::const_iterator found = mMacros.find(name);
	if (found != mMacros.end())
		return false;
	
	// Now add it
	mMacros.insert(cdstrmap::value_type(name, macro));
	
	return true;
}

// Change an item if not duplicate
bool CTextMacros::Change(const cdstring& oldname, const cdstring& newname, const cdstring& macro)
{
	// Try to find the old name - fail if it does not exist
	cdstrmap::iterator found = mMacros.find(oldname);
	if (found == mMacros.end())
		return false;
	
	// Check for oldname and newname the same
	if (oldname == newname)
	{
		// Just change existing entry
		(*found).second = macro;
	}
	else
	{
		// See if new name exists - cannot overwrite it
		cdstrmap::iterator newfound = mMacros.find(newname);
		if (newfound != mMacros.end())
			return false;
		
		// Delete the old one and add the new one
		Delete(oldname);
		Add(newname, macro);
	}
		
	return true;
}

// Delete an item if not duplicate
bool CTextMacros::Delete(const cdstring& name)
{
	// Try to find it - fail if it does not exist
	cdstrmap::iterator found = mMacros.find(name);
	if (found == mMacros.end())
		return false;
	
	// Now remove it
	mMacros.erase(found);
	
	return true;
}

// See if entry already exists
bool CTextMacros::Contains(const cdstring& name) const
{
	// Count matches
	return mMacros.count(name) > 0;
}

// return matching macro if possible
const cdstring& CTextMacros::Lookup(const char* name) const
{
	// Try to find it
	cdstrmap::const_iterator found = mMacros.find(name);
	if (found != mMacros.end())
		return (*found).second;
	else	
		return cdstring::null_str;
}
	
// Read/write prefs
cdstring CTextMacros::GetInfo(void) const
{
	// Convert map to s-expression
	cdstring info;
	info.CreateSExpression(mMacros);
	info.ConvertFromOS();
	info += cSpace;

	// Write out the key
	info += mKey.GetInfo();
	return info;
}

bool CTextMacros::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	// Parse s-expression
	cdstring::ParseSExpression(info, mMacros, true);
	
	// Read in the key
	mKey.SetInfo(info, vers_prefs);
	return true;
}
