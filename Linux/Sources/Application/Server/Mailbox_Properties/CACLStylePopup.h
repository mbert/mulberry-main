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


// Header for CACLStylePopup class

#ifndef __CACLSTYLEPOPUP__MULBERRY__
#define __CACLSTYLEPOPUP__MULBERRY__

#include "TPopupMenu.h"
#include "HPopupMenu.h"

#include "CACL.h"
#include "CPreferenceValue.h"

// Consts

// Classes
class CACLStylePopup : public HPopupMenu
{

public:
	enum
	{
		eNewUser = 1,
		eDeleteUser,
		eFirstStyle
	};

	CACLStylePopup(const JCharacter*	title,
					JXContainer*		enclosure,
					const HSizingOption	hSizing,
					const VSizingOption	vSizing,
					const JCoordinate	x,
					const JCoordinate	y,
					const JCoordinate	w,
					const JCoordinate	h)
		: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h), 
		  mMbox(false), mList(NULL) {}

	virtual void	Reset(bool mbox);						// Reset items
	virtual void	DoNewStyle(SACLRight rights);			// Add new style
	virtual void	DoDeleteStyle();						// Delete existing styles

protected:
	bool								mMbox;
	CPreferenceValueMap<SACLStyleList>*	mList;
};

#endif
