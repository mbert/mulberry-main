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


#include "JXMultiImageButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"

#include <JXWindowPainter.h>
#include <JXImage.h>
#include <JXColormap.h>
#include <jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXMultiImageButton::JXMultiImageButton
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXButton(enclosure, hSizing, vSizing, x,y, w,h),
	itsImageID(0), itsPushedImageID(0)
{
	// Our 3D buttons have 3-pixel border
	SetBorderWidth(3);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXMultiImageButton::~JXMultiImageButton()
{
}

/******************************************************************************
 SetImage

 ******************************************************************************/

void
JXMultiImageButton::SetImage
	(
	ResIDT imageID
	)
{
	itsImageID         = imageID;
	itsPushedImageID   = imageID;

	Refresh();
}

/******************************************************************************
 SetImages

 ******************************************************************************/

void
JXMultiImageButton::SetImages
	(
	ResIDT imageID,
	ResIDT pushedImageID
	)
{
	itsImageID         = imageID;
	itsPushedImageID   = pushedImageID ? pushedImageID : imageID;

	Refresh();
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXMultiImageButton::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	unsigned long bkgnd = 0x00FFFFFF;
	JColorIndex bkgnd_index = GetColormap()->Get3DShadeColor();
	if (IsActive() && IsPushed())
		bkgnd_index = GetColormap()->Get3DShadeColor();
	else
		bkgnd_index = GetColormap()->GetDefaultBackColor();
	{
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	JXImage* draw_me = NULL;
	JRect actual_rect(GetBounds());

	if (IsActive())
		draw_me = CIconLoader::GetIcon(IsPushed() ? itsPushedImageID : itsImageID, this, actual_rect.width() >= 26 ? 32 : 16, bkgnd);
	else
		draw_me = CIconLoader::GetIcon(itsImageID, this, actual_rect.width() >= 26 ? 32 : 16, bkgnd, CIconLoader::eDisabled);

	// Center icon in button area
	int hoffset = (GetBoundsWidth() - draw_me->GetBounds().width())/2;
	int voffset = (GetBoundsHeight() - draw_me->GetBounds().height())/2;
	p.JPainter::Image(*draw_me, draw_me->GetBounds(), actual_rect.left + hoffset, actual_rect.top + voffset);
}

void JXMultiImageButton::DrawBackground(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = IsPushed();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	CDrawUtils::DrawBackground(p, frame, selected && use_frame, enabled);
}

void JXMultiImageButton::DrawBorder(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = IsPushed();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	if (use_frame)
		CDrawUtils::Draw3DBorder(p, frame, selected, enabled);
}
