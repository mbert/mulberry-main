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


// CFocusBorder.cp - UI widget that implements a 3D divider

#include "CFocusBorder.h"

#include "StPenState.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <jXPainterUtil.h>

CFocusBorder::CFocusBorder(JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h)
	: JXDecorRect(enclosure, hSizing, vSizing, x,y, w,h)
{
	mFocussed = false;
	mHasFocus = true;
	mTransparent = false;
	SetBorderWidth(3);
}

void CFocusBorder::SetFocus(bool focus)
{
	if (mFocussed != focus)
	{
		mFocussed = focus;
		Refresh();
	}
}

void CFocusBorder::DrawBackground(JXWindowPainter& p, const JRect& origFrameG)
{
	if (!mTransparent)
		// Do inherited
		JXDecorRect::DrawBackground(p, origFrameG);
}

void CFocusBorder::DrawBorder(JXWindowPainter& p, const JRect&	frame)
{
	StPenState save(&p);

	if (mHasFocus && IsFocussed())
	{
		// Shrink frame by one as wide line is centered on the rect outline
		// but we want it inside
		JRect actual_frame(frame);
		actual_frame.Shrink(1, 1);
		p.SetLineWidth(3);
		p.SetPenColor((p.GetColormap())->GetBlueColor());
		p.JPainter::Rect(actual_frame);
	}
	else if (GetBorderWidth() > 0)
	{
		p.SetLineWidth(1);
		p.SetPenColor((p.GetColormap())->GetDefaultBackColor());
		p.JPainter::Rect(frame);

		JRect actual_frame(frame);
		actual_frame.Shrink(1, 1);
		JXDrawDownFrame(p, actual_frame, 2);
	}
}
