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


// Source for CTableScrollbarSet class

#include "CTableScrollbarSet.h"

#include <JXScrollbar.h>

const JSize	kScrollBarWidth = 15;

/******************************************************************************
 Constructor

 ******************************************************************************/

CTableScrollbarSet::CTableScrollbarSet
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
	GetVScrollbar()->AdjustSize(0, -kScrollBarWidth);
	GetVScrollbar()->Move(0, kScrollBarWidth);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CTableScrollbarSet::~CTableScrollbarSet()
{
}

void CTableScrollbarSet::NoTitles()
{
	GetVScrollbar()->AdjustSize(0, kScrollBarWidth);
	GetVScrollbar()->Move(0, -kScrollBarWidth);
}

/******************************************************************************
 ShowScrollbars

 ******************************************************************************/

void
CTableScrollbarSet::ShowScrollbars
	(
	const JBoolean showH,
	const JBoolean showV
	)
{
	if (GetHScrollbar()->WouldBeVisible() == showH &&
		GetVScrollbar()->WouldBeVisible() == showV)
		{
		return;
		}

	JCoordinate sEnclW = GetBoundsWidth();
	JCoordinate sEnclH = GetBoundsHeight();

	if (showH)
		{
		GetHScrollbar()->Show();
		sEnclH -= kScrollBarWidth;
		}
	else
		{
		GetHScrollbar()->Hide();
		}

	if (showV)
		{
		GetVScrollbar()->Show();
		sEnclW -= kScrollBarWidth;
		}
	else
		{
		GetVScrollbar()->Hide();
		}

	GetHScrollbar()->SetSize(sEnclW, kScrollBarWidth);
	GetVScrollbar()->SetSize(kScrollBarWidth, sEnclH - kScrollBarWidth);

	// We do this last because JXScrollableWidget
	// will call us again.

	// Width is always total width minus scroll bar, even if scroll bar hidden
	GetScrollEnclosure()->SetSize(GetBoundsWidth() - kScrollBarWidth, sEnclH);
}
