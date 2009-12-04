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


#ifndef __CFORMATELEMENT__MULBERRY__
#define __CFORMATELEMENT__MULBERRY__

#include "ETag.h"

#include "cdstring.h"

#if __dest_os == __win32_os
#include "jWin.h"
#include "Mac2Win.h"
#endif

#if __dest_os == __linux_os
class CFormattedTextDisplay;
typedef CFormattedTextDisplay CDisplayFormatter;

#include <jColor.h>

#elif __dest_os == __mac_os || __dest_os == __mac_os_x
#define CDisplayFormatter CFormattedTextDisplay
class CDisplayFormatter;
#else
class CDisplayFormatter;
#endif

class CFormatElement
{
public:
	CFormatElement();
	CFormatElement(SInt32 start, SInt32 stop);
	virtual ~CFormatElement();
	void setStart(SInt32 start);
	SInt32 getStart();
	void setStop(SInt32 stop);
	virtual void draw(CDisplayFormatter *display)=0;
	CFormatElement *next;
protected:
	int start;
	int stop;
};


class CFontFormatElement : public CFormatElement
{
public:
	CFontFormatElement();
	CFontFormatElement(SInt32 start, SInt32 stop, const char *font);
	virtual ~CFontFormatElement();
	void draw(CDisplayFormatter *display);
private:
	cdstring font;
};


class CColorFormatElement : public CFormatElement
{
public:
	CColorFormatElement();
	CColorFormatElement(SInt32 start, SInt32 stop, int r, int g, int b);
	CColorFormatElement(SInt32 start, SInt32 stop, RGBColor color);
	CColorFormatElement(SInt32 start, SInt32 stop, const char* color, bool background = false);
	void draw(CDisplayFormatter *display);
	void setColor(int r, int g, int b);
	RGBColor getColor() const
		{ return color; }
private:
	RGBColor color;
};


class CFontSizeFormatElement : public CFormatElement
{
public:
	CFontSizeFormatElement();
	CFontSizeFormatElement(SInt32 start, SInt32 stop, short change, bool add = true, bool override = false);
	void draw(CDisplayFormatter *display);
private:
	short sizeChange;
	bool adding;
	bool overrideprefs;
};

class CAlignmentFormatElement : public CFormatElement
{
public:
	CAlignmentFormatElement();
	CAlignmentFormatElement(SInt32 start, SInt32 stop, ETag tag);
	void draw(CDisplayFormatter *display);
private:
	ETag tagid;
};

class CFaceFormatElement : public CFormatElement
{
public:
	CFaceFormatElement();
	CFaceFormatElement(SInt32 start, SInt32 stop, ETag tag);
	void draw(CDisplayFormatter *display);
private:
	ETag tagid;
};

class CExcerptFormatElement : public CFormatElement
{
public:
	CExcerptFormatElement();
	CExcerptFormatElement(SInt32 start, SInt32 stop);
	CExcerptFormatElement(SInt32 start, SInt32 stop, char ex);
	void draw(CDisplayFormatter *display);
private:
	char Excerpt;
};

#endif
