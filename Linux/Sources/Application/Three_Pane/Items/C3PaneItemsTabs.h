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


// C3PaneItemsTabs

#ifndef __C3PANEITEMSTABS__MULBERRY__
#define __C3PANEITEMSTABS__MULBERRY__

#include "JXTabs.h"

#include "cdstring.h"

class C3PaneItems;
class CMultibitPopup;

class C3PaneItemsTabs : public JXTabs
{
public:
	C3PaneItemsTabs(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);
	virtual ~C3PaneItemsTabs() {}

	void	SetOwner(C3PaneItems* owner)
		{ mOwner = owner; }

	void	SetActiveTabs(bool active)
		{ mActiveTabs = active; }
	bool	HasActiveTabs() const
		{ return mActiveTabs; }

protected:
	C3PaneItems*	mOwner;
	cdstring		mMenuBase; // ID for its menu
	CMultibitPopup*	mPopup;
	JIndex			mPopupIndex;
	bool			mActiveTabs;
	
	virtual void OnRightClick(JIndex index, const JPoint& pt,
								const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers);
	virtual void OnShiftClick(JIndex index);

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void MenuChoice(JIndex choice, JIndex btn);
};

#endif
