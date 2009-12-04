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


#include "CSplitter.h"
#include <JXDragPainter.h>
#include <JXColormap.h>
#include <JXCursor.h>
#include <jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

CSplitter::CSplitter
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
	mElastic1 = true;
	mElastic2 = true;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CSplitter::~CSplitter()
{
}

void CSplitter::InitViews(bool horiz)
{
	// Set horizontal state
	mHorizontal = horiz;

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

	// Init subviews not already done
	if (!mSub1)
	{
		JCoordinate size = GetTotalSize();
		JArray<JCoordinate> sizes;
		sizes.AppendElement(size / 2);
		sizes.AppendElement(size - sizes.GetElement(1) - kDragRegionSize);

		mSub1 = CreateCompartment(1, sizes.GetElement(1));
		mSub1->Activate();

		mSub2 = CreateCompartment(2, sizes.GetElement(2));
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
	}
}
			
void CSplitter::ShowView(bool view1, bool view2)
{
	// Only if changing
	if ((mVisible1 == view1) && (mVisible2 == view2))
		return;

	// One must be visible
	bool was_visible1 = mVisible1;
	bool was_visible2 = mVisible2;
	mVisible1 = view1 || !view2;
	mVisible2 = view2;
	
	// Hide the unwanted ones
	if (!mVisible1 && was_visible1)
		mSub1->Hide();
	if (!mVisible2 && was_visible2)
		mSub2->Hide();

	// Always remember current divider
	mDividerPos = GetCompartmentSize(1);

	// Adjust the contents to new view state
	ApertureResized(0, 0);

	// Show the wanted ones
	if (mVisible1 && !was_visible1)
		mSub1->Show();
	if (mVisible2 && !was_visible2)
		mSub2->Show();
}

void CSplitter::SetMinima(long min1, long min2)
{
	bool adjust = (mMin1 != min1) || (mMin2 != min2);
	mMin1 = min1;
	mMin2 = min2;

	SetMinCompartmentSize(1, mMin1);
	SetMinCompartmentSize(2, mMin2);

	if (adjust)
		SetCompartmentSizes();
}

void CSplitter::SetLockSize(JCoordinate size)
{
	// One must be locked
	if (!mLock1 && !mLock2)
		return;

	// Get current sizes of each partition
	JArray<JCoordinate> sizes = GetCompartmentSizes();

	// Adjust to value we want
	sizes.SetElement(mLock1 ? 1 : 2, size);
	sizes.SetElement(mLock1 ? 2 : 1, GetTotalSize() - size - kDragRegionSize);

	// Never allow sizes less than 16
	if (sizes.GetElement(1) < 16)
		sizes.SetElement(1, 16);
	if (sizes.GetElement(2) < 16)
		sizes.SetElement(2, 16);

	// Set new sizes
	JPartition::SetCompartmentSizes(sizes);
}

void CSplitter::SetLocks(bool lock1, bool lock2)
{
	// Only one locked
	mLock1 = lock1;
	mLock2 = lock2 && !mLock1;

	if (mLock1 || mLock2)
		SetElasticIndex(mLock1 ? 1 : 2);
	else
		SetElasticIndex(0);
}

void CSplitter::SetElastic(bool elastic1, bool elastic2)
{
	mElastic1 = elastic1;
	mElastic2 = elastic2;
}

void CSplitter::MoveSplitPos(JCoordinate moveby)
{
	// Get current sizes of each partition
	JArray<JCoordinate> sizes = GetCompartmentSizes();
	JSize size1 = sizes.GetElement(1);
	JSize size2 = sizes.GetElement(2);

	// Adjust by move amount
	sizes.SetElement(1, size1 + moveby);
	sizes.SetElement(2, size2 - moveby);

	// Never allow sizes less than 16
	if (sizes.GetElement(1) < 16)
		sizes.SetElement(1, 16);
	if (sizes.GetElement(2) < 16)
		sizes.SetElement(2, 16);

	// Set new sizes
	JPartition::SetCompartmentSizes(sizes);
}

long CSplitter::GetRelativeSplitPos() const
{
	// Get total size
	JCoordinate total = GetTotalSize();

	// Get ratio of frame to split pos
	float ratio = ((float) GetCompartmentSize(1)) / total;
	
	// Normalise to a long
	return (long) (0x10000000 * ratio);
}

void CSplitter::SetRelativeSplitPos(long split)
{
	// Get total size
	JCoordinate total = GetTotalSize();
	
	// Unnormalise ratio
	float ratio = split / ((float) 0x10000000);
	
	// Set new divide pos and remember old
	mDividerPos = (long) (total * ratio);

	// Move and resize sub views for change if both visible
	if (mVisible1 && mVisible2)
	{
		JArray<JCoordinate> sizes;
		sizes.AppendElement(mDividerPos);
		sizes.AppendElement(total - mDividerPos - kDragRegionSize);

		// Never allow sizes less than 16
		if (sizes.GetElement(1) < 16)
			sizes.SetElement(1, 16);
		if (sizes.GetElement(2) < 16)
			sizes.SetElement(2, 16);

		JPartition::SetCompartmentSizes(sizes);
	}
}

/******************************************************************************
 GetTotalSize (virtual)

 ******************************************************************************/

JCoordinate
CSplitter::GetTotalSize()
	const
{
	return mHorizontal ? GetApertureHeight() : GetApertureWidth();
}

/******************************************************************************
 CreateCompartment (virtual)

	Adjust the width of each compartment.

 ******************************************************************************/

