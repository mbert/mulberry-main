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

#include "CPrefsTabSubPanel.h"


// Constants

// Classes
class CTextFieldX;
class LCheckBox;
class LGAColorSwatchControl;

struct SStyleTraits;
struct SStyleTraits2;

class	CPrefsDisplayPanel : public CPrefsTabSubPanel
{
public:
					CPrefsDisplayPanel();
					CPrefsDisplayPanel(LStream *inStream);
	virtual 		~CPrefsDisplayPanel();

protected:
			struct SStyleItems
			{
				LGAColorSwatchControl* mColor;
				LCheckBox* 		mBold;
				LCheckBox* 		mItalic;
				LCheckBox*  	mUnderline;
			};

			struct SFullStyleItems
			{
				LGAColorSwatchControl* mColor;
				LCheckBox* 		mBold;
				LCheckBox* 		mItalic;
				LCheckBox*	 	mStrike;
				LCheckBox*  	mUnderline;
			};

			struct SFullStyleItems2
			{
				LGAColorSwatchControl*	mColor;
				LCheckBox* 			 	mUseColor;
				LGAColorSwatchControl*	mBkgColor;
				LCheckBox*				mUseBkgColor;
				LCheckBox* 		mBold;
				LCheckBox* 		mItalic;
				LCheckBox*	 	mStrike;
				LCheckBox*  	mUnderline;
				CTextFieldX*  	mName;
			};

			void			GetControls(SStyleItems& items, PaneIDT color_c, PaneIDT bold_c,
										PaneIDT italic_c, PaneIDT underline_c);
			void			SetStyle(SStyleItems& items, const SStyleTraits& traits);
			SStyleTraits&	GetStyle(SStyleItems& items, SStyleTraits& traits);

			void			GetControls(SFullStyleItems& items, PaneIDT color_c, PaneIDT bold_c,
										PaneIDT italic_c, PaneIDT strike_c, PaneIDT underline_c);
			void			SetStyle(SFullStyleItems& items, const SStyleTraits& traits);
			SStyleTraits&	GetStyle(SFullStyleItems& items, SStyleTraits& traits);

			void			GetControls(SFullStyleItems2& items, PaneIDT color_c, PaneIDT usecolor_c, PaneIDT bkgcolor_c, PaneIDT usebkgcolor_c,
										PaneIDT bold_c, PaneIDT italic_c, PaneIDT strike_c, PaneIDT underline_c, PaneIDT name_c);
			void			SetStyle(SFullStyleItems2& items, const SStyleTraits2& traits);
			SStyleTraits2&	GetStyle(SFullStyleItems2& items, SStyleTraits2& traits);
};

#endif
