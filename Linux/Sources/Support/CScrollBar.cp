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


// CScrollBar.cp - UI widget that implements a 3D divider

#include "CScrollBar.h"

#include "CDrawUtils.h"

#include "HResourceMap.h"

#include <JXColormap.h>
#include <JXImage.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>

const JCoordinate kMinThumbLength       = 12;
//const JSize kScaleThumbShrink           = 5;
const JCoordinate kMinDoubleArrowFactor = 6;

JXImage* CScrollBar::sScrollUp = NULL;
JXImage* CScrollBar::sScrollDown = NULL;
JXImage* CScrollBar::sScrollLeft = NULL;
JXImage* CScrollBar::sScrollRight = NULL;

CScrollBar::CScrollBar(JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h)
	: JXScrollbar(enclosure, hSizing, vSizing, x,y, w,h)
{
	SetBorderWidth(0);
}

void CScrollBar::Draw(JXWindowPainter& p, const JRect&	rect)
{
	if (!sScrollUp)
		sScrollUp = iconFromResource(IDI_SCROLLBAR_UP, this, 16, 0x00CCCCCC);
	if (!sScrollDown)
		sScrollDown = iconFromResource(IDI_SCROLLBAR_DOWN, this, 16, 0x00CCCCCC);
	if (!sScrollLeft)
		sScrollLeft = iconFromResource(IDI_SCROLLBAR_LEFT, this, 16, 0x00CCCCCC);
	if (!sScrollRight)
		sScrollRight = iconFromResource(IDI_SCROLLBAR_RIGHT, this, 16, 0x00CCCCCC);

	if (IsHorizontal())
		{
		MyDrawHoriz(p);
		}
	else
		{
		MyDrawVert(p);
		}

	// draw the thumb

	CDrawUtils::DrawScrollBorder(p, itsThumbRect, false, true);
}

void CScrollBar::DrawBorder(JXWindowPainter& p, const JRect& frame)
{
	//JXScrollbar::DrawBorder(p, frame);
}

void CScrollBar::MyDrawHoriz(JPainter& p)
{
	p.SetFilling(kTrue);

	const JRect bounds = GetBounds();

	JCoordinate arrowWidth = bounds.height();
	itsDecrArrowRect[0] = JRect(bounds.top, bounds.left,
								bounds.bottom, bounds.left + arrowWidth);
	itsIncrArrowRect[0] = JRect(bounds.top, bounds.right - arrowWidth,
								bounds.bottom, bounds.right);

	if (bounds.width() > kMinDoubleArrowFactor * arrowWidth)
		{
		arrowWidth *= 2;
		itsDecrArrowRect[1] = JRect(bounds.top, bounds.right - arrowWidth,
									bounds.bottom, itsIncrArrowRect[0].left);
		itsIncrArrowRect[1] = JRect(bounds.top, itsDecrArrowRect[0].right,
									bounds.bottom, bounds.left + arrowWidth);
		}
	else
		{
		itsDecrArrowRect[1] = JRect(0,0,0,0);
		itsIncrArrowRect[1] = JRect(0,0,0,0);
		}

	// // available length - thumb length = max value - min value
	// thumb length = (page size / document size) * available length

	const JCoordinate availLength = bounds.width() - 2*arrowWidth;
	JCoordinate thumbLength =
//		availLength - (itsMaxValue - kMinValue + kScaleThumbShrink-1)/kScaleThumbShrink;
		(itsPageStepSize * availLength) / (itsMaxValue + itsPageStepSize - kMinValue);
	if (thumbLength < kMinThumbLength)
		{
		thumbLength = kMinThumbLength;
		}
	const JCoordinate minThumbLoc = bounds.left + arrowWidth;
	const JCoordinate maxThumbLoc = bounds.right - arrowWidth - thumbLength;
	JFloat thumbLocScale          = 0.0;
	if (itsMaxValue > kMinValue)	// avoid divide by zero
		{
		thumbLocScale =
			(maxThumbLoc - minThumbLoc)/(JFloat)(itsMaxValue - kMinValue);
		}
	JCoordinate thumbLoc =
		minThumbLoc + JRound(thumbLocScale * (itsValue - kMinValue));
	if (thumbLoc > maxThumbLoc)
		{
		thumbLoc = maxThumbLoc;
		}
	if (thumbLoc < minThumbLoc)
		{
		thumbLoc = minThumbLoc;
		}
	itsThumbRect = JRect(bounds.top, thumbLoc,
						 bounds.bottom, thumbLoc + thumbLength);
	itsThumbDragRect = JRect(bounds.top, bounds.left + arrowWidth,
							 bounds.bottom, bounds.right - arrowWidth);

	itsDecrementPageRect = JRect(bounds.top, itsThumbDragRect.left,
								 bounds.bottom, itsThumbRect.left);
	itsIncrementPageRect = JRect(bounds.top, itsThumbRect.right,
								 bounds.bottom, itsThumbDragRect.right);

	// draw the arrows

	const JSize borderWidth     = GetBorderWidth();
	const JColorIndex backColor = (p.GetColormap())->GetDefaultBackColor();

	if (itsDecrPushedFlag)
	{
		CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[0], true, true);
		p.JPainter::Image(*sScrollLeft, sScrollLeft->GetBounds(), itsDecrArrowRect[0].left, itsDecrArrowRect[0].top);
		if (!itsDecrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[1], true, true);
			p.JPainter::Image(*sScrollLeft, sScrollLeft->GetBounds(), itsDecrArrowRect[1].left, itsDecrArrowRect[1].top);
		}
	}
	else
	{
		CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[0], false, true);
		p.JPainter::Image(*sScrollLeft, sScrollLeft->GetBounds(), itsDecrArrowRect[0].left, itsDecrArrowRect[0].top);
		if (!itsDecrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[1], false, true);
			p.JPainter::Image(*sScrollLeft, sScrollLeft->GetBounds(), itsDecrArrowRect[1].left, itsDecrArrowRect[1].top);
		}
	}

	if (itsIncrPushedFlag)
	{
		CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[0], true, true);
		p.JPainter::Image(*sScrollRight, sScrollRight->GetBounds(), itsIncrArrowRect[0].left, itsIncrArrowRect[0].top);
		if (!itsIncrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[1], true, true);
			p.JPainter::Image(*sScrollRight, sScrollRight->GetBounds(), itsIncrArrowRect[1].left, itsIncrArrowRect[1].top);
		}
	}
	else
	{
		CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[0], false, true);
		p.JPainter::Image(*sScrollRight, sScrollRight->GetBounds(), itsIncrArrowRect[0].left, itsIncrArrowRect[0].top);
		if (!itsIncrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[1], false, true);
			p.JPainter::Image(*sScrollRight, sScrollRight->GetBounds(), itsIncrArrowRect[1].left, itsIncrArrowRect[1].top);
		}
	}

}

