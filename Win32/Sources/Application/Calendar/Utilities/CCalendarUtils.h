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

/* 
	CCalendarUtils.h

	Author:			
	Description:	<describe the PPxUtils class here>
*/

#ifndef CCalendarUtils_H
#define CCalendarUtils_H

#include <stdint.h>

namespace CCalendarUtils
{

	inline double	GetAlpha(const uint32_t& colour)
	{
		return ((colour & 0xFF000000) >> 24) / 255.0;
	}
	inline double	GetRed(const uint32_t& colour)
	{
		return ((colour & 0x00FF0000) >> 16) / 255.0;
	}
	inline double	GetGreen(const uint32_t& colour)
	{
		return ((colour & 0x0000FF00) >> 8) / 255.0;
	}
	inline double	GetBlue(const uint32_t& colour)
	{
		return (colour & 0x000000FF) / 255.0;
	}
	inline void		FlattenColours(double& red, double& green, double& blue)
	{
		red = (red + 1.0) / 2.5;
		green = (green + 1.0) / 2.5;
		blue = (blue + 1.0) / 2.5;
	}
	inline void		UnflattenColours(double& red, double& green, double& blue)
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

	inline COLORREF	GetWinColor(const uint32_t& colour)
	{
		COLORREF col = RGB((colour & 0x00FF0000) >> 16, (colour & 0x0000FF00) >> 8, colour & 0x000000FF);
		return col;
	}
	inline COLORREF	GetWinColor(double red, double green, double blue)
	{
		COLORREF col = RGB((short) (255.0 * red), (short) (255.0 * green), (short) (255.0 * blue));
		return col;
	}

	inline uint32_t GetCalColor(const COLORREF& color)
	{
		return (GetRValue(color) << 16) | (GetGValue(color) << 8) | GetBValue(color);
	}

}

#endif	// CCalendarUtils_H
