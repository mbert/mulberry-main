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


#include "JXColorButton.h"

#include "StPenState.h"

#include <JXChooseColorDialog.h>
#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <jAssert.h>
#include <jXGlobals.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXColorButton::JXColorButton
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXButton(enclosure, hSizing, vSizing, x,y, w,h)
{
	// Never have a border
	SetBorderWidth(0);

	// Always listen to ourself
	ListenTo(this);

	mChooser = NULL;
	mDirector = NULL;
}

/******************************************************************************
 SetColor

 ******************************************************************************/

void
JXColorButton::SetColor(const JRGB& color)
{
	mColor = color;
	Redraw();
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXColorButton::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	StPenState save(&p);

	// Get rect for border
	JRect actual_rect(GetBounds());
	JRect border(actual_rect);
	border.right -= 2;
	border.bottom -= 2;
	JSize border_width = IsPushed() ? 2 : 1;

	// Draw border
	p.SetPenColor(IsActive() ? GetColormap()->GetBlackColor() : GetColormap()->Get3DShadeColor());
	p.SetLineWidth(border_width);
	p.RectInside(border);

	// Draw color (lighten when disabled)
	border.Shrink(border_width, border_width);
	JRGB color = mColor;
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

	// Now do shadow
	if (IsActive())
	{
		p.SetPenColor(GetColormap()->Get3DShadeColor());
		p.SetLineWidth(2);
		p.SetPenLocation(actual_rect.left + 2, actual_rect.bottom - 1);
		p.LineTo(actual_rect.right - 1, actual_rect.bottom - 1);
		p.SetPenLocation(actual_rect.right - 1, actual_rect.bottom);
		p.LineTo(actual_rect.right - 1, actual_rect.top + 2);
	}
}

void JXColorButton::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed) && (sender == this))
	{
		OnClick();
		return;
	}
	else if (sender == mChooser && message.Is(JXDialogDirector::kDeactivated))
	{
		const JXDialogDirector::Deactivated* info = dynamic_cast(const JXDialogDirector::Deactivated*, &message);
		assert( info != NULL );
		if (info->Successful())
			SetColor(GetColormap()->JColormap::GetRGB(mChooser->GetColor()));
		mChooser = NULL;
		return;
	}

	JXButton::Receive(sender, message);
}

void JXColorButton::OnClick()
{
	JColorIndex cindex;
	GetColormap()->JColormap::AllocateStaticColor(mColor, &cindex);

	assert( mDirector != NULL);
	mChooser = new JXChooseColorDialog(mDirector, cindex);
	assert( mChooser != NULL );

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
