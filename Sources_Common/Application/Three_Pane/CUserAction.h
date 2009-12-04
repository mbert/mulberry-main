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


// Header for CUserAction class

#ifndef __CUSERACTION__MULBERRY__
#define __CUSERACTION__MULBERRY__

#include "CUserAction.h"

#include "CKeyAction.h"

// Classes

class CUserAction
{
public:
	CUserAction();
	CUserAction(const CUserAction& copy)
		{ _copy(copy); }

	CUserAction& operator=(const CUserAction& copy)		// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	static cdstring GetKeyDescriptor(unsigned char key, const CKeyModifiers& mods);

	void SetSelection(bool selection)
		{ mSelection = selection; }
	bool GetSelection() const
		{ return mSelection; }

	void SetSingleClick(bool click)
		{ mSingleClick = click; }
	bool GetSingleClick() const
		{ return mSingleClick; }
	CKeyModifiers& GetSingleClickModifiers()
		{ return mClickModifiers; }
	const CKeyModifiers& GetSingleClickModifiers() const
		{ return mClickModifiers; }

	void SetDoubleClick(bool click)
		{ mDoubleClick = click; }
	bool GetDoubleClick() const
		{ return mDoubleClick; }
	CKeyModifiers& GetDoubleClickModifiers()
		{ return mDoubleClickModifiers; }
	const CKeyModifiers& GetDoubleClickModifiers() const
		{ return mDoubleClickModifiers; }

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
	bool			mSelection;
	bool			mSingleClick;
	CKeyModifiers	mClickModifiers;
	bool			mDoubleClick;
	CKeyModifiers	mDoubleClickModifiers;

	unsigned char	mKey;
	CKeyModifiers	mKeyModifiers;
	
	void _copy(const CUserAction& copy);
};

#endif
