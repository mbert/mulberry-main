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


#include "CFormatElement.h"
#include "ETag.h"
#include "CHTMLCharacterEntity.h"
#include "CStringUtils.h"

#include <string.h>
#include <stdlib.h>

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include "CFormattedTextDisplay.h"
#elif __dest_os == __win32_os
#include "CDisplayFormatter.h"
#endif

#if __dest_os == __win32_os
#include "Mac2Win.h"
#endif

#if __dest_os == __linux_os
#include <ctype.h>
#include <string.h>

#include <JXColormap.h>
#endif

CFormatElement::CFormatElement()
{
	start = -1;
	stop = -1;
	next = NULL;
}

CFormatElement::CFormatElement(SInt32 start, SInt32 stop)
{
	CFormatElement::start = start;
	CFormatElement::stop = stop;
	next = NULL;
}

CFormatElement::~CFormatElement()
{
}

SInt32 CFormatElement::getStart()
{
	return start;
}

void CFormatElement::setStart(SInt32 start)
{
	CFormatElement::start = start;
}

void CFormatElement::setStop(SInt32 stop)
{
	CFormatElement::stop = stop;
}

CFontFormatElement::CFontFormatElement() : CFormatElement()
{
}

CFontFormatElement::CFontFormatElement(SInt32 start, SInt32 stop, const char* in_font) : CFormatElement(start, stop)
{
	font = in_font;
}

CFontFormatElement::~CFontFormatElement()
{
}

void CFontFormatElement::draw(CDisplayFormatter *display)
{
	display->FontFormat(font.c_str_mod(), start, stop);
}

void CColorFormatElement::setColor(int r, int g, int b)
{
#if __dest_os == __win32_os
	color = RGB(r >> 8, g >> 8, b >> 8);
#elif __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
	//Looks like these are 16 bit values so we'll see if it works
	color.red = r;
	color.green = g;
	color.blue = b;
#endif
}

CColorFormatElement::CColorFormatElement() : CFormatElement()
{
	setColor(-1,-1,-1);
}

CColorFormatElement::CColorFormatElement(SInt32 start, SInt32 stop, int r, int g, int b) : CFormatElement(start, stop)
{
	setColor(r,g,b);
}

CColorFormatElement::CColorFormatElement(SInt32 start, SInt32 stop, RGBColor color) : CFormatElement(start, stop)
{
	CColorFormatElement::color = color;
}

CColorFormatElement::CColorFormatElement(SInt32 start, SInt32 stop, const char *col, bool background) : CFormatElement(start, stop)
{
	// Look for named color (must be alpha) (no commas)
	if (isalpha(col[0]) && !strchr(col, ','))
	{
		// Scan color map for matching entry
		bool done = false;
		for(const SMapHTMLColor* map = cHTMLColorEntity; !done && *map->html; map++)
		{
			// Must be case sensitive compare
			if (!::strcmpnocase(col, map->html))
			{
				int r = (map->color & 0x00FF0000);
				r = (r >> 8) | (r >> 16);
				int g = (map->color & 0x0000FF00);
				g = g | (g >> 8);
				int b = (map->color & 0x000000FF);
				b = b | (b << 8);
				setColor(r,g,b);
				done = true;
			}
		}

		// Default to black (text) or white (background)
		if (!done)
		{
			if (background)
				setColor(0xFFFF, 0xFFFF, 0xFFFF);
			else
				setColor(0x0000, 0x0000, 0x0000);
		}
	}

	// Look for HTML hex value
	else if(col[0] == '#')
	{
		long value = ::strtol(col+1, NULL, 16);
		int r = (value & 0x00FF0000);
		r = (r >> 8) | (r >> 16);
		int g = (value & 0x0000FF00);
		g = g | (g >> 8);
		int b = (value & 0x000000FF);
		b = b | (b << 8);
		setColor(r,g,b);
	}

	// Look for enriched hex value
	else if (isxdigit(col[0]))
	{
		char *value;
		int r = 0;
		int g = 0;
		int b = 0;
		cdstring copy(col);
		if ((value = ::strtok(copy, ",")) != nil)
			r = (int) ::strtol(value, NULL, 16);
		if ((value = ::strtok(NULL, ",")) != nil)
			g = (int) ::strtol(value, NULL, 16);
		if ((value = ::strtok(NULL, ",")) != nil)
			b = (int) ::strtol(value, NULL, 16);
		setColor(r,g,b);
	}
	else
		setColor(-1, -1, -1);
}

void CColorFormatElement::draw(CDisplayFormatter *display)
{

	display->ColorFormat(color, start, stop);
}


CFontSizeFormatElement::CFontSizeFormatElement() : CFormatElement()
{
	sizeChange = 0;
	adding = false;
	overrideprefs = false;
}

CFontSizeFormatElement::CFontSizeFormatElement(SInt32 start, SInt32 stop, short change, bool add, bool override) : CFormatElement(start, stop)
{
	sizeChange = change;
	adding = add;
	overrideprefs = override;
}

void CFontSizeFormatElement::draw(CDisplayFormatter *display)
{
	display->FontSizeFormat(sizeChange, adding, start, stop, overrideprefs);
}


CAlignmentFormatElement::CAlignmentFormatElement() : CFormatElement()
{
	tagid = E_NOTHING;
}

CAlignmentFormatElement::CAlignmentFormatElement(SInt32 start, SInt32 stop, ETag tag) : CFormatElement(start, stop)
{
	tagid = tag;
}

void CAlignmentFormatElement::draw(CDisplayFormatter *display)
{
	display->AlignmentFormat(tagid, start, stop);
}


CFaceFormatElement::CFaceFormatElement() : CFormatElement()
{
	tagid = E_NOTHING;
}

CFaceFormatElement::CFaceFormatElement(SInt32 start, SInt32 stop, ETag tag) : CFormatElement(start,stop)
{
	tagid = tag;
}

void CFaceFormatElement::draw(CDisplayFormatter *display)
{
	display->FaceFormat(tagid, start,stop);
}

CExcerptFormatElement :: CExcerptFormatElement() : CFormatElement()
{
	Excerpt = ' ';
}

CExcerptFormatElement :: CExcerptFormatElement(SInt32 start, SInt32 stop) : CFormatElement(start, stop)
{
	Excerpt = ' ';
}

CExcerptFormatElement :: CExcerptFormatElement(SInt32 start, SInt32 stop, char ex) : CFormatElement(start, stop)
{
	Excerpt = ex;
}

void CExcerptFormatElement::draw(CDisplayFormatter *display)
{
	display->ExcerptFormat(Excerpt, start, stop);
}

