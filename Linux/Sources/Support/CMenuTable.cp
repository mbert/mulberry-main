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


// CMenuTable.cp - UI widget that implements a menu in the menu bar

#include "CMenuTable.h"

#include "CIconLoader.h"
#include "CInvertImage.h"
#include "CLighterImage.h"

#include "HResourceMap.h"
#include "StPenState.h"

#include <JXColormap.h>
#include <JXImage.h>
#include <JXTextMenuData.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>

JXImage* CMenuTable::sCheckMark = NULL;
JXImage* CMenuTable::sSubMenu = NULL;
JXImage* CMenuTable::sSubMenuPushed = NULL;
JXImage* CMenuTable::sSubMenuDisabled = NULL;
JXImage* CMenuTable::sScrollUp = NULL;
JXImage* CMenuTable::sScrollDown = NULL;

void CMenuTable::Draw(JXWindowPainter& p, const JRect& rect)
{
	// Bypass all menu table calls so we can do our own drawing
	JXTable::Draw(p, rect);

	p.ResetClipRect();
	MyDrawScrollRegions(p);

	p.ResetClipRect();

	const JCoordinate w  = GetApertureWidth();
	const JSize rowCount = GetRowCount();
	for (JIndex i=1; i<=rowCount; i++)
		{
		if (itsTextMenuData->HasSeparator(i))
			{
			JRect r = GetCellRect(JPoint(1,i));
			r.top   = r.bottom - kSeparatorHeight;
			r.right = r.left + w;
			JXDrawDownFrame(p, r, kSeparatorHeight/2);
			}
		}
}

void CMenuTable::TableDrawCell(JPainter& p, const JPoint& cell, const JRect& origRect)
{
	// Paint selection colour into background
	unsigned long bkgnd = 0x00CCCCCCC;
	if (cell.y == itsHilightRow)
	{
		StPenState save(&p);

		JRect rect = AdjustRectForSeparator(cell.y, origRect);
		rect.bottom--;

		JColorIndex bkgnd_index = GetColormap()->GetDefaultSelectionColor();
		p.SetPenColor(bkgnd_index);
		p.SetFilling(kTrue);
		p.Rect(rect);

		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	// Use our special check mark
	if (cell.x == kCheckboxColumnIndex)
	{
		JRect rect = AdjustRectForSeparator(cell.y, origRect);
		rect.left += kHilightBorderWidth;
		MyDrawIconCheck(p, cell.y, rect);
		return;
	}
	else if (cell.x == kSubmenuColumnIndex && itsTextMenuData->HasSubmenu(cell.y))
	{
		JRect rect = AdjustRectForSeparator(cell.y, origRect);

		rect.right -= kHilightBorderWidth;
		rect.left   = rect.right - JXMenuTable::kSubmenuColWidth;
		MyDrawSubmenuIndicator(p, rect, itsTextMenuData->IsEnabled(cell.y), ((JIndex) cell.y) == itsHilightRow);
		return;
	}
	else if (cell.x == kImageColumnIndex)
		{
		// Look for image ID
		JIndex imageID;
		if (itsTextMenuData->GetImageID(cell.y, &imageID))
			{
			// The image is actually an icon id and we load the real icon here
			JXImage* image = CIconLoader::GetIcon(imageID, this, 16, bkgnd, itsTextMenuData->IsEnabled(cell.y) ? CIconLoader::eNormal : CIconLoader::eDisabled);
			p.Image(*image, image->GetBounds(), origRect);
			return;
			}
		}

	// Do inherited
	JXTextMenuTable::TableDrawCell(p, cell, origRect);
}

void CMenuTable::MyDrawIconCheck(JPainter& p, const JIndex itemIndex, const JRect& rect)
{
	// Make sure popup indicator is loaded
	if (!sCheckMark)
		sCheckMark = iconFromResource(IDI_MENU_TICKMARK, this, 16, 0x00CCCCCC);

	JBoolean isRadio, isChecked;
	const JBoolean isCheckbox = itsTextMenuData->IsCheckbox(itemIndex, &isRadio, &isChecked);

	if ((isRadio || isCheckbox) && isChecked)
	{
		const JPoint center(rect.xcenter(), rect.ycenter());
		JRect boxRect(center, center);
		boxRect.Shrink(-8, -8);
		p.JPainter::Image(*sCheckMark, sCheckMark->GetBounds(), boxRect.left, boxRect.top);
	}
}

void CMenuTable::MyDrawSubmenuIndicator(JPainter& p, const JRect& rect, bool enabled, bool hilighted)
{
	// Make sure indicator is loaded
	if (!sSubMenu)
		sSubMenu = iconFromResource(IDI_MENU_SUBMENU, this, 16, 0x00CCCCCC);
	if (!sSubMenuPushed)
	{
		sSubMenuPushed = new CInvertImage(*sSubMenu);
		static_cast<CInvertImage*>(sSubMenuPushed)->InvertImage();
	}
	if (!sSubMenuDisabled)
	{
		sSubMenuDisabled = new CLighterImage(*sSubMenu);
		static_cast<CLighterImage*>(sSubMenuDisabled)->LightenImage();
	}

	JRect r(-8, -8, 8, 8);
	r.Shift(rect.center());
	if (enabled)
	{
		if (hilighted)
			p.JPainter::Image(*sSubMenuPushed, sSubMenuPushed->GetBounds(), r.left, r.top);
		else
			p.JPainter::Image(*sSubMenu, sSubMenu->GetBounds(), r.left, r.top);
	}
	else
		p.JPainter::Image(*sSubMenuDisabled, sSubMenuDisabled->GetBounds(), r.left, r.top);

}

void CMenuTable::MyDrawScrollRegions(JPainter& p)
{
	// Make sure indicators are loaded
	if (!sScrollUp)
		sScrollUp = iconFromResource(IDI_MENU_SCROLLUP, this, 16, 0x00CCCCCC);
	if (!sScrollDown)
		sScrollDown = iconFromResource(IDI_MENU_SCROLLDOWN, this, 16, 0x00CCCCCC);

	if (itsHasScrollUpFlag && itsMouseInScrollUpFlag)
		p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsScrollUpArrowRect.left, itsScrollUpArrowRect.top);
	else if (itsHasScrollUpFlag)
		p.JPainter::Image(*sScrollUp, sScrollUp->GetBounds(), itsScrollUpArrowRect.left, itsScrollUpArrowRect.top);

	if (itsHasScrollDownFlag && itsMouseInScrollDownFlag)
		p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsScrollDownArrowRect.left, itsScrollDownArrowRect.top);
	else if (itsHasScrollDownFlag)
		p.JPainter::Image(*sScrollDown, sScrollDown->GetBounds(), itsScrollDownArrowRect.left, itsScrollDownArrowRect.top);
}
