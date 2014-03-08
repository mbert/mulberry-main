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


#include "CStaticText.h"

#include "cdstring.h"
#include "cdustring.h"

#include <jXConstants.h>
#include <jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

CStaticText::CStaticText
	(
	const JCharacter*	text,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXTEBase16(kStaticText, JI2B(w==0), kFalse, NULL,
			 enclosure, hSizing, vSizing, x,y,
			 (w>0 ? w : 100), (h>0 ? h : 100))
{
	SetBorderWidth(0);
	CStaticTextX(text, w,h);
}

CStaticText::CStaticText
	(
	const JCharacter*	text,
	const JBoolean		wordWrap,
	const JBoolean		selectable,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXTEBase16((selectable ? kSelectableText : kStaticText),
			 JNegate(wordWrap), kFalse, scrollbarSet,
			 enclosure, hSizing, vSizing, x,y,
			 (w>0 ? w : 100), (h>0 ? h : 100))
{
	assert( !wordWrap || w > 0 );
	CStaticTextX(text, w,h);
}

// private

void
CStaticText::CStaticTextX
	(
	const JCharacter*	text,
	const JCoordinate	origW,
	const JCoordinate	origH
	)
{
	SetDefaultFontSize(kJXDefaultFontSize);
	TESetLeftMarginWidth(kMinLeftMarginWidth);
	SetBreakCROnly(true);
	SetText(text);

	JCoordinate w = origW;
	JCoordinate h = origH;
	if (w == 0)
		{
		w = GetMinBoundsWidth() + 2*GetBorderWidth();
		SetSize(w, GetFrameHeight());
		}
	if (h == 0)
		{
		h = GetMinBoundsHeight() + 2*GetBorderWidth();
		SetSize(w, h);
		}
	
	mTransparent = false;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CStaticText::~CStaticText()
{
}

// Set all text from utf8
void CStaticText::SetText(const cdstring& all)
{
	cdustring utf16all(all);
	SetText(utf16all);
}

// Set all text from utf8
void CStaticText::SetText(const char* txt, size_t size)
{
	if ((txt != NULL) && (size == -1))
		size = ::strlen(txt);

	cdustring utf16all(txt, size);
	SetText(utf16all);
}

// Set all text from utf16
void CStaticText::SetText(const cdustring& all)
{
	JString16 temp(all);
	SetText16(temp);
	Refresh();
}

// Set number
void CStaticText::SetNumber(long num)
{
	cdstring txt(num);
	SetText(txt);
}

// Get all text as utf8
void CStaticText::GetText(cdstring& all) const
{
	cdustring utf16;
	GetText(utf16);
	all = utf16.ToUTF8();
}

// Get all text as utf8
cdstring CStaticText::GetText() const
{
	cdstring result;
	GetText(result);
	return result;
}

// Get all text as utf16
void CStaticText::GetText(cdustring& all) const
{
	all = GetText16();
}

void CStaticText::DrawBackground(JXWindowPainter& p, const JRect& frame)
{
	// Don't draw background when transparent
	if (!mTransparent)
		JXTEBase16::DrawBackground(p, frame);
}
