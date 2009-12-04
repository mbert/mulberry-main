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


#include "JXMultiImageCheckbox.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"

#include <JXWindowPainter.h>
#include <JXImage.h>
#include <JXColormap.h>
#include <jAssert.h>
#include <jXPainterUtil.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

JXMultiImageCheckbox::JXMultiImageCheckbox
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
	JXCheckbox(enclosure, hSizing, vSizing, x,y, w,h),
	itsOffImageID(0),
	itsOffPushedImageID(0),
	itsOnImageID(0),
	itsOnPushedImageID(0)
{
	// Our 3D buttons have 3-pixel border
	SetBorderWidth(3);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXMultiImageCheckbox::~JXMultiImageCheckbox()
{
}

/******************************************************************************
 SetImage

 ******************************************************************************/

void	JXMultiImageCheckbox::SetImage(ResIDT offImageID)
{
	itsOffImageID         = offImageID;
	itsOffPushedImageID   = offImageID;
	itsOnImageID          = offImageID;
	itsOnPushedImageID    = offImageID;

	Refresh();
}

/******************************************************************************
 SetImages

 ******************************************************************************/

void	JXMultiImageCheckbox::SetImages(ResIDT offImageID, ResIDT offpushedImageID,
										ResIDT onImageID, ResIDT onpushedImageID)
{
	itsOffImageID         = offImageID;
	itsOnImageID          = onImageID ? onImageID : offImageID;
	itsOffPushedImageID   = offpushedImageID ? offpushedImageID : onImageID;
	itsOnPushedImageID    = onpushedImageID ? onpushedImageID : offImageID;

	Refresh();
}

/******************************************************************************
 Draw (virtual protected)

 ******************************************************************************/

void
JXMultiImageCheckbox::Draw
	(
	JXWindowPainter&	p,
	const JRect&		rect
	)
{
	unsigned long bkgnd = 0x00FFFFFF;
	JColorIndex bkgnd_index = GetColormap()->Get3DShadeColor();
	if (IsActive() && DrawChecked())
		bkgnd_index = GetColormap()->Get3DShadeColor();
	else
		bkgnd_index = GetColormap()->GetDefaultBackColor();
	{
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	ResIDT iconID = 0;
	bool disabled = false;
	JXImage* draw_me = NULL;
	JRect actual_rect(GetBounds());

	if (IsActive())
	{
		if (IsChecked())
			iconID = DrawChecked() ? itsOnImageID  : itsOnPushedImageID;
		else
			iconID = !DrawChecked() ? itsOffImageID : itsOffPushedImageID;
	}
	else
	{
		iconID = itsOffImageID;
		disabled = true;
	}
	draw_me = CIconLoader::GetIcon(iconID, this, actual_rect.width() >= 32 ? 32 : 16, bkgnd, disabled ? CIconLoader::eDisabled : CIconLoader::eNormal);

	// Center icon in button area
	int hoffset = (GetBoundsWidth() - draw_me->GetBounds().width())/2;
	int voffset = (GetBoundsHeight() - draw_me->GetBounds().height())/2;
	p.JPainter::Image(*draw_me, draw_me->GetBounds(), actual_rect.left + hoffset, actual_rect.top + voffset);
}

void JXMultiImageCheckbox::DrawBackground(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	CDrawUtils::DrawBackground(p, frame, selected && use_frame, enabled);
}

void JXMultiImageCheckbox::DrawBorder(JXWindowPainter& p, const JRect&	frame)
{
	bool selected = DrawChecked();
	bool enabled = IsActive();
	bool use_frame = GetBorderWidth();

	if (use_frame)
		CDrawUtils::Draw3DBorder(p, frame, selected, enabled);
}
