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


// CToolbarButton.cpp : implementation file
//


#include "CToolbarButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"

#include "StPenState.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <JXImage.h>

#include <JXDisplay.h>
#include <JXWindow.h>

/////////////////////////////////////////////////////////////////////////////
// CToolbarButton

CToolbarButton::CToolbarButton(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: JXTextButton(label, enclosure, hSizing, vSizing, x,y, w,h)
{
	itsIsCheckedFlag = kFalse;
	mImageID = 0;
	mImageSelID = 0;
	mTracking = false;
	mSmallIcon = false;
	mShowIcon = true;
	mShowCaption = true;
	mClickAndPopup = false;
	mDragMode = false;
	SetBorderWidth(0);
}

CToolbarButton::~CToolbarButton()
{
}

const int cTitleHeight = 12;
const int cTitleOffset = 2;
const int cClickAndPopupWidth = 12;

void CToolbarButton::SetState(const JBoolean on)
{
	if (itsIsCheckedFlag != on)
	{
		itsIsCheckedFlag = on;
		Refresh();
	}
}

bool CToolbarButton::HasPopup() const
{
	// Not this class - derived class will have popup
	return false;
}

void CToolbarButton::SetSmallIcon(bool small_icon)
{
	if (mSmallIcon ^ small_icon)
	{
		mSmallIcon = small_icon;
		Refresh();
	}
}

void CToolbarButton::SetShowIcon(bool icon)
{
	if (mShowIcon ^ icon)
	{
		mShowIcon = icon;
		Refresh();
	}
}

void CToolbarButton::SetShowCaption(bool caption)
{
	if (mShowCaption ^ caption)
	{
		mShowCaption = caption;
		Refresh();
	}
}

void CToolbarButton::HandleMouseEnter()
{
	mTracking = true && !mDragMode;
	JXTextButton::HandleMouseEnter();
	Redraw();
}

void CToolbarButton::HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers)
{
	JXTextButton::HandleMouseHere(pt, modifiers);
}

void CToolbarButton::HandleMouseLeave()
{
	mTracking = false;
	JXTextButton::HandleMouseLeave();
	Redraw();
}

void CToolbarButton::Draw(JXWindowPainter& p, const JRect& rect)
{
	JRect actual_rect(GetBounds());

	// Draw frame then content
	DrawFrame(p, actual_rect);
	DrawContent(p, actual_rect);
}

// Draw the appropriate button frame
void CToolbarButton::DrawFrame(JXWindowPainter& p, const JRect& rect)
{
	// Check selected state/enabled state (adjust for toggle operation)
	bool enabled = IsActive();
	bool selected = IsChecked() && !mDragMode;
	bool pushed_frame = (IsChecked() ^ IsPushed()) && !mDragMode;

	if (enabled && (pushed_frame || mTracking))
	{
		// Standard frame
		CDrawUtils::DrawSimpleBorder(p, rect, pushed_frame, enabled);
		
		// Special line for click-and-popup
		if (GetClickAndPopup())
		{
			JRect lrect = rect;
			lrect.left = lrect.right - cClickAndPopupWidth;
			lrect.top += 2;
			lrect.bottom -= 2;
			CDrawUtils::DrawSimpleLine(p, lrect, true, pushed_frame, enabled);
		}
	}	
	else
		CDrawUtils::DrawBackground(p, rect, false, true);
}

// Draw the appropriate icon
void CToolbarButton::DrawContent(JXWindowPainter& p, const JRect& rect)
{
	if (mShowIcon)
		DrawIcon(p, rect);

	if (mShowCaption)
		DrawTitle(p, rect);

	if (HasPopup())
		DrawPopupGlyph(p, rect);
}

// Draw the appropriate icon
void CToolbarButton::DrawIcon(JXWindowPainter& p, const JRect& rect)
{
	// Draw 3D frame
	StPenState save(&p);

	// Icon size
	JSize icon_size = mSmallIcon ? 16 : 32;

	// Adjust rect for click-and -popup
	JRect lrect = rect;
	if (GetClickAndPopup())
		lrect.right -= cClickAndPopupWidth;
	
	// Check selected state/enabled state (adjust for toggle operation)
	bool enabled = IsActive();
	bool selected = IsChecked() && !mDragMode;
	bool pushed_frame = (IsChecked() ^ IsPushed()) && !mDragMode;

	JXImage* draw_icon = NULL;
	if (enabled)
		draw_icon = CIconLoader::GetIcon(selected && mImageSelID ? mImageSelID : mImageID, this, mSmallIcon ? 16 : 32, pushed_frame ? 0x00808080 : 0x00CCCCCC);
	else
		draw_icon = CIconLoader::GetIcon(mImageID, this, mSmallIcon ? 16 : 32, 0x00CCCCCC, CIconLoader::eDisabled);

	// Always center horizontally
	int hpos = (lrect.right - lrect.left - icon_size) / 2;
	if (HasPopup() && !GetClickAndPopup()&& !mShowCaption)
		hpos -= 2;
	
	// Center vertically if no caption
	int vpos = lrect.top + (mSmallIcon ? 2 : 0);

	p.JPainter::Image(*draw_icon, draw_icon->GetBounds(), hpos, vpos);
}

// Draw the appropriate text
void CToolbarButton::DrawTitle(JXWindowPainter& p, const JRect& rect)
{
	// Draw 3D frame
	StPenState save(&p);

	// Check selected state/enabled state (adjust for toggle operation)
	bool enabled = IsActive();
	bool selected = IsChecked() && !mDragMode;
	bool pushed_frame = (IsChecked() ^ IsPushed()) && !mDragMode;

	// Adjust rect for click-and -popup
	JRect lrect = rect;
	if (GetClickAndPopup())
		lrect.right -= cClickAndPopupWidth;
	lrect.top = lrect.bottom - cTitleHeight - cTitleOffset;
	lrect.bottom -= cTitleOffset;

	if (selected && enabled)
		itsFontStyle.color = p.GetColormap()->GetWhiteColor();
	else if (enabled)
		itsFontStyle.color = p.GetColormap()->GetBlackColor();
	else
		itsFontStyle.color = p.GetColormap()->GetInactiveLabelColor();

	p.SetFont(itsFontName, itsFontSize, itsFontStyle);
	p.String(lrect.left, lrect.top, itsLabel, 0,
			 lrect.width(), JPainter::kHAlignCenter,
			 lrect.height(), JPainter::kVAlignCenter);
}

// Draw the appropriate icon
void CToolbarButton::DrawPopupGlyph(JXWindowPainter& p, const JRect& rect)
{
	// Save drawing state
	StPenState save(&p);

	// Check selected state/enabled state (adjust for toggle operation)
	bool enabled = IsActive();

	// Always on right-hand side horizontally
	int hpos = rect.right - 19;
	
	// Center vertically if no caption
	int vpos = rect.top;
	if (mShowIcon)
		vpos += mSmallIcon ? 2 : 13;
	else
		vpos = rect.bottom - 18;

	JXImage* draw_icon = enabled ? CIconLoader::GetIcon(IDI_POPUPBTN, this, 16, 0x00CCCCCC) :
									CIconLoader::GetIcon(IDI_POPUPBTN, this, 16, 0x00CCCCCC, CIconLoader::eDisabled);
	p.JPainter::Image(*draw_icon, draw_icon->GetBounds(), hpos, vpos);

}
