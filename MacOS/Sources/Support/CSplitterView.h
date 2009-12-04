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


// Header for CSplitterView class

#ifndef __CSPLITTERVIEW__MULBERRY__
#define __CSPLITTERVIEW__MULBERRY__


// Resources

// Messages

// Classes

class CSplitterView : public LView
{
public:
	enum { class_ID = 'SplV' };

					CSplitterView();
					CSplitterView(LStream *inStream);
	virtual 		~CSplitterView();
	
	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Track split divider
	virtual void	AdjustMouseSelf(Point inPortPt,
									const EventRecord &inMacEvent,
									RgnHandle outMouseRgn);			// Adjust cursor

	virtual void	CalcDividerRect(Rect& divRect);
	virtual void	DrawSelf();	
	virtual void	ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh);

			void	InstallViews(LView* view1, LView* view2, bool horiz);
			
			void	ShowView(bool view1, bool view2);
			void	SetMinima(SInt16 min1, SInt16 min2);
			void	SetLocks(bool lock1, bool lock2);
			void	SetVisibleLock(bool visible_lock)
				{ mVisibleLock = visible_lock; }
			void	SetLockResize(bool lock_resize)
				{ mLockResize = lock_resize; }

			long	GetTotalSize() const;
			long	GetRelativeSplitPos() const;
			void	SetRelativeSplitPos(long split);
			SInt16	GetPixelSplitPos() const
				{ return mDividerPos; }
			void	SetPixelSplitPos(SInt16 split);
			void	ChangedDividerPos(long change);

protected:
	LView*			mSub1;				// Sub-panes created to hold split views
	LView*			mSub2;				// Sub-panes created to hold split views
	
	SInt16			mDividerPos;		// Current position of divider
	SInt16			mMin1;				// Minimum size for first pane
	SInt16			mMin2;				// Minimum size for second pane
	bool			mHorizontal;		// Is divider horizontal
	bool			mVisible1;			// Is first pane visible
	bool			mVisible2;			// Is second pane visible
	bool			mLock1;				// Is first pane size locked
	bool			mLock2;				// Is second pane size locked
	bool			mVisibleLock;		// Is divider visible when locked
	bool			mLockResize;		// Divider is locked to keep first pane size constant when resizing

private:
	static PicHandle	sHorizGrab;
	static PicHandle	sVertGrab;
	
			void	InitSplitterView();
			void	InitViews();
			void	RepositionViews();
			void	AdjustContents();
};

#endif
