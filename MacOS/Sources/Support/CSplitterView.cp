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


// Source for CSplitterView class

#include "CSplitterView.h"

#include "CMessageWindow.h"

#include <UAppearance.h>
#include <UGraphicUtils.h>
#include <UGAColorRamp.h>

const long cDividerWidth = 6;
const long cDividerInset = 0;
const long cDividerInsetWidth = 6;
const long cDividerGrabShort = 6;
const long cDividerGrabLong = 24;

PicHandle CSplitterView::sHorizGrab = NULL;
PicHandle CSplitterView::sVertGrab = NULL;

// Default constructor
CSplitterView::CSplitterView()
{
	InitSplitterView();
}

// Constructor from stream
CSplitterView::CSplitterView(LStream *inStream)
		: LView(inStream)
{
	InitSplitterView();
}

// Default destructor
CSplitterView::~CSplitterView()
{
}

void CSplitterView::InitSplitterView()
{
	// Init variables
	mSub1 = NULL;
	mSub2 = NULL;
	
	mDividerPos = 0;
	mMin1 = 32;
	mMin2 = 32;
	mHorizontal = true;
	mVisible1 = true;
	mVisible2 = true;
	mLock1 = false;
	mLock2 = false;
	mVisibleLock = false;
	mLockResize = false;
	
	// Make sure we have pictures
	if (!sHorizGrab)
		sHorizGrab = ::GetPicture(137);
	if (!sVertGrab)
		sVertGrab = ::GetPicture(136);
}

