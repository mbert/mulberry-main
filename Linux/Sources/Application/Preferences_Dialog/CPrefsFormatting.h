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


// CPrefsFormatting.h : header file
//

#ifndef __CPREFSFORMATTING__MULBERRY__
#define __CPREFSFORMATTING__MULBERRY__

#include "CPrefsPanel.h"

#include "CFontPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsFormatting dialog

class JXTextCheckbox;

class CPrefsFormatting : public CPrefsPanel
{
// Construction
public:
	CPrefsFormatting(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

	virtual void AboutToShow();

protected:
// begin JXLayout1

    CFontNamePopup* 	mListFont;
    CFontSizePopup* 	mListSize;
    CFontNamePopup* 	mDisplayFont;
    CFontSizePopup* 	mDisplaySize;
    CFontNamePopup* 	mPrinterFont;
    CFontSizePopup* 	mPrinterSize;
    CFontNamePopup* 	mCaptionFont;
    CFontSizePopup* 	mCaptionSize;
    CFontNamePopup* 	mHTMLFont;
    CFontSizePopup* 	mHTMLSize;
    CFontNamePopup* 	mFixedFont;
    CFontSizePopup* 	mFixedSize;
    CFontSizePopup* 	mMinimumSize;
    JXTextCheckbox*     mUseStyles;

// end JXLayout1
	bool mPrefsSet;
};

#endif
