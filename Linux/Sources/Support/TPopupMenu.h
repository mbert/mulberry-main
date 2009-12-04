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

#ifndef __TPOPUPMENU_H
#define __TPOPUPMENU_H

#include "CMenu.h"

template <class T> class TPopupMenu : public T
{
public:
	TPopupMenu
	(
		const JCharacter*	title,
		JXContainer*		enclosure,
		const CMenu::HSizingOption	hSizing,
		const CMenu::VSizingOption	vSizing,
		const JCoordinate	x,
		const JCoordinate	y,
		const JCoordinate	w,
		const JCoordinate	h
	)
	: T(title, enclosure, hSizing, vSizing, x, y, w, h), mValue(0)
    	{ TPopupMenuX(x, y, w, h); }

	TPopupMenu
	(
		JXImage* image,
		const JBoolean menuOwnsImage,
		JXContainer*		enclosure,
		const CMenu::HSizingOption	hSizing,
		const CMenu::VSizingOption	vSizing,
		const JCoordinate	x,
		const JCoordinate	y,
		const JCoordinate	w,
		const JCoordinate	h
	)
	: T(image, menuOwnsImage, enclosure, hSizing, vSizing, x, y, w, h), mValue(0)
    	{ TPopupMenuX(x, y, w, h); }

	virtual void SetValue(JIndex value);
	JIndex GetValue() const
		{ return mValue; }

	const JString& GetCurrentItemText() const
		{ return this->GetItemText(mValue); }

	virtual void	SetToPopupChoice(const JBoolean isPopup,
									 const JIndex initialChoice);

			void	SetPopupByName(const char* name);

protected:
	JIndex mValue;

			void TPopupMenuX(const JCoordinate	x,
								const JCoordinate	y,
								const JCoordinate	w,
								const JCoordinate	h);
	virtual void Receive(JBroadcaster* sender, const CMenu::Message& message);
	virtual void AdjustPopupChoiceTitle(const JIndex index);
	virtual void Draw(JXWindowPainter& p, const JRect& rect);
};

#endif
