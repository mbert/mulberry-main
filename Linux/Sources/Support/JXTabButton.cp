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


#include "JXTabButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "StPenState.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <JXImage.h>
#include <JXRadioGroup.h>

const JSize cUnselectOffset = 2;
const JSize cIconOffset = 18;

const JCharacter* JXTabButton::kRightClick = "RightClick::JXTabButton";
const JCharacter* JXTabButton::kShiftClick = "ShiftClick::JXTabButton";

/******************************************************************************
 Constructor

 ******************************************************************************/

JXTabButton::JXTabButton
	(
	const JIndex id,
	const JCharacter*	label,
	JXRadioGroup*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXTextRadioButton(id, label, enclosure, hSizing, vSizing, x,y, w,h)
{
	SetBorderWidth(0);
	mUseIcon = false;
	mIconID = 0;
	mIcon = NULL;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXTabButton::~JXTabButton()
{
}

void JXTabButton::SetIcon(JIndex icon)
{
	// Change icon state and adjust size
	if (icon)
	{
		if (!mUseIcon)
			AdjustSize(cIconOffset, 0);
		mUseIcon = true;
		if (mIconID != icon)
		{
			// Delete old one
			delete mIcon;
			mIcon = 0;
			
			// Add new one
			mIconID = icon;
			mIcon = CIconLoader::GetIcon(icon, this, 16, 0x00CCCCCC);
		}
	}
	else
	{
		if (mUseIcon)
			AdjustSize(-cIconOffset, 0);
		mUseIcon = false;
	}
}

void JXTabButton::HandleMouseDown(const JPoint& pt, const JXMouseButton button, const JSize clickCount,
									const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Look for right-click
	if (button == kJXRightButton)
		// Broadcast the right-click
		Broadcast(RightClick(pt, buttonStates, modifiers));
	else if ((button == kJXLeftButton) && modifiers.shift())
		// Broadcast the shift-click
		Broadcast(ShiftClick());
	else
		// Do default
		JXTextRadioButton::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXTabButton::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	// Change colour if pushed
	bool selected = DrawChecked();

	// Adjust for selected state
	JRect actual_rect(GetBounds());
	if (!selected)
		actual_rect.top += cUnselectOffset;

	// See if icon is present
	if (mUseIcon && mIcon)
	{
		p.JPainter::Image(*mIcon, mIcon->GetBounds(),
					actual_rect.left + 6,
					actual_rect.top + (actual_rect.height() - mIcon->GetBounds().height())/2 - 1);
		actual_rect.left += mIcon->GetBounds().width() + 6;
	}

	// Draw text
	p.SetFont(itsFontName, itsFontSize, itsFontStyle);
	p.String(actual_rect.left, actual_rect.top, itsLabel, 0,
			 actual_rect.width(), JPainter::kHAlignCenter,
			 actual_rect.height(), JPainter::kVAlignCenter);
}

void JXTabButton::DrawBackground(JXWindowPainter& p, const JRect& frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	// Just draw the default background everywhere
	// Do it here so that DrawBackground does not stomp
	// over the border we want to draw
	{
		StPenState save(&p);

		p.SetPenColor(GetColormap()->GetDefaultBackColor());
		p.SetFilling(kTrue);
		p.JPainter::Rect(frame);
	}

	if (true)
	{
		StPenState save(&p);

		// Top lines
		if (!selected)
		{
			p.SetPenColor(GetColormap()->GetDefaultBackColor());
			p.SetPenLocation(frame.left, frame.top);
			p.LineTo(frame.right - 1, frame.top);
			p.SetPenLocation(frame.left, frame.top + 1);
			p.LineTo(frame.right - 1, frame.top + 1);
		}

		// Adjust for selected state
		JRect actual_frame(frame);
		bool do_left = true;
		bool do_right = true;
		if (!selected)
		{
			actual_frame.top += cUnselectOffset;
			if (GetID() == 1)
				actual_frame.left += cUnselectOffset;
			else
				do_left = (GetRadioGroup()->GetSelectedItem() + 1 != GetID());
			do_right = (GetRadioGroup()->GetSelectedItem() != GetID() + 1);
		}

		// Outer line #1
		p.SetPenColor(p.GetColormap()->Get3DLightColor());
		if (do_left)
		{
			p.SetPenLocation(actual_frame.left, actual_frame.bottom - 1 - (selected ? 0 : kJXDefaultBorderWidth));
			p.LineTo(actual_frame.left, actual_frame.top + 2);
		}
		p.SetPenLocation(actual_frame.left + 1, actual_frame.top);
		p.LineTo(actual_frame.right - 2, actual_frame.top);

		// Inner line #1
		if (do_left)
		{
			p.SetPenLocation(actual_frame.left + 1, actual_frame.bottom - 1 - (selected ? 0 : kJXDefaultBorderWidth));
			p.LineTo(actual_frame.left + 1, actual_frame.top + 1);
			p.Point(actual_frame.left + 2, actual_frame.top + 2);
		}
		p.SetPenLocation(actual_frame.left + 1, actual_frame.top + 1);
		p.LineTo(actual_frame.right - 2, actual_frame.top + 1);

		// Outer line #2
		p.SetPenColor(p.GetColormap()->Get3DShadeColor());
		if (do_right)
		{
			p.SetPenLocation(actual_frame.right - 1, actual_frame.top + 1);
			p.LineTo(actual_frame.right - 1, actual_frame.bottom - 1 - (selected ? 0 : kJXDefaultBorderWidth));
		}

		// Inner line #2
		if (do_right)
		{
			p.SetPenLocation(actual_frame.right - 2, actual_frame.top + 2);
			p.LineTo(actual_frame.right - 2, actual_frame.bottom - 1 - (selected ? 0 : kJXDefaultBorderWidth));
		}

		// Bottom lines
		p.SetPenColor(selected ? GetColormap()->GetDefaultBackColor() : GetColormap()->Get3DLightColor());
		p.SetPenLocation(actual_frame.left + selected ? 2 : 0, actual_frame.bottom - 1);
		p.LineTo(actual_frame.right - (selected ? 3 : 1), actual_frame.bottom - 1);
		p.SetPenLocation(actual_frame.left + selected ? 2 : 0, actual_frame.bottom - 2);
		p.LineTo(actual_frame.right - (selected ? 3 : 1), actual_frame.bottom - 2);
	}
}

void JXTabButton::DrawBorder(JXWindowPainter& p, const JRect& frame)
{
	// Do nothing - this is done in DrawBackground
}
