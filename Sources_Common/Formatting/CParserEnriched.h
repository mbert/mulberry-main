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


#ifndef __CPARSERENRICHED__MULBERRY__
#define __CPARSERENRICHED__MULBERRY__

#include "cdstring.h"
#include "cdustring.h"
#include "CFormatList.h"
#include "CParserEnrichedStack.h"
#include "ETag.h"

class CParserEnriched
{
public:
	CParserEnriched(const unichar_t* st, CFormatList* format, bool use_styles = true);
	CParserEnriched(CFormatList*);
	CParserEnriched();
	~CParserEnriched();

	void	SetFontScale(long scale);

	const unichar_t* Parse(int offset, bool for_display, bool quote = false, bool forward = false);
	void RawParse(int offset);

private:
	CFormatList* 			mFormatList;
	bool					mUseStyles;
	bool					mDoAlignment;
	int 					mStrLen;
	unichar_t* 				mTxt;
	int 					mBaseFontSize;
	int 					mStyledFontSize;
	int 					mFixedFontSize;
	cdstring 				mFixedFontName;
	CParserEnrichedStack*	mStack;
	
	void InitParserEnriched();

	void HandleFormat(unichar_t* format, int index);
	void HandleScaledSize(long start, long stop, long relsize, bool fixed = false);

	void FlushStack(int index);
	ETag GetTag(unichar_t* format);
	void DrawTag(int start, int stop);
};

#endif
