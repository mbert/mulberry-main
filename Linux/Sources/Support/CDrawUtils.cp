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


// Source for CDrawUtils class

#include "CDrawUtils.h"

#include "StPenState.h"

#include <JColormap.h>
#include <JPainter.h>
#include <JRect.h>
#include <JXColormap.h>
#include <JXImage.h>

void CDrawUtils::DrawBackground(JPainter& p, const JRect& rect, bool selected, bool enabled)
{
	// Save the pen state
	StPenState save(&p);
	p.SetLineWidth(1);

	JRect temp = rect;

	if (enabled)
		p.SetPenColor(selected ? p.GetColormap()->Get3DShadeColor() : p.GetColormap()->GetDefaultBackColor());
	else
		p.SetPenColor(p.GetColormap()->GetDefaultBackColor());
	p.SetFilling(kTrue);
	p.JPainter::Rect(temp);
}

void CDrawUtils::Draw3DBorder(JPainter& p, const JRect& rect, bool selected, bool enabled, bool frame, bool filled)
{
	// Save the pen state
	StPenState save(&p);

	// Always draw three-pixel border in ordinary color
	p.SetPenColor(selected ? p.GetColormap()->Get3DShadeColor() : p.GetColormap()->GetDefaultBackColor());
	p.SetLineWidth(3);
	p.SetFilling(kFalse);
	p.RectInside(rect);

	p.SetLineWidth(1);

	if (enabled)
	{
		if (filled)
			DrawBackground(p, rect, selected && frame, true);

		if (frame)
		{
			// Black outline
			p.SetPenColor(p.GetColormap()->GetBlackColor());
			p.Line(rect.left, rect.top, rect.right - 1, rect.top);
			p.Line(rect.right - 1, rect.top, rect.right - 1, rect.bottom - 1);
			p.Line(rect.right - 1, rect.bottom - 1, rect.left, rect.bottom - 1);
			p.Line(rect.left, rect.bottom - 1, rect.left, rect.top);

			// Outer line #1
			p.SetPenColor(selected ? p.GetColormap()->GetGray10Color() : p.GetColormap()->GetDefaultBackColor());
			p.Line(rect.left + 1, rect.bottom - 3, rect.left + 1, rect.top + 1);
			p.LineTo(rect.right - 3, rect.top + 1);

			// Outer line #2
			p.SetPenColor(selected ? p.GetColormap()->GetGray80Color() : p.GetColormap()->Get3DShadeColor());
			p.Line(rect.right - 2, rect.top + 2, rect.right - 2, rect.bottom - 2);
			p.LineTo(rect.left + 2, rect.bottom - 2);

			// Inner line #1
			p.SetPenColor(selected ? p.GetColormap()->GetGray40Color() : p.GetColormap()->GetWhiteColor());
			p.Line(rect.left + 2, rect.bottom - 4, rect.left + 2, rect.top + 2);
			p.LineTo(rect.right - 4, rect.top + 2);

			// Inner line #2
			p.SetPenColor(selected ? p.GetColormap()->GetGray60Color() : p.GetColormap()->GetGray70Color());
			p.Line(rect.right - 3, rect.top + 3, rect.right - 3, rect.bottom - 3);
			p.LineTo(rect.left + 3, rect.bottom - 3);
		}
	}
	else
	{
		if (filled)
			DrawBackground(p, rect, false, false);

		if (frame)
		{
			// Dark gray outline
			p.SetPenColor(p.GetColormap()->GetInactiveLabelColor());
			p.Line(rect.left, rect.top, rect.right - 1, rect.top);
			p.Line(rect.right - 1, rect.top, rect.right - 1, rect.bottom - 1);
			p.Line(rect.right - 1, rect.bottom - 1, rect.left + 1, rect.bottom - 1);
			p.Line(rect.left, rect.bottom - 1, rect.left, rect.top);
		}
	}
}

