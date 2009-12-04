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

#include <UNX_LScrollable.h>

#include "CMulberryCommon.h"

#include "StValueChanger.h"

// ---------------------------------------------------------------------------
//	Constants

//const SInt16		max_PortOrigin	= 16384;
//const SInt16		min_PortOrigin	= -32768;
//const SInt32		mask_Lo14Bits	= 0x00003FFF;


// ---------------------------------------------------------------------------
//	Class Variables

// ---------------------------------------------------------------------------
//	 LScrollable									Default Constructor		  [public]
// ---------------------------------------------------------------------------

LScrollable::LScrollable(JXScrollbarSet* scrollbarSet,
					   JXContainer* enclosure,
					   const HSizingOption hSizing, const VSizingOption vSizing,
					   const JCoordinate x, const JCoordinate y,
					   const JCoordinate w, const JCoordinate h) :
	JXScrollableWidget(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
}


// ---------------------------------------------------------------------------
//	 ~LScrollable								Destructor				  [public]
// ---------------------------------------------------------------------------
//

LScrollable::~LScrollable()
{
}

#pragma mark -

// ---------------------------------------------------------------------------
//	 GetImageSize
// ---------------------------------------------------------------------------
//	Pass back the dimensions of a View's Image

void
LScrollable::GetImageSize(
	JPoint	&outSize) const
{
	outSize.x = GetBoundsGlobal().width();
	outSize.y = GetBoundsGlobal().height();
}


// ---------------------------------------------------------------------------
//	 GetScrollPosition
// ---------------------------------------------------------------------------
//	Pass back the location of a View's Frame within its Image

void
LScrollable::GetScrollPosition(
	JPoint	&outScrollPosition) const
{
	outScrollPosition.x = GetApertureGlobal().left - GetBoundsGlobal().left;
	outScrollPosition.y = GetApertureGlobal().top - GetBoundsGlobal().top;
}


#pragma mark -

// ---------------------------------------------------------------------------
//	 ScrollImageTo
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
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	ScrollImageBy(imageLoc.x + inLeftLocation,
				  imageLoc.y + inTopLocation,
				  inRefresh);
}


// ---------------------------------------------------------------------------
//	 ScrollImageBy
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

	Scroll(-inLeftDelta, -inTopDelta);
}


// ---------------------------------------------------------------------------
//	 ScrollPinnedImageTo
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
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	return ScrollPinnedImageBy(
				imageLoc.x + inLeftLocation,
				imageLoc.y + inTopLocation,
				inRefresh);
}


// ---------------------------------------------------------------------------
//	 ScrollPinnedImageBy
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
	JRect apframe = GetAperture();
	JRect imframe = GetBounds();
	if (inLeftDelta != 0) {				// Pin horizontal
		inLeftDelta = CalcPinnedScrollDelta(
								inLeftDelta,
								apframe.left,
								apframe.right,
								imframe.left,
								imframe.right);
	}

	if (inTopDelta != 0) {				// Pin vertical
		inTopDelta = CalcPinnedScrollDelta(
								inTopDelta,
								apframe.top,
								apframe.bottom,
								imframe.top,
								imframe.bottom);
	}

										// No scroll if both deltas are 0
	bool	scrolled = (inLeftDelta != 0) || (inTopDelta != 0);

	if (scrolled) {
		ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
	}

	return scrolled;
}


// ---------------------------------------------------------------------------
//	 CalcPinnedScrollDelta
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
//	 SetScrollUnit
// ---------------------------------------------------------------------------

void
LScrollable::SetScrollUnit(
	const JPoint	&inScrollUnit)
{
	SetHorizStepSize(inScrollUnit.x);
	SetVertStepSize(inScrollUnit.y);
}


// ---------------------------------------------------------------------------
//	 GetScrollUnit
// ---------------------------------------------------------------------------

void
LScrollable::GetScrollUnit(
	JPoint	&outScrollUnit) const
{
	outScrollUnit.x = itsHStepSize;
	outScrollUnit.y = itsVStepSize;
}

#pragma mark -

