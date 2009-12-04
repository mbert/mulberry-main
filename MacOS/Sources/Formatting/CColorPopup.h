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


#ifndef __CCOLORPOPUP__MULBERRY__
#define __CCOLORPOPUP__MULBERRY__


#include <LBevelButton.h>
#include "CEditFormattedTextDisplay.h"

class RGB;

class CColorPopup : public LBevelButton
{
public:
	enum { class_ID = 'Colo' };

	static RGBColor sBlack;
	static RGBColor sWhite;
	static RGBColor sRed;
	static RGBColor sGreen;
	static RGBColor sBlue;
	static RGBColor sMagenta;
	static RGBColor sMulberry;
	static RGBColor sYellow;
	static RGBColor sCyan;
	
							CColorPopup(LStream *inStream);
	virtual					~CColorPopup();
	virtual void			FinishCreateSelf();
	void 					AddColor(char *theName, RGB theColor);
	void 					selectColor(SInt32 color);
	void 					Ambiguate();
	virtual void			SetValue(SInt32 inValue);
	void					setColor(RGBColor color)
								{ CurrentColor = color; mValue = eother; Draw(nil); }
	RGBColor				getColor();	

	virtual	void			SetMacMenuH(MenuHandle inMacMenuH)				// Do nothing
								{ }
	virtual	MenuHandle		GetMacMenuH(void)
								{ return sColorMenu; }				// Use the common Menu handle
	virtual	MenuHandle		LoadPopupMenuH(void) const						// Do nothing
								{ return nil; }


private:	
	RGBColor CurrentColor;
	
	static MenuHandle sColorMenu;
};

#endif
