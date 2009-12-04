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


// CMenu.h - UI widget that implements a menu in the menu bar

#ifndef __CMENU__MULBERRY__
#define __CMENU__MULBERRY__

#include <JXTextMenu.h>

class CMenu : public JXTextMenu
{
public:

	CMenu(const JCharacter* title, JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h)
	: JXTextMenu(title, enclosure, hSizing, vSizing, x,y, w,h)
		{ CMenuX(); }
	CMenu(JXImage* image, const JBoolean menuOwnsImage,
			   JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h)
	: JXTextMenu(image, menuOwnsImage, enclosure, hSizing, vSizing, x,y, w,h)
		{ CMenuX(); }

	CMenu(JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure)
	: JXTextMenu(owner, itemIndex, enclosure)
		{ CMenuX(); }


	virtual ~CMenu() {}

protected:
	virtual JXMenuDirector*	CreateWindow(JXWindowDirector* supervisor);

private:
	void CMenuX();

	// not allowed

	CMenu(const CMenu& source);
	const CMenu& operator=(const CMenu& source);

};

#endif
