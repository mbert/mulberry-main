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


#include "JXTextCheckbox3.h"

#include "CDrawUtils.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXTextCheckbox3::JXTextCheckbox3
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
	mValue = 0;
}

void JXTextCheckbox3::SetValue(int value)
{
	SetState(JBoolean(value == 1));
	mValue = value;
}
void JXTextCheckbox3::SetState(const JBoolean on)
{
	// Do inherited operation
	JXTextCheckbox::SetState(on);
	mValue = (IsChecked() ? 1 : 0);
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

const JCoordinate kMarginWidth   = 5;
const JCoordinate kBoxHeight     = 10;
const JCoordinate kBoxHalfHeight = 5;

void
JXTextCheckbox3::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	// Default draw
	JXTextCheckbox::Draw(p, rect);

	// Special for third state
	if (mValue == 2)
	{
		const JRect bounds  = GetBounds();
		const JCoordinate y = bounds.ycenter();

		// Draw dash through checkbox
		JRect boxRect(y - kBoxHalfHeight, kMarginWidth,
							y + kBoxHalfHeight, kMarginWidth + kBoxHeight);
		if (IsActive())
			p.SetPenColor((GetColormap())->GetBlackColor());
		else
			p.SetPenColor((GetColormap())->GetInactiveLabelColor());
		JCoordinate toppos = boxRect.top + kBoxHalfHeight - 1;
		boxRect.Shrink(2, 2);
		p.Line(boxRect.left, toppos, boxRect.right, toppos);
		++toppos;
		p.Line(boxRect.left, toppos, boxRect.right, toppos);
	}
}
