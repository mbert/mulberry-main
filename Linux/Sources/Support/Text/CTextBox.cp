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


// Source for CTextBox class

#include "CTextBox.h"

#include <JPainter.h>
#include <JRect.h>

void CTextBox::DrawText(JPainter* pDC, const char* theTxt, const JRect& box, EDrawStringAlignment align)
{
	// Adjust width for slop
	const unsigned long cSlop = 4;
	long width = box.width() - cSlop;
	size_t len = ::strlen(theTxt);

	// Short cut for short strings
	int str_width = pDC->GetStringWidth(theTxt);
	if ((str_width < width) || (len < 2))
		::DrawClippedStringUTF8(pDC, theTxt, JPoint(box.left, box.top), box, align);
	else
	{
		JRect actual_box(box);

		// Break into regions separated by whitespace
		const char* start = theTxt;
		const char* last_stop = theTxt;
		const char* stop = theTxt;
		const char* end = theTxt + ::strlen(theTxt);
		while(start < end)
		{
			// Find next word
			last_stop = stop;
			while((stop < end) && !isspace(*stop))
			{
				stop++;
			}
			
			// Special case - single space
			if (start == stop)
			{
				stop++;
				continue;
			}
			
			// Measure length of current text as it would be drawn
			cdstring temp(start, stop - start);
			str_width = pDC->GetStringWidth(temp);
			
			// If less than current width, try another word
			if ((str_width < width) && (stop < end))
			{
				stop++;
				continue;
			}
			
			// See if we have a previous word, or whether we have one long word that will be truncated
			if ((last_stop == start) || (str_width < width))
				last_stop = stop;

			// Draw string for previous OK width
			temp.assign(start, last_stop - start);
			::DrawClippedStringUTF8(pDC, temp, JPoint(actual_box.left, actual_box.top), actual_box, align);
			
			// Adjust to next portion
			start = last_stop;
			stop = start;
			
			// Adjust vertical offset by one line and skip if no more room
			actual_box.top += pDC->GetLineHeight();
			if (actual_box.top >= actual_box.bottom)
				break;
		}
	}
}
