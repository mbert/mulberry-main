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


// Source for CScrollbarSet class

#include "CScrollbarSet.h"

#include "CScrollBar.h"

#include <JXWindowPainter.h>
#include <jXPainterUtil.h>
#include <jAssert.h>

const JSize	kScrollBarWidth = 15;

/******************************************************************************
 Constructor

 ******************************************************************************/

CScrollbarSet::CScrollbarSet
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
	JXScrollbarSet(enclosure, hSizing, vSizing, x,y, w,h)
{
	// Replace JX scrollers with our own fancy ones
	delete itsHScrollbar;
	itsHScrollbar =
		new CScrollBar(this, kHElastic, kFixedBottom,
						0,h-kScrollBarWidth, w-kScrollBarWidth,kScrollBarWidth);
	assert( itsHScrollbar != NULL );
	itsHScrollbar->Hide();

	delete itsVScrollbar;
	itsVScrollbar =
		new CScrollBar(this, kFixedRight, kVElastic,
						w-kScrollBarWidth,0, kScrollBarWidth,h-kScrollBarWidth);
	assert( itsVScrollbar != NULL );
	itsVScrollbar->Hide();
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CScrollbarSet::~CScrollbarSet()
{
}

void CScrollbarSet::DrawBorder(JXWindowPainter& p, const JRect& frame)
{
	// Only if border width set
	const JSize w = GetBorderWidth();
	if (w > 0)
		JXDrawDownFrame(p, frame, w);
}
