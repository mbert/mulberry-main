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

#include "CSplitterView.h"

#include "CContainerWnd.h"
#include "CMulberryCommon.h"

#include <cmath>

IMPLEMENT_DYNAMIC(CSplitterView, CWnd)

BEGIN_MESSAGE_MAP(CSplitterView, CWnd)
	//{{AFX_MSG_MAP(CSplitterWnd)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

const long cDividerWidth = 6;
const long cDividerInset = 0;
const long cDividerInsetWidth = 6;
const long cDividerGrabShort = 6;
const long cDividerGrabLong = 24;

HCURSOR CSplitterView::sVertCursor = NULL;
HCURSOR CSplitterView::sHorizCursor = NULL;
CBitmap* CSplitterView::sHorizGrab = NULL;
CBitmap* CSplitterView::sVertGrab = NULL;

CSplitterView::CSplitterView()
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
	mMouseOverDivider = false;
	mTracking = false;
	mRestoreFocus = NULL;

	// Load cursors first time through
	if (!sVertCursor)
		sVertCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
	if (!sHorizCursor)
		sHorizCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZENS);
	
	// Make sure we have pictures
	if (!sHorizGrab)
	{
		sHorizGrab = new CBitmap;
		sHorizGrab->LoadBitmap(IDB_SPLITTERHORIZ);
	}
	if (!sVertGrab)
	{
		sVertGrab = new CBitmap;
		sVertGrab->LoadBitmap(IDB_SPLITTERVERT);
	}
}

CSplitterView::~CSplitterView()
{
	delete mSub1;
	delete mSub2;
}

int CSplitterView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	mCurrentSize = CSize(lpCreateStruct->cx, lpCreateStruct->cy);

	return 0;
}

void CSplitterView::InitViews()
{
	CRect client;
	GetClientRect(client);

	// Determine the gap between frames
	// If its locked the divider might not be visible
	long gap = 0;
	if (!mLock1 && !mLock2 || mVisibleLock)
		gap = cDividerWidth;

	// Determine widths & heights
	CRect sub1;
	CRect sub2;
	
	if (mHorizontal)
	{
		// First view left/top/full width/height=divider
		sub1.SetRect(0, 0, client.Width(), mDividerPos);
		
		// Second view left/top+divider/full width/height-top
		sub2.SetRect(0, mDividerPos + gap, client.Width(), client.Height());
	}
	else
	{
		// First view left/top/width=divider/full height
		sub1.SetRect(0, 0, mDividerPos, client.Height());
		
		// Second view left+divider/top/width-left/full height
		sub2.SetRect(mDividerPos + gap, 0, client.Width(), client.Height());
	}
	
	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	dwStyle &= ~WS_BORDER;

	// Create first one	
	mSub1 = new CContainerWnd;
	mSub1->Create(NULL, NULL, dwStyle, sub1, this, IDC_STATIC);
	
	// Create second one	
	mSub2 = new CContainerWnd;
	mSub2->Create(NULL, NULL, dwStyle, sub2, this, IDC_STATIC);
}

void CSplitterView::InstallViews(CWnd* view1, CWnd* view2, bool horiz)
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
	view1->SetParent(mSub1);
	view2->SetParent(mSub2);
	
	// Fit each view to parent
	::ExpandChildToFit(mSub1, view1, true, true);
	::ExpandChildToFit(mSub2, view2, true, true);

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
	if (!mVisible1)
		mSub1->ShowWindow(SW_HIDE);
	if (!mVisible2)
		mSub2->ShowWindow(SW_HIDE);
	
	// Adjust the contents to new view state
	AdjustContents();

	// Show the wanted ones
	if (mVisible1)
		mSub1->ShowWindow(SW_SHOW);
	if (mVisible2)
		mSub2->ShowWindow(SW_SHOW);
	
	RedrawWindow();
}

void CSplitterView::SetMinima(long min1, long min2)
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
	CRect client;
	GetClientRect(client);
	
	return mHorizontal ? client.Height() : client.Width();
}

long CSplitterView::GetRelativeSplitPos() const
{
	// Get ratio of frame to split pos
	long size = GetTotalSize();
	if (size == 0)
		size = 1;
	double ratio = ((double) mDividerPos) / size;
	
	// Normalise to a long
	return (long)(0x10000000 * ratio);
}

