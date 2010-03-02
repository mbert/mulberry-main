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
	CCalendarUtils.cp

	Author:			
	Description:	<describe the CCalendarUtils class here>
*/

#include "CCalendarUtils.h"

namespace CCalendarUtils {

#pragma mark -

void LightenColours(double& red, double& green, double& blue, bool lighter)
{
	double h = 0.0;
	double s = 0.0;
	double v = 0.0;

	RGB2HSV(red, green, blue, h, s, v);
	s = (lighter ? 0.45 : 0.6) * s;
	v = lighter ? 0.95 : 0.9;

	HSV2RGB(h, s, v, red, green, blue);
}

void DarkenColours(double& red, double& green, double& blue)
{
	double h = 0.0;
	double s = 0.0;
	double v = 0.0;

	RGB2HSV(red, green, blue, h, s, v);
	s = 1.0;
	v = 0.5;

	HSV2RGB(h, s, v, red, green, blue);
}

void RGB2HSV(const double& r, const double& g, const double& b, double& h, double& s, double& v)
{
	h = 0.0;
	s = 0.0;
	v = 0.0;

	double rgbmax = std::max(r, 0.0);
	double rgbmin = std::min(r, 1.0);
	rgbmax = std::max(g, rgbmax);
	rgbmin = std::min(g, rgbmin);
	rgbmax = std::max(b, rgbmax);
	rgbmin = std::min(b, rgbmin);
	double maxmindiff = rgbmax - rgbmin;

	if (maxmindiff == 0.0)
		h = 0.0;
	else if (rgbmax == r)
	{
		if (g >= b)
			h = 60.0 * (g - b) / maxmindiff;
		else
			h = 60.0 * (g - b) / maxmindiff + 360.0;
	}
	else if (rgbmax == g)
		h = 60.0 * (b - r) / maxmindiff + 120.0;
	else if (rgbmax == b)
		h = 60.0 * (r - g) / maxmindiff + 240.0;
	if (h >= 360.0)
		h = h - ((long)(h / 360)) * 360.0;

	if (rgbmax == 0.0)
		s = 0.0;
	else
		s = maxmindiff / rgbmax;

	v = rgbmax;
}

void HSL2RGBComponent(const double& p, const double& q, const double& tc, double& c)
{
	if (tc < 1.0/6.0)
		c = p + ((q - p) * 6.0  * tc);
	else if (tc < 1.0/2.0)
		c = q;
	else if (tc < 2.0/3.0)
		c = p + ((q - p) * 6.0  * (2.0/3.0 - tc));
	else
		c = p;
}

void HSV2RGB(const double& h, const double& s, const double& v, double& r, double& g, double& b)
{
	r = 0.0;
	g = 0.0;
	b = 0.0;

	long hi = ( (long) (h / 60.0) ) % 6L;
	double f = (h / 60.0) - hi;

	double p = v * (1.0 - s);
	double q = v * (1.0 - f * s);
	double t = v * (1.0 - (1.0 - f) * s);

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
