/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CDisplayFormatter : header file
//

#ifndef __CDISPLAYFORMATTER__MULBERRY__
#define __CDISPLAYFORMATTER__MULBERRY__

#include "ETag.h"
#include "CMIMESupport.h"
#include "CCmdEditView.h"

#include "cdustring.h"

/////////////////////////////////////////////////////////////////////////////
// CDisplayFormatter 


class CFormattedTextDisplay;
class CLetterTextEditView;

class CFormatList;
class CClickList;
class CClickElement;
class CAnchorClickElement;
typedef std::map<cdstring, CAnchorClickElement> CAnchorElementMap;

class CDisplayFormatter
{
	friend class CFormattedTextDisplay;
	friend class CLetterTextEditView;

// Construction
public:
	CDisplayFormatter(CFormattedTextDisplay* disp);
	CDisplayFormatter(CLetterTextEditView* disp);
	virtual ~CDisplayFormatter();

protected:
	struct SLineInfo
	{
		long start;
		long stop;
		long depth;
	};

	CFormatList*		mList;
	CFormatList*		mOverrideList;
	CClickList*			mClickList;
	CAnchorElementMap*	mAnchorMap;
	EView 				mView;
	long				mFontScale;
	RGBColor			mBackground;
	RGBColor			mTextColor;

	cdustring			mText;
	
	CFormattedTextDisplay*	mMessageText;
	CLetterTextEditView*	mLetterText;

	void ResetColors();
		
public:
	void FaceFormat(ETag tagid);
	void FaceFormat(ETag tagid, SInt32 start, SInt32 stop);
	void FontFormat(const char* font);
	void FontFormat(const char* font, SInt32 start, SInt32 stop);
	void ColorFormat(RGBColor color);
	void ColorFormat(RGBColor color, SInt32 start, SInt32 stop);
	void FontSizeFormat(short size, bool adding, bool overrideprefs = false);
	void FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs = false);
	void AlignmentFormat(ETag tagid);
	void AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop);
	void ExcerptFormat(char Excerpt, SInt32 start, SInt32 stop);
	bool GetLineRange(SInt32 &first, SInt32 &last, SInt32 start, SInt32 stop);
	
	DWORD GetContinuousCharSelection(CHARFORMAT &currentFormat);
	DWORD GetContinuousParaSelection(PARAFORMAT &currentFormat);
	
	bool LaunchURL(const cdstring& url);
	CFormatList* GetList()
		{ return mList; }
	bool DoAnchor(const CClickElement* anchor);

	void DrawHeader(SInt32, SInt32);
	void DrawHeaderDirect(SInt32, SInt32);
	void ParseHeader(const char* header, EView parsing);
	void InsertFormattedHeader(const char* header);
	void InsertFormatted(EView parsing);

	void InsertFormattedText(const unichar_t* body, EContentSubType subtype, bool quote, bool forward);

	const unichar_t*  ParseBody(const unichar_t* body, EContentSubType, EView parsing, long quote, bool use_styles = true);
	const cdustring& GetParsedText() const
	{
		return mText;
	}
	int CharFromPos(CPoint point) const;
	CClickElement* FindCursor(int po);

	void DoQuotation(void);
	void DrawQuotation(long start, long stop, long depth);
	long GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes);

	CTextDisplay& GetTextDisplay();
	CRichEditCtrl& GetRichEditCtrl();

protected:
	void Reset(bool click = false, long scale = 0);
};

/////////////////////////////////////////////////////////////////////////////

#endif
