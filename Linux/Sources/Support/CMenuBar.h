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


// CMenuBar.h - UI widget that implements a 3D divider

#ifndef __CMENUBAR__MULBERRY__
#define __CMENUBAR__MULBERRY__

#include <JXMenuBar.h>

class CMenuBar : public JXMenuBar
{
public:

	const static int cMenuBarHeight = 24;

	CMenuBar(JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	virtual ~CMenuBar() {}

protected:
	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);

private:
	// not allowed

	CMenuBar(const CMenuBar& source);
	const CMenuBar& operator=(const CMenuBar& source);
};

#endif
