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


#ifndef _H_UNX_LScrollable
#define _H_UNX_LScrollable

#include <JXScrollableWidget.h>

// ---------------------------------------------------------------------------

class LScrollable : public JXScrollableWidget
{
public:
	LScrollable(JXScrollbarSet* scrollbarSet,
					   JXContainer* enclosure,
					   const HSizingOption hSizing, const VSizingOption vSizing,
					   const JCoordinate x, const JCoordinate y,
					   const JCoordinate w, const JCoordinate h);
	virtual				~LScrollable();

	virtual void		GetImageSize( JPoint& outSize ) const;

	void				GetImageLocation( JPoint& outLocation ) const
							{
								outLocation = GetBoundsGlobal().topLeft() - GetApertureGlobal().topLeft();
							}

	virtual void		GetScrollPosition( JPoint& outScrollPosition ) const;

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

	void				ResizeImageTo(
								SInt32				inWidth,
								SInt32				inHeight,
								bool				inRefresh);
								
	virtual void		ResizeImageBy(
								SInt32				inWidthDelta,
								SInt32				inHeightDelta,
								bool				inRefresh);

	void				SetScrollUnit( const JPoint& inScrollUnit );

	void				GetScrollUnit( JPoint& outScrollUnit ) const;

	void				ImageToLocalPoint(
								const JPoint&		inImagePt,
								JPoint&				outLocalPt) const;
								
	void				LocalToImagePoint(
								const JPoint&		inLocalPt,
								JPoint&				outImagePt) const;

	void				ImageToLocalRect(
								const JRect&		inImageRect,
								JRect&				outLocalRect) const;
								
	void				LocalToImageRect(
								const JRect&		inLocalRect,
								JRect&				outImageRect) const;

	bool				ImagePointIsInFrame(
								SInt32				inHorizImage,
								SInt32				inVertImage) const;
								
	bool				ImageRectIntersectsFrame(
								SInt32				inLeftImage,
								SInt32				inTopImage,
								SInt32				inRightImage,
								SInt32				inBottomImage) const;
};

#endif
