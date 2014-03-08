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

#include "TPopupMenu.h"

#include "CMulberryCommon.h"

#include "CFontNameMenu.h"
#include "CFontSizeMenu.h"

#include <JString.h>
#include <JXColormap.h>
#include <JXImage.h>
#include <JXWindowPainter.h>
#include <jXPainterUtil.h>
#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

const JCoordinate _kTotalArrowWidth   = 28;
const JCoordinate _kArrowWidth        = 16;
const JCoordinate _kArrowHalfHeight   = 4;
const JCoordinate _kArrowShrinkWidth  = 4;
const JCoordinate _kArrowShrinkHeight = 2;

template <class T> void TPopupMenu<T>::TPopupMenuX(const JCoordinate	x,
								const JCoordinate	y,
								const JCoordinate	w,
								const JCoordinate	h)
{
	// Resize back to its original after title change forced dynamic size
	this->SetSize(std::max(w, _kTotalArrowWidth + 2*kJXDefaultBorderWidth), h);

	// By default all items are enabled
	// Whoever owns this popup will have to handle any enabling etc
	this->SetUpdateAction(JXMenu::kDisableNone);

	// Always listen to ourselves
	TPopupMenu<T>::ListenTo(this);
}

template <class T> void TPopupMenu<T>::SetValue(JIndex value)
{
	// Set internal cached popup value
	if (mValue != value)
	{
		mValue = value;

		// Force change to popup title
		SetToPopupChoice(kTrue, value);
	}
}

template <class T> void TPopupMenu<T>::Receive(JBroadcaster* sender, const CMenu::Message& message)
{
	if (sender == this && message.Is(JXMenu::kNeedsUpdate))
	{
		if (mValue)
			this->CheckItem(mValue);
	}
	else
		T::Receive(sender, message);
}

template <class T> void TPopupMenu<T>::SetToPopupChoice(const JBoolean isPopup, const JIndex initialChoice)
{
	// Bypass JXTextMenu which sets the title and forces an unwanted resize
	JXMenu::SetToPopupChoice(isPopup, initialChoice);
}

template <class T> void TPopupMenu<T>::SetPopupByName(const char* name)
{
	JIndex index;
	for(index = 1; index <= this->GetItemCount(); index++)
	{
		if (this->GetItemText(index) == name)
		{
			SetValue(index);
			break;
		}
	}
	
	if ((index > this->GetItemCount()) && (this->GetItemCount() != 0))
		SetValue(1);
}

template <class T> void TPopupMenu<T>::AdjustPopupChoiceTitle(const JIndex index)
{
	// Always remember current size and reset after title change
	const JCoordinate w = this->GetFrameWidth();
  
	JString newTitle;
  
	if (this->IsPopupChoice() && 0 < index && index <= this->GetItemCount())
	{
		if (!newTitle.IsEmpty())
			newTitle += ":  ";
		newTitle += this->GetItemText(index);
	}
	SetValue(index);

	this->SetTitleText(newTitle);

	const JCoordinate dw = w - this->GetFrameWidth();
	if (dw != 0)
		this->AdjustSize(dw, 0);
}

// This is pretty much a copy of JXMenuDraw but with our requirements applied
template <class T> void TPopupMenu<T>::Draw(JXWindowPainter& p, const JRect& rect)
{
	const JRect bounds = this->GetBounds();

	const JCoordinate borderWidth = this->GetBorderWidth();
	if (borderWidth == 0)
		JXDrawUpFrame(p, bounds, kJXDefaultBorderWidth);

	JRect r = bounds;
	if (borderWidth > 0)
	{
		JRect ra  = bounds;
		ra.top    = bounds.ycenter() - _kArrowHalfHeight;
		ra.bottom = ra.top + 2*_kArrowHalfHeight;

		if (this->GetPopupArrowPosition() == JXMenu::kArrowAtRight)
		{
			ra.left  = bounds.right - (_kTotalArrowWidth + _kArrowWidth)/2;
			ra.right = ra.left + _kArrowWidth;
			r.right -= _kTotalArrowWidth;
		}
		else
		{
			assert( this->GetPopupArrowPosition() == JXMenu::kArrowAtLeft );
			ra.right = bounds.left + (_kTotalArrowWidth + _kArrowWidth)/2;
			ra.left  = ra.right - _kArrowWidth;
			r.left  += _kTotalArrowWidth;
		}

		if (this->IsActive())
		{
			JXDrawDownArrowDown(p, ra, kJXDefaultBorderWidth);
		}
		else
		{
			JXFillArrowDown(p, ra, (this->GetColormap())->GetInactiveLabelColor());
			ra.Shrink(_kArrowShrinkWidth, _kArrowShrinkHeight);
			JXFillArrowDown(p, ra, this->GetBackColor());
		}
	}

	// Look for title & icon of selected item
	const JXImage* image = NULL;
	r.left += 2;
	r.right += 6;		// Gets tighter packing of string to arrow
	if (mValue && this->GetItemImage(mValue, &image) || this->GetTitleImage(&image))
	{
		
		p.JPainter::Image(*image, image->GetBounds(), r.left + 2,
					r.top + (r.height() - image->GetBounds().height())/2);
		r.left += image->GetBounds().width() + 4;
	}

	{
		p.SetFont(JGetDefaultFontName(), kJXDefaultFontSize, this->GetColormap()->GetBlackColor());
		JPoint origin(r.topLeft());
		origin.y += (r.height() - p.GetLineHeight())/2;
		::DrawClippedStringUTF8(&p, this->GetTitleText(), origin, r, eDrawString_Left);
	}
}

// Instantiate the ones we use
template class TPopupMenu<CMenu>;
template class TPopupMenu<CFontNameMenu>;
template class TPopupMenu<CFontSizeMenu>;
