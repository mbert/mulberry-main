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


#ifndef __CPARSERHTMLSTACK__MULBERRY__
#define __CPARSERHTMLSTACK__MULBERRY__

#include "ETag.h"
#include "cdustring.h"

#if __framework == __jx
#include <jColor.h>
#endif

class CParserHTMLStackElement
{
public:
	CParserHTMLStackElement(ETag, int);
	virtual ~CParserHTMLStackElement();

	ETag 						mTagid;
	int							mStart;
	cdustring					mParam;
	CParserHTMLStackElement*	mNext;
};



class CParserHTMLFontStackElement : public CParserHTMLStackElement
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CParserHTMLFontStackElement(ETag, int start, RGBColor color, int size, int font);
#elif __dest_os == __win32_os || __linux_os
	CParserHTMLFontStackElement(ETag, int start, RGBColor color, int size, const char* font);
#else
#error __dest_os
#endif
	virtual ~CParserHTMLFontStackElement();
	
	RGBColor	mColor;
	int			mSize;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	int			mFont;
#elif __dest_os == __win32_os  || __linux_os
	cdstring	mFont;
#else
#error __dest_os
#endif
	
	int GetRed()
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __linux_os
		{ return mColor.red; }
#elif __dest_os == __win32_os
		{ return GetRValue(mColor); }
#else
#error __dest_os
#endif
	int GetGreen()
#if __dest_os == __mac_os || __dest_os == __mac_os_x  || __linux_os
		{ return mColor.green; }
#elif __dest_os == __win32_os
		{ return GetGValue(mColor); }
#else
#error __dest_os
#endif
	int GetBlue()
#if __dest_os == __mac_os || __dest_os == __mac_os_x  || __linux_os
		{ return mColor.blue; }
#elif __dest_os == __win32_os
		{ return GetBValue(mColor); }
#else
#error __dest_os
#endif
	int GetSize()
		{ return mSize; }
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	int GetFont()
		{ return mFont; }
#elif __dest_os == __win32_os  || __linux_os
	const char* GetFont()
		{ return mFont.c_str(); }
#else
#error __dest_os
#endif
};


class CParserHTMLStack
{
public:
	CParserHTMLStack();
	~CParserHTMLStack();

	void deleteStack(CParserHTMLStackElement *);
	void push(CParserHTMLStackElement *element);

	CParserHTMLStackElement* pop();
	CParserHTMLStackElement* pop(ETag desired);

private:
	CParserHTMLStackElement* mStack;
};



#endif
