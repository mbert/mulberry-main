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

// Dictionary Page Scroller

#include "CDictionaryPageScroller.h"

#include "CSpellPlugin.h"
#include "CStaticText.h"

#include <LScrollBar.h>
#include <LTextColumn.h>

CDictionaryPageScroller::CDictionaryPageScroller()
{
}

CDictionaryPageScroller::CDictionaryPageScroller(LStream *inStream)
	: CBetterScrollerX(inStream)
{
	mWordTop = NULL;
	mWordBottom = NULL;
	mPageCount = NULL;
	mDisplayPages = false;
}

CDictionaryPageScroller::~CDictionaryPageScroller()
{
}

// Get details of sub-panes
void CDictionaryPageScroller::FinishCreateSelf(void)
{
	// Do inherited
	CBetterScrollerX::FinishCreateSelf();

	// Get items
	mPageCount = (CStaticText*) FindPaneByID(paneid_DictionaryPageScrollerPageCount);
}

void CDictionaryPageScroller::AdjustScrollBars()
{
	// Do not allow horiz adjustment
	LScrollBar* temp = mHorizontalBar;
	mHorizontalBar = NULL;

	CBetterScrollerX::AdjustScrollBars();

	mHorizontalBar = temp;
}

// Manual scroll
void CDictionaryPageScroller::HorizScroll(SInt16 inPart)
{
	SInt16 horizUnits = 0;

	switch (inPart)
	{				// Determine how much to scroll

		case kControlUpButtonPart:	// Scroll left one unit
		case kControlPageUpPart:		// Scroll left by Frame width
			horizUnits = -1;
			break;

		case kControlDownButtonPart:	// Scroll right one unit
		case kControlPageDownPart:		// Scroll right by Frame width
			horizUnits = 1;
			break;
	}

	if (horizUnits != 0)
	{
		mHorizontalBar->IncrementValue(horizUnits);
		SetPage(mHorizontalBar->GetValue());
		mHorizontalBar->FocusDraw();
	}
}

// Manual scroll
void CDictionaryPageScroller::VertScroll(SInt16 inPart)
{
	// Check for paging
	if (mHorizontalBar->GetMinValue() != mHorizontalBar->GetMaxValue())
	{
		// Check for limit
		if ((mVerticalBar->GetValue() == mVerticalBar->GetMinValue()) &&
			(mHorizontalBar->GetValue() != mHorizontalBar->GetMinValue()) &&
			(inPart == kControlUpButtonPart))
		{
			HorizScroll(kControlUpButtonPart);
			mVerticalBar->SetValue(mVerticalBar->GetMaxValue());
			mScrollingView->ScrollPinnedImageTo(0, 0x7FFFFFFF, true);
			mVerticalBar->FocusDraw();
			return;
		}
		else if ((mVerticalBar->GetValue() == mVerticalBar->GetMaxValue()) &&
				 (mHorizontalBar->GetValue() != mHorizontalBar->GetMaxValue()) &&
				 (inPart == kControlDownButtonPart))
		{
			HorizScroll(kControlDownButtonPart);
			mVerticalBar->SetValue(mVerticalBar->GetMinValue());
			mScrollingView->ScrollPinnedImageTo(0, 0, true);
			mVerticalBar->FocusDraw();
			return;
		}
	}

	// Do default
	CBetterScrollerX::VertScroll(inPart);
}

void CDictionaryPageScroller::ThumbScroll(LScrollBar* inScrollBar, SInt32 inThumbValue)
{
	if (inScrollBar == mVerticalBar)
		CBetterScrollerX::ThumbScroll(inScrollBar, inThumbValue);
	else
		SetPage(inThumbValue);
}

// Display dictionary in list
bool CDictionaryPageScroller::DisplayDictionary(const char* word, bool force_draw)
{
	bool result = false;

	// Get page buffer and find closest location if switching from guesses
	const char* buf;
	size_t buflen;
	int match = mSpeller->GetPage(word, &buf, &buflen);
	mHorizontalBar->SetValue(mSpeller->CurrentPage());

	// Set page counter if more than one
	if (mPageCount && mDisplayPages && mSpeller->CountPages())
	{
		cdstring str;
		str.reserve(256);
		::snprintf(str.c_str_mod(), 256, "%d of %d", mSpeller->CurrentPage() + 1, mSpeller->CountPages() + 1);
		mPageCount->SetText(str);
		if (force_draw)
			mPageCount->Draw(NULL);
	}

	{
		StDeferTableAdjustment no_refresh((LTextColumn*) mScrollingView);

		// Remove any existing
		((LTextColumn*) mScrollingView)->RemoveAllRows(false);

		// Do for each word
		const char* start = buf;
		const char* stop = buf;
		const char* current = buf;
		int row = 1;
		while(current - buf < buflen)
		{
			stop = current;
			((LTextColumn*) mScrollingView)->InsertRows(1, row++, current, ::strlen(current) + 1);

			current += ::strlen(current) + 1;
		}

		// Set word range if more than one page
		if (mWordTop && mDisplayPages && mSpeller->CountPages())
		{
			mWordTop->SetText(start);
			if (force_draw)
				mWordTop->Draw(NULL);
		}
		if (mWordBottom && mDisplayPages && mSpeller->CountPages())
		{
			mWordBottom->SetText(stop);
			if (force_draw)
				mWordBottom->Draw(NULL);
		}

		// Only select row if some entries and was previously showing guesses
		if ((row > 1) && (match >= 0))
		{
			// Select matching cell
			STableCell aCell(match + 1, 1);
			((LTextColumn*) mScrollingView)->SelectCell(aCell);

			// Must allow adjustments before scrolling selected cell into view
			((LTextColumn*) mScrollingView)->SetDeferAdjustment(false);
			((LTextColumn*) mScrollingView)->ScrollCellIntoFrame(aCell);

			result = true;
		}
		else
			BroadcastMessage(mPaneID);
	}

	// Force redraw if required (ie live scrolling)
	if (force_draw)
		((LTextColumn*) mScrollingView)->Draw(NULL);
	else
		((LTextColumn*) mScrollingView)->Refresh();

	return result;
}

void CDictionaryPageScroller::DisplayPages(bool display)
{
	mDisplayPages = display;
	if (!mDisplayPages)
	{
		cdstring dummy;
		mWordTop->SetText(dummy);
		mWordBottom->SetText(dummy);
		mPageCount->SetText(dummy);
	}
}

void CDictionaryPageScroller::SetPage(SInt32 inNewValue)
{
	mSpeller->SetPage(inNewValue);
	BroadcastMessage(mPaneID);
	DisplayDictionary(NULL, true);
}

void CDictionaryPageScroller::SetCount(SInt32 inNewValue)
{
	cdstring str;
	str.reserve(256);
	::snprintf(str.c_str_mod(), 256, "%d", inNewValue);
	mPageCount->SetText(str);
}
