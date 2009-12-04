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


#include "CSplitterView.h"

#include "HResourceMap.h"

#include "lround.h"

#include <JXDragPainter.h>
#include <JXColormap.h>
#include <JXCursor.h>
#include <JXImage.h>
#include <jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

const long cDividerWidth = 6;

CSplitterView::CSplitterView
	(
	const JArray<JCoordinate>&	sizes,
	const JIndex				elasticIndex,
	const JArray<JCoordinate>&	minSizes,
	JXContainer*				enclosure,
	const HSizingOption			hSizing,
	const VSizingOption			vSizing,
	const JCoordinate			x,
	const JCoordinate			y,
	const JCoordinate			w,
	const JCoordinate			h
	)
	:
	JXPartition(sizes, elasticIndex, minSizes,
				enclosure, hSizing, vSizing, x,y, w,h)
{
	itsDragType = kInvalidDrag;
	SetDefaultCursor(JXGetDragHorizLineCursor(GetDisplay()));
	itsDragAllLineCursor = JXGetDragAllHorizLineCursor(GetDisplay());

	SetElasticSize();

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
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CSplitterView::~CSplitterView()
{
}

void CSplitterView::InitViews()
{
	// Set proper cursors
	if (mHorizontal)
	{
		SetDefaultCursor(JXGetDragHorizLineCursor(GetDisplay()));
		itsDragAllLineCursor = JXGetDragAllHorizLineCursor(GetDisplay());
	}
	else
	{
		SetDefaultCursor(JXGetDragVertLineCursor(GetDisplay()));
		itsDragAllLineCursor = JXGetDragAllVertLineCursor(GetDisplay());
	}

	// Determine the gap between frames
	// If its locked the divider might not be visible
	long gap = 0;
	if (!mLock1 && !mLock2 || mVisibleLock)
		gap = cDividerWidth;

	// Init subviews not already done
	JCoordinate size = GetTotalSize();
	JArray<JCoordinate> sizes;
	sizes.AppendElement(mDividerPos);
	sizes.AppendElement(size - mDividerPos - gap);

	mSub1 = CreateCompartment(0, 1, sizes.GetElement(1));
	mSub1->Activate();

	mSub2 = CreateCompartment(0, 2, sizes.GetElement(2));
	mSub2->Activate();

	// Never allow sizes less than 16
	if (sizes.GetElement(1) < 16)
		sizes.SetElement(1, 16);
	if (sizes.GetElement(2) < 16)
		sizes.SetElement(2, 16);

	// Set all compartment sizes
	JPartition::SetCompartmentSizes(sizes);

	// Reset minimum size
	SetMinima(mMin1, mMin2);

	mHorizSplit = bmpFromResource(IDB_SPLITTERHORIZ, this);
	mVertSplit = bmpFromResource(IDB_SPLITTERVERT, this);
}
			
void CSplitterView::InstallViews(JXWidget* view1, JXWidget* view2, bool horiz)
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
		JCoordinate size = GetTotalSize();
		mDividerPos = size / 2;
		
		// Readjust the relative split position
		old_rel_split = GetRelativeSplitPos();
	}
	
	// Init subviews not already done
	if (!mSub1)
		InitViews();

	// Put each view inside sub-views
	view1->SetEnclosure(mSub1);
	view2->SetEnclosure(mSub2);
	
	// Fit each view to parent
	view1->FitToEnclosure(kTrue, kTrue);
	view2->FitToEnclosure(kTrue, kTrue);

	// Check for change in horizontal state
	if (old_horiz != mHorizontal)
	{
		// Reposition the views
		RepositionViews();
		
		// Reset the split pos to the same relative position as it
		// was in th old orientation
		SetRelativeSplitPos(old_rel_split);
		
		// Set proper cursors
		if (mHorizontal)
		{
			SetDefaultCursor(JXGetDragHorizLineCursor(GetDisplay()));
			itsDragAllLineCursor = JXGetDragAllHorizLineCursor(GetDisplay());
		}
		else
		{
			SetDefaultCursor(JXGetDragVertLineCursor(GetDisplay()));
			itsDragAllLineCursor = JXGetDragAllVertLineCursor(GetDisplay());
		}
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
	if (!mVisible1)
		mSub1->Hide();
	if (!mVisible2)
		mSub2->Hide();

	// Adjust the contents to new view state
	AdjustContents();

	// Show the wanted ones
	if (mVisible1)
		mSub1->Show();
	if (mVisible2)
		mSub2->Show();
}

