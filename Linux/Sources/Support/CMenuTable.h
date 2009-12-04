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


// CMenuTable.h - UI widget that implements a menu in the menu bar

#ifndef __CMENUTABLE__MULBERRY__
#define __CMENUTABLE__MULBERRY__

#include <JXTextMenuTable.h>

class JXImage;

class CMenuTable : public JXTextMenuTable
{
public:

	CMenuTable(JXMenu* menu, JXTextMenuData* data, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: JXTextMenuTable(menu, data, enclosure, hSizing, vSizing, x, y, w, h)
		{ }

	virtual ~CMenuTable() {}

protected:
	virtual void	Draw(JXWindowPainter& p, const JRect& rect);
	virtual void	TableDrawCell(JPainter& p, const JPoint& cell, const JRect& rect);

	void	MyDrawIconCheck(JPainter& p, const JIndex itemIndex, const JRect& rect);
	void	MyDrawSubmenuIndicator(JPainter& p, const JRect& rect, bool enabled, bool hilighted);
	void	MyDrawScrollRegions(JPainter& p);

private:
	static JXImage*	sCheckMark;
	static JXImage*	sSubMenu;
	static JXImage*	sSubMenuPushed;
	static JXImage*	sSubMenuDisabled;
	static JXImage*	sScrollUp;
	static JXImage*	sScrollDown;

	// not allowed

	CMenuTable(const CMenuTable& source);
	const CMenuTable& operator=(const CMenuTable& source);
};

#endif