void CSplitterView::InitViews()
{

	// Set common pane info
	SPaneInfo pane;
	pane.visible = true;
	pane.enabled = false;
	pane.userCon = 0L;
	pane.superView = this;
	pane.bindings.left =
	pane.bindings.right =
	pane.bindings.top = 
	pane.bindings.bottom = true;
	
	// Set common view info
	SViewInfo view;
	view.imageSize.width = 0;
	view.imageSize.height = 0;
	view.scrollPos.h = 0;
	view.scrollPos.v = 0;
	view.scrollUnit.h = 1;
	view.scrollUnit.v = 1;
	view.reconcileOverhang = false;
	
	// Determine widths & heights
	UInt16 width1;
	UInt16 height1;
	UInt16 width2;
	UInt16 height2;
	UInt32 left1;
	UInt32 top1;
	UInt32 left2;
	UInt32 top2;
	
	if (mHorizontal)
	{
		// First view left/top/full width/height=divider
		left1 = 0;
		top1 = 0;
		width1 = mFrameSize.width;
		height1 = mDividerPos;
		
		// Second view left/top+divider/full width/height-top
		left2 = 0;
		top2 = mDividerPos + cDividerWidth;
		width2 = mFrameSize.width;
		height2 = mFrameSize.height - top2;
	}
	else
	{
		// First view left/top/width=divider/full height
		left1 = 0;
		top1 = 0;
		width1 = mDividerPos;
		height1 = mFrameSize.height;
		
		// Second view left+divider/top/width-left/full height
		left2 = mDividerPos + cDividerWidth;
		top2 = 0;
		width2 = mFrameSize.width - left2;
		height2 = mFrameSize.height;
	}
	
	// Do the first one
	pane.paneID = 'Spl1';
	pane.width = width1;
	pane.height = height1;
	pane.left = left1;
	pane.top = top1;
	
	// Set bindings to never resize - we do it by hand
	pane.bindings.left = false;
	pane.bindings.top = false;
	pane.bindings.right = false;
	pane.bindings.bottom = false;

	// Create first one	
	mSub1 = new LView(pane, view);
	mSub1->Enable();
	
	// Do the second one
	pane.paneID = 'Spl2';
	pane.width = width2;
	pane.height = height2;
	pane.left = left2;
	pane.top = top2;

	// Set bindings to never resize - we do it by hand
	pane.bindings.left = false;
	pane.bindings.top = false;
	pane.bindings.right = false;
	pane.bindings.bottom = false;

	// Create second one	
	mSub2 = new LView(pane, view);
	mSub2->Enable();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CSplitterView::InstallViews(LView* view1, LView* view2, bool horiz)
{
	bool old_horiz = mHorizontal;

	// Grab the current split pos in case the horizonal state changes
	long old_rel_split = GetRelativeSplitPos();

	// Set horizontal state
	mHorizontal = horiz;

	// Init divider pos before creating new subviews
	if (!mDividerPos)
	{
		// Start with split at half-way
		mDividerPos = GetTotalSize() / 2;
		
		// Readjust the relative split position
		old_rel_split = GetRelativeSplitPos();
	}
	
	// Init subviews not already done
	if (!mSub1)
		InitViews();

	// Put each view inside sub-views
	view1->PutInside(mSub1);
	view2->PutInside(mSub2);
	
	// Fit each view to parent
	mSub1->ExpandSubPane(view1, true, true);
	mSub2->ExpandSubPane(view2, true, true);

	// Check for change in horizontal state
	if (old_horiz != mHorizontal)
	{
		// Reposition the views
		RepositionViews();
		
		// Reset the split pos to the same relative position as it
		// was in th old orientation
		SetRelativeSplitPos(old_rel_split);
	}
}
			
void CSplitterView::ShowView(bool view1, bool view2)
{
	// Only if changing
	if ((mVisible1 == view1) && (mVisible2 == view2))
		return;

	// One must be visible
	mVisible1 = view1 || !view2;
	mVisible2 = view2;
	
	// Hide the unwanted ones
	if (!mVisible1 && (mSub1->GetVisibleState() != triState_Off))
		mSub1->Hide();
	if (!mVisible2 && (mSub2->GetVisibleState() != triState_Off))
		mSub2->Hide();
	
	// Adjust the contents to new view state
	AdjustContents();

	// Show the wanted ones
	if (mVisible1 && (mSub1->GetVisibleState() == triState_Off))
		mSub1->Show();
	if (mVisible2 && (mSub2->GetVisibleState() == triState_Off))
		mSub2->Show();
	
	Refresh();
}

void CSplitterView::SetMinima(SInt16 min1, SInt16 min2)
{
	bool adjust = (mMin1 != min1) || (mMin2 != min2);
	mMin1 = min1;
	mMin2 = min2;
	
	if (adjust)
		AdjustContents();
}

void CSplitterView::SetLocks(bool lock1, bool lock2)
{
	// Only one locked
	mLock1 = lock1;
	mLock2 = !lock1 && lock2;
	
	// Adjust the contents to new view state
	AdjustContents();
}

long CSplitterView::GetTotalSize() const
{
	// Get frame
	Rect frame;
	CalcLocalFrameRect(frame);
	
	return mHorizontal ? (frame.bottom - frame.top) : (frame.right - frame.left);
}

long CSplitterView::GetRelativeSplitPos() const
{
	// Get ratio of frame to split pos
	long size = GetTotalSize();
	if (size == 0)
		size = 1;
	double ratio = ((double) mDividerPos) / GetTotalSize();
	
	// Normalise to a long
	return ::roundtol(0x10000000 * ratio);
}

void CSplitterView::SetRelativeSplitPos(long split)
{
	// Unnormalise ratio
	double ratio = split / ((double) 0x10000000);
	
	// Set new divide pos and remember old
	long old_pos = mDividerPos;
	mDividerPos = ::roundtol(GetTotalSize() * ratio);

	// Make sure minima are enforced
	if (GetTotalSize() - mDividerPos < mMin2)
		mDividerPos = GetTotalSize() - mMin2;
	if (mDividerPos < mMin1)
		mDividerPos = mMin1;

	// Get actual change of divider
	long change = mDividerPos - old_pos;

	// Move and resize sub views for change if both visible
	if (mVisible1 && mVisible2)
		ChangedDividerPos(change);
}

void CSplitterView::SetPixelSplitPos(SInt16 split)
{
	// Set new divide pos and remember old
	long old_pos = mDividerPos;
	mDividerPos = split;

	// Make sure minima are enforced
	if (GetTotalSize() - mDividerPos < mMin2)
		mDividerPos = GetTotalSize() - mMin2;
	if (mDividerPos < mMin1)
		mDividerPos = mMin1;

	// Get actual change of divider
	long change = mDividerPos - old_pos;

	// Move and resize sub views for change if both visible
	if (mVisible1 && mVisible2)
		ChangedDividerPos(change);
}

void CSplitterView::ChangedDividerPos(long change)
{
	if (mHorizontal)
	{
		mSub1->ResizeFrameBy(0, change, false);
		mSub2->MoveBy(0, change, false);
		mSub2->ResizeFrameBy(0, -change, false);
	}
	else
	{
		mSub1->ResizeFrameBy(change, 0, false);
		mSub2->MoveBy(change, 0, false);
		mSub2->ResizeFrameBy(-change, 0, false);
	}
	
	Refresh();
}

// Allow split pane change
void CSplitterView::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Only if both visible
	if (!mVisible1 || !mVisible2)
	{
		LView::ClickSelf(inMouseDown);
		return;
	}

	// Only if unlocked
	if (mLock1 || mLock2)
	{
		LView::ClickSelf(inMouseDown);
		return;
	}
	
	// Get divider rect
	Rect divider;
	CalcDividerRect(divider);

	// Get frame
	Rect frame;
	CalcLocalFrameRect(frame);

	FocusDraw();
	
	// Look for click in divide
	if (::PtInRect(inMouseDown.whereLocal, &divider))
	{
		// Create gray line region
		RgnHandle gray_line = ::NewRgn();
		if (mHorizontal)
		{
			divider.top++;
			divider.bottom--;
		}
		else
		{
			divider.left++;
			divider.right--;
		}
		::RectRgn(gray_line, &divider);

		// Create limit rect and adjust for minimum sizes
		Rect limit = frame;
		if (mHorizontal)
		{
			limit.top += mMin1;
			limit.bottom -= mMin2;
		}
		else
		{
			limit.left += mMin1;
			limit.right -= mMin2;
		}

		// Create slop rect
		Rect slop = limit;
		::InsetRect(&slop, -64, -64);
		slop.top = 0;

		// Drag gray line
		SInt32 moved = ::DragGrayRgn(gray_line, inMouseDown.whereLocal, &limit ,&slop, mHorizontal ? vAxisOnly : hAxisOnly, NULL);

		// Clear up
		::DisposeRgn(gray_line);

		// Check for actual move
		if (moved && (moved != 0x80008000))
		{
			SInt32 change = (mHorizontal ? HiWord(moved) : LoWord(moved));

			// Set divider to new pos
			mDividerPos += change;

			// Move and resize sub views for change
			ChangedDividerPos(change);
		}
	}
	else
	{
		LView::ClickSelf(inMouseDown);
		return;
	}
}

