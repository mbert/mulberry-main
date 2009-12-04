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


//	A Pane subclass which supports scrolling and SubPanes

#include <WIN_LScrollable.h>

#include "CMulberryCommon.h"

#include "StValueChanger.h"

IMPLEMENT_DYNCREATE(LScrollable, CWnd)

BEGIN_MESSAGE_MAP(LScrollable, CWnd)
	//{{AFX_MSG_MAP(CGrayBackground)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

extern UINT PASCAL _AfxGetMouseScrollLines();

// ---------------------------------------------------------------------------
//	Constants

//const SInt16		max_PortOrigin	= 16384;
//const SInt16		min_PortOrigin	= -32768;
//const SInt32		mask_Lo14Bits	= 0x00003FFF;


// ---------------------------------------------------------------------------
//	Class Variables

// ---------------------------------------------------------------------------
//	¥ LScrollable									Default Constructor		  [public]
// ---------------------------------------------------------------------------

LScrollable::LScrollable()
{
	mImageRect.SetRect(0, 0, 0, 0);

	mScrollUnit = CPoint(1, 1);

	mReconcileOverhang = true;
	mBroadcastScrollEvent = true;
	
	mHasScrollbars = true;
	mDoScrollbarUpdate = true;
	mHasHScroll = false;
	mHasVScroll = false;
}


// ---------------------------------------------------------------------------
//	¥ ~LScrollable								Destructor				  [public]
// ---------------------------------------------------------------------------
//

LScrollable::~LScrollable()
{
}

#pragma mark -

#pragma mark -

// Adjust splits
void LScrollable::OnSize(UINT nType, int cx, int cy)
{
	// Get current scroll relative position
	double scroll_vert = 0.0;
	if (mHasScrollbars && mHasVScroll && (cy != 0))
	{
		SCROLLINFO scinfo;
		GetScrollInfo(SB_VERT, &scinfo, SIF_ALL);
		if (scinfo.nMax > scinfo.nPage)
			scroll_vert = ((double)scinfo.nPos) / (scinfo.nMax - scinfo.nPage);
	}
	double scroll_horiz = 0.0;
	if (mHasScrollbars && mHasHScroll && (cx != 0))
	{
		SCROLLINFO scinfo;
		GetScrollInfo(SB_HORZ, &scinfo, SIF_ALL);
		if (scinfo.nMax > scinfo.nPage)
			scroll_horiz = ((double)scinfo.nPos) / (scinfo.nMax - scinfo.nPage);
	}

	CWnd::OnSize(nType, cx, cy);

	// Reset frame/image state
	ReconcileFrameAndImage(true);
	ImageChanged();
	
	// Now restore relative scroll positions
	if (mHasScrollbars)
	{
		SInt32 vertScroll = 0;
		SInt32 horizScroll = 0;
		if (mHasVScroll && (cy != 0))
		{
			SCROLLINFO scinfo;
			GetScrollInfo(SB_VERT, &scinfo, SIF_ALL);
			vertScroll = scroll_vert * (scinfo.nMax - scinfo.nPage) - scinfo.nPos;
		}
		if (mHasHScroll && (cx != 0))
		{
			SCROLLINFO scinfo;
			GetScrollInfo(SB_HORZ, &scinfo, SIF_ALL);
			horizScroll = scroll_horiz * (scinfo.nMax - scinfo.nPage) - scinfo.nPos;
		}
		if ((horizScroll != 0) || (vertScroll != 0))
			ScrollPinnedImageBy(horizScroll, vertScroll, true);
	}	
}

