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


// Source for CFocusX class

#include "CFocusX.h"

#include <UGAColorRamp.h>
#include <UGraphicUtils.h>

// Special draw
void CFocusX::DrawBorder(SInt16 inBitDepth)
{
	RGBColor	tempColor;

	if (GetInsetSubPane()) {
		tempColor = Color_Black;
		Rect	insetRect;

		bool	activeAndEnabled = GetInsetSubPane()->IsActive() and
								   GetInsetSubPane()->IsEnabled();

												// Figure out the inset pane's bounding rect
		GetInsetSubPane()->CalcPortFrameRect ( insetRect );
		PortToLocalPoint ( topLeft ( insetRect ));
		PortToLocalPoint ( botRight ( insetRect ));

		if ( inBitDepth >= 4 ) {			// COLOR
												// If requested then we go ahead and frame the
												// inset sub pane if there is one
			if (FrameInsetSubPane()) {
												// Now we can proceed with the drawing of the frame
												// around the inset sub pane
												// BLACK INNER FRAME
												// TOPLEFT EDGES
				if (not activeAndEnabled) {
					tempColor = UGAColorRamp::GetColor(colorRamp_Gray10);
				}

				::RGBForeColor ( &tempColor );
				UGraphicUtils::TopLeftSide ( insetRect, -1, -1, -1, -1 );

											// BOTTOM RIGHT EDGES
												// If a notch has been requested then we draw the
												// frame with a notch otherwise we simply frame it
				if ( NotchInsetFrame ()) {
					UGraphicUtils::BottomRightSideWithNotch ( insetRect, -1, -1, -1, -1, mNotchWidth );
				} else {
					UGraphicUtils::BottomRightSide  ( insetRect, -1, -1, -1, -1 );
				}

				// CD: Always inset the shadow
				::MacInsetRect(&insetRect, -1, -1);
			}

			if (WantsFacePainted() &&			// if we want to draw shadows
				activeAndEnabled &&				// and shadows are called for
				(!HasFocus() || !CanFocus())) {	// and we're not drawing the focus ring
				tempColor = UGAColorRamp::GetColor(colorRamp_Gray7);
				::RGBForeColor ( &tempColor );
				UGraphicUtils::TopLeftSide ( insetRect, -1, -1, 0, 0 );

											// BOTTOM RIGHT LIGHT EDGES
				::RGBForeColor ( &Color_White );

												// If a notch has been requested then handle it,
												// otherwise just do the edge drawing
				if ( NotchInsetFrame ()) {
					UGraphicUtils::BottomRightSideWithNotch ( insetRect, 0, 0, -1, -1, mNotchWidth );
				} else {
					UGraphicUtils::BottomRightSide ( insetRect, 0, 0, -1, -1 );
				}
			}

		} else {							// BLACK & WHITE
												// No we can proceed with the drawing of the frame
												// around the inset sub pane
												// BLACK INNER FRAME
												// TOPLEFT EDGES
			::RGBForeColor ( &Color_Black );
			UGraphicUtils::TopLeftSide ( insetRect, -1, -1, -1, -1 );

											// BOTTOM RIGHT EDGES
												//	If a notch has been requested then we draw the
												// frame with a notch otherwise we simply frame it
			if ( NotchInsetFrame ()) {
				UGraphicUtils::BottomRightSideWithNotch ( insetRect, -1, -1, -1, -1, mNotchWidth );
			} else {
				UGraphicUtils::BottomRightSide  ( insetRect, -1, -1, -1, -1 );
			}
		}
	}
}