// Display column change cursor
void CSplitterView::AdjustMouseSelf(Point inPortPt, const EventRecord& inMacEvent, RgnHandle outMouseRgn)
{
	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	// Only if unlocked
	if (mLock1 || mLock2)
		return;
	
	// Get divider rect
	Rect divider;
	CalcDividerRect(divider);

	// Check for mouse over divider
	PortToLocalPoint(inPortPt);
	if (::PtInRect(inPortPt, &divider))
	{
		// Loca the cursor
		UCursor::SetThemeCursor(mHorizontal ? kThemeResizeUpDownCursor : kThemeResizeLeftRightCursor);

		// Set region to the divider rect
		LocalToPortPoint(topLeft(divider));
		LocalToPortPoint(botRight(divider));
		::RectRgn(outMouseRgn, &divider);
	}
}

void CSplitterView::CalcDividerRect(Rect& divRect)
{
	// If a lock is present and divider is not visible when locked
	// then return an empty rect
	if ((mLock1 || mLock2) && !mVisibleLock)
	{
		divRect.left = 0;
		divRect.right = 0;
		divRect.top = 0;
		divRect.bottom = 0;
	}
	else
	{
		// Start with frame
		CalcLocalFrameRect(divRect);
		
		// Adjust for divider pos
		if (mHorizontal)
		{	
			divRect.top = mDividerPos + cDividerInset;
			divRect.bottom = divRect.top + cDividerInsetWidth;
		}
		else
		{
			divRect.left = mDividerPos + cDividerInset;
			divRect.right = divRect.left + cDividerInsetWidth;
		}
	}
}