// ---------------------------------------------------------------------------
//	 ResizeImageTo
// ---------------------------------------------------------------------------
//	Set the Image size to the specified pixel dimensions

void
LScrollable::ResizeImageTo(
	SInt32		inWidth,
	SInt32		inHeight,
	bool		inRefresh)
{
	ResizeImageBy(inWidth - GetBoundsGlobal().width(),
				  inHeight - GetBoundsGlobal().height(),
				  inRefresh);
}


// ---------------------------------------------------------------------------
//	 ResizeImageBy
// ---------------------------------------------------------------------------
//	Change the Image size by the specified pixel increments

void
LScrollable::ResizeImageBy(
	SInt32		inWidthDelta,
	SInt32		inHeightDelta,
	bool		inRefresh)
{
	SetBounds(GetBoundsGlobal().width() + inWidthDelta, GetBoundsGlobal().height() + inHeightDelta);
}


#pragma mark -

// ---------------------------------------------------------------------------
//	 ImageToLocalPoint
// ---------------------------------------------------------------------------
//	Convert point from Image (32-bit) to Local (16-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::ImageToLocalPoint(
	const JPoint	&inImagePt,
	JPoint			&outLocalPt) const
{
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	outLocalPt = inImagePt + imageLoc;
}


// ---------------------------------------------------------------------------
//	 LocalToImagePoint
// ---------------------------------------------------------------------------
//	Convert point from Local (16-bit) to Image (32-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::LocalToImagePoint(
	const JPoint		&inLocalPt,
	JPoint		&outImagePt) const
{
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	outImagePt = inLocalPt - imageLoc;
}

// ---------------------------------------------------------------------------
//	 ImageToLocalRect
// ---------------------------------------------------------------------------
//	Convert point from Image (32-bit) to Local (16-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::ImageToLocalRect(
	const JRect	&inImageRect,
	JRect			&outLocalRect) const
{
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	outLocalRect = inImageRect;
	outLocalRect.Shift(imageLoc);
}


// ---------------------------------------------------------------------------
//	 LocalToImageRect
// ---------------------------------------------------------------------------
//	Convert point from Local (16-bit) to Image (32-bit) coordinates
//
//	Image and Local coordinates are different only when the Image size
//	is greater than 16K (15-bit)

void
LScrollable::LocalToImageRect(
	const JRect		&inLocalRect,
	JRect		&outImageRect) const
{
	JPoint imageLoc;
	GetImageLocation(imageLoc);
	outImageRect = inLocalRect;
	outImageRect.Shift(-imageLoc);
}

// ---------------------------------------------------------------------------
//	 ImagePointIsInFrame
// ---------------------------------------------------------------------------
//	Return whether a Point specified in Image Coords is within the
//	Frame of a View

bool
LScrollable::ImagePointIsInFrame(
	SInt32	inHorizImage,
	SInt32	inVertImage) const
{
	JRect image_rect = GetAperture();			// Get image frame of aperture
	
	JPoint ipt(inHorizImage, inVertImage);

	return image_rect.Contains(ipt);
}


// ---------------------------------------------------------------------------
//	 ImageRectIntersectsFrame
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
			// Covert input to Global Coords

	SInt32	inLeft = inLeftImage + GetBoundsGlobal().left;
	SInt32	inRight = inRightImage + GetBoundsGlobal().left;
	SInt32	inTop = inTopImage + GetBoundsGlobal().top;
	SInt32	inBottom = inBottomImage + GetBoundsGlobal().top;

			// Get individual coordinates of Frame in Global Coords

	JRect frame = GetApertureGlobal();
	SInt32	frLeft = frame.left;
	SInt32	frRight = frame.right;
	SInt32	frTop  = frame.top;
	SInt32	frBottom = frame.bottom;

			// Determine if rectangles intersect by comparing
			// each edge of the input rectangle to the opposing
			// edge of the frame rectangle

	return ( (inLeft < frRight)  &&
			 (inTop < frBottom)  &&
			 (inRight > frLeft)  &&
			 (inBottom > frTop) );
}