void LScrollable::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (!mHasHScroll)
		return;

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);

	// calc new y position
	SCROLLINFO si;
	GetScrollInfo(SB_HORZ, &si, SIF_ALL);
	int x = si.nPos;
	int xOrig = x;
	int xMax = si.nMax - si.nPage;
	if (xMax == 1)
		xMax = 0;
	nPos = si.nTrackPos;
	
	CRect client;
	GetClientRect(client);

	switch (nSBCode)
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= mScrollUnit.y;
		break;
	case SB_LINEDOWN:
		x += mScrollUnit.x;
		break;
	case SB_PAGEUP:
		x -= client.Width() - mScrollUnit.x;
		break;
	case SB_PAGEDOWN:
		x += client.Width() - mScrollUnit.x;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		x = nPos;
		break;
	}

	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;
	
	// Don't update the scroll bar when doing scrollinf off the scrollbar
	StValueChanger<bool> _no_update(mDoScrollbarUpdate, false);

	// do scroll and update scroll positions
	ScrollImageBy(x- xOrig, 0, true);
	if (x != xOrig)
		SetScrollPos(SB_HORZ, x);
}

void LScrollable::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (!mHasVScroll)
		return;

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);

	// calc new y position
	SCROLLINFO si;
	GetScrollInfo(SB_VERT, &si, SIF_ALL);
	int y = si.nPos;
	int yOrig = y;
	int yMax = si.nMax - si.nPage;
	if (yMax == 1)
		yMax = 0;
	nPos = si.nTrackPos;
	
	CRect client;
	GetClientRect(client);

	switch (nSBCode)
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= mScrollUnit.y;
		break;
	case SB_LINEDOWN:
		y += mScrollUnit.y;
		break;
	case SB_PAGEUP:
		y -= client.Height() - mScrollUnit.y;
		break;
	case SB_PAGEDOWN:
		y += client.Height() - mScrollUnit.y;
		break;
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		y = nPos;
		break;
	}

	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;
	
	// Don't update the scroll bar when doing scrollinf off the scrollbar
	StValueChanger<bool> _no_update(mDoScrollbarUpdate, false);

	// do scroll and update scroll positions
	ScrollImageBy(0, y - yOrig, true);
	if (y != yOrig)
		SetScrollPos(SB_VERT, y);
}

// Handle character
void LScrollable::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CRect client;
	GetClientRect(client);

	switch(nChar)
	{
	case VK_PRIOR:
		ScrollPinnedImageBy(0, -(client.Height() - mScrollUnit.y), true);
		break;
	case VK_NEXT:
		ScrollPinnedImageBy(0, client.Height() - mScrollUnit.y, true);
		break;
	case VK_HOME:
		ScrollPinnedImageTo(0, 0, true);
		break;
	case VK_END:
		ScrollPinnedImageTo(INT_MAX/2, INT_MAX/2, true);
		break;
	default:
		// Do inherited action
		return CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

BOOL LScrollable::OnMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	// we don't handle anything but scrolling just now
	if (fFlags & (MK_SHIFT | MK_CONTROL))
		return FALSE;

	// we can't get out of it--perform the scroll ourselves
	return DoMouseWheel(fFlags, zDelta, point);
}

BOOL LScrollable::DoMouseWheel(UINT fFlags, short zDelta, CPoint point)
{
	// if we have a vertical scroll bar, the wheel scrolls that
	// if we have _only_ a horizontal scroll bar, the wheel scrolls that
	// otherwise, don't do any work at all

	DWORD dwStyle = GetStyle();
	CScrollBar* pBar = GetScrollBarCtrl(SB_VERT);
	BOOL bHasVertBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_VSCROLL);

	pBar = GetScrollBarCtrl(SB_HORZ);
	BOOL bHasHorzBar = ((pBar != NULL) && pBar->IsWindowEnabled()) ||
					(dwStyle & WS_HSCROLL);

	if (!bHasVertBar && !bHasHorzBar)
		return FALSE;

	CRect client;
	GetClientRect(client);

	BOOL bResult = FALSE;
	UINT uWheelScrollLines = _AfxGetMouseScrollLines();
	short zDeltaAbs = ::abs(zDelta);
	int nToScroll;
	int nDisplacement;

	if (bHasVertBar)
	{
		nToScroll = ::MulDiv(zDeltaAbs, uWheelScrollLines, WHEEL_DELTA);
		if (nToScroll == -1 || uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = client.Height();
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * mScrollUnit.y;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
			nDisplacement = min(nDisplacement, client.Height());
		}
		ScrollPinnedImageBy(0, nDisplacement, true);
		bResult = true;
	}
	else if (bHasHorzBar)
	{
		nToScroll = ::MulDiv(zDeltaAbs, uWheelScrollLines, WHEEL_DELTA);
		if (nToScroll == -1 || uWheelScrollLines == WHEEL_PAGESCROLL)
		{
			nDisplacement = client.Width();
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
		}
		else
		{
			nDisplacement = nToScroll * mScrollUnit.x;
			if (zDelta > 0)
				nDisplacement = -nDisplacement;
			nDisplacement = min(nDisplacement, client.Width());
		}
		ScrollPinnedImageBy(nDisplacement, 0, true);
		bResult = true;
	}

	if (bResult)
		UpdateWindow();

	return bResult;
}

