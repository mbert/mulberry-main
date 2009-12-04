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

// CVCard.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 03-Aug-2002
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a vCard object.
//
// History:
// 03-Aug-2002: Created initial header and implementation.
//

#ifndef __CVCARD_MULBERRY__
#define __CVCARD_MULBERRY__

#include "CVCardItem.h"

class CVCard
{
	typedef std::multimap<cdstring, CVCardItem> CVCardItems;

public:
	CVCard() {}
	~CVCard() {}
	
	CVCardItem& AddItem(const cdstring& name, const cdstring& value);
	void AddItem(const cdstring& name, const CVCardItem& item);

	unsigned long CountItems(const cdstring& name);
	const cdstring& GetValue(const cdstring& name);

	unsigned long CountItems(const cdstring& name, const cdstring& param_name, const cdstring& param_value);
	const cdstring& GetValue(const cdstring& name, const cdstring& param_name, const cdstring& param_value);
	
	unsigned long CountItems(const cdstring& name, const cdstrmap& params);
	const cdstring& GetValue(const cdstring& name, const cdstrmap& params);
	
	bool Read(std::istream& in);
	void Write(std::ostream& out);
	
private:
	CVCardItems		mItems;

	void ReadItem(cdstring& str, bool old_version);

	void WriteItem(std::ostream& out, const cdstring& name, const CVCardItem& item);

	// Encode/decode vCard formal syntax
	
	// text-value - utf8/escaped
	cdstring DecodeTextValue(const cdstring& str);
	cdstring EncodeTextValue(const cdstring& str);

	// addr-value - utf8/escaped
	cdstring DecodeTextAddrValue(const cdstring& str);
	cdstring EncodeTextAddrValue(const cdstring& str);

	// n-value - utf8/escaped
	cdstring DecodeTextNValue(const cdstring& str);
	cdstring EncodeTextNValue(const cdstring& str);
};

#endif
