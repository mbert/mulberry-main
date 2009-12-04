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


// Header for CColorPopup class

#ifndef __CCOLORPOPUP__MULBERRY__
#define __CCOLORPOPUP__MULBERRY__

#include "TPopupMenu.h"
#include "HPopupMenu.h"

// Classes
class JXChooseColorDialog;
class JXWindowDirector;

class CColorPopup : public HPopupMenu
{
public:
	enum
	{
		eColorIndex_Black = 1,
		eColorIndex_Red,
		eColorIndex_Green,
		eColorIndex_Blue,
		eColorIndex_Yellow,
		eColorIndex_Cyan,
		eColorIndex_Magenta,
		eColorIndex_Mulberry,
		eColorIndex_White,
		eColorIndex_Other
	};

	static RGBColor sBlack;
	static RGBColor sWhite;
	static RGBColor sRed;
	static RGBColor sGreen;
	static RGBColor sBlue;
	static RGBColor sMagenta;
	static RGBColor sMulberry;
	static RGBColor sYellow;
	static RGBColor sCyan;
		
							CColorPopup(const JCharacter* title, JXContainer* enclosure,
								 const HSizingOption hSizing,
								 const VSizingOption vSizing,
								 const JCoordinate x, const JCoordinate y,
								 const JCoordinate w, const JCoordinate h);
	virtual					~CColorPopup();
	virtual void			OnCreate(JXWindowDirector* director);

	void 					SelectColor(JIndex color);
	void 					Ambiguate();

	void					SetColor(RGBColor color)
		{ mCurrentColor = color; mValue = eColorIndex_Other; Refresh(); }
	RGBColor				GetColor() const
		{ return mCurrentColor; }

protected:
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	Draw(JXWindowPainter& p, const JRect& rect);

private:	
	RGBColor				mCurrentColor;
	JXWindowDirector*		mDirector;
	JXChooseColorDialog*	mChooser;
	
	void	ChooseColor();
};

#endif
