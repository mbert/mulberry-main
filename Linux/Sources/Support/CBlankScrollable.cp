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


// Source for CBlankScrollable class

#include "CBlankScrollable.h"

#include "CDrawUtils.h"

#include <JXColormap.h>
#include <JXWindowPainter.h>

void CBlankScrollable::Init()
{
	// Always show scrollbars
	AlwaysShowScrollbars();

	// Fit to enclosure
	FitToEnclosure();

	// Adjust bounds to size of aperture
	AdjustBounds(-2, 0);

	SetFocusColor((GetColormap())->GetDefaultBackColor());
}

void CBlankScrollable::Draw(JXWindowPainter& p, const JRect& rect)
{
	// Draw grey background
	CDrawUtils::DrawBackground(p, rect, false, false);
}

void CBlankScrollable::SetBounds(const JCoordinate w, const JCoordinate h)
{
	JXScrollableWidget::SetBounds(w, h);
}

void CBlankScrollable::AdjustBounds(const JCoordinate dw, const JCoordinate dh)
{
	// Adjust bounds based on input
	JRect boundsG = GetBoundsGlobal();
	JXScrollableWidget::SetBounds(boundsG.width() + dw, boundsG.height() + dh);
}

// Always tie bounds width to aperture
void CBlankScrollable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	if (dw != 0)
		AdjustBounds(dw, 0);
	JXScrollableWidget::ApertureResized(dw, dh);
}
