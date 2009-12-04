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


// Source for CTooltip class

#include "CTooltip.h"

#include "CTextDisplay.h"

const int cHBorder = 8;
const int cVBorder = 4;
const int cVBottomOffset = 4;
const int cVTopOffset = 2;

CTooltip* CTooltip::sTooltip = NULL;

// Constructor from stream
CTooltip::CTooltip(LStream *inStream)
	: LWindow(inStream)
{
	mTxt = NULL;
	mEnterTime = 0;
	mTarget = NULL;
	mTargetPos.h = mTargetPos.v = 0;
	mAutoPos = true;
	mEnabled = true;
	
	sTooltip = this;
}

// Default destructor
CTooltip::~CTooltip()
{
	sTooltip = NULL;
}

void CTooltip::ShowTooltip(LPane* target, const cdstring& txt)
{
	// Must have tooltip window
	if (!sTooltip)
		InitTooltip();

	// Start it
	SPoint32 pos = {0, 0};
	sTooltip->StartTooltip(target, pos, true, txt);
}

void CTooltip::ShowTooltip(LPane* target, SPoint32 pos, const cdstring& txt)
{
	// Must have tooltip window
	if (!sTooltip)
		InitTooltip();

	// Start it
	sTooltip->StartTooltip(target, pos, false, txt);
}

void CTooltip::HideTooltip()
{
	// Simply hide the window if it exists
	if (sTooltip)
		sTooltip->StopTooltip();
}

void CTooltip::EnableTooltips(bool enable)
{
	// Simply hide the window if it exists
	if (sTooltip)
		sTooltip->EnableTooltip(enable);
}

void CTooltip::FinishCreateSelf(void)
{
	// Do inherited
	LWindow::FinishCreateSelf();
	
	// Find text pane
	mTxt = (CTextDisplay*) FindPaneByID(paneid_TooltipText);
	Rect margins = { 1, 1, 1, 1 };  /* t,l,b,r */
	mTxt->SetMargins(margins);
}

void CTooltip::SpendTime(const EventRecord& inMacEvent)
{
	// See whether timeout has occurred
	if (mEnabled && (::TickCount() > mEnterTime + 30))
	{
		// Show the tooltip
		ShowIt();
		
		// Stop further periodical
		StopIdling();
	}
}

void CTooltip::InitTooltip()
{
	// Create tooltip window
	sTooltip = (CTooltip*) LWindow::CreateWindow(paneid_TooltipWindow, NULL);
}

void CTooltip::StartTooltip(LPane* target, SPoint32 pos, bool auto_pos, const cdstring& txt)
{
	// Cache items
	mTarget = target;
	mTargetPos = pos;
	mAutoPos = auto_pos;
	mTitle = txt;
	
	// Get current time
	mEnterTime = ::TickCount();
	
	// Start idling
	StartIdling();
}

void CTooltip::ShowIt()
{
	// Not if target is missing
	if (!mTarget)
		return;

	// If text is empty, do not show
	if (mTitle.empty())
		return;

	// Set text in static text
	mTxt->SetText(mTitle);
	
	// Now resize width to lareg value for resize of window
	mTxt->ResizeFrameBy(1000, 0, false);
	
	// Get width of text
	unsigned long width = mTxt->GetFullWidth();
	width += 8;

	// Get height of text
	unsigned long height = mTxt->GetFullHeight();
	//height += 4;

	// Now resize width back
	mTxt->ResizeFrameBy(-1000, 0, false);
	
	// Resize the tooltip window width only
	ResizeWindowTo(width, height);	

	// Get screen rect of target pane
	Rect target_rect;
	mTarget->CalcPortFrameRect(target_rect);
	mTarget->PortToGlobalPoint(topLeft(target_rect));
	mTarget->PortToGlobalPoint(botRight(target_rect));

	// Position for tooltip window
	SPoint16 new_pos;
	if (mAutoPos)
	{
		new_pos.h = (target_rect.right + target_rect.left - width)/2;
		new_pos.v = target_rect.bottom + cVBottomOffset;
	}
	else
	{
		// Center horizontally, align to top
		new_pos.h = mTargetPos.h - width/2;
		new_pos.v = mTargetPos.v + cVBottomOffset;
	}

	// Get the target's screen's rectangle or default
	GDHandle screen = UWindows::FindDominantDevice(target_rect);
	if (screen == NULL)
		screen = ::GetMainDevice();
	Rect* screen_rect = &(**screen).gdRect;

	// Shift left edge on screen
	if (new_pos.h < 0)
		new_pos.h = 0;

	// Shift right edge on screen
	if (new_pos.h + width > screen_rect->right)
		new_pos.h = screen_rect->right - width;
	
	// If bottom off screen, put tooltip above target
	if (new_pos.v + height > screen_rect->bottom)
		new_pos.v = target_rect.top - height - cVTopOffset;

	// Move it correct place
	MoveWindowTo(new_pos.h, new_pos.v);
	
	// Show it and do immediate update
	Select();
	Show();
	UpdatePort();
}

void CTooltip::StopTooltip()
{
	// Simply hide the window
	if (GetVisibleState() != triState_Off)
	{
		Hide();
		UDesktop::NormalizeWindowOrder();
		
		// Force immediate redraw of area under tooltip
		if (mTarget)
			mTarget->UpdatePort();
	}
	mTarget = NULL;
	
	// Turn ff idling
	if (IsIdling())
		StopIdling();
}

void CTooltip::EnableTooltip(bool enable)
{
	// Simply hide the window
	if (mEnabled ^ enable)
	{
		mEnabled = enable;
		
		if (!mEnabled)
			StopTooltip();
	}
}