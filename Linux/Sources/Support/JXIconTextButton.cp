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


#include "JXIconTextButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <JXImage.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXIconTextButton::JXIconTextButton
	(
	const JIndex id,
	const JCharacter* label,
	JXRadioGroup* enclosure,
	const HSizingOption hSizing, const VSizingOption vSizing,
	const JCoordinate x, const JCoordinate y,
	const JCoordinate w, const JCoordinate h
	)
	:
	JXTextRadioButton(id, label, enclosure, hSizing, vSizing, x,y, w,h)
{
	SetBorderWidth(kJXDefaultBorderWidth);
	itsImageID = 0;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXIconTextButton::~JXIconTextButton()
{
}

void
JXIconTextButton::SetImage
	(
	ResIDT			imageID
	)
{
	itsImageID         = imageID;

	Refresh();
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXIconTextButton::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	// Change colour if pushed
	bool selected = DrawChecked();

	JRect actual_rect(GetBounds());

	unsigned long bkgnd = 0x00FFFFFF;
	JColorIndex bkgnd_index = GetColormap()->Get3DShadeColor();
	if (IsActive() && selected)
		bkgnd_index = GetColormap()->Get3DShadeColor();
	else
		bkgnd_index = GetColormap()->GetDefaultBackColor();
	{
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	JXImage* draw_me = NULL;

	if (IsActive())
		draw_me = CIconLoader::GetIcon(itsImageID, this, 16, bkgnd);
	else
		draw_me = CIconLoader::GetIcon(itsImageID, this, 16, bkgnd, CIconLoader::eDisabled);

	// Try icon if present
	if (draw_me)
	{
		p.JPainter::Image(*draw_me, draw_me->GetBounds(),
					actual_rect.left + 2,
					actual_rect.top + (actual_rect.height() - draw_me->GetBounds().height())/2);
		actual_rect.left += draw_me->GetBounds().width() + 4;
	}

	if (selected && IsActive())
		itsFontStyle.color = p.GetColormap()->GetWhiteColor();
	else if (IsActive())
		itsFontStyle.color = p.GetColormap()->GetBlackColor();
	else
		itsFontStyle.color = p.GetColormap()->GetInactiveLabelColor();

	p.SetFont(itsFontName, itsFontSize, itsFontStyle);
	p.String(actual_rect.left, actual_rect.top, itsLabel, 0,
			 actual_rect.width(), draw_me ? JPainter::kHAlignLeft : JPainter::kHAlignCenter,
			 actual_rect.height(), JPainter::kVAlignCenter);
}

void JXIconTextButton::DrawBackground(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	CDrawUtils::DrawBackground(p, frame, selected && use_frame, enabled);
}

void JXIconTextButton::DrawBorder(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	if (use_frame)
		CDrawUtils::DrawSimpleBorder(p, frame, selected, enabled);
}
