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


// CQuickSearchSavedPopup.h : header file
//

#ifndef __CQuickSearchSavedPopup__MULBERRY__
#define __CQuickSearchSavedPopup__MULBERRY__

#include "TPopupMenu.h"
#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchSavedPopup window

class CQuickSearchSavedPopup : public HPopupMenu
{
// Construction
public:
	enum
	{
		eChoose = 1,
		//eDivider,
		eFirst
	};

	CQuickSearchSavedPopup(const JCharacter*	title,
						   JXContainer*			enclosure,
						   const HSizingOption	hSizing,
						   const VSizingOption	vSizing,
						   const JCoordinate	x,
						   const JCoordinate	y,
						   const JCoordinate	w,
						   const JCoordinate	h);
	virtual ~CQuickSearchSavedPopup();

	virtual void	OnCreate();

protected:
	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

private:
	void	SyncMenu();

};

/////////////////////////////////////////////////////////////////////////////

#endif
