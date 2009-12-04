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


#ifndef __CPARSERHTML__MULBERRY__
#define __CPARSERHTML__MULBERRY__


#include "CParserHTMLStack.h"

#include "cdustring.h"

class CFormatList;
class CClickList;
class cdstring;
class CAnchorClickElement;
typedef std::map<cdstring, CAnchorClickElement> CAnchorElementMap;
#if __dest_os == __linux_os
#include <JPtrArray.h>
class JString;
#endif

class CParserHTML
{
public:
	CParserHTML(const unichar_t* st, CFormatList* format, CClickList* click, CAnchorElementMap* anchor, bool use_styles = true);
	CParserHTML(CFormatList*, CClickList*, CAnchorElementMap*);
	CParserHTML();
	~CParserHTML();
	
	RGBColor	GetBackgroundColor() const
		{ return mBackground; }
	RGBColor	GetTextColor() const
		{ return mTextColor; }

	void		SetFontScale(long scale);

	static long FindParam(const unichar_t* param, cdustring& response, bool quoted = false);
	static bool GetLatinChar(unichar_t* format, std::ostream* out, unsigned long* added);
	static bool GetLatinChar(wchar_t charNum, std::ostream* out, unsigned long* added);
	static bool HandleAmpChar(unichar_t* format, std::ostream* out, unsigned long* added);
	
	static long FindParamUTF8(const char* param, cdstring& response, bool quoted = false);
	static bool GetLatinCharUTF8(char* format, std::ostream* out, unsigned long* added);
	static bool GetLatinCharUTF8(wchar_t charNum, std::ostream* out, unsigned long* added);
	static bool HandleAmpCharUTF8(char* format, std::ostream* out, unsigned long* added);
	
	const unichar_t* Parse(int offset, bool for_display, bool quote = false, bool forward = false);
	void RawParse(int offset);

private:
	CParserHTMLStack	mHStack;
	CFormatList*		mFormatList;
	CClickList*			mClickList;
	CAnchorElementMap*	mAnchorMap;
	bool				mUseStyles;
	bool				mDoAlignment;
	int					mStrLen;
	unichar_t*			mTxt;
	cdustring			mBaseScheme;
	cdustring			mBaseLocation;
	int					mBaseFontSize;
	int					mStyledFontSize;
	int					mFixedFontSize;
	cdstring			mFixedFontName;
	RGBColor			mBackground;
	RGBColor			mTextColor;
#if __dest_os == __win32_os
	int					mLogpix;
#elif __dest_os == __linux_os
	JPtrArray<JString>	mFontNames;
#endif

	ETag GetTag(unichar_t* format, long& offset);

	void InitParserHTML();

	void HandleBody(unichar_t* param);
	void HandleHeading(long start, long stop, ETag tag, unichar_t *param);
	void HandleFormat(unichar_t* format,int index);
	void HandleImage(unichar_t* param, std::ostream* out, unsigned long* added);
	void HandleFont(long start, long stop, unichar_t* param);
	void HandleAnchor(long start, long stop, unichar_t* param);
	void HandleScaledSize(long start, long stop, long relsize, bool fixed = false);

	void FlushStack(int index);
	
	bool ParseURL(unichar_t* param, cdustring &scheme, cdustring &location);
	bool ParseAnchor(unichar_t* param, cdustring &location);
	void ConvertAmpChar(cdustring& txt);
	void DrawURL(long start, long stop);
	void DrawTag(long start, long stop);
	
	const unichar_t* CommentTag(const unichar_t* start, const char* tag, size_t len);
};



#endif
