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
	CCalendarUtils.cp

	Author:			
	Description:	<describe the CCalendarUtils class here>
*/

#include "CCalendarUtils.h"

namespace CCalendarUtils {

#pragma mark -

void LightenColours(float& red, float& green, float& blue, bool lighter)
{
	float h = 0.0F;
	float s = 0.0F;
	float v = 0.0F;

	RGB2HSV(red, green, blue, h, s, v);
	s = (lighter ? 0.45F : 0.6F) * s;
	v = lighter ? 0.95F : 0.9F;

	HSV2RGB(h, s, v, red, green, blue);
}

void DarkenColours(float& red, float& green, float& blue)
{
	float h = 0.0F;
	float s = 0.0F;
	float v = 0.0F;

	RGB2HSV(red, green, blue, h, s, v);
	s = 1.0F;
	v = 0.5F;

	HSV2RGB(h, s, v, red, green, blue);
}

void RGB2HSV(const float& r, const float& g, const float& b, float& h, float& s, float& v)
{
	h = 0.0F;
	s = 0.0F;
	v = 0.0F;

	float rgbmax = std::max(r, 0.0F);
	float rgbmin = std::min(r, 1.0F);
	rgbmax = std::max(g, rgbmax);
	rgbmin = std::min(g, rgbmin);
	rgbmax = std::max(b, rgbmax);
	rgbmin = std::min(b, rgbmin);
	float maxmindiff = rgbmax - rgbmin;

	if (maxmindiff == 0.0F)
		h = 0.0F;
	else if (rgbmax == r)
	{
		if (g >= b)
			h = 60.0F * (g - b) / maxmindiff;
		else
			h = 60.0F * (g - b) / maxmindiff + 360.0F;
	}
	else if (rgbmax == g)
		h = 60.0F * (b - r) / maxmindiff + 120.0F;
	else if (rgbmax == b)
		h = 60.0F * (r - g) / maxmindiff + 240.0F;
	if (h >= 360.0F)
		h = h - ((long)(h / 360)) * 360.0F;

	if (rgbmax == 0.0F)
		s = 0.0F;
	else
		s = maxmindiff / rgbmax;

	v = rgbmax;
}

void HSL2RGBComponent(const float& p, const float& q, const float& tc, float& c)
{
	if (tc < 1.0F/6.0F)
		c = p + ((q - p) * 6.0F  * tc);
	else if (tc < 1.0F/2.0F)
		c = q;
	else if (tc < 2.0F/3.0F)
		c = p + ((q - p) * 6.0F  * (2.0F/3.0F - tc));
	else
		c = p;
}

void HSV2RGB(const float& h, const float& s, const float& v, float& r, float& g, float& b)
{
	r = 0.0F;
	g = 0.0F;
	b = 0.0F;

	long hi = ( (long) (h / 60.0F) ) % 6L;
	float f = (h / 60.0F) - hi;

	float p = v * (1.0F - s);
	float q = v * (1.0F - f * s);
	float t = v * (1.0F - (1.0F - f) * s);

	switch(hi)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	case 5:
		r = v;
		g = p;
		b = q;
		break;
	}
}

} // namespace CCalendarUtils
