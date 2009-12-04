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

void DrawClippedStringUTF8(CDC* pDC, const char* theTxt, CPoint start, CRect clip,
						EDrawStringAlignment align, EDrawStringClip clip_string = eClipString_Right);	// Draw clipped string if too long

void MoveWindowBy(CWnd* pWnd, int dx, int dy, bool redraw = true);			// Move window by specific amount
void MoveWindowTo(CWnd* pWnd, int cx, int cy, bool redraw = true);			// Move window to specific position
void ResizeWindowBy(CWnd* pWnd, int dx, int dy, bool redraw = true);		// Resize window by specific amount
void ResizeWindowTo(CWnd* pWnd, int cx, int cy, bool redraw = true);		// Resize window to specific size
void ExpandChildToFit(CWnd* parent, CWnd* child, bool horiz, bool vert, int border = 0);	// Expand child to size of client area in parent

void RectOnScreen(CRect& rect, CWnd* owner);					// Make sure some portion of title bar is on screen

cdstring GetNumericFormat(unsigned long number);

void PlayNamedSound(const cdstring& name);

UINT AppendMenuFlags(int& pos, bool separator);

bool BrowseForFolder(const cdstring& title, cdstring& folder, CWnd* owner);
bool GetSpecialFolderPath(int nFolder, cdstring& folder);
bool ResolveShortcut(cdstring& path);

UINT CheckboxToggle(CButton* btn);
void OnUpdateMenuTitle(CCmdUI* pCmdUI, CString& new_title);


// Stop drawing until done
class StNoRedraw
{
public:
					StNoRedraw(CWnd* wnd);
					~StNoRedraw();
private:
	CWnd*			mWnd;
	bool			mWasVisible;
};

#endif
