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

#ifndef H_CMonthPopup
#define H_CMonthPopup

#include "TPopupMenu.h"
#include "HPopupMenu.h"

// ===========================================================================
//	CMonthPopup

class	CMonthPopup : public HPopupMenu
{
public:

			CMonthPopup(const JCharacter*	title,
						   JXContainer*			enclosure,
						   const HSizingOption	hSizing,
						   const VSizingOption	vSizing,
						   const JCoordinate	x,
						   const JCoordinate	y,
						   const JCoordinate	w,
						   const JCoordinate	h)
			: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CMonthPopup() {}

	virtual void		OnCreate();

	void		SetMonth(int32_t month);
	int32_t		GetMonth() const;
};

#endif