void CSplitterView::DrawSelf()
{
	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	// Get divider rect
	Rect divider;
	CalcDividerRect(divider);

	// Ignore if empty
	if (::EmptyRect(&divider))
		return;

	// Need to repaint background adjusting origin for Aqua drawing
	{
		SInt16	bitDepth;
		bool	hasColor;
		GetDeviceInfo(bitDepth, hasColor);

		Rect background = divider;
		::OffsetRect(&background, -mPortOrigin.h, -mPortOrigin.v);

		StClipOriginState	saveClipOrigin(Point_00);
		UAppearance::ApplyThemeBackground(kThemeBackgroundWindowHeader, &background, IsActive() ? kThemeStateActive : kThemeStateInactive, bitDepth, hasColor);
		::EraseRect(&background);
	}

	// Get pict area
	Rect pict_area = divider;
	if (mHorizontal)
	{	
		pict_area.left = (pict_area.left + pict_area.right - cDividerGrabLong) / 2;
		pict_area.right = pict_area.left + cDividerGrabLong;
		
		if (sHorizGrab)
			::DrawPicture(sHorizGrab, &pict_area);
	}
	else
	{
		pict_area.top = (pict_area.top + pict_area.bottom - cDividerGrabLong) / 2;
		pict_area.bottom = pict_area.top + cDividerGrabLong;
		
		if (sVertGrab)
			::DrawPicture(sVertGrab, &pict_area);
	}	
}

// Adjust max value of split divider
void CSplitterView::ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh)
{
	// Do inherited
	LView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);

	// Adjust divider
	if (mVisible1 && mVisible2 && !mLockResize)
	{
		// If second pane is locked, move divider by resize amount
		if (mLock2)
			// Resize first by full amount
			mDividerPos += (mHorizontal ? inHeightDelta : inWidthDelta);
			
		// Else if neither pane is locked we move divider by ratio
		// otherwise when first pane is locked we leave divider alone
		else if (!mLock1)
		{
			// Get old and new sizes
			SDimension16 psize;
			GetFrameSize(psize);
			SInt16 new_size = (mHorizontal ? psize.height : psize.width);
			SInt16 old_size = new_size - (mHorizontal ? inHeightDelta : inWidthDelta);
			if (mDividerPos == 0)
				mDividerPos = 1;
			double ratio = (1.0 * old_size) / mDividerPos;
			if (ratio == 0.0)
				ratio = 1.0;

			// Resize both by proportional amount
			mDividerPos = new_size / ratio;
		}
	}

	// Adjust the contents
	AdjustContents();
}

void CSplitterView::RepositionViews()
{
	// Expand sub-panes to fill entire splitter
	ExpandSubPane(mSub1, true, true);
	ExpandSubPane(mSub2, true, true);

	// Do normal contents adjustment which will reposition and resize the sub-panes
	AdjustContents();
}

void CSplitterView::AdjustContents()
{
	// Don't adjust if nothing present
	if (!mSub1 || !mSub2)
		return;

	// Fit to current divider details
	if (mVisible1 ^ mVisible2)
	{
		// Get the only one visible
		LView* visible = (mVisible1 ? mSub1 : mSub2);
		
		// Fit to enclosure
		ExpandSubPane(visible, true, true);
	}
	else
	{
		// Get current full size
		SDimension16 psize;
		GetFrameSize(psize);
		
		// Make sure divider minima are correct
		if ((mHorizontal ? psize.height : psize.width) - mDividerPos < mMin2)
			mDividerPos = (mHorizontal ? psize.height : psize.width) - mMin2;
		else if (mDividerPos < mMin1)
			mDividerPos = mMin1;

		// Resize to divider pos
		SDimension16 size;
		mSub1->GetFrameSize(size);
		if (mHorizontal)
		{
			size.width = psize.width;
			size.height = mDividerPos;
		}
		else
		{
			size.width = mDividerPos;
			size.height = psize.height;
		}
		mSub1->ResizeFrameTo(size.width, size.height, true);
		
		// Determine the gap between frames
		// If its locked the divider might not be visible
		SInt16 gap = 0;
		if (!mLock1 && !mLock2 || mVisibleLock)
			gap = cDividerWidth;

		// Resize to remainder
		mSub2->GetFrameSize(size);
		if (mHorizontal)
		{
			size.width = psize.width;
			size.height = psize.height - mDividerPos - gap;
		}
		else
		{
			size.width = psize.width - mDividerPos - gap;
			size.height = psize.height;
		}
		mSub2->ResizeFrameTo(size.width, size.height, true);
		
		// Move to pos
		SPoint32 loc;
		GetFrameLocation(loc);
		SPoint32 subloc;
		mSub2->GetFrameLocation(subloc);
		if (mHorizontal)
			mSub2->MoveBy(0, loc.v + mDividerPos + gap - subloc.v, true);
		else
			mSub2->MoveBy(loc.h + mDividerPos + gap - subloc.h, 0, true);
	}
}
