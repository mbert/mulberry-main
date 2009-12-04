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

#include "CTextDisplay.h"

#include "CMessageWindow.h"
#include "CMIMESupport.h"

#include "cdstring.h"
#include "cdustring.h"
#include "templs.h"
#include "ETag.h"

class CFormatList;
class CClickElement;
class CClickList;
class CAnchorClickElement;
typedef std::map<cdstring, CAnchorClickElement> CAnchorElementMap;

class CFormattedTextDisplay : public CTextDisplay
{
public:
	enum { class_ID = 'FTDi' };
	
	CFormattedTextDisplay(LStream* inStream);						
	virtual ~CFormattedTextDisplay();

	void Reset(bool click = false, long scale = 0);

	void DrawHeader(SInt32, SInt32);
	void DrawHeaderDirect(SInt32, SInt32);
	void ParseHeader(const char* header, EView parsing);			// UTF8 in
	void InsertFormattedHeader(const char* header);					// UTF8 in
	void InsertFormatted(EView parsing);
	const unichar_t* ParseBody(const unichar_t* body, EContentSubType, EView parsing, long quote, bool use_styles = true);	// UTF16 in
	const cdustring& GetParsedText() const
	{
		return mText;
	}

	void InsertFormattedText(const unichar_t* body, EContentSubType subtype, bool quote, bool forward);					// UTF16 in

	virtual	Boolean ObeyCommand(CommandT inCommand,
							void* ioParam);							// Handle commands our way
	virtual void FindCommandStatus(CommandT inCommand,
						Boolean &outEnabled, Boolean &outUsesMark,
						UInt16 &outMark, Str255 outName);			// Handle menus our way
	virtual bool HandleContextMenuEvent(const EventRecord& cmmEvent);

	virtual void AdjustMouseSelf(Point, const EventRecord&, RgnHandle);
	virtual void MouseLeave();

	virtual void DeactivateSelf();
	virtual void ClickSelf(const SMouseDownEvent& inMouseDown);

	CFormatList* GetList()
		{ return mList; }
	bool DoAnchor(const CClickElement* anchor);

	void FaceFormat(ETag tagid, SInt32 start, SInt32 stop);
	void FontFormat(char* font, SInt32 start, SInt32 stop);
	void ColorFormat(RGBColor color, SInt32 start, SInt32 stop);
	void FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs = false);
	void AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop);
	void ExcerptFormat(char Excerpt, SInt32 start, SInt32 stop);
	bool GetLineRange(SInt32 &first, SInt32 &last, SInt32 start, SInt32 stop);
	bool LaunchURL(const char* url);
	virtual void DoQuotation(void);
	void DrawQuotation(long start, long stop, long depth);
	long GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes);

protected:
	static LMenu*		sURLContextMenu;
	CFormatList*		mList;
	CFormatList*		mOverrideList;
	CClickList*			mClickList;
	CClickElement*		mClickElement;
	CAnchorElementMap*	mAnchorMap;
	bool				mHasTooltip;
	RGBColor			mBackground;
	RGBColor			mTextColor;
	cdustring			mText;
	bool				mHandleClick;
	EView				mView;
	long				mFontScale;

	void ResetColors();

	void CaptureAddress();

private:
	void InitFormattedTextDisplay();
	
};

struct lineInfo
{
	long start;
	long stop;
	long depth;
};

#endif