JXWidgetSet*
CSplitter::CreateCompartment
	(
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

/******************************************************************************
 SetCompartmentSizes (virtual protected)

 ******************************************************************************/

void
CSplitter::SetCompartmentSizes()
{
	const JCoordinate fixed_size = mHorizontal ? GetApertureWidth() : GetApertureHeight();

	if (mVisible1 && mVisible2)
	{
		JCoordinate pos = 0;
		for (JIndex i=1; i<=2; i++)
			{
			const JCoordinate variable_size = GetCompartmentSize(i);

			// Don't do this test as we may be setting a new minimum later on
			//assert( variable_size >= GetMinCompartmentSize(i) );

			JXContainer* compartment = GetCompartment(i);
			compartment->Place(mHorizontal ? 0 : pos, mHorizontal ? pos : 0);
			compartment->SetSize(mHorizontal ? fixed_size : variable_size,
									mHorizontal ? variable_size : fixed_size);
			pos += variable_size + kDragRegionSize;
			}
	}
	else
	{
		JXContainer* sub = mVisible1 ? mSub1 : mSub2;
		sub->Place(0, 0);
		sub->SetSize(GetApertureWidth(), GetApertureHeight());
	}
}

/******************************************************************************
 Draw (virtual protected)

	Draw 2 lines in each drag region.

 ******************************************************************************/

void
CSplitter::Draw
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
	if (mHorizontal)
	{
		const JCoordinate x1 = ap.left+1;
		const JCoordinate x2 = ap.right-2;

		const JSize compartmentCount = GetCompartmentCount();
		JCoordinate y = 0;
		for (JIndex i=1; i<compartmentCount; i++)
			{
			y += GetCompartmentSize(i);
			p.Line(x1, y+1, x2, y+1);
			p.Line(x1, y+3, x2, y+3);
			y += kDragRegionSize;
			}
	}
	else
	{
		const JCoordinate y1 = ap.top+1;
		const JCoordinate y2 = ap.bottom-2;

		const JSize compartmentCount = GetCompartmentCount();
		JCoordinate x = 0;
		for (JIndex i=1; i<compartmentCount; i++)
			{
			x += GetCompartmentSize(i);
			p.Line(x+1, y1, x+1, y2);
			p.Line(x+3, y1, x+3, y2);
			x += kDragRegionSize;
			}
	}
}

/******************************************************************************
 HandleMouseDown (virtual protected)

	Any mouse click that we receive means start dragging.

 ******************************************************************************/

void
CSplitter::HandleMouseDown
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
CSplitter::HandleMouseDrag
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
			JPainter* p = GetDragPainter();
			assert( p != NULL );

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
CSplitter::HandleMouseUp
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

		JPainter* p = GetDragPainter();
		assert( p != NULL );

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
		}

	itsDragType = kInvalidDrag;
}

/******************************************************************************
 AdjustCursor (virtual protected)

 ******************************************************************************/

void
CSplitter::AdjustCursor
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
CSplitter::ApertureResized
	(
	const JCoordinate dw,
	const JCoordinate dh
	)
{
	JXWidgetSet::ApertureResized(dw,dh);

	// Adjust divider
	if (mVisible1 && mVisible2)
	{
		JArray<JCoordinate> sizes(GetCompartmentSizes());
		if (mLock1 || !mElastic1 && mElastic2)
			// Resize second by full amount
			sizes.SetElement(2, GetCompartmentSize(2) + (mHorizontal ? dh : dw));
		else if (mLock2 || mElastic1 && !mElastic2)
			// Resize first by full amount
			sizes.SetElement(1, GetCompartmentSize(1) + (mHorizontal ? dh : dw));
		else
		{
			// Get old and new sizes
			unsigned long new_size = GetTotalSize();
			unsigned long old_size = new_size - (mHorizontal ? dh : dw);
			double ratio = (1.0 * old_size) / GetCompartmentSize(1);

			// Resize both by proportional amount
			mDividerPos = (unsigned long) (new_size / ratio);

			// Taking minima into account - first minima always overrides
			if (new_size - mDividerPos - kDragRegionSize < mMin2)
				mDividerPos = new_size - mMin2 - kDragRegionSize;
			if (mDividerPos < mMin1)
				mDividerPos = mMin1;

			// Set new sizes
			sizes.SetElement(1, mDividerPos);
			sizes.SetElement(2, new_size - mDividerPos - kDragRegionSize);
		}

		// Never allow sizes less than 16
		if (sizes.GetElement(1) < 16)
			sizes.SetElement(1, 16);
		if (sizes.GetElement(2) < 16)
			sizes.SetElement(2, 16);

		// Now set the new sizes
		JPartition::SetCompartmentSizes(sizes);
	}

	// Adjust the contents
	SetCompartmentSizes();
}

// Calculate local rect for splitter bar
void CSplitter::GetSplitbarRect(JRect& splitbar)
{
	// Only if both visible
	if (mVisible1 && mVisible2)
	{
		const JRect ap = GetAperture();
		JCoordinate size1 = GetCompartmentSize(1);
		splitbar.Set(ap.top + (mHorizontal ? size1 : 0),
						ap.left + (mHorizontal ? 0 : size1),
						mHorizontal ? ap.top + size1 + kDragRegionSize : ap.bottom,
						mHorizontal ? ap.right : ap.left + size1 + kDragRegionSize);
	}
	else
		splitbar.Set(0, 0, 0, 0);
}
