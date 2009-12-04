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

#ifndef _STPENSTATE_H
#define _STPENSTATE_H

#include <JPainter.h>

class StPenState
{
public:
	StPenState(JPainter *p)
	{
		mPainter = p;

		mLineWidth = p->GetLineWidth();
		mPenColor = p->GetPenColor();
		mFillFlag = p->IsFilling();
		mDrawDashedLinesFlag = p->LinesAreDashed();
		mFontSize = p->GetFontSize();
		mFontStyle = p->GetFontStyle();
		mFontID = p->GetFontID();
	}
	~StPenState()
	{
		mPainter->SetLineWidth(mLineWidth);
		mPainter->SetPenColor(mPenColor);
		mPainter->SetFilling(mFillFlag);
		mPainter->DrawDashedLines(mDrawDashedLinesFlag);
		mPainter->SetFont(mFontID, mFontSize, mFontStyle);
	}

private:
	JPainter*	mPainter;
	JSize		mLineWidth;
	JColorIndex	mPenColor;
	JBoolean	mFillFlag;
	JBoolean	mDrawDashedLinesFlag;
	JSize		mFontSize;
	JFontStyle	mFontStyle;
	JFontID		mFontID;
};

#endif
