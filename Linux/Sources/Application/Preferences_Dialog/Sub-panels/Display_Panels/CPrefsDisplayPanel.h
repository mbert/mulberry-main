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


// Header for CPrefsDisplayPanel class

#ifndef __CPREFSDISPLAYPANEL__MULBERRY__
#define __CPREFSDISPLAYPANEL__MULBERRY__

#include "CTabPanel.h"

// Constants

// Classes

struct SStyleTraits;
struct SStyleTraits2;
class CTextInputField;
class JXColorButton;
class JXTextCheckbox;

class CPrefsDisplayPanel : public CTabPanel
{
public:
	CPrefsDisplayPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

protected:
			struct SStyleItems
			{
				JXColorButton*		mColor;
				JXTextCheckbox* 	mBold;
				JXTextCheckbox* 	mItalic;
				JXTextCheckbox*  	mUnderline;
			};

			struct SFullStyleItems
			{
				JXColorButton*		mColor;
				JXTextCheckbox* 	mBold;
				JXTextCheckbox* 	mItalic;
				JXTextCheckbox* 	mStrike;
				JXTextCheckbox*  	mUnderline;
			};

			struct SFullStyleItems2
			{
				JXColorButton*		mColor;
				JXTextCheckbox* 	mUseColor;
				JXColorButton*		mBkgColor;
				JXTextCheckbox*		mUseBkgColor;
				JXTextCheckbox* 	mBold;
				JXTextCheckbox* 	mItalic;
				JXTextCheckbox*	 	mStrike;
				JXTextCheckbox*  	mUnderline;
				CTextInputField*  	mName;
			};

			void			GetControls(SStyleItems& items,JXColorButton* color_c, JXTextCheckbox* bold_c,
										JXTextCheckbox* italic_c, JXTextCheckbox* underline_c);
			void			SetStyle(SStyleItems& items, const SStyleTraits& traits);
			SStyleTraits&	GetStyle(SStyleItems& items, SStyleTraits& traits);

			void			GetControls(SFullStyleItems& items, JXColorButton* color_c, JXTextCheckbox* bold_c,
										JXTextCheckbox* italic_c, JXTextCheckbox* strike_c, JXTextCheckbox* underline_c);
			void			SetStyle(SFullStyleItems& items, const SStyleTraits& traits);
			SStyleTraits&	GetStyle(SFullStyleItems& items, SStyleTraits& traits);

			void			GetControls(SFullStyleItems2& items, JXColorButton* color_c, JXTextCheckbox* usecolor_c, JXColorButton* bkgnd_c, JXTextCheckbox* usebkgnd_c,
										JXTextCheckbox* bold_c, JXTextCheckbox* italic_c, JXTextCheckbox* strike_c, JXTextCheckbox* underline_c, CTextInputField* name_c);
			void			SetStyle(SFullStyleItems2& items, const SStyleTraits2& traits);
			SStyleTraits2&	GetStyle(SFullStyleItems2& items, SStyleTraits2& traits);
};

#endif
