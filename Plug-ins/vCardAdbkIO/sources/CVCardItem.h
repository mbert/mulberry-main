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

// CVCardItem.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 03-Aug-2002
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a vCard object item.
//
// History:
// 03-Aug-2002: Created initial header and implementation.
//

#ifndef __CVCARDITEM_MULBERRY__
#define __CVCARDITEM_MULBERRY__

#include "cdstring.h"

class CVCardItem
{
public:
	CVCardItem() {}
	CVCardItem(const cdstring& val) : mValue(val) {}
	CVCardItem(const CVCardItem& copy)
		{ _copy(copy); }
	~CVCardItem() {}
	
	void AddParam(const cdstring& name, const cdstring& value)
		{ mParams.insert(cdstrmultimap::value_type(name, value)); }
	cdstrmultimap& Params()
		{ return mParams; }
	const cdstrmultimap& GetParams() const
		{ return mParams; }
	
	cdstring& Value()
		{ return mValue; }
	const cdstring& GetValue() const
		{ return mValue; }
	void SetValue(const cdstring& val)
		{ mValue = val; }
	
private:
	cdstrmultimap	mParams;
	cdstring		mValue;
	
	void _copy(const CVCardItem& copy)
		{ mParams = copy.mParams; mValue = copy.mValue; }
};

#endif