void CDrawUtils::DrawScrollBorder(JPainter& p, const JRect& rect, bool selected, bool enabled)
{
	// Save the pen state
	StPenState save(&p);

	// Always draw three-pixel border in ordinary color
	p.SetPenColor(selected ? p.GetColormap()->Get3DShadeColor() : p.GetColormap()->GetDefaultBackColor());
	p.SetLineWidth(2);
	p.SetFilling(kFalse);
	p.RectInside(rect);

	p.SetLineWidth(1);

	if (enabled)
	{
		DrawBackground(p, rect, false, true);

		// Outer line #1
		p.SetPenColor(selected ? p.GetColormap()->Get3DShadeColor() : p.GetColormap()->GetDefaultBackColor());
		p.Line(rect.left, rect.bottom - 2, rect.left, rect.top);
		p.LineTo(rect.right - 2, rect.top);

		// Outer line #2
		p.SetPenColor(selected ? p.GetColormap()->Get3DShadeColor() : p.GetColormap()->GetBlackColor());
		p.Line(rect.right - 1, rect.top, rect.right - 1, rect.bottom - 1);
		p.LineTo(rect.left, rect.bottom - 1);

		// Inner line #1
		p.SetPenColor(selected ? p.GetColormap()->GetDefaultBackColor() : p.GetColormap()->GetWhiteColor());
		p.Line(rect.left + 1, rect.bottom - 3, rect.left + 1, rect.top + 1);
		p.LineTo(rect.right - 3, rect.top + 1);

		// Inner line #2
		p.SetPenColor(selected ? p.GetColormap()->GetDefaultBackColor() : p.GetColormap()->Get3DShadeColor());
		p.Line(rect.right - 2, rect.top + 1, rect.right - 2, rect.bottom - 2);
		p.LineTo(rect.left + 1, rect.bottom - 2);
	}
	else
	{
		DrawBackground(p, rect, false, false);

		// Dark gray outline
		p.SetPenColor(p.GetColormap()->GetInactiveLabelColor());
		p.Line(rect.left, rect.top, rect.right - 1, rect.top);
		p.Line(rect.right - 1, rect.top, rect.right - 1, rect.bottom - 1);
		p.Line(rect.right - 1, rect.bottom - 1, rect.left + 1, rect.bottom - 1);
		p.Line(rect.left, rect.bottom - 1, rect.left, rect.top);
	}
}

void CDrawUtils::DrawSimpleBorder(JPainter& p, const JRect& rect, bool selected, bool enabled, bool frame, bool filled)
{
	// Save the pen state
	StPenState save(&p);

	if (enabled)
	{
		if (filled)
			DrawBackground(p, rect, selected && frame, true);

		if (frame)
		{
			// Black outline
			p.SetPenColor(p.GetColormap()->GetBlackColor());
			p.Line(rect.left, rect.top, rect.right - 1, rect.top);
			p.Line(rect.right - 1, rect.top, rect.right - 1, rect.bottom - 1);
			p.Line(rect.right - 1, rect.bottom - 1, rect.left, rect.bottom - 1);
			p.Line(rect.left, rect.bottom - 1, rect.left, rect.top);

			// Outer line #1
			p.SetPenColor(selected ? p.GetColormap()->GetBlackColor() : p.GetColormap()->GetWhiteColor());
			p.Line(rect.left + 1, rect.bottom - 2, rect.left + 1, rect.top + 1);
			p.LineTo(rect.right - 2, rect.top + 1);

			// Outer line #2
			p.SetPenColor(selected ? p.GetColormap()->GetDefaultBackColor() : p.GetColormap()->Get3DShadeColor());
			p.Line(rect.right - 2, rect.top + 2, rect.right - 2, rect.bottom - 2);
			p.LineTo(rect.left + 2, rect.bottom - 2);
		}
	}
	else
	{
		if (filled)
			DrawBackground(p, rect, false, false);

		if (frame)
		{
			// Dark gray outline
			p.SetFilling(kFalse);
			p.SetPenColor(p.GetColormap()->GetInactiveLabelColor());
			p.Rect(rect);
		}
	}
}

// Draw the appropriate button frame
void CDrawUtils::DrawSimpleLine(JPainter& p, const JRect& rect, bool vertical, bool selected, bool enabled)
{
	// Save the pen state
	StPenState save(&p);

	if (enabled)
	{
		// Line #1
		p.SetPenColor(selected ? p.GetColormap()->GetWhiteColor() : p.GetColormap()->GetInactiveLabelColor());
		p.Line(rect.left, rect.top, vertical ? rect.left : rect.right - 1, vertical ? rect.bottom - 1 : rect.top);

		// Line #2
		p.SetPenColor(selected ? p.GetColormap()->GetInactiveLabelColor() : p.GetColormap()->GetWhiteColor());
		p.Line(rect.left + (vertical ? 1 : 0), rect.top + (vertical ? 0 : 1),
				vertical ? rect.left + 1 : rect.right - 1, vertical ? rect.bottom - 1 : rect.top + 1);
	}
}
