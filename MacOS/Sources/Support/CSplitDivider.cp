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


// Source for CSplitDivider class

#include "CSplitDivider.h"

#include "CMessageWindow.h"



// Default constructor
CSplitDivider::CSplitDivider()
{
	mHorizontal = true;
	mValueMessage = msg_SplitDividerMove;
}

// Constructor from stream
CSplitDivider::CSplitDivider(LStream *inStream)
		: LControl(inStream)
{
	mHorizontal = true;
	mValueMessage = msg_SplitDividerMove;
}

// Default destructor
CSplitDivider::~CSplitDivider()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Allow split pane change
void CSplitDivider::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Get split pane frame
	Rect frame;
	CalcLocalFrameRect(frame);
	short click_diff = inMouseDown.whereLocal.v - frame.top;

	// Create gray line region
	RgnHandle gray_line = ::NewRgn();
	Rect line = {frame.top + 1, frame.left, frame.top + 2, frame.right - 1};
	LocalToPortPoint(topLeft(line));
	LocalToPortPoint(botRight(line));
	PortToGlobalPoint(topLeft(line));
	PortToGlobalPoint(botRight(line));
	::RectRgn(gray_line, &line);

	// Create limit rect from control max/min
	Rect limit = {frame.top + mMinValue - mValue + click_diff, frame.left, frame.bottom + mMaxValue - mValue, frame.right - 1};
	LocalToPortPoint(topLeft(limit));
	LocalToPortPoint(botRight(limit));
	PortToGlobalPoint(topLeft(limit));
	PortToGlobalPoint(botRight(limit));

	// Create slop rect
	Rect slop = limit;
	::InsetRect(&slop, -64, -64);
	slop.top = 0;

	// Get start point
	Point startPt = inMouseDown.whereLocal;
	LocalToPortPoint(startPt);
	PortToGlobalPoint(startPt);

	// Focus on desktop
	GrafPtr save_port;
	::GetPort(&save_port);
	::SetPort(UScreenPort::GetScreenPort());

	// Drag gray line
	SInt32 moved = ::DragGrayRgn(gray_line, startPt, &limit ,&slop, vAxisOnly, nil);

	// Clear up
	::DisposeRgn(gray_line);
	::SetPort(save_port);

	// Adjust column only if moved legally
	if (moved != 0x80008000)
		IncrementValue(moved >> 16);

}

// Display column change cursor
void CSplitDivider::AdjustMouseSelf(Point inPortPt, const EventRecord& inMacEvent, RgnHandle outMouseRgn)
{
	// Set cursor
	UCursor::SetThemeCursor(mHorizontal ? kThemeResizeUpDownCursor : kThemeResizeLeftRightCursor);
}

void CSplitDivider::DrawSelf()
{
	StColorPenState::Normalize();
	Rect	frame;
	CalcLocalFrameRect(frame);
	::EraseRect(&frame);
	::MoveTo(frame.left, frame.top);
	::LineTo(frame.right-1, frame.top);
	::MoveTo(frame.left, frame.top+2);
	::LineTo(frame.right-1, frame.top+2);
}
