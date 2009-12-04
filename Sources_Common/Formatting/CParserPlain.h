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


#ifndef __CPARSERPLAIN__MULBERRY__
#define __CPARSERPLAIN__MULBERRY__

#include "CFormatList.h"
#include "CUStringUtils.h"

#include "cdustring.h"
#include "templs.h"

class CClickList;

class CParserPlain
{
 public:
	CParserPlain(const unichar_t* p, CFormatList* format, CClickList* clicks, bool delete_p = true);
	~CParserPlain();

	void SetQuoteDepth(long quote)
		{ mQuoteDepth = quote; }

	const unichar_t* Parse(int offset);

	void AddURL(const char* value);
	void LookForURLs(int);

	void Probe(int char_offset, int offset, char delimiter);

 private:	 
	CFormatList*			mFormatList;
	CClickList*				mClickList;
	cdustrvect				mFindURLS;
	const unichar_t* 		mTxt;
	bool 					mDeleteData;
	int 					mStrLen;
	long					mQuoteDepth;

	long GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes);
	void DrawURL(SInt32 start, SInt32 stop);

};

#endif
