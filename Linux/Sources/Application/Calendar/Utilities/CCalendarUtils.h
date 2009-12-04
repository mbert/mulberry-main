/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

/* 
	CCalendarUtils.h

	Author:			
	Description:	<describe the PPxUtils class here>
*/

#ifndef CCalendarUtils_H
#define CCalendarUtils_H

#include <cstdint>

#include <jColor.h>

namespace CCalendarUtils
{

	inline float	GetAlpha(const uint32_t& colour)
	{
		return ((colour & 0xFF000000) >> 24) / 255.0;
	}
	inline float	GetRed(const uint32_t& colour)
	{
		return ((colour & 0x00FF0000) >> 16) / 255.0;
	}
	inline float	GetGreen(const uint32_t& colour)
	{
		return ((colour & 0x0000FF00) >> 8) / 255.0;
	}
	inline float	GetBlue(const uint32_t& colour)
	{
		return (colour & 0x000000FF) / 255.0;
	}
	inline void		FlattenColours(float& red, float& green, float& blue)
	{
		red = (red + 1.0) / 2.5;
		green = (green + 1.0) / 2.5;
		blue = (blue + 1.0) / 2.5;
	}
	inline void		UnflattenColours(float& red, float& green, float& blue)
	{
		red = 2.5 * red - 1.0;
		if (red > 1.0)
			red = 1.0;
		else if (red < 0.0)
			red = 0.0;
		green = 2.5 * green - 1.0;
		if (green > 1.0)
			green = 1.0;
		else if (green < 0.0)
			green = 0.0;
		blue = 2.5 * blue - 1.0;
		if (blue > 1.0)
			blue = 1.0;
		else if (blue < 0.0)
			blue = 0.0;
	}
	void LightenColours(float& red, float& green, float& blue, bool lighter = false);
	void DarkenColours(float& red, float& green, float& blue);
	void RGB2HSV(const float& r, const float& g, const float& b, float& h, float& s, float& v);
	void HSV2RGB(const float& h, const float& s, const float& v, float& r, float& g, float& b);

	inline JRGB	GetRGBColor(const uint32_t& colour)
	{
		JRGB col;
		col.red = (colour & 0x00FF0000) >> 16;
		col.red |= (col.red << 8);
		col.green = (colour & 0x0000FF00) >> 8;
		col.green |= (col.green << 8);
		col.blue = (colour & 0x000000FF);
		col.blue |= (col.blue << 8);
		
		return col;
	}
	inline JRGB	GetRGBColor(float red, float green, float blue)
	{
		JRGB col;
		col.red = (unsigned short) (255.0 * red);
		col.red |= (col.red << 8);
		col.green = (unsigned short) (255.0 * green);
		col.green |= (col.green << 8);
		col.blue = (unsigned short) (255.0 * blue);
		col.blue |= (col.blue << 8);
		
		return col;
	}
	inline JRGB	GetGreyColor(float grey)
	{
		JRGB col;
		col.red = (unsigned short) (255.0 * grey);
		col.red |= (col.red << 8);
		col.green = (unsigned short) (255.0 * grey);
		col.green |= (col.green << 8);
		col.blue = (unsigned short) (255.0 * grey);
		col.blue |= (col.blue << 8);

		return col;
	}

	inline uint32_t GetCalColor(const JRGB& color)
	{
		return ((color.red & 0xFF00) << 8) | (color.green & 0xFF00) | ((color.blue & 0xFF00) >> 8);
	}

}

#endif	// CCalendarUtils_H
