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


// CSplitter

// Class that implements a tab control and manages its panels as well

#include "CSplitter.h"

#include "CContainerView.h"

IMPLEMENT_DYNAMIC(CSplitter, CSplitterWnd)

CSplitter::CSplitter()
{
	mDividerPos = 128;
	mHorizontal = true;
	mVisible1 = true;
	mVisible2 = true;

	// Set zero width border for better sub-pane alignment
	m_cxBorder = m_cyBorder = 2;
}

CSplitter::~CSplitter()
{
}

void CSplitter::CreateViews(bool horiz, CCreateContext* pContext)
{
	mHorizontal = horiz;

	// Create the views
	CreateView(0, 0, RUNTIME_CLASS(CContainerView), CSize(0, 0), pContext);
	CreateView(mHorizontal ? 1 : 0, mHorizontal ? 0 : 1, RUNTIME_CLASS(CContainerView), CSize(0, 0), pContext);

	// Cache the sub-window objects
	mSub1 = GetPane(0, 0);
	mSub2 = GetPane(mHorizontal ? 1 : 0, mHorizontal ? 0 : 1);

	// Initialise splitter pos
	CRect client;
	GetClientRect(client);
	if (mHorizontal)
	{
		SetRowInfo(0, client.Height() / 2, 64);
		SetRowInfo(1, client.Height() / 2, 64);
	}
	else
	{
		SetColumnInfo(0, client.Width() / 2, 64);
		SetColumnInfo(1, client.Width() / 2, 64);
	}
}

void CSplitter::ShowView(bool view1, bool view2)
{
	// Only if changing
	if ((mVisible1 == view1) && (mVisible2 == view2))
		return;

	// Get current sizes
	int curr1;
	int min1;
	int curr2;
	int min2;

	if (mHorizontal)
	{
		GetRowInfo(0, curr1, min1);
		GetRowInfo(1, curr2, min2);
	}
	else
	{
		GetColumnInfo(0, curr1, min1);
		GetColumnInfo(1, curr2, min2);
	}

	// Update divider pos if both visible
	if (mVisible1 && mVisible2)
		mDividerPos = curr1;

	// Explicitly set invisible one to zero size
	if (!mVisible1)
		curr1 = 0;
	else if (!mVisible2)
		curr2 = 0;

	// One must be visible
	bool was_visible1 = mVisible1;
	bool was_visible2 = mVisible2;
	mVisible1 = view1 || !view2;
	mVisible2 = view2;
	
	// Hide the unwanted ones
	if (!mVisible1 && was_visible1)
		mSub1->ShowWindow(SW_HIDE);
	if (!mVisible2 && was_visible2)
		mSub2->ShowWindow(SW_HIDE);
	
	// Adjust row details

	// Create new sizes based on visiblity
	int newcurr1 = 0;
	int newmin1 = 0;
	int newcurr2 = 0;
	int newmin2 = 0;
	
	if (mVisible1 && mVisible2)
	{
		// Both visible => use last divider pos
		newcurr1 = mDividerPos;
		newcurr2 = curr1 + curr2 - newcurr1;
		newmin1 = 64;
		newmin2 = 64;
	}
	else if (mVisible1)
	{
		// Give full size to first pane
		newcurr1 = (curr1 + curr2);
		newcurr2 = 0;
		newmin1 = 64;
		newmin2 = 0;
	}
	else if (mVisible2)
	{
		// Give full size to second pane
		newcurr1 = 0;
		newcurr2 = (curr1 + curr2);
		newmin1 = 0;
		newmin2 = 64;
	}
	
	// Set new sizes and minima
	if (mHorizontal)
	{
		SetRowInfo(0, newcurr1, newmin1);
		SetRowInfo(1, newcurr2, newmin2);
		
		m_nRows = (mVisible1 && mVisible2 ? 2 : 1);
	}
	else
	{
		SetColumnInfo(0, newcurr1, newmin1);
		SetColumnInfo(1, newcurr2, newmin2);

		m_nCols = (mVisible1 && mVisible2 ? 2 : 1);
	}

	// Adjust the contents to new view state
	RecalcLayout();

	// Show the wanted ones
	if (mVisible1 && !was_visible1)
		mSub1->ShowWindow(SW_SHOW);
	if (mVisible2 && !was_visible2)
		mSub2->ShowWindow(SW_SHOW);
}

long CSplitter::GetRelativeSplitPos() const
{
	// Get frame
	CRect frame;
	GetClientRect(frame);
	unsigned long total_size = (mHorizontal ? frame.Height() : frame.Width());
	if (total_size == 0)
		total_size = 1;
	
	// Update divider pos if both visible
	if (mVisible1 && mVisible2)
	{
		int curr1;
		int min1;
		if (mHorizontal)
			GetRowInfo(0, curr1, min1);
		else
			GetColumnInfo(0, curr1, min1);
		mDividerPos = curr1;
	}

	// Get ratio of frame to split pos
	float ratio = ((float) mDividerPos) / total_size;
	
	// Normalise to a long
	return 0x10000000 * ratio;
}

void CSplitter::SetRelativeSplitPos(long split)
{
	// Get frame
	CRect frame;
	GetClientRect(frame);
	unsigned long total_size = (mHorizontal ? frame.Height() : frame.Width());
	
	// Unnormalise ratio
	float ratio = split / ((float) 0x10000000);
	
	// Set new divide pos
	mDividerPos = total_size * ratio;

	// Move and resize sub views for change if both visible
	if (mVisible1 && mVisible2)
	{
		if (mHorizontal)
		{
			SetRowInfo(0, mDividerPos, 64);
			SetRowInfo(1, total_size - mDividerPos, 64);
		}
		else
		{
			SetColumnInfo(0, mDividerPos, 64);
			SetColumnInfo(1, total_size - mDividerPos, 64);
		}

		// Adjust the contents to new view state
		RecalcLayout();
	}
}

