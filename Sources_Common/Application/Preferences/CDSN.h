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


// CDisplayItem : class to implement a favourite item

#ifndef __CDSN__MULBERRY__
#define __CDSN__MULBERRY__

#include "cdstring.h"

#include <ostream>

class CDSN
{
public:
	CDSN();
	CDSN(const CDSN& copy)
		{ _copy(copy); }
	~CDSN() {}
	
	CDSN& operator=(const CDSN& copy)					// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CDSN& comp) const;						// Compare with same type

	bool GetMDN() const
		{ return mMDN; }
	void SetMDN(bool set)
		{ mMDN = set; }

	bool GetRequest() const
		{ return mRequest; }
	void SetRequest(bool set)
		{ mRequest = set; }

	bool GetSuccess() const
		{ return mSuccess; }
	void SetSuccess(bool set)
		{ mSuccess = set; }

	bool GetFailure() const
		{ return mFailure; }
	void SetFailure(bool set)
		{ mFailure = set; }

	bool GetDelay() const
		{ return mDelay; }
	void SetDelay(bool set)
		{ mDelay = set; }

	bool GetFull() const
		{ return mFull; }
	void SetFull(bool set)
		{ mFull = set; }

	void WriteHeaderToStream(std::ostream& out) const;
	void ReadHeader(char* p);

	// Read/write prefs
	cdstring GetInfo(void) const;
	bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	bool		mMDN;
	bool		mRequest;
	bool		mSuccess;
	bool		mFailure;
	bool		mDelay;
	bool		mFull;
	
	void _copy(const CDSN& copy);
};

#endif