void CSplitterView::SetMinima(long min1, long min2)
{
	bool adjust = (mMin1 != min1) || (mMin2 != min2);
	mMin1 = min1;
	mMin2 = min2;
	
	SetMinCompartmentSize(GetCompartment(1), mMin1);
	SetMinCompartmentSize(GetCompartment(2), mMin2);

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

long CSplitterView::GetRelativeSplitPos() const
{
	// Get total size
	JCoordinate total = GetTotalSize();
	if (total == 0)
		total = 1;

	// Get ratio of frame to split pos
	double ratio = ((double) mDividerPos) / total;
	
	// Normalise to a long
	return ::lround(0x10000000 * ratio);
}

void CSplitterView::SetRelativeSplitPos(long split)
{
	// Not if locked
	if (mLock1 || mLock2)
		return;

	// Get total size
	JCoordinate total = GetTotalSize();
	
	// Unnormalise ratio
	double ratio = split / ((double) 0x10000000);
	
	// Set new divide pos
	SetPixelSplitPos(::lround(total * ratio));
}

void CSplitterView::SetPixelSplitPos(long split)
{
	// If its zero, use half
	if (split == 0)
		split = GetTotalSize() / 2;

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
	if (change && mVisible1 && mVisible2)
	{
		// Must adjust the compartments
		JArray<JCoordinate> sizes(GetCompartmentSizes());

		// Set new sizes
		sizes.SetElement(1, mDividerPos);
		sizes.SetElement(2, GetTotalSize() - mDividerPos - cDividerWidth);

		// Now set the new sizes
		JPartition::SetCompartmentSizes(sizes);

		// Do actual sub-pane change
		ChangedDividerPos(change);
	}
}

void CSplitterView::ChangedDividerPos(long change)
{
	// Determine the gap between frames
	// If its locked the divider might not be visible
	long gap = 0;
	if (!mLock1 && !mLock2 || mVisibleLock)
		gap = cDividerWidth;

	// Set new sizes
	JArray<JCoordinate> sizes(GetCompartmentSizes());
	sizes.SetElement(1, mDividerPos);
	sizes.SetElement(2, GetTotalSize() - mDividerPos - gap);

	// Never allow sizes less than 16
	if (sizes.GetElement(1) < 16)
		sizes.SetElement(1, 16);
	if (sizes.GetElement(2) < 16)
		sizes.SetElement(2, 16);

	if (mHorizontal)
	{
		mSub1->AdjustSize(0, change);
		mSub2->Move(0, change);
		mSub2->AdjustSize(0, -change);
	}
	else
	{
		mSub1->AdjustSize(change, 0);
		mSub2->Move(change, 0);
		mSub2->AdjustSize(-change, 0);
	}
	
	Refresh();
}

/******************************************************************************
 GetTotalSize (virtual)

 ******************************************************************************/

JCoordinate
CSplitterView::GetTotalSize()
	const
{
	return mHorizontal ? GetApertureHeight() : GetApertureWidth();
}

/******************************************************************************
 CreateCompartment (virtual)

	Adjust the width of each compartment.

 ******************************************************************************/

JXWidgetSet*
CSplitterView::CreateCompartment
	(
	const JIndex index,
	const JCoordinate position,
	const JCoordinate size
	)
{
	JXWidgetSet* compartment = NULL;
	if (mHorizontal)
		compartment = new JXWidgetSet(this, kHElastic, kFixedTop,
										0,position, GetApertureWidth(), size);
	else
		compartment = new JXWidgetSet(this, kFixedLeft, kVElastic,
										position,0, size, GetApertureHeight());
	assert( compartment != NULL );
	GetCompartments()->Append(compartment);
	return compartment;
}

void
CSplitterView::SetCompartmentSizes()
{
}

/******************************************************************************
 Draw (virtual protected)

	Draw 2 lines in each drag region.

 ******************************************************************************/

void
CSplitterView::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	p.SetPenColor((p.GetColormap())->GetGray60Color());

	const JRect ap = GetAperture();
	if (mHorizontal && mHorizSplit)
	{
		JRect icon_rect = mHorizSplit->GetBounds();
		
		JCoordinate y_top = GetCompartmentSize(1);
		JCoordinate y_bottom = y_top + cDividerWidth;

		JCoordinate hpos = (ap.width() - icon_rect.width()) / 2;
		JCoordinate vpos = y_top + (cDividerWidth - icon_rect.height()) / 2;
		
		p.JPainter::Image(*mHorizSplit, mHorizSplit->GetBounds(), hpos, vpos);
	}
	else if (!mHorizontal && mVertSplit)
	{
		JRect icon_rect = mVertSplit->GetBounds();
		
		JCoordinate y_left = GetCompartmentSize(1);
		JCoordinate y_right = y_left + cDividerWidth;

		JCoordinate hpos = y_left + (cDividerWidth - icon_rect.width()) / 2;
		JCoordinate vpos = (ap.height() - icon_rect.height()) / 2;
		
		p.JPainter::Image(*mVertSplit, mVertSplit->GetBounds(), hpos, vpos);
	}
}

