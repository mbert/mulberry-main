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


#ifndef __CFORMATTEDTEXTDISPLAY__MULBERRY__
#define __CFORMATTEDTEXTDISPLAY__MULBERRY__


#include "CSpacebarEditView.h"

#include "CMIMEContent.h"

#include "ETag.h"
#include "cdstring.h"
#include "cdustring.h"
#include "templs.h"

#include <JColorList.h>

class CFormatList;
class CClickList;
class CClickElement;
class CAnchorClickElement;
typedef std::map<cdstring, CAnchorClickElement> CAnchorElementMap;

class CFormattedTextDisplay : public CSpacebarEditView
{
public:
	CFormattedTextDisplay(JXContainer* enclosure,
			const HSizingOption hSizing, const VSizingOption vSizing,
			const JCoordinate x, const JCoordinate y,
			const JCoordinate w, const JCoordinate h);
	virtual ~CFormattedTextDisplay();

	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	void ParseHeader(const char* header, EView parsing);
	void InsertFormattedHeader(const char* header);
	void InsertFormatted(EView parsing);

	void InsertFormattedText(const unichar_t* body, EContentSubType subtype, bool quote, bool forward);

	const unichar_t*  ParseBody(const unichar_t* body, EContentSubType, EView parsing, long quote, bool use_styles = true);
	const cdustring& GetParsedText() const
	{
		return mText;
	}

	void DoQuotation(void);
	long GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes);
	void DrawQuotation(long start, long stop, long depth);
	void DrawHeader(SInt32 start, SInt32 stop);
	void DrawHeaderDirect(SInt32, SInt32);
	
	bool DoAnchor(const CClickElement* anchor);

	//empty out text
	void Reset(bool click = false, long scale = 0);
 
	void FaceFormat(ETag tagid);
	void FaceFormat(ETag tagid, SInt32 start, SInt32 stop);
	void FontFormat(const char* font);
	void FontFormat(const char* font, SInt32 start, SInt32 stop);
	void ColorFormat(JRGB color);
	void ColorFormat(JRGB color, SInt32 start, SInt32 stop);
	void FontSizeFormat(short size, bool adding, bool overrideprefs = false);
	void FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs = false);
	void AlignmentFormat(ETag tagid);
	void AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop);
	void ExcerptFormat(char Excerpt, SInt32 start, SInt32 stop);

	bool LaunchURL(const cdstring& url);

protected:
	struct lineInfo
	{
		long start;
		long stop;
		long depth;
	};

	CFormatList*		mList;
	CClickList*			mClickList;
	CFormatList*		mOverrideList;
	JColorIndex			mBackground;
	JColorIndex			mTextColor;
	cdustring			mText;
	bool				mHandleClick;
	CClickElement*		mClickElement;
	CClickElement*		mContextClickElement;
	CAnchorElementMap*	mAnchorMap;
	bool				mHasTooltip;
	EView				mView;
	long				mFontScale;
	JCursorIndex		mLinkCursor;
	JXTextMenu*			mURLPopup;

	JRunArray<Font> 	format;
	JColorList			mColorList;

	virtual void	AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers);
	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void	HandleMouseUp(const JPoint& pt, const JXMouseButton button,
								  const JXButtonStates& buttonStates,
								  const JXKeyModifiers& modifiers);
	CClickElement* FindCursor(int po);

	virtual void 	ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	void ResetColors();

	void OnOpenLink();
	void OnCopyLink();
};

typedef CFormattedTextDisplay CDisplayFormatter;

#endif
