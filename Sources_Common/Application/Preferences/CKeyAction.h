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


// Header for CKeyAction class

#ifndef __CKEYACTION__MULBERRY__
#define __CKEYACTION__MULBERRY__

#include "cdstring.h"

// Classes
#if __dest_os == __linux_os
class JXKeyModifiers;
#endif

class CKeyModifiers
{
public:
	enum EModifier
	{
		eShift = 0,
		eAlt,
		eCmd,
		eControl,
		eNumModifiers
	};

	CKeyModifiers();
	CKeyModifiers(const CKeyModifiers& copy)
		{ _copy(copy); }
	CKeyModifiers(int mods);								// Init from os modifiers
#if __dest_os == __linux_os
	CKeyModifiers(const JXKeyModifiers& mods);								// Init from os modifiers
#endif

	CKeyModifiers& operator=(const CKeyModifiers& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CKeyModifiers& comp) const;		// Compare with OS modifiers

	static cdstring GetModifiersDescriptor(const CKeyModifiers& mods);

	void Set(bool set, EModifier mod)
		{ mMods[mod] = set; }
	bool Get(EModifier mod) const
		{ return mMods[mod]; }

	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:			
	bool mMods[eNumModifiers];
	
	void _copy(const CKeyModifiers& copy);
};

class CKeyAction
{
public:
	CKeyAction();
	CKeyAction(unsigned char key, const CKeyModifiers& mods);
	CKeyAction(const CKeyAction& copy)
		{ _copy(copy); }

	CKeyAction& operator=(const CKeyAction& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CKeyAction& comp) const		// Compare with same type
		{ return (mKey == comp.mKey) && (mKeyModifiers == comp.mKeyModifiers); }

	static cdstring GetKeyDescriptor(const CKeyAction& key);

	void SetKey(unsigned char key)
		{ mKey = key; }
	unsigned char GetKey() const
		{ return mKey; }
	CKeyModifiers& GetKeyModifiers()
		{ return mKeyModifiers; }
	const CKeyModifiers& GetKeyModifiers() const
		{ return mKeyModifiers; }

	cdstring GetInfo() const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);

private:			
	unsigned char	mKey;
	CKeyModifiers	mKeyModifiers;
	
	void _copy(const CKeyAction& copy);
};

#endif
