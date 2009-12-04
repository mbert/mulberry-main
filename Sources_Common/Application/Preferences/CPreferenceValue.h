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


// CPreferenceValue : template class to handle preferences values

#ifndef __CPREFERENCEVALUE__MULBERRY__
#define __CPREFERENCEVALUE__MULBERRY__

#include "COptionsMap.h"

struct SPrefsACL
{
	enum
	{
		ePrefsACL_None = 0,
		ePrefsACL_Dirty = 1L << 0,
		ePrefsACL_ReadOnly = 1L << 1
	};
	
	long mRights;
	
	SPrefsACL()
		{ mRights = ePrefsACL_None; }
	SPrefsACL(const SPrefsACL& copy)
		{ mRights = copy.mRights; }

	SPrefsACL& operator=(const SPrefsACL& copy)
		{ mRights = copy.mRights; return *this; }

	bool IsDirty() const
		{ return ((mRights & ePrefsACL_Dirty) != 0); }
	void SetDirty(bool dirty)
		{ mRights = (dirty ? (mRights | ePrefsACL_Dirty) : (mRights & ~ePrefsACL_Dirty)); }

	bool IsReadOnly() const
		{ return ((mRights & ePrefsACL_ReadOnly) != 0); }
	void SetReadOnly(bool read_only)
		{ mRights = (read_only ? (mRights | ePrefsACL_Dirty) : (mRights & ~ePrefsACL_Dirty)); }
};

class cdstring;

class COptionsMap;

template <class T> class CPreferenceValue
{
	friend class CPreferences;

public:
	CPreferenceValue() {}
	explicit CPreferenceValue(const T& value)
		{ mValue = value; }
	explicit CPreferenceValue(const CPreferenceValue<T>& copy)
		{ mValue = copy.mValue;
		  mACL = copy.mACL; }
	~CPreferenceValue() {}
	
	CPreferenceValue<T>& operator=(const CPreferenceValue<T>& copy)		// assign to self
		{ mValue = copy.mValue;
		  mACL = copy.mACL;
		  return *this; }

	int operator==(const CPreferenceValue<T>& comp) const				// compare with same
		{ return (mValue == comp.mValue); }
	int operator!=(const CPreferenceValue<T>& comp) const				// compare with same
		{ return !operator==(comp); }

	const T& GetValue() const
		{ return mValue; }
	T& Value()
		{ return mValue; }
	void SetValue(const T& value, bool dirty = true)
		{ if (!(mValue == value)) SetDirty(dirty);
			mValue = value; }
	
	bool IsDirty() const
		{ return mACL.IsDirty(); }
	void SetDirty(bool dirty = true)
		{ mACL.SetDirty(dirty); }

	bool IsReadOnly() const
		{ return mACL.IsReadOnly(); }
	void SetReadOnly(bool read_only = true)
		{ mACL.SetReadOnly(read_only); }

protected:
	T			mValue;
	SPrefsACL	mACL;
};

template <class T> class CPreferenceValueMap : public CPreferenceValue<T>
{
	friend class CPreferences;

public:
	CPreferenceValueMap() {}
	explicit CPreferenceValueMap(const T& value)
		: CPreferenceValue<T>(value)
		{}
	explicit CPreferenceValueMap(const CPreferenceValueMap<T>& copy)
		: CPreferenceValue<T>(copy)
		{}
	~CPreferenceValueMap() {}

	void 	WriteToMap(const cdstring& key,
						COptionsMap* theMap,
						bool dirty_only)								// Write data to a stream
		{ if (!dirty_only || this->IsDirty())
			{ theMap->WriteValue(key, this->mValue); this->SetDirty(false); } }

	bool	ReadFromMap(const cdstring& key,
							COptionsMap* theMap,
							NumVersion& vers_prefs)						// Read data from a stream
		{ bool result = theMap->ReadValue(key, this->mValue, vers_prefs);
			this->SetDirty(!result);				// Mark as dirty if not read in
			return result; }
};

template <class T> class CPreferenceValueInt : public CPreferenceValue<T>
{
	friend class CPreferences;

public:
	CPreferenceValueInt() {}
	explicit CPreferenceValueInt(const T& value)
		: CPreferenceValue<T>(value)
		{}
	explicit CPreferenceValueInt(const CPreferenceValueInt<T>& copy)
		: CPreferenceValue<T>(copy)
		{}
	~CPreferenceValueInt() {}

	void 	WriteToMap(const cdstring& key,
						COptionsMap* theMap,
						bool dirty_only)								// Write data to a stream
		{ if (!dirty_only || this->IsDirty())
			{ theMap->WriteValue(key, (int)this->mValue); this->SetDirty(false); } }

	bool	ReadFromMap(const cdstring& key,
							COptionsMap* theMap,
							NumVersion& vers_prefs)						// Read data from a stream
		{ this->SetDirty(false);
		  int temp;
		  bool result = theMap->ReadValue(key, temp, vers_prefs);
		  if (result) this->mValue = (T) temp;
		  return result;}
};

#endif
