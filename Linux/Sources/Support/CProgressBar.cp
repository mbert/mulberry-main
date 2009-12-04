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


// Source for CProgressBar class

#include "CProgressBar.h"

#include "StPenState.h"

#include <JXWindowPainter.h>
#include <JXColormap.h>
#include <jAssert.h>
#include <jXGlobals.h>

// Constants
const unsigned long cBandWidth = 10;						// Width of coloured band area
const unsigned long cBandGap = 10;						// Width of backgroung band area
const unsigned long cBandSize = cBandWidth + cBandGap;	// Total band width

// Constructor
CProgressBar::CProgressBar(JXContainer* enclosure, const HSizingOption hSizing, const VSizingOption vSizing,
	const JCoordinate x, const JCoordinate y, const JCoordinate w, const JCoordinate h)
	: JXProgressIndicator(enclosure, hSizing, vSizing, x,y, w,h)
{
	mIndeterminate = false;
	mCtr = 0;
}

// Start progress
void CProgressBar::Start()
{
	mLastTime = JXGetApplication()->GetCurrentTime();
	mCtr = 0;
	Redraw();
}

// Spend Time
void CProgressBar::SpendTime()
{
	// Update display every 20 ms
	Time now = JXGetApplication()->GetCurrentTime();
	if (now - mLastTime > 20)
	{
		mLastTime = now;
		mCtr++;
		if (mCtr > cBandSize)
			mCtr = 0;
		Redraw();
	}
}

// Draw
void CProgressBar::Draw(JXWindowPainter& p, const JRect& rect)
{
	StPenState save(&p);

	unsigned long numBands = rect.width() / cBandSize + 3;
	for(JCoordinate y = rect.top; y <= rect.bottom; y++)
	{
		JIndex left = rect.left - 2 * cBandSize + mCtr + y - rect.top;
		p.SetPenColor(p.GetColormap()->GetBlackColor());
		for(JIndex i = 0; i < numBands; i++)
		{
			p.Line(left + i * cBandSize, y, left + i * cBandSize + cBandWidth, y);
		}

		p.SetPenColor(p.GetColormap()->GetDefaultBackColor());
		for(JIndex i = 0; i < numBands; i++)
		{
			p.Line(left + i * cBandSize + cBandWidth, y, left + i * cBandSize + cBandSize, y);
		}
	}
}
