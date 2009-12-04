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

#include "HBackgroundImage.h"

#include <JXDisplay.h>
#include <JXWindow.h>
#include <JXWindowPainter.h>
#include <JXCursorAnimator.h>
#include <JXCursorAnimationTask.h>
#include <JXDNDManager.h>
#include <JXHintManager.h>
#include <jXUtil.h>
#include <jXGlobals.h>
#include <JString.h>
#include <jAssert.h>

HBackgroundImage::HBackgroundImage(JXContainer* enclosure,
			  const HSizingOption hSizing, const VSizingOption vSizing,
			  const JCoordinate x, const JCoordinate y,
			  const JCoordinate w, const JCoordinate h)
  :JXImageWidget(enclosure, hSizing, vSizing, x, y, w, h)
{
}

void HBackgroundImage::DrawAll(JXWindowPainter& p, const JRect& origFrameG)
{
	JRect clipRectG = GetFrameGlobal();
	if (!JIntersection(clipRectG, origFrameG, &clipRectG))
		{
		return;
		}

	JRect apClipRectG = GetApertureGlobal();
	const JBoolean apVisible =
		JIntersection(apClipRectG, origFrameG, &apClipRectG);

	Region visRegion = NULL;
	if (apVisible)
		{
		XRectangle xClipRect = JXJToXRect(apClipRectG);
		visRegion = JXRectangleRegion(&xClipRect);

		}

	// prepare the painter to draw

	const JRect boundsG = GetBoundsGlobal();

	p.SetOrigin(boundsG.left, boundsG.top);
	p.JPainter::Reset(clipRectG);

	// draw background and contents, if visible

	if (visRegion != NULL && XEmptyRegion(visRegion))
		{
		// If nothing else is visible, we can quit now.

		XDestroyRegion(visRegion);
		return;
		}
	else if (visRegion != NULL)
		{
		// convert visRegion to local coordinates

		XOffsetRegion(visRegion, -boundsG.left, -boundsG.top);
		}

	if (apVisible)
		{
		if (visRegion != NULL)
			{
			p.Reset(apClipRectG, visRegion);
			}
		else
			{
			p.JPainter::Reset(apClipRectG);
			}

		const JRect apLocal = GetAperture();
		DrawBackground(p, apLocal);

		// draw contents clipped to aperture, if visible

		JRect boundsClipRectG;
		if (JIntersection(boundsG, apClipRectG, &boundsClipRectG))
			{
			p.ResetAllButClipping();
			Draw(p, JXContainer::GlobalToLocal(boundsClipRectG));
			}
		}

	// Now do inherited
	JXImageWidget::DrawAll(p, origFrameG);

	// clean up

	if (visRegion != NULL)
		{
		XDestroyRegion(visRegion);
		}
}