// ---------------------------------------------------------------------------
//	¥ GetImageSize
// ---------------------------------------------------------------------------
//	Pass back the dimensions of a View's Image

void
LScrollable::GetImageSize(
	CPoint	&outSize) const
{
	outSize.x = mImageRect.Width();
	outSize.y = mImageRect.Height();
}


// ---------------------------------------------------------------------------
//	¥ GetScrollPosition
// ---------------------------------------------------------------------------
//	Pass back the location of a View's Frame within its Image

void
LScrollable::GetScrollPosition(
	CPoint	&outScrollPosition) const
{
	outScrollPosition.x = - mImageRect.left;
	outScrollPosition.y = - mImageRect.top;
}


// ---------------------------------------------------------------------------
//	¥ SetReconcileOverhang
// ---------------------------------------------------------------------------
//	Specify whether to reconcile the Frame and Image when there is overhang
//
//	See ReconcileFrameAndImage() for comments

void
LScrollable::SetReconcileOverhang(
	bool	inSetting)
{
	mReconcileOverhang = inSetting;
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ ScrollImageTo
// ---------------------------------------------------------------------------
//	Scroll Image to the specified horizontal and vertical locations
//
//	Scrolling moves the Image relative to the Frame and Port, so that a
//	different portion of the Image is visible thru the Frame.
//
//	When scrolled to (0, 0), the top left of the Image coincides with
//	the top left of the Frame (home position).
//
//	If inRefresh is true, the Port containing the View is updated
//	immediately, rather than refreshed at the next update event.
//	Scrolling usually happens during mouse down tracking, so we want
//	immediate visual feedback.

void
LScrollable::ScrollImageTo(
	SInt32		inLeftLocation,
	SInt32		inTopLocation,
	bool		inRefresh)
{
	ScrollImageBy(mImageRect.left + inLeftLocation,
				  mImageRect.top + inTopLocation,
				  inRefresh);
}


// ---------------------------------------------------------------------------
//	¥ ScrollImageBy
// ---------------------------------------------------------------------------
//	Scroll Image by specified horizontal and vertical increments
//
//	Scrolling moves the Image relative to the Frame and Port, so that a
//	different portion of the Image is visible thru the Frame.
//
//	Positive deltas scroll right and down.
//	Negative deltas scroll left and up.
//
//	If inRefresh is true, the Port containing the View is updated
//	immediately, rather than refreshed at the next update event.
//	Scrolling usually happens during mouse down tracking, so we want
//	immediate visual feedback.

void
LScrollable::ScrollImageBy(
	SInt32		inLeftDelta,			// Pixels to scroll horizontally
	SInt32		inTopDelta,				// Pixels to scroll vertically
	bool		inRefresh)
{
	if ((inLeftDelta == 0)  &&  (inTopDelta == 0)) {
		return;							// Do nothing
	}

	// Move each control
	HWND hWndChild = ::GetWindow(GetSafeHwnd(), GW_CHILD);
	if (hWndChild != NULL)
	{
		for (; hWndChild != NULL; hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
		{
			CRect rect;
			::GetWindowRect(hWndChild, &rect);
			ScreenToClient(&rect);
			::SetWindowPos(hWndChild, NULL,
				rect.left - inLeftDelta, rect.top - inTopDelta, 0, 0,
				SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOREDRAW);
		}
	}

	CRect frame;
	GetClientRect(frame);

	if (inRefresh && IsWindowVisible()) {

			// Check if any portion of what is visible now will be
			// visible after the scroll. If so, it should be faster
			// to move the bits rather than redrawing them.

		SInt32	absLeftDelta = inLeftDelta;
		if (absLeftDelta < 0) {
			absLeftDelta = -absLeftDelta;
		}
		SInt32	absTopDelta = inTopDelta;
		if (absTopDelta < 0) {
			absTopDelta = -absTopDelta;
		}

		if ( (absLeftDelta < frame.Width()) &&
			 (absTopDelta < frame.Height()) ) {
			ScrollBits(inLeftDelta, inTopDelta);
		} else {
			RedrawWindow(NULL, NULL, RDW_INVALIDATE);
		}
	}

	// Move Image relative to the Port
	mImageRect.OffsetRect(-inLeftDelta, -inTopDelta);

	ImageChanged();
}


// ---------------------------------------------------------------------------
//	¥ ScrollPinnedImageTo
// ---------------------------------------------------------------------------
//	Scroll Image to the specified horizontal and vertical locations but
//	not beyond the edge of the Frame.
//
//	Return true if the View actually scrolls

bool
LScrollable::ScrollPinnedImageTo(
	SInt32		inLeftLocation,
	SInt32		inTopLocation,
	bool		inRefresh)
{
	return ScrollPinnedImageBy(
				mImageRect.left + inLeftLocation,
				mImageRect.top + inTopLocation,
				inRefresh);
}


// ---------------------------------------------------------------------------
//	¥ ScrollPinnedImageBy
// ---------------------------------------------------------------------------
//	Scroll Image by specified horizontal and vertical increments, but
//	don't scroll beyond an edge of the Frame
//
//	Return true if the View actually scrolls

bool
LScrollable::ScrollPinnedImageBy(
	SInt32		inLeftDelta,			// Pixels to scroll horizontally
	SInt32		inTopDelta,				// Pixels to scroll vertically
	bool		inRefresh)
{
	CRect frame;
	GetClientRect(frame);
	if (inLeftDelta != 0) {				// Pin horizontal
		inLeftDelta = CalcPinnedScrollDelta(
								inLeftDelta,
								0,
								frame.Width(),
								mImageRect.left,
								mImageRect.right);
	}

	if (inTopDelta != 0) {				// Pin vertical
		inTopDelta = CalcPinnedScrollDelta(
								inTopDelta,
								0,
								frame.Height(),
								mImageRect.top,
								mImageRect.bottom);
	}

										// No scroll if both deltas are 0
	bool	scrolled = (inLeftDelta != 0) || (inTopDelta != 0);

	if (scrolled) {
		ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
	}

	return scrolled;
}


// ---------------------------------------------------------------------------
//	¥ CalcPinnedScrollDelta
// ---------------------------------------------------------------------------

SInt32
LScrollable::CalcPinnedScrollDelta(
	SInt32		inDelta,
	SInt32		inFrameMin,
	SInt32		inFrameMax,
	SInt32		inImageMin,
	SInt32		inImageMax) const
{
	SInt32	pinnedDelta = inDelta;		// In case there's no pinning

	SInt32	tryMin = inImageMin - inDelta;	// Potential new min/max for
	SInt32	tryMax = inImageMax - inDelta;	//   Image after scroll

		// Check for pin conditions. We may need to pin if
		//
		//		(tryMin > FrameMin)  or  (tryMax < FrameMax)
		//
		// because we always want the Frame to be entirely within
		// the Image.
		//
		// However, the Image can be smaller than the Frame. In that
		// case, we prefer to align the min edges rather than the
		// max edges.

	if ( ((inDelta < 0) && (inImageMin >= inFrameMin))  ||
		 ((inDelta > 0) && (inImageMin <= inFrameMin) &&
		 	(inImageMax <= inFrameMax)) ) {

		pinnedDelta = 0;						// Pin to current location

	} else
	if ( ((inDelta < 0) && (tryMin > inFrameMin))  ||
		 ((inDelta > 0) && (tryMin < inFrameMin) && (tryMax < inFrameMax) &&
		 	((inImageMax - inImageMin) < (inFrameMax - inFrameMin))) ) {

		pinnedDelta = inImageMin - inFrameMin;	// Pin to align min edges

	} else
	if ( ((inDelta > 0) && (tryMin < inFrameMin) && (tryMax < inFrameMax)) ) {

		pinnedDelta = inImageMax - inFrameMax;	// Pin to align max edges
	}

	return pinnedDelta;
}

// ---------------------------------------------------------------------------
//	¥ ScrollBits
// ---------------------------------------------------------------------------
//	Scroll the pixels of a View
//
//	Called internally by ScrollImageBy to shift the pixels

void
LScrollable::ScrollBits(
	SInt32		inLeftDelta,			// Pixels to scroll horizontally
	SInt32		inTopDelta)				// Pixels to scroll vertically
{

	ScrollWindowEx(-inLeftDelta, -inTopDelta, NULL, NULL, NULL, NULL, SW_INVALIDATE);
}


// ---------------------------------------------------------------------------
//	¥ AutoScrollImage
// ---------------------------------------------------------------------------
//	Scroll the Image if the specified point is outside the View's Frame.
//
//	Returns true if the View actually scrolled
//
//	Call this function while tracking the mouse to scroll a View in the
//	direction of the mouse location.

bool
LScrollable::AutoScrollImage(
	const CPoint&	inLocalPt)
{
	bool	scrolled = false;
	CRect	frame;
	GetClientRect(frame);

	SInt32	horizScroll = 0;
	if (inLocalPt.x < frame.left) {				// AutoScroll left
		horizScroll = -mScrollUnit.x;
	} else if (inLocalPt.x > frame.right) {		// AutoScroll right
		horizScroll = mScrollUnit.x;
	}

	SInt32	vertScroll = 0;
	if (inLocalPt.y < frame.top) {				// AutoScroll up
		vertScroll = -mScrollUnit.y;
	} else if (inLocalPt.y > frame.bottom) {	// AutoScroll down
		vertScroll = mScrollUnit.y;
	}

	if ((horizScroll != 0) || (vertScroll != 0)) {
		scrolled = ScrollPinnedImageBy(horizScroll, vertScroll, true);
	}

	return scrolled;
}


// ---------------------------------------------------------------------------
//	¥ SetScrollUnit
// ---------------------------------------------------------------------------

void
LScrollable::SetScrollUnit(
	const CPoint	&inScrollUnit)
{
	mScrollUnit = inScrollUnit;

	if (mScrollUnit.x < 1) {		// ScrollUnit must be a positive number
		mScrollUnit.x = 1;
	}

	if (mScrollUnit.y < 1) {
		mScrollUnit.y = 1;
	}

	ImageChanged();
}


// ---------------------------------------------------------------------------
//	¥ GetScrollUnit
// ---------------------------------------------------------------------------

void
LScrollable::GetScrollUnit(
	CPoint	&outScrollUnit) const
{
	outScrollUnit = mScrollUnit;
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ ResizeImageTo
// ---------------------------------------------------------------------------
//	Set the Image size to the specified pixel dimensions

void
LScrollable::ResizeImageTo(
	SInt32		inWidth,
	SInt32		inHeight,
	bool		inRefresh)
{
	ResizeImageBy(inWidth - mImageRect.Width(),
				  inHeight - mImageRect.Height(),
				  inRefresh);
}


// ---------------------------------------------------------------------------
//	¥ ResizeImageBy
// ---------------------------------------------------------------------------
//	Change the Image size by the specified pixel increments

void
LScrollable::ResizeImageBy(
	SInt32		inWidthDelta,
	SInt32		inHeightDelta,
	bool		inRefresh)
{
	mImageRect.right += inWidthDelta;
	mImageRect.bottom += inHeightDelta;

	ReconcileFrameAndImage(inRefresh);

	ImageChanged();
}


// ---------------------------------------------------------------------------
//	¥ ReconcileFrameAndImage
// ---------------------------------------------------------------------------
//	Adjusts the Image so that it fits within the Frame
//
//	This function addresses the problem of what to do when you scroll a
//	View to at or near the bottom or right, then make the View's Frame
//	larger. This would normally expose some "undefined" area below or
//	to the right of the Image.
//
//	If mReconcileOverhang is true, this function scrolls the Image so that
//	the bottom right corner is at the bottom right of the Frame. However,
//	it never moves the top left corner of the Image beyond the top left
//	of the Frame. Therefore, the only time "undefined" area is exposed is
//	when the Frame is larger than the Image.
//
//	For Views with fixed Image sizes, such as drawings where the Image size
//	is the size of a printed page, set mReconcileOverhang to true. The user
//	does not normally want to see past the bottom or right of such Views.
//
//	For Views with variable Image sizes, such as text blocks where the size
//	of the Image depends on the number of lines of text, set
//	mReconcileOverhang to false. The user may want to see the undefined
//	area in anticipation of the Image growing.

void
LScrollable::ReconcileFrameAndImage(
	bool	inRefresh)
{
	if (mReconcileOverhang) {
		CRect frame;
		GetClientRect(frame);

		CPoint	currScrollPos;
		GetScrollPosition(currScrollPos);
		CPoint	newScrollPos = currScrollPos;

									// Reconcile Vertical position
		if ( frame.Height() > mImageRect.bottom ) {
									// Frame extends below Image
			 newScrollPos.y = mImageRect.Height() - frame.Height();
			 if (newScrollPos.y < 0) {
			 	newScrollPos.y = 0;
			 }
		}

									// Reconcile horizontal position
		if ( frame.Width() > mImageRect.right ) {
									// Frame extends right Image
			 newScrollPos.x = mImageRect.Width() - frame.Width();
			 if (newScrollPos.x < 0) {
			 	newScrollPos.x = 0;
			 }
		}

		if ( (newScrollPos.y != currScrollPos.y) ||
			 (newScrollPos.x != currScrollPos.x) ) {
			 						// Scroll to keep bottom right in
			 						//   an appropriate position
			ScrollImageTo(newScrollPos.x, newScrollPos.y, false);
			
			// Always do refresh if change occurs
			//if (inRefresh) {
				RedrawWindow(NULL, NULL, RDW_INVALIDATE);
			//}
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ ImageChanged
// ---------------------------------------------------------------------------
//	Notification parent that the Image changed size, location,
//	or scroll units. Also called when client size changes
//
//	Override to respond to such changes.

void
LScrollable::ImageChanged()
{
	if (!mDoScrollbarUpdate || (m_hWnd == NULL))
		return;

	CRect client;
	GetClientRect(client);

	// Vertical
	{
		SCROLLINFO scinfo;
		scinfo.fMask = SIF_ALL;
		if (mHasScrollbars && (mImageRect.Height() - client.Height() > 0))
		{
			scinfo.nMin = 0;
			scinfo.nMax = mImageRect.Height();
			scinfo.nPage = client.Height();
			scinfo.nPos = client.top - mImageRect.top;
			mHasVScroll = true;
		}
		else
		{
			scinfo.nMin = 0;
			scinfo.nMax = 0;
			scinfo.nPage = 0;
			scinfo.nPos = 0;
			mHasVScroll = false;
		}
		SetScrollInfo(SB_VERT, &scinfo);
	}
	
	// Horizontal
	{
		SCROLLINFO scinfo;
		scinfo.fMask = SIF_ALL;
		if (mHasScrollbars && (mImageRect.Width() - client.Width() > 0))
		{
			scinfo.nMin = 0;
			scinfo.nMax = mImageRect.Width();
			scinfo.nPage = client.Width();
			scinfo.nPos = client.left - mImageRect.left;
			mHasHScroll = true;
		}
		else
		{
			scinfo.nMin = 0;
			scinfo.nMax = 0;
			scinfo.nPage = 0;
			scinfo.nPos = 0;
			mHasHScroll = false;
		}
		SetScrollInfo(SB_HORZ, &scinfo);
	}
	
	// Only if required
	if (mBroadcastScrollEvent)
	{
	}
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ ImageToLocalPoint
// ---------------------------------------------------------------------------
//	Convert point from Image (32-bit) to Local (16-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::ImageToLocalPoint(
	const CPoint	&inImagePt,
	CPoint			&outLocalPt) const
{
	outLocalPt.x = inImagePt.x + mImageRect.left;
	outLocalPt.y = inImagePt.y + mImageRect.top;
}


// ---------------------------------------------------------------------------
//	¥ LocalToImagePoint
// ---------------------------------------------------------------------------
//	Convert point from Local (16-bit) to Image (32-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::LocalToImagePoint(
	const CPoint		&inLocalPt,
	CPoint		&outImagePt) const
{
	outImagePt.x = inLocalPt.x - mImageRect.left;
	outImagePt.y = inLocalPt.y - mImageRect.top;
}

// ---------------------------------------------------------------------------
//	¥ ImageToLocalRect
// ---------------------------------------------------------------------------
//	Convert point from Image (32-bit) to Local (16-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::ImageToLocalRect(
	const CRect	&inImageRect,
	CRect			&outLocalRect) const
{
	outLocalRect = inImageRect;
	outLocalRect += mImageRect.TopLeft();
}


// ---------------------------------------------------------------------------
//	¥ LocalToImageRect
// ---------------------------------------------------------------------------
//	Convert point from Local (16-bit) to Image (32-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::LocalToImageRect(
	const CRect		&inLocalRect,
	CRect		&outImageRect) const
{
	outImageRect = inLocalRect;
	outImageRect -= mImageRect.TopLeft();
}

// ---------------------------------------------------------------------------
//	¥ ImagePointIsInFrame
// ---------------------------------------------------------------------------
//	Return whether a Point specified in Image Coords is within the
//	Frame of a View

bool
LScrollable::ImagePointIsInFrame(
	SInt32	inHorizImage,
	SInt32	inVertImage) const
{
	CRect rect;
	GetClientRect(rect);
	
	CPoint pt;
	pt.x = inHorizImage + mImageRect.left;
	pt.y = inVertImage + mImageRect.top;

	return rect.PtInRect(pt);
}


// ---------------------------------------------------------------------------
//	¥ ImageRectIntersectsFrame
// ---------------------------------------------------------------------------
//	Return whether a Rectangle specified in Image Coords intersects
//	the Frame of a View

bool
LScrollable::ImageRectIntersectsFrame(
	SInt32	inLeftImage,
	SInt32	inTopImage,
	SInt32	inRightImage,
	SInt32	inBottomImage) const
{
			// Covert input to Port Coords

	SInt32	inLeft = inLeftImage + mImageRect.left;
	SInt32	inRight = inRightImage + mImageRect.left;
	SInt32	inTop = inTopImage + mImageRect.top;
	SInt32	inBottom = inBottomImage + mImageRect.top;

			// Get individual coordinates of Frame in Port Coords

	CRect frame;
	GetClientRect(frame);
	SInt32	frLeft = 0;
	SInt32	frRight = frLeft + frame.Width();
	SInt32	frTop  = 0;
	SInt32	frBottom = frTop + frame.Height();

			// Determine if rectangles intersect by comparing
			// each edge of the input rectangle to the opposing
			// edge of the frame rectangle

	return ( (inLeft < frRight)  &&
			 (inTop < frBottom)  &&
			 (inRight > frLeft)  &&
			 (inBottom > frTop) );
}
