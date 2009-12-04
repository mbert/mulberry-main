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


// CSplitterView

// Class that implements a better splitter window

#ifndef __CSPLITTERVIEW__MULBERRY__
#define __CSPLITTERVIEW__MULBERRY__

#include "CCommanderProtect.h"

// Classes
class CContainerWnd;

class CSplitterView : public CWnd
{
	DECLARE_DYNAMIC(CSplitterView)

// Implementation
public:
	CSplitterView();
	~CSplitterView();

	void InstallViews(CWnd* view1, CWnd* view2, bool horiz);
			
	void ShowView(bool view1, bool view2);
	void SetMinima(long min1, long min2);
	void SetLocks(bool lock1, bool lock2);
	void SetVisibleLock(bool visible_lock)
		{ mVisibleLock = visible_lock; }
	void	SetLockResize(bool lock_resize)
		{ mLockResize = lock_resize; }

	long	GetTotalSize() const;
	long	GetRelativeSplitPos() const;
	void	SetRelativeSplitPos(long split);
	long	GetPixelSplitPos() const
		{ return mDividerPos; }
	void	SetPixelSplitPos(long split);
	void	ChangedDividerPos(long change);

	void CalcDividerRect(CRect& divRect);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

protected:
	CContainerWnd*	mSub1;
	CContainerWnd*	mSub2;

	mutable long	mDividerPos;
	long			mMin1;				// Minimum size for first pane
	long			mMin2;				// Minimum size for second pane
	bool			mHorizontal;		// Is divider horizontal
	bool			mVisible1;			// Is first pane visible
	bool			mVisible2;			// Is second pane visible
	bool			mLock1;				// Is first pane size locked
	bool			mLock2;				// Is second pane size locked
	bool			mVisibleLock;		// Is divider visible when locked
	bool			mLockResize;		// Divider is locked to keep first pane size constant when resizing
	bool			mMouseOverDivider;	// Mouse is over divider
	bool			mTracking;			// Tracking mouse
	CWnd*			mRestoreFocus;		// Window to restore to focus after tracking
	CRect			mTrackLimits;		// Limit of tracking
	CRect			mTrackRect;			// Rect of divider during tracking
	CPoint			mTrackPt;			// Track point
	CPoint			mTrackMovePt;		// Track point
	CSize			mCurrentSize;		// Current size
	CCommanderProtect		mCmdProtect;					// Protect commands

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg	void OnLButtonUp(UINT nFlags, CPoint point);				// Clicked somewhere
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

private:
	static HCURSOR sVertCursor;
	static HCURSOR sHorizCursor;
	static CBitmap*	sHorizGrab;
	static CBitmap*	sVertGrab;

	void	InitViews();
	void	RepositionViews();
	void	AdjustContents();

	void	OnInvertTracker(const CRect& rect);
};

#endif
