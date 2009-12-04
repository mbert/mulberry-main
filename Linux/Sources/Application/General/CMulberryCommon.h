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


// CMulberryCommon.h

#ifndef __CMULBERRYCOMMON__MULBERRY__
#define __CMULBERRYCOMMON__MULBERRY__

#include "cdstring.h"

class JPainter;
class JXWindow;
class JXButton;
class JRect;
class JPoint;

enum EDrawStringAlignment
{
	eDrawString_Left,
	eDrawString_Center,
	eDrawString_Right,
	eDrawStringJustify
};

enum EDrawStringClip
{
	eClipString_Left,
	eClipString_Center,
	eClipString_Right
};

// Draw clipped string if too long
void DrawClippedStringUTF8(JPainter* pDC, const char* theTxt, 
		       const JPoint& start, const JRect& clip,
		       EDrawStringAlignment align, EDrawStringClip clip_string = eClipString_Right);	

// Move window by specific amount
void MoveWindowBy(JXWindow* pWnd, int dx, int dy, bool redraw = true);

// Resize window by specific amount
void ResizeWindowBy(JXWindow* pWnd, int dx, int dy, bool redraw = true);

// Make sure some portion of title bar is on screen
void RectOnScreen(JRect& rect);

cdstring GetNumericFormat(unsigned long number);

void MessageBeep(int i);
void PlayNamedSound(const char* name);

// Stop drawing until done
class JXWidget;
class StNoRedraw
{
public:
	StNoRedraw(JXWidget* inPane);
	~StNoRedraw();
private:
	JXWidget*	mPane;
	bool	mSaveState;
};

#endif
