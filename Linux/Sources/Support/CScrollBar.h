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


// CScrollBar.h - UI widget that implements a 3D divider

#ifndef __CSCROLLBAR__MULBERRY__
#define __CSCROLLBAR__MULBERRY__

#include <JXScrollbar.h>

class JXImage;

class CScrollBar : public JXScrollbar
{
public:
	CScrollBar(JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	virtual ~CScrollBar() {}

protected:
	virtual void	Draw(JXWindowPainter& p, const JRect& rect);
	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);

private:
	static JXImage*	sScrollUp;
	static JXImage*	sScrollDown;
	static JXImage*	sScrollLeft;
	static JXImage*	sScrollRight;

	void	MyDrawHoriz(JPainter& p);
	void	MyDrawVert(JPainter& p);

	// not allowed

	CScrollBar(const CScrollBar& source);
	const CScrollBar& operator=(const CScrollBar& source);
};

#endif
