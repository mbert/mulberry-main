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


// Source for CPrintText class

#include "CPrintText.h"

#include "CMulberryCommon.h"

#include <UDrawingUtils.h>

// Constructor from stream
CPrintText::CPrintText(LStream *inStream) :
	LCaption(inStream)
{
}

// Default destructor
CPrintText::~CPrintText()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CPrintText::DrawSelf()
{
	Rect	frame;
	CalcLocalFrameRect(frame);

	SInt16	just = UTextTraits::SetPortTextTraits(mTxtrID);

	RGBColor	textColor;
	::GetForeColor(&textColor);

	ApplyForeAndBackColors();
	::RGBForeColor(&textColor);

	FontInfo	fontInfo;				// Determine vertical line spacing
	::GetFontInfo(&fontInfo);			//   based on font characteristics

	SInt16 lineHeight = (SInt16) (fontInfo.ascent + fontInfo.descent + fontInfo.leading);
	SInt16 lineBase   = (SInt16) (frame.top + fontInfo.ascent);

	::MoveTo(frame.left, lineBase);

	//::DrawClippedStringUTF8(mText, frame.right - frame.left, (just == teJustCenter) ? eDrawString_Center : eDrawString_Left);
	UTextDrawing::DrawWithJustification(&mText[0UL], mText.length(), frame, just, true);
}