// Calculate local rect for splitter bar
void CSplitterView::GetSplitbarRect(JRect& splitbar)
{
	// Only if both visible
	if (mVisible1 && mVisible2)
	{
		const JRect ap = GetAperture();
		JCoordinate size1 = GetCompartmentSize(1);
		splitbar.Set(ap.top + (mHorizontal ? size1 : 0),
						ap.left + (mHorizontal ? 0 : size1),
						mHorizontal ? ap.top + size1 + cDividerWidth : ap.bottom,
						mHorizontal ? ap.right : ap.left + size1 + cDividerWidth);
	}
	else
		splitbar.Set(0, 0, 0, 0);
}

/******************************************************************************
 HandleMouseDown (virtual protected)

	Any mouse click that we receive means start dragging.

 ******************************************************************************/

void
CSplitterView::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	// Only if both visible and not locked
	if (!mVisible1 || !mVisible2 || mLock1 || mLock2)
		return;

	itsDragType = kInvalidDrag;
	if (button != kJXLeftButton)
		{
		return;
		}

	// Must be inside split bar
	JRect splitbar;
	GetSplitbarRect(splitbar);
	if (!splitbar.Contains(pt))
		return;

	if (modifiers.meta())
		{
		itsDragType = kDragAll;
		PrepareToDragAll(mHorizontal ? pt.y : pt.x, &itsMinDrag, &itsMaxDrag);
		}
	else
		{
		itsDragType = kDragOne;
		PrepareToDrag(mHorizontal ? pt.y : pt.x, &itsMinDrag, &itsMaxDrag);
		}

	JPainter* p = CreateDragInsidePainter();

	const JRect ap = GetAperture();
	if (mHorizontal)
		p->Line(ap.left, pt.y, ap.right, pt.y);
	else
		p->Line(pt.x, ap.top, pt.x, ap.bottom);
	itsPrevPt = pt;
}

/******************************************************************************
 HandleMouseDrag (virtual protected)

 ******************************************************************************/

void
CSplitterView::HandleMouseDrag
	(
	const JPoint&			origPt,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	// Only if both visible and not locked
	if (!mVisible1 || !mVisible2 || mLock1 || mLock2)
		return;

	if (itsDragType != kInvalidDrag)
		{
		JPoint pt = origPt;
		JCoordinate& pos = (mHorizontal ? pt.y : pt.x);

		// keep compartment width larger than minimum

		if (pos < itsMinDrag)
			{
			pos = itsMinDrag;
			}
		else if (pos > itsMaxDrag)
			{
			pos = itsMaxDrag;
			}

		// check if we have moved

		if (pos != (mHorizontal ? itsPrevPt.y : itsPrevPt.x))
			{
			JPainter* p = NULL;
			const JBoolean ok = GetDragPainter(&p);
			assert( ok );

			JRect ap = GetAperture();
			if (mHorizontal)
			{
				p->Line(ap.left, itsPrevPt.y, ap.right, itsPrevPt.y);
				p->Line(ap.left, pt.y, ap.right, pt.y);
			}
			else
			{
				p->Line(itsPrevPt.x, ap.top, itsPrevPt.x, ap.bottom);
				p->Line(pt.x, ap.top, pt.x, ap.bottom);
			}

			itsPrevPt = pt;
			}
		}
}

/******************************************************************************
 HandleMouseUp (virtual protected)

	We set the column width to itsPrevPt because that is what the user saw.

 ******************************************************************************/

void
CSplitterView::HandleMouseUp
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	// Only if both visible and not locked
	if (!mVisible1 || !mVisible2 || mLock1 || mLock2)
		return;

	if (itsDragType != kInvalidDrag)
		{
		// erase the line

		JPainter* p = NULL;
		const JBoolean ok = GetDragPainter(&p);
		assert( ok );

		JRect ap = GetAperture();
		if (mHorizontal)
			p->Line(ap.left, itsPrevPt.y, ap.right, itsPrevPt.y);
		else
			p->Line(itsPrevPt.x, ap.top, itsPrevPt.x, ap.bottom);

		DeleteDragPainter();

		// set the compartment widths

		if (itsDragType == kDragAll)
			{
			AdjustCompartmentsAfterDragAll(mHorizontal ? itsPrevPt.y : itsPrevPt.x);
			}
		else
			{
			assert( itsDragType == kDragOne );
			AdjustCompartmentsAfterDrag(mHorizontal ? itsPrevPt.y : itsPrevPt.x);
			}

		// update divider pos
		long old_pos = mDividerPos;
		mDividerPos = GetCompartmentSize(1);
		if (old_pos != mDividerPos)
			ChangedDividerPos(mDividerPos - old_pos);
		}

	itsDragType = kInvalidDrag;
}