void CSplitterView::SetRelativeSplitPos(long split)
{
	// Unnormalise ratio
	double ratio = split / ((double) 0x10000000);
	
	// Set new divide pos and remember old
	long old_pos = mDividerPos;
	mDividerPos = (long)(GetTotalSize() * ratio);

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

void CSplitterView::SetPixelSplitPos(long split)
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
	// Get frame
	CRect client;
	GetClientRect(client);
	
	// Determine the gap between frames
	// If its locked the divider might not be visible
	long gap = 0;
	if (!mLock1 && !mLock2 || mVisibleLock)
		gap = cDividerWidth;

	// Determine widths & heights
	CRect sub1;
	CRect sub2;
	
	if (mHorizontal)
	{
		// First view left/top/full width/height=divider
		sub1.SetRect(0, 0, client.Width(), mDividerPos);
		
		// Second view left/top+divider/full width/height-top
		sub2.SetRect(0, mDividerPos + gap, client.Width(), client.Height());
	}
	else
	{
		// First view left/top/width=divider/full height
		sub1.SetRect(0, 0, mDividerPos, client.Height());
		
		// Second view left+divider/top/width-left/full height
		sub2.SetRect(mDividerPos + gap, 0, client.Width(), client.Height());
	}
	
	mSub1->MoveWindow(sub1);
	mSub2->MoveWindow(sub2);
}

// Allow split pane change
void CSplitterView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Only if both visible
	if (!mVisible1 || !mVisible2)
	{
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	// Only if unlocked
	if (mLock1 || mLock2)
	{
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}
	
	if (mTracking)
		return;

	// Get divider rect
	CalcDividerRect(mTrackRect);

	// Get frame
	CRect client;
	GetClientRect(client);
	
	// Look for click in divide
	if (mTrackRect.PtInRect(point))
	{
		// Create limit rect and adjust for minimum sizes
		mTrackLimits = client;
		if (mHorizontal)
		{
			mTrackLimits.top += mMin1;
			mTrackLimits.bottom -= mMin2;
		}
		else
		{
			mTrackLimits.left += mMin1;
			mTrackLimits.right -= mMin2;
		}

		// Start tracking
		mTracking = true;
		mTrackPt = point;
		mTrackMovePt = point;
		OnInvertTracker(mTrackRect);

		// steal focus and capture
		SetCapture();
		mRestoreFocus = SetFocus();

	}
	else
		CWnd::OnLButtonDown(nFlags, point);
}

// Allow split pane change
void CSplitterView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// Do split change if tracking
	if (mTracking)
	{
		mTracking = false;

		ReleaseCapture();

		// erase tracker rectangle
		OnInvertTracker(mTrackRect);

		// Check for actual move
		long moveby = mHorizontal ? (mTrackMovePt.y - mTrackPt.y) : (mTrackMovePt.x - mTrackPt.x);
		if (moveby)
		{
			// Set divider to new pos
			mDividerPos += moveby;

			// Move and resize sub views for change
			ChangedDividerPos(moveby);
		}
		
		// Reset focus
		if (mRestoreFocus)
			mRestoreFocus->SetFocus();
	}
}

void CSplitterView::OnMouseMove(UINT nFlags, CPoint point)
{
	// Always start in not over state
	mMouseOverDivider = false;

	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	// Only if unlocked
	if (mLock1 || mLock2)
		return;
	
	if (mTracking)
	{
		// Clip point to limit rect
		if (mHorizontal)
		{
			if (point.y < mTrackLimits.top)
				point.y = mTrackLimits.top;
			else if (point.y > mTrackLimits.bottom)
				point.y = mTrackLimits.bottom;
		}
		else
		{
			if (point.x < mTrackLimits.left)
				point.x = mTrackLimits.left;
			else if (point.x > mTrackLimits.right)
				point.x = mTrackLimits.right;
		}
			
		// See if it has moved
		long moveby = mHorizontal ? (point.y - mTrackMovePt.y) : (point.x - mTrackMovePt.x);
		if (moveby)
		{
			// Update current track point
			mTrackMovePt = point;

			// Move the track line
			OnInvertTracker(mTrackRect);
			mTrackRect.OffsetRect(mHorizontal ? 0 : moveby, mHorizontal ? moveby : 0);
			OnInvertTracker(mTrackRect);
		}
	}
	else
	{
		// Get divider rect
		CRect divider;
		CalcDividerRect(divider);

		// Check for mouse over divider
		mMouseOverDivider = divider.PtInRect(point);
	}
}

// Display column change cursor
BOOL CSplitterView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if ((nHitTest == HTCLIENT) && (pWnd == this) && mMouseOverDivider &&
		(::GetKeyState(VK_MENU) >= 0) && (::GetKeyState(VK_SHIFT) >= 0) && (::GetKeyState(VK_CONTROL) >= 0))
	{
		::SetCursor(mHorizontal ? sHorizCursor : sVertCursor);
		return TRUE;
	}

	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CSplitterView::CalcDividerRect(CRect& divRect)
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
		GetClientRect(divRect);

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

