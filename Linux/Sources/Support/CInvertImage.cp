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


// Source for CInvertImage class

#include "CInvertImage.h"

#include <JXColormap.h>

CInvertImage::CInvertImage(const JXImage& copy) : JXImage(copy)
{
}

CInvertImage::~CInvertImage()
{
}

void CInvertImage::InvertImage()
{
	XImage* ximage = GetImage();

	JXColormap* colormap = GetXColormap();
	const JCoordinate width  = GetWidth();
	const JCoordinate height = GetHeight();
	for (JCoordinate y=0; y<height; y++)
	{
		for (JCoordinate x=0; x<width; x++)
		{
			unsigned long xPixel = XGetPixel(ximage, x,y);
			unsigned long p1 = xPixel & 0xFF;
			unsigned long p2 = (xPixel >> 8) & 0xFF;
			unsigned long p3 = (xPixel >> 16) &0xFF;
			p1 = 255 - p1;
			p2 = 255 - p2;
			p3 = 255 - p3;
			xPixel = p1 | (p2 << 8) | (p3 << 16);
			XPutPixel(ximage, x,y, xPixel);
		}
	}
	ConvertToDefaultState();
}
