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


// CMulberryCommon.cp

#include "CMulberryCommon.h"

#include "CMulberryApp.h"
#include "CSoundManager.h"

#include <JPainter.h>
#include <JXWindow.h>
#include <stdio.h>
#include <JPoint.h>
#include <JRect.h>
#include <JXDisplay.h>

void MessageBeep(int i)
{
	CMulberryApp::sApp->GetCurrentDisplay()->Beep();
}

// Draw clipped string if too long
void DrawClippedStringUTF8(JPainter* pDC, const char* theTxt, const JPoint& start, const JRect& clip,
						EDrawStringAlignment align, EDrawStringClip clip_string)
{
	// Adjust width for slop
	const unsigned long cSlop = 4;
	long width = clip.right - start.x - cSlop;
	size_t len = ::strlen(theTxt);

	// Check actual width against allowed width
	int str_width = pDC->GetStringWidth(theTxt);
	if ((str_width < width) || (len < 2))
	{
		JPainter::HAlignment alignment;
		switch(align)
		{
		case eDrawString_Left:
		case eDrawStringJustify:
			alignment = JPainter::kHAlignLeft;
			break;
		case eDrawString_Center:
			alignment = JPainter::kHAlignCenter;
			break;
		case eDrawString_Right:
			alignment = JPainter::kHAlignRight;
			break;
		}

		// String will fit - draw unmodified
		pDC->String(JPoint(start.x, start.y), theTxt, width, alignment);
	}
	else
	{
		// Copy string since it will be modified
		char str[256];
		if (len < 256)
			::strcpy(str, theTxt);
		else
		{
			len = 255;
			::strncpy(str, theTxt, len);
			str[len] = 0;
		}

		switch(clip_string)
		{
		case eClipString_Left:
			// Put 3 '.' at start of string
			if (len)
				str[1] = '.';
			if (len > 1)
				str[2] = '.';
			if (len > 2)
				str[3] = '.';
			break;

		case eClipString_Center:
			// Put 3 '.' at start of string
			if (len)
				str[len/2] = '.';
			if (len > 1)
				str[len/2 - 1] = '.';
			if (len > 2)
				str[len/2 + 1] = '.';
			break;
		
		case eClipString_Right:
			// Put 3 '.' at end of string
			if (len)
				str[len] = '.';
			if (len > 1)
				str[len-1] = '.';
			if (len > 2)
				str[len-2] = '.';
			break;
		}

		// Reduce length of string and recalculate width
		for(; len > 0; len--)
		{
			if ((long) pDC->GetStringWidth(str) < width)
				break;

			switch(clip_string)
			{
			case eClipString_Left:
				// Move string one char to left
				::memmove(&str[0], &str[1], len - 1);

				// Shorten string
				if (len > 2)
					str[2] = '.';
				break;
			
			case eClipString_Center:
			{
				// Move string one char to left at its center
				unsigned long center = len/2;
				::memmove(&str[center], &str[center + 1], len - center);
				if (center)
					str[center - 1] = '.';
				if (center < len - 1)
					str[center + 1] = '.';
				break;
			}

			case eClipString_Right:
				// Add another period
				if (len > 3)
					str[len-3] = '.';

				// Shorten string
				str[len] = 0;
				break;
			}
		}

		// Draw clipped string
		pDC->String(JPoint(start.x, start.y), str, width, JPainter::kHAlignLeft);
	}

}

void MoveWindowBy(JXWindow* pWnd, int dx, int dy, bool redraw)
{
	pWnd->Move(dx,dy);
	if (redraw)
		pWnd->Refresh();
}

void ResizeWindowBy(JXWindow* pWnd, int dx, int dy, bool redraw)
{
	pWnd->AdjustSize(dx, dy);
	if (redraw)
		pWnd->Refresh();
}

// Make sure some portion of title bar is on screen
void RectOnScreen(JRect& rect)
{
#if NOTYET
	// Get task bar pos and see if at top
	CRect desktop;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, (Rect*) desktop, 0);
	desktop.right -= 32;
	desktop.bottom -= 32;

	POINT pt;
	pt.x = rect.left;
	pt.y = rect.top;

	if (!desktop.PtInRect(pt))
	{
		POINT diff = {0, 0};

		if (rect.left < desktop.left)
			diff.x = desktop.left - rect.left;
		else if (rect.left > desktop.right)
			diff.x = desktop.right - rect.left;

		if (rect.top < desktop.top)
			diff.y = desktop.top - rect.top;
		else if (rect.top > desktop.bottom)
			diff.y = desktop.bottom - rect.top;
		
		rect.OffsetRect(diff);
	}
#endif
}

cdstring GetNumericFormat(unsigned long number)
{
	cdstring result;
	result.reserve(64);
	if (number >= 10000000UL)
		::snprintf(result.c_str_mod(), 64, "%ld M", (number >> 20) + (number & (1L << 19) ? 1 : 0));
	else if (number >= 10000UL)
		::snprintf(result.c_str_mod(), 64, "%ld K", (number >> 10) + (number & (1L << 9) ? 1 : 0));
	else
		::snprintf(result.c_str_mod(), 64, "%ld", number);
	
	return result;
}

void PlayNamedSound(const char* name)
{
	if (!name || !*name || !CSoundManager::sSoundManager.PlaySound(name))
		::MessageBeep(-1);
}

StNoRedraw::StNoRedraw(JXWidget* inPane)
{
	mPane		= inPane;

	if (inPane != NULL)
	{
		mSaveState = inPane->IsVisible();
		inPane->SetVisibleState(kFalse);
	}
}

StNoRedraw::~StNoRedraw()
{
	if (mPane != NULL)
	{
		mPane->SetVisibleState(mSaveState);
		if (mSaveState == kTrue)
			mPane->Refresh();
	}
}
