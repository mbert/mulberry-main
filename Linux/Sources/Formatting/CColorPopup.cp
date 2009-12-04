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


// Source for CColorPopup class

#include "CColorPopup.h"

#include <JXChooseColorDialog.h>
#include <JXColormap.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>
#include <jXGlobals.h>

#include <cassert>

const JCoordinate _kTotalArrowWidth   = 28;
const JCoordinate _kArrowWidth        = 16;
const JCoordinate _kArrowHalfHeight   = 4;
const JCoordinate _kArrowShrinkWidth  = 4;
const JCoordinate _kArrowShrinkHeight = 2;

RGBColor CColorPopup::sBlack = JRGB(0,0,0);
RGBColor CColorPopup::sRed = JRGB(65535,0,0);
RGBColor CColorPopup::sGreen = JRGB(0,65535,0);
RGBColor CColorPopup::sBlue = JRGB(0,0,65535);
RGBColor CColorPopup::sYellow = JRGB(65535, 65535, 0);
RGBColor CColorPopup::sCyan = JRGB(0,65535,65535);
RGBColor CColorPopup::sMagenta = JRGB(65535,0,65535);
RGBColor CColorPopup::sMulberry = JRGB(39321,13107,26214);
RGBColor CColorPopup::sWhite = JRGB(65535,65535,65535);

CColorPopup::CColorPopup(const JCharacter* title,
									JXContainer* enclosure,
									 const HSizingOption hSizing,
									 const VSizingOption vSizing,
									 const JCoordinate x, const JCoordinate y,
									 const JCoordinate w, const JCoordinate h)
	: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
{
	mCurrentColor = sBlack;
	mDirector = NULL;
	mChooser = NULL;
}


CColorPopup::~CColorPopup()
{
}

void CColorPopup::OnCreate(JXWindowDirector* director)
{
	mDirector = director;

	SetMenuItems("Black %r | Red %r | Green %r | Blue %r | Yellow %r | Cyan %r | Magenta %r | Mulberry %r | White %r %l | Other... %r");
	SetUpdateAction(JXMenu::kDisableNone);
}

void CColorPopup::SelectColor(JIndex color)
{
	bool different = (mValue != color);
	mValue = color;

	switch(color)
	{
	case eColorIndex_Black:
		mCurrentColor = sBlack;
		break;
	case eColorIndex_Red:
		mCurrentColor = sRed;
		break;
	case eColorIndex_Green:
		mCurrentColor = sGreen;
		break;
	case eColorIndex_Blue:
		mCurrentColor = sBlue;
		break;
	case eColorIndex_Yellow:
		mCurrentColor = sYellow;
		break;
	case eColorIndex_Cyan:
		mCurrentColor = sCyan;
		break;
	case eColorIndex_Magenta:
		mCurrentColor = sMagenta;
		break;
	case eColorIndex_Mulberry:
		mCurrentColor = sMulberry;
		break;
	case eColorIndex_White:
		mCurrentColor = sWhite;
		break;
	case eColorIndex_Other:
		ChooseColor();
		return;
	}
			
	if (different && (color != eColorIndex_Other))
		Refresh();
}


void CColorPopup::Ambiguate()
{
}

void CColorPopup::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mChooser && message.Is(JXDialogDirector::kDeactivated))
	{
		const JXDialogDirector::Deactivated* info = dynamic_cast<const JXDialogDirector::Deactivated*>(&message);
		if (info->Successful())
			SetColor(GetColormap()->JColormap::GetRGB(mChooser->GetColor()));
		mChooser = NULL;
		return;
	}

	HPopupMenu::Receive(sender, message);
}

// This is pretty much a copy of JXMenuDraw but with our requirements applied
void CColorPopup::Draw(JXWindowPainter& p, const JRect& rect)
{
	const JRect bounds = GetBounds();

	const JCoordinate borderWidth = GetBorderWidth();
	if (borderWidth == 0)
		JXDrawUpFrame(p, bounds, kJXDefaultBorderWidth);

	JRect r = bounds;
	if (borderWidth > 0)
	{
		JRect ra  = bounds;
		ra.top    = bounds.ycenter() - _kArrowHalfHeight;
		ra.bottom = ra.top + 2*_kArrowHalfHeight;

		if (GetPopupArrowPosition() == kArrowAtRight)
		{
			ra.left  = bounds.right - (_kTotalArrowWidth + _kArrowWidth)/2;
			ra.right = ra.left + _kArrowWidth;
			r.right -= _kTotalArrowWidth;
		}
		else
		{
			assert( GetPopupArrowPosition() == kArrowAtLeft );
			ra.right = bounds.left + (_kTotalArrowWidth + _kArrowWidth)/2;
			ra.left  = ra.right - _kArrowWidth;
			r.left  += _kTotalArrowWidth;
		}

		if (IsActive())
		{
			JXDrawDownArrowDown(p, ra, kJXDefaultBorderWidth);
		}
		else
		{
			JXFillArrowDown(p, ra, GetColormap()->GetInactiveLabelColor());
			ra.Shrink(_kArrowShrinkWidth, _kArrowShrinkHeight);
			JXFillArrowDown(p, ra, GetBackColor());
		}
	}

	// Look for title & icon of selected item
	const JXImage* image = NULL;
	r.left += 2;
	r.top += 2;
	r.right -= 2;
	r.bottom -= 2;

	// Get rect for border
	JRect border(r);

	// Draw border
	p.SetPenColor(IsActive() ? GetColormap()->GetBlackColor() : GetColormap()->Get3DShadeColor());
	p.SetLineWidth(1);
	p.RectInside(border);

	// Draw color (lighten when disabled)
	border.Shrink(1, 1);
	JRGB color = mCurrentColor;
	if (!IsActive())
	{
		color /= 2;
		color.red += 0x8000;
		color.green += 0x8000;
		color.blue += 0x8000;
	}
	JColorIndex cindex;
	GetColormap()->JColormap::AllocateStaticColor(color, &cindex);
	p.SetPenColor(cindex);
	p.SetLineWidth(1);
	p.SetFilling(kTrue);
	p.RectInside(border);
}

void CColorPopup::ChooseColor()
{
	JColorIndex cindex;
	GetColormap()->JColormap::AllocateStaticColor(mCurrentColor, &cindex);

	mChooser = new JXChooseColorDialog(mDirector, cindex);

	ListenTo(mChooser);
	mChooser->BeginDialog();

	// display the inactive cursor in all the other windows

	JXApplication* app = JXGetApplication();
	app->DisplayInactiveCursor();

	// block with event loop running until we get a response

	JXWindow* window = mChooser->GetWindow();
	while (mChooser)
		app->HandleOneEventForWindow(window);
}