void CScrollBar::MyDrawVert(JPainter& p)
{
	p.SetFilling(kTrue);

	const JRect bounds = GetBounds();

	JCoordinate arrowHeight = bounds.width();
	itsDecrArrowRect[0] = JRect(bounds.top, bounds.left,
								bounds.top + arrowHeight, bounds.right);
	itsIncrArrowRect[0] = JRect(bounds.bottom - arrowHeight, bounds.left,
								bounds.bottom, bounds.right);

	if (bounds.height() > kMinDoubleArrowFactor * arrowHeight)
		{
		arrowHeight *= 2;
		itsDecrArrowRect[1] = JRect(bounds.bottom - arrowHeight, bounds.left,
									itsIncrArrowRect[0].top, bounds.right);
		itsIncrArrowRect[1] = JRect(itsDecrArrowRect[0].bottom, bounds.left,
									bounds.top + arrowHeight, bounds.right);
		}
	else
		{
		itsDecrArrowRect[1] = JRect(0,0,0,0);
		itsIncrArrowRect[1] = JRect(0,0,0,0);
		}

	// // available length - thumb length = max value - min value
	// thumb length = (page size / document size) * available length

	const JCoordinate availLength = bounds.height() - 2*arrowHeight;
	JCoordinate thumbLength =
//		availLength - (itsMaxValue - kMinValue + kScaleThumbShrink-1)/kScaleThumbShrink;
		(itsPageStepSize * availLength) / (itsMaxValue + itsPageStepSize - kMinValue);
	if (thumbLength < kMinThumbLength)
		{
		thumbLength = kMinThumbLength;
		}
	const JCoordinate minThumbLoc = bounds.top + arrowHeight;
	const JCoordinate maxThumbLoc = bounds.bottom - arrowHeight - thumbLength;
	JFloat thumbLocScale          = 0.0;
	if (itsMaxValue > kMinValue)	// avoid divide by zero
		{
		thumbLocScale =
			(maxThumbLoc - minThumbLoc)/(JFloat)(itsMaxValue - kMinValue);
		}
	JCoordinate thumbLoc =
		minThumbLoc + JRound(thumbLocScale * (itsValue - kMinValue));
	if (thumbLoc > maxThumbLoc)
		{
		thumbLoc = maxThumbLoc;
		}
	if (thumbLoc < minThumbLoc)
		{
		thumbLoc = minThumbLoc;
		}
	itsThumbRect = JRect(thumbLoc, bounds.left,
						 thumbLoc + thumbLength, bounds.right);
	itsThumbDragRect = JRect(bounds.top + arrowHeight, bounds.left,
							 bounds.bottom - arrowHeight, bounds.right);

	itsDecrementPageRect = JRect(itsThumbDragRect.top, bounds.left,
								 itsThumbRect.top, bounds.right);
	itsIncrementPageRect = JRect(itsThumbRect.bottom, bounds.left,
								 itsThumbDragRect.bottom, bounds.right);

	// draw the arrows

	const JSize borderWidth     = GetBorderWidth();
	const JColorIndex backColor = (p.GetColormap())->GetDefaultBackColor();

	if (itsDecrPushedFlag)
	{
		CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[0], true, true);
		p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsDecrArrowRect[0].left, itsDecrArrowRect[0].top);
		if (!itsDecrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[1], true, true);
			p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsDecrArrowRect[1].left, itsDecrArrowRect[1].top);
		}
	}
	else
	{
		CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[0], false, true);
		p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsDecrArrowRect[0].left, itsDecrArrowRect[0].top);
		if (!itsDecrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsDecrArrowRect[1], false, true);
			p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsDecrArrowRect[1].left, itsDecrArrowRect[1].top);
		}
	}
	if (itsIncrPushedFlag)
	{
		CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[0], true, true);
		p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsIncrArrowRect[0].left, itsIncrArrowRect[0].top);
		if (!itsIncrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[1], true, true);
			p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsIncrArrowRect[1].left, itsIncrArrowRect[1].top);
		}
	}
	else
	{
		CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[0], false, true);
		p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsIncrArrowRect[0].left, itsIncrArrowRect[0].top);
		if (!itsIncrArrowRect[1].IsEmpty())
		{
			CDrawUtils::DrawScrollBorder(p, itsIncrArrowRect[1], false, true);
			p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsIncrArrowRect[1].left, itsIncrArrowRect[1].top);
		}
	}

}
