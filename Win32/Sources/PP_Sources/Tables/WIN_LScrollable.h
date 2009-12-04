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


#ifndef _H_WIN_LScrollable
#define _H_WIN_LScrollable

// ---------------------------------------------------------------------------

class LScrollable : public CWnd
{
	DECLARE_DYNCREATE(LScrollable)

public:
						LScrollable();
	virtual				~LScrollable();

	virtual void		GetImageSize( CPoint& outSize ) const;

	void				GetImageLocation( CPoint& outLocation ) const
							{
								outLocation = mImageRect.TopLeft();
							}

	virtual void		GetScrollPosition( CPoint& outScrollPosition ) const;

	void				SetReconcileOverhang( bool inSetting );

	void				ScrollImageTo(
								SInt32				inLeftLocation,
								SInt32				inTopLocation,
								bool				inRefresh);

	virtual void		ScrollImageBy(
								SInt32				inLeftDelta,
								SInt32				inTopDelta,
								bool				inRefresh);

	bool				ScrollPinnedImageTo(
								SInt32				inLeftLocation,
								SInt32				inTopLocation,
								bool				inRefresh);

	virtual bool		ScrollPinnedImageBy(
								SInt32				inLeftDelta,
								SInt32				inTopDelta,
								bool				inRefresh);

	virtual SInt32		CalcPinnedScrollDelta(
								SInt32				inDelta,
								SInt32				inFrameMin,
								SInt32				inFrameMax,
								SInt32				inImageMin,
								SInt32				inImageMax) const;

	virtual void		ScrollBits(
								SInt32				inLeftDelta,
								SInt32				inTopDelta);

	virtual bool		AutoScrollImage( const CPoint& inLocalPt );

	void				ResizeImageTo(
								SInt32				inWidth,
								SInt32				inHeight,
								bool				inRefresh);
								
	virtual void		ResizeImageBy(
								SInt32				inWidthDelta,
								SInt32				inHeightDelta,
								bool				inRefresh);

	void				SetScrollUnit( const CPoint& inScrollUnit );

	void				GetScrollUnit( CPoint& outScrollUnit ) const;

	virtual void		ImageChanged();

	void				ImageToLocalPoint(
								const CPoint&		inImagePt,
								CPoint&				outLocalPt) const;
								
	void				LocalToImagePoint(
								const CPoint&		inLocalPt,
								CPoint&				outImagePt) const;

	void				ImageToLocalRect(
								const CRect&		inImageRect,
								CRect&				outLocalRect) const;
								
	void				LocalToImageRect(
								const CRect&		inLocalRect,
								CRect&				outImageRect) const;

	bool				ImagePointIsInFrame(
								SInt32				inHorizImage,
								SInt32				inVertImage) const;
								
	bool				ImageRectIntersectsFrame(
								SInt32				inLeftImage,
								SInt32				inTopImage,
								SInt32				inRightImage,
								SInt32				inBottomImage) const;

protected:
	CRect			mImageRect;
	CPoint			mScrollUnit;
	bool			mReconcileOverhang;
	bool			mBroadcastScrollEvent;
	bool			mHasScrollbars;
	bool			mDoScrollbarUpdate;
	bool			mHasHScroll;
	bool			mHasVScroll;

	virtual void		ReconcileFrameAndImage( bool inRefresh );

			BOOL		DoMouseWheel(UINT fFlags, short zDelta, CPoint point);

	//{{AFX_MSG(CGrayBackground)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg	BOOL OnMouseWheel(UINT fFlags, short zDelta, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