/******************************************************************************
 AdjustCursor (virtual protected)

 ******************************************************************************/

void
CSplitterView::AdjustCursor
	(
	const JPoint&			pt,
	const JXKeyModifiers&	modifiers
	)
{
	// Only if both visible and not locked
	if (!mVisible1 || !mVisible2 || mLock1 || mLock2)
		return;

	// Must be inside split bar
	JRect splitbar;
	GetSplitbarRect(splitbar);
	if (!splitbar.Contains(pt))
		return;

	if (modifiers.meta())
		{
		DisplayCursor(itsDragAllLineCursor);
		}
	else
		{
		JXPartition::AdjustCursor(pt, modifiers);
		}
}


void
CSplitterView::ApertureResized
	(
	const JCoordinate dw,
	const JCoordinate dh
	)
{
	JXWidgetSet::ApertureResized(dw,dh);
	JRect ap_new = GetAperture();

	// Adjust divider
	if (mVisible1 && mVisible2 && !mLockResize)
	{
		// If second pane is locked, move divider by resize amount
		if (mLock2)
			// Resize first by full amount
			mDividerPos += (mHorizontal ? dh : dw);
			
		// Else if neither pane is locked we move divider by ratio
		// otherwise when first pane is locked we leave divider alone
		else if (!mLock1)
		{
			if (mDividerPos == 0)
				mDividerPos = 1;

			// Get old and new sizes
			long new_size = (mHorizontal ? ap_new.height() : ap_new.width());
			long old_size = new_size - (mHorizontal ? dh : dw);
			double ratio = (1.0 * old_size) / mDividerPos;
			if (ratio == 0)
				ratio = 1;

			// Resize both by proportional amount
			mDividerPos = ::lround(new_size / ratio);
		}
	}

	// Adjust the contents
	AdjustContents();
}

void CSplitterView::RepositionViews()
{
	// Expand sub-panes to fill entire splitter
	mSub1->FitToEnclosure(kTrue, kTrue);
	mSub2->FitToEnclosure(kTrue, kTrue);

	// Do normal contents adjustment which will reposition and resize the sub-panes
	AdjustContents();
}

void CSplitterView::AdjustContents()
{
	// Don't adjust if nothing present
	if (!mSub1 || !mSub2)
		return;

	// Set lock details
	if (mLock1 || mLock2)
		SetElasticIndex(mLock1 ? 1 : 2);
	else
		SetElasticIndex(0);

	// Fit to current divider details
	if (mVisible1 ^ mVisible2)
	{
		JXWidgetSet* sub = mVisible1 ? mSub1 : mSub2;
		sub->FitToEnclosure(kTrue, kTrue);
	}
	else
	{
		JArray<JCoordinate> sizes(GetCompartmentSizes());

		// Make sure divider minima are correct
		if (GetTotalSize() - mDividerPos < mMin2)
			mDividerPos = GetTotalSize() - mMin2;
		else if (mDividerPos < mMin1)
			mDividerPos = mMin1;

		// Determine the gap between frames
		// If its locked the divider might not be visible
		long gap = 0;
		if (!mLock1 && !mLock2 || mVisibleLock)
			gap = cDividerWidth;

		// Set new sizes
		sizes.SetElement(1, mDividerPos);
		sizes.SetElement(2, GetTotalSize() - mDividerPos - gap);

		// Now set the new sizes
		JPartition::SetCompartmentSizes(sizes);

		// Layout sub panes
		const JCoordinate fixed_size = mHorizontal ? GetApertureWidth() : GetApertureHeight();
		JCoordinate pos = 0;
		for (JIndex i = 1; i <= 2; i++)
		{
			const JCoordinate variable_size = GetCompartmentSize(i);

			// Don't do this test as we may be setting a new minimum later on
			//assert( variable_size >= GetMinCompartmentSize(i) );

			JXContainer* compartment = GetCompartment(i);
			compartment->Place(mHorizontal ? 0 : pos, mHorizontal ? pos : 0);
			compartment->SetSize(mHorizontal ? fixed_size : variable_size,
									mHorizontal ? variable_size : fixed_size);
			pos += variable_size + gap;
		}
	}
	
	Refresh();
}
