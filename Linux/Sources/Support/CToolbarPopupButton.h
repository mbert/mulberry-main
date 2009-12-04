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


// Header for CToolbarPopupButton class

#ifndef __CTOOLBARPOPUPBUTTON__MULBERRY__
#define __CTOOLBARPOPUPBUTTON__MULBERRY__

#include "CToolbarButton.h"

// Classes
class JXTextMenu;

class CToolbarPopupButton : public CToolbarButton
{
public:
	CToolbarPopupButton(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual 		~CToolbarPopupButton();

	virtual bool 	HasPopup() const;
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void SetValue(JIndex value)
		{ mValue = value; }
	JIndex GetValue() const
		{ return mValue; }

	virtual const JXTextMenu* GetPopupMenu() const
		{ return mMenu; }
	virtual JXTextMenu*	GetPopupMenu()
		{ return mMenu; }

protected:
	JXTextMenu*		mMenu;
	JIndex			mValue;

	virtual void HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	virtual void	UpdateMenu();

			void	LButtonDownBtn(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
			void	LButtonDownPopup(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

public:
	static const JCharacter* kMenuSelect;
	static const JCharacter* kMenuUpdate;

	class MenuSelect : public JBroadcaster::Message
	{
	public:

		MenuSelect(JXTextMenu* menu, JIndex index) :
			JBroadcaster::Message(kMenuSelect), itsMenu(menu), itsIndex(index)
			{ }

		JXTextMenu* GetMenu() const
			{ return itsMenu; }
		JIndex GetIndex() const
			{ return itsIndex; }

	private:
		JXTextMenu*				itsMenu;
		JIndex					itsIndex;
	};

	class MenuUpdate : public JBroadcaster::Message
	{
	public:

		MenuUpdate(CToolbarPopupButton* pbtn) :
			JBroadcaster::Message(kMenuUpdate), itsBtn(pbtn)
			{ }

		CToolbarPopupButton* GetBtn() const
			{ return itsBtn; }

	private:
		CToolbarPopupButton*	itsBtn;
	};
};

#endif
