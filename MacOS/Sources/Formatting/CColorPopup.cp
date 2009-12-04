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


#include "CColorPopup.h"
#include "RGB.h"
#include "cdstring.h"

#include <Quickdraw.h>
#include <ColorPicker.h>

MenuHandle CColorPopup::sColorMenu = nil;
RGBColor CColorPopup::sBlack = {0,0,0};
RGBColor CColorPopup::sWhite = {65535,65535,65535};
RGBColor CColorPopup::sRed = {65535,0,0};
RGBColor CColorPopup::sGreen = {0,65535,0};
RGBColor CColorPopup::sBlue = {0,0,65535};
RGBColor CColorPopup::sMagenta = {65535,0,65535};
RGBColor CColorPopup::sMulberry = {39321,13107,26214};
RGBColor CColorPopup::sYellow = {65535, 65535, 0};
RGBColor CColorPopup::sCyan = {0,65535,65535};

const ResIDT MENU_ColorPopup = 165;

CColorPopup::CColorPopup(LStream *inStream) : LBevelButton(inStream)
{
	CurrentColor = sBlack;
}


CColorPopup::~CColorPopup()
{

}


void CColorPopup::FinishCreateSelf()
{



	if(sColorMenu == nil)
		sColorMenu = ::GetMenu(MENU_ColorPopup);
	SetMenuMinMax();

	LBevelButton::FinishCreateSelf();
}


void CColorPopup::AddColor(char *theName, RGB color)
{
	MenuHandle menuH = GetMacMenuH();
	::AppendMenu(menuH, "\p <B");
	::SetMenuItemText(menuH, ::CountMenuItems(menuH), LStr255(theName));

	SetMenuMinMax();

	// SetValue(::CountMenuItems(menuH));
}

void CColorPopup::SetValue(SInt32 inValue){
	// ¥ Setup the current item if needed
	if ((GetValue () != inValue) || (inValue == 10))
	{
		// ¥ Get the current item setup in the menu
		MenuHandle menuH = GetMacMenuH ();
		if ( menuH )
			SetCurrentMenuItem ( inValue );

		// ¥ Call our superclass to handle the setting of the value
		// NOTE: we specifically do not call the LGAIconButton superclass
		// but instead its superclass as we don't want the popup to become
		// selected when we set the value which is used differently for a popup

		if (inValue == 10)
		{
			/*RGBColor Color = CurrentColor;

			Point		where = { 0, 0 };
			Str255	prompt = "\pSelect swatch color:";
			RGBColor outColor;

			UDesktop::Deactivate ();

			if ( ::GetColor ( where, prompt, &Color, &outColor ))
			{
				CurrentColor = outColor;
				Refresh ();
			}

			*/UDesktop::Activate ();
		}
		else
		{
			switch(inValue)
			{
				case eblack:
					CurrentColor = sBlack;
					break;
				case ered:
					CurrentColor = sRed;
					break;
				case egreen:
					CurrentColor = sGreen;
					break;
				case eblue:
					CurrentColor = sBlue;
					break;
				case eyellow:
					CurrentColor = sYellow;
					break;
				case ecyan:
					CurrentColor = sCyan;
					break;
				case emagenta:
					CurrentColor = sMagenta;
					break;
				case emulberry:
					CurrentColor = sMulberry;
					break;
				case ewhite:
					CurrentColor = sWhite;
					break;
			}
		}

		mValue = -1;
		LControl::SetValue ( inValue );

	}

}

void CColorPopup::selectColor(SInt32 color)
{
	bool different = (mValue != color);
	mMenuChoice = mValue = color;

	switch(color)
	{
	case eblack:
		CurrentColor = sBlack;
		break;
	case ered:
		CurrentColor = sRed;
		break;
	case egreen:
		CurrentColor = sGreen;
		break;
	case eblue:
		CurrentColor = sBlue;
		break;
	case eyellow:
		CurrentColor = sYellow;
		break;
	case ecyan:
		CurrentColor = sCyan;
		break;
	case emagenta:
		CurrentColor = sMagenta;
		break;
	case emulberry:
		CurrentColor = sMulberry;
		break;
	case ewhite:
		CurrentColor = sWhite;
		break;
	case eother:
		RGBColor Color = CurrentColor;

		Point		where = { 0, 0 };
		Str255	prompt = "\pSelect swatch color:";
		RGBColor outColor;

		UDesktop::Deactivate ();

		if ( ::GetColor ( where, prompt, &Color, &outColor ))
		{
			CurrentColor = outColor;
			Refresh ();
		}

		UDesktop::Activate ();
		break;
	}
			
	// Set the icon resource id
	ControlButtonContentInfo controlInfo;
	controlInfo.contentType = kControlContentIconSuiteRes;
	controlInfo.u.resID = 3500 + color - 1;
	SetContentInfo(controlInfo);

	if (different && (color != eother))
		Refresh();
}


void CColorPopup::Ambiguate(){


		if ( GetValue () != 0)
		{
			// ¥ Get the current item setup in the menu
			MenuHandle menuH = GetMacMenuH ();
			if ( menuH )
			{
				SetCurrentMenuItem (0);
			}

			// ¥ Call our superclass to handle the setting of the value
			// NOTE: we specifically do not call the LGAIconButton superclass
			// but instead its superclass as we don't want the popup to become
			// selected when we set the value which is used differently for a popup

			mValue = 0;			//   Store new value
			Refresh();
		}

}

RGBColor CColorPopup::getColor(){
	return CurrentColor;

}