void CSplitterView::OnPaint()
{
	// Always setup the DC to ensure update region etc is reset
	CPaintDC dc(this);

	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	// Get divider rect
	CRect divider;
	CalcDividerRect(divider);
	dc.FillSolidRect(divider, afxData.clrBtnFace);

	// Ignore if empty
	if (divider.IsRectEmpty())
		return;

	// Get full divider area
	CRect pict_area = divider;
	if (mHorizontal)
	{	
		pict_area.left = (pict_area.left + pict_area.right - cDividerGrabLong) / 2;
		pict_area.right = pict_area.left + cDividerGrabLong;
		
		if (sHorizGrab)
			dc.DrawState(CPoint(pict_area.left, pict_area.top), CSize(pict_area.Width(), pict_area.Height()),
							sHorizGrab, DST_BITMAP | (IsWindowEnabled() ? DSS_NORMAL : DSS_DISABLED));
	}
	else
	{
		pict_area.top = (pict_area.top + pict_area.bottom - cDividerGrabLong) / 2;
		pict_area.bottom = pict_area.top + cDividerGrabLong;
		
		if (sVertGrab)
			dc.DrawState(CPoint(pict_area.left, pict_area.top), CSize(pict_area.Width(), pict_area.Height()),
							sVertGrab, DST_BITMAP | (IsWindowEnabled() ? DSS_NORMAL : DSS_DISABLED));
	}	
	
#if 0
	// Only if both visible
	if (!mVisible1 || !mVisible2)
		return;

	// Get divider rect
	CRect divider;
	CalcDividerRect(divider);

	// Ignore if empty
	if (divider.IsRectEmpty())
		return;

	// Get full divider area
	CRect divider_area = divider;
	if (mHorizontal)
	{	
		divider_area.top -= cDividerInset;
		divider_area.bottom += cDividerInsetWidth;
	}
	else
	{
		divider_area.left -= cDividerInset;
		divider_area.right += cDividerInsetWidth;
	}

	// Draw frame around it
	dc.Draw3dRect(divider_area, afxData.clrBtnHilite, afxData.clrBtnShadow);
	divider_area.InflateRect(-CX_BORDER, -CY_BORDER);

	// fill the middle
	COLORREF clr = afxData.clrBtnFace;
	dc.FillSolidRect(divider_area, clr);
	
	// Draw splitter indicator
	dc.Draw3dRect(divider, afxData.clrBtnHilite, afxData.clrBtnShadow);
#endif
}

void CSplitterView::OnInvertTracker(const CRect& rect)
{
	// pat-blt without clip children on
	CDC* pDC = GetDC();
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);
}

// Adjust max value of split divider
void CSplitterView::OnSize(UINT nType, int cx, int cy)
{
	// Get current frame
	int dx = cx - mCurrentSize.cx;
	int dy = cy - mCurrentSize.cy;
	
	// Do inherited
	CWnd::OnSize(nType, cx, cy);
	mCurrentSize.cx = cx;
	mCurrentSize.cy = cy;

	// Adjust divider
	if (mVisible1 && mVisible2 && !mLockResize)
	{
		// If second pane is locked, move divider by resize amount
		if (mLock2)
			// Resize first by full amount
			mDividerPos += (mHorizontal ? dy : dx);
			
		// Else if neither pane is locked we move divider by ratio
		// otherwise when first pane is locked we leave divider alone
		else if (!mLock1)
		{
			// Get old and new sizes
			long new_size = (mHorizontal ? cy : cx);
			long old_size = new_size - (mHorizontal ? dy : dx);
			if (mDividerPos != 0)
			{
				double ratio = (1.0 * old_size) / mDividerPos;
				if (ratio == 0.0)
					ratio = 1.0;

				// Resize both by proportional amount
				mDividerPos = new_size / ratio;
			}
		}
	}

	// Adjust the contents
	AdjustContents();
}

void CSplitterView::RepositionViews()
{
	// Expand sub-panes to fill entire splitter
	::ExpandChildToFit(this, mSub1, true, true);
	::ExpandChildToFit(this, mSub2, true, true);

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
		CContainerWnd* visible = (mVisible1 ? mSub1 : mSub2);
		
		// Fit to enclosure
		::ExpandChildToFit(this, visible, true, true);
	}
	else
	{
		// Get current full size
		CRect client;
		GetClientRect(client);
		
		// Make sure divider minima are correct
		if ((mHorizontal ? client.Height() : client.Width()) - mDividerPos < mMin2)
			mDividerPos = (mHorizontal ? client.Height() : client.Width()) - mMin2;
		else if (mDividerPos < mMin1)
			mDividerPos = mMin1;

		// Resize to divider pos
		CRect size;
		if (mHorizontal)
			size.SetRect(0, 0, client.Width(), mDividerPos);
		else
			size.SetRect(0, 0, mDividerPos, client.Height());
		mSub1->MoveWindow(size, FALSE);
		
		// Determine the gap between frames
		// If its locked the divider might not be visible
		long gap = 0;
		if (!mLock1 && !mLock2 || mVisibleLock)
			gap = cDividerWidth;

		// Resize to remainder
		if (mHorizontal)
			size.SetRect(0, mDividerPos + gap, client.Width(), client.Height());
		else
			size.SetRect(mDividerPos + gap, 0, client.Width(), client.Height());
		mSub2->MoveWindow(size, FALSE);
	}
	
	RedrawWindow();
}

BOOL CSplitterView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Send command message to parent if it wants it
	if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

