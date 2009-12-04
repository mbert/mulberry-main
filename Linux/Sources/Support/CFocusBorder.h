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


// CFocusBorder.h - UI widget that implements a 3D divider

#ifndef __CFOCUSBORDER__MULBERRY__
#define __CFOCUSBORDER__MULBERRY__

#include <JXDecorRect.h>

const unsigned long cFocusBorderInset = 3;

class CFocusBorder : public JXDecorRect
{
public:

	CFocusBorder(JXContainer* enclosure,
			   const HSizingOption hSizing, const VSizingOption vSizing,
			   const JCoordinate x, const JCoordinate y,
			   const JCoordinate w, const JCoordinate h);

	virtual ~CFocusBorder() {}

	bool	IsFocussed() const
		{ return mFocussed; }
	void	SetFocus(bool focus);
	void	HasFocus(bool focus)
		{ mHasFocus = focus; }

	void	SetTransparent(bool transparent)
		{ mTransparent = transparent; }

protected:
	bool	mFocussed;
	bool	mHasFocus;
	bool	mTransparent;

	virtual void	DrawBackground(JXWindowPainter& p, const JRect& frameG);
	virtual void	DrawBorder(JXWindowPainter& p, const JRect& frame);

private:
	// not allowed

	CFocusBorder(const CFocusBorder& source);
	const CFocusBorder& operator=(const CFocusBorder& source);
};

#endif
