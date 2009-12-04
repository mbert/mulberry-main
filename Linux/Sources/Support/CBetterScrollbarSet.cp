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


// Source for CBetterScrollbarSet class

#include "CBetterScrollbarSet.h"
#include <JXScrollableWidget.h>
#include <JXScrollbar.h>
#include <JXWindow.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>
#include <jAssert.h>

const JSize	kScrollBarWidth = 15;

/******************************************************************************
 Constructor

 ******************************************************************************/

CBetterScrollbarSet::CBetterScrollbarSet
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	CScrollbarSet(enclosure, hSizing, vSizing, x,y, w,h)
{
	mAllowHorizScroll = true;
	mAllowVertScroll = true;
	mTopIndent = false;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CBetterScrollbarSet::~CBetterScrollbarSet()
{
}

void CBetterScrollbarSet::SetAllowScroll(bool horiz, bool vert)
{
	mAllowHorizScroll = horiz;
	mAllowVertScroll = vert;

	// Now hide show as appropriate
	ShowScrollbars(JBoolean(GetHScrollbar()->WouldBeVisible() && mAllowHorizScroll),
					JBoolean(GetVScrollbar()->WouldBeVisible() && mAllowVertScroll));
}


void CBetterScrollbarSet::SetTopIndent(bool indent)
{
	// Adjust if changing
	if (mTopIndent != indent)
	{
		GetVScrollbar()->AdjustSize(0, (indent ? -1 : 1) * kScrollBarWidth);
		GetVScrollbar()->Move(0, (indent ? 1 : -1) * kScrollBarWidth);
	}
	mTopIndent = indent;
}

/******************************************************************************
 ShowScrollbars

 ******************************************************************************/

void
CBetterScrollbarSet::ShowScrollbars
	(
	const JBoolean showH,
	const JBoolean showV
	)
{
	if ((GetHScrollbar()->WouldBeVisible() == (showH && mAllowHorizScroll)) &&
		(GetVScrollbar()->WouldBeVisible() == (showV && mAllowVertScroll)))
		{
		return;
		}

	JCoordinate sEnclW = GetBoundsWidth();
	JCoordinate sEnclH = GetBoundsHeight();

	if (showH && mAllowHorizScroll)
		{
		GetHScrollbar()->Show();
		sEnclH -= kScrollBarWidth;
		}
	else
		{
		GetHScrollbar()->Hide();
		}

	if (showV && mAllowVertScroll)
		{
		GetVScrollbar()->Show();
		sEnclW -= kScrollBarWidth;
		}
	else
		{
		GetVScrollbar()->Hide();
		}

	GetHScrollbar()->SetSize(sEnclW, kScrollBarWidth);
	GetVScrollbar()->SetSize(kScrollBarWidth, sEnclH - (mTopIndent ? kScrollBarWidth : 0));

	// We do this last because JXScrollableWidget
	// will call us again.

	// Width is always total width minus scroll bar, even if scroll bar hidden
	GetScrollEnclosure()->SetSize(GetBoundsWidth() - kScrollBarWidth, sEnclH);
}
