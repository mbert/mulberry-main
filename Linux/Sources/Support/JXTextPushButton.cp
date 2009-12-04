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


#include "JXTextPushButton.h"

#include "CDrawUtils.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXTextPushButton::JXTextPushButton
	(
	const JCharacter*	label,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXTextCheckbox(label, enclosure, hSizing, vSizing, x,y, w,h)
{
	SetBorderWidth(kJXDefaultBorderWidth);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXTextPushButton::~JXTextPushButton()
{
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXTextPushButton::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	JRect actual_rect(GetBounds());

	// Change colour if pushed
	bool selected = DrawChecked();
	bool enabled = IsActive();

	if (selected && enabled)
		itsFontStyle.color = p.GetColormap()->GetWhiteColor();
	else if (enabled)
		itsFontStyle.color = p.GetColormap()->GetBlackColor();
	else
		itsFontStyle.color = p.GetColormap()->GetInactiveLabelColor();

	p.SetFont(itsFontName, itsFontSize, itsFontStyle);
	p.String(actual_rect.left, actual_rect.top, itsLabel, 0,
			 actual_rect.width(), JPainter::kHAlignCenter,
			 actual_rect.height(), JPainter::kVAlignCenter);
}

void JXTextPushButton::DrawBackground(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	CDrawUtils::DrawBackground(p, frame, selected && use_frame, enabled);
}

void JXTextPushButton::DrawBorder(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	if (use_frame)
		CDrawUtils::DrawSimpleBorder(p, frame, selected, enabled);
}
