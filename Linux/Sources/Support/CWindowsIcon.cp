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


// Source for CWindowsIcon class

#include "CWindowsIcon.h"

#include "CGeneralException.h"

#include <JColormap.h>

#include <cassert>

unsigned long AlphaBlend(unsigned long rgba, unsigned long back, unsigned long mask, unsigned long state);
unsigned long AlphaBlend(unsigned long rgba, unsigned long back, unsigned long mask, unsigned long state)
{
	// Modify rgba based on state
	unsigned long rgba_state = rgba;
	switch(state)
	{
	case 0:	// Normal
		break;
	case 1:	// Disabled
	{
		// Ligten each color component
		unsigned long r_rgba = (rgba & 0x00FF0000) >> 16;
		unsigned long g_rgba = (rgba & 0x0000FF00) >> 8;
		unsigned long b_rgba = (rgba & 0x000000FF);
		r_rgba = (r_rgba >> 1) + 128;
		g_rgba = (g_rgba >> 1) + 128;
		b_rgba = (b_rgba >> 1) + 128;

		// Add new components into pixel
		rgba_state &= 0xFF000000;
		rgba_state |= (r_rgba << 16) | (g_rgba << 8) | b_rgba;
		break;
	}
	case 2:	// Inverted
	{
		// Invert each color component
		unsigned long r_rgba = (rgba & 0x00FF0000) >> 16;
		unsigned long g_rgba = (rgba & 0x0000FF00) >> 8;
		unsigned long b_rgba = (rgba & 0x000000FF);
		r_rgba = 255 - r_rgba;
		g_rgba = 255 - g_rgba;
		b_rgba = 255 - b_rgba;

		// Add new components into pixel
		rgba_state &= 0xFF000000;
		rgba_state |= (r_rgba << 16) | (g_rgba << 8) | b_rgba;
		break;
	}
	}

	unsigned char alpha = (rgba_state & 0xFF000000) >> 24;
	if (alpha == 0)
		return mask;
	else if (alpha == 0xFF)
		return rgba_state & 0x00FFFFFF;
	else
	{
		unsigned long r_rgba = (rgba_state & 0x00FF0000) >> 16;
		unsigned long g_rgba = (rgba_state & 0x0000FF00) >> 8;
		unsigned long b_rgba = (rgba_state & 0x000000FF);
		unsigned long r_back = (back & 0x00FF0000) >> 16;
		unsigned long g_back = (back & 0x0000FF00) >> 8;
		unsigned long b_back = (back & 0x000000FF);
		
		unsigned long result = 0;
		result |= (r_rgba * alpha + r_back * (0xFF - alpha)) >> 8;
		result <<= 8;
		result |= (g_rgba * alpha + g_back * (0xFF - alpha)) >> 8;
		result <<= 8;
		result |= (b_rgba * alpha + b_back * (0xFF - alpha)) >> 8;
		return result;
	}
}

CWindowsIcon::CWindowsIcon(JXDisplay* display, JXColormap* colormap, const unsigned long* data, unsigned long size, unsigned long bkgnd, unsigned long state)
	: JXImage(display, colormap)
{
	ParseData(data, size, bkgnd, state);
}

CWindowsIcon::~CWindowsIcon()
{
}

void CWindowsIcon::ParseData(const unsigned long* ico, unsigned long size, unsigned long bkgnd, unsigned long state)
{
	// Get pointer to family structure
	unsigned long entry_count = ico[0];
	if ((entry_count == 0) || (entry_count > 4))
		throw CGeneralException(-1);

	// Now look for an entry that matches the requested size
	const unsigned long* entry = &ico[1 + ico[0]];
	bool found = false;
	for(unsigned long i = 0; i < entry_count; i++)
	{
		if (ico[i + 1] == size)
		{
			found = true;
			break;
		}
		entry += ico[i + 1] * ico[i + 1];
	}
	
	if (!found)
		//throw CGeneralException(-1);
		return;

	SetDimensions(size, size);

	std::set<unsigned long> colorset;
	colorset.insert(0xBAADF00D);
	for(unsigned long y = 0; y < size; y++)
	{
		for(unsigned long x = 0; x < size; x++)
		{
			unsigned long pixel = *(unsigned long*)(entry + size * y + x);
			pixel = AlphaBlend(pixel, bkgnd, 0xBAADF00D, state);
			
			colorset.insert(pixel);
		}
	}

	// build color table

	GetColormap()->PrepareForMassColorAllocation();

	const JSize colorCount = colorset.size();

	JColorIndex* colorTable = new JColorIndex [ colorCount ];
	assert( colorTable != NULL );

	const int maskColor     = colorCount - 1;
	const JBoolean hasMask  = kTrue;

	const JColorIndex blackColor = GetColormap()->GetBlackColor();

	JIndex i=0;
	for (std::set<unsigned long>::const_iterator iter = colorset.begin(); iter != colorset.end(); iter++, i++)
	{
		JSize r;
		JSize g;
		JSize b;
		r = ((*iter) & 0x00FF0000) >> 16;
		r |= (r << 8);
		g = ((*iter) & 0x0000FF00) >> 8;
		g |= (g << 8);
		b = (*iter) & 0x000000FF;
		b |= (b << 8);

		if (!(hasMask && i == (JIndex) maskColor) &&
			!GetColormap()->AllocateStaticColor(r,
											  g,
											  b,
											  &colorTable[i]))
		{
			GetColormap()->MassColorAllocationFinished();
			return;
		}
	}

	GetColormap()->MassColorAllocationFinished();

	// allocate space for image data

	unsigned short* data;
	unsigned short** cols;
	const JError allocErr = AllocateImageData(size, size, &data, &cols);
	if (!allocErr.OK())
	{
		return;
	}

	// decode image data

	for (JCoordinate y = 0; y < size; y++)
	{
		for (JCoordinate x = 0; x < size; x++)
		{
			unsigned long pixel = *(unsigned long*)(entry + size * y + x);
			pixel = AlphaBlend(pixel, bkgnd, 0xBAADF00D, state);

			JIndex i=0;
			for (std::set<unsigned long>::const_iterator iter = colorset.begin(); iter != colorset.end(); iter++, i++)
			{
				if (*iter == pixel)
					break;
			}

			cols[x][y] = i;
		}
	}

	SetImageData(colorCount, colorTable, cols, hasMask, maskColor);

	// clean up

	delete [] colorTable;
	delete [] data;
	delete [] cols;
}
