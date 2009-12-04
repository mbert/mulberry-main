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


// Header for CDataItem class

#ifndef __CDATAITEM__MULBERRY__
#define __CDATAITEM__MULBERRY__

#include "cdstring.h"

// Classes
class char_stream;

class CDataItemBase
{
public:
	CDataItemBase() {}
	virtual ~CDataItemBase() {}

	virtual cdstring GetInfo() const = 0;								// Get text expansion for prefs
	virtual void SetInfo(char_stream& info, NumVersion vers_prefs) = 0;	// Convert text to items

};

// Templatised version
template <class T> class CDataItem : public CDataItemBase
{
public:
	CDataItem() {}
	CDataItem(const T& data)
		{ mData = data; }
	CDataItem(const CDataItem& copy)
		{ mData = copy.mData; }
	virtual ~CDataItem() {}

	CDataItem& operator=(const CDataItem& copy)							// Assignment with same type
		{ if (this != &copy) { mData = copy.mData; } return *this; }
	
	const T& GetData() const
		{ return mData; }
	T& GetData()
		{ return mData; }
	void SetData(const T& data)
		{ mData = data; }

	virtual cdstring GetInfo() const ;								// Get text expansion for prefs
	virtual void SetInfo(char_stream& info, NumVersion vers_prefs);	// Convert text to items

private:
	T	mData;
};

#endif
