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


#ifndef __CPREFSFORMATTING__MULBERRY__
#define __CPREFSFORMATTING__MULBERRY__

#include "CPrefsPanel.h"


// Constants

// Panes


const	PaneIDT		paneid_PrefsStyled = 5011;
const	PaneIDT		paneid_ListFontMenu = 'DFNT';
const	PaneIDT		paneid_ListSizeMenu = 'DSIZ';
const	PaneIDT		paneid_DisplayFontMenu = 'FONT';
const	PaneIDT		paneid_DisplaySizeMenu = 'SIZE';
const	PaneIDT		paneid_PrintFontMenu = 'PFNT';
const	PaneIDT		paneid_PrintSizeMenu = 'PSIZ';
const	PaneIDT		paneid_CaptionFontMenu = 'CFNT';
const	PaneIDT		paneid_CaptionSizeMenu = 'CSIZ';
const	PaneIDT		paneid_FixedFont = 'FFNT';
const	PaneIDT		paneid_FixedSize = 'FSIZ';
const	PaneIDT		paneid_UseStyles = 'STYL';
const	PaneIDT		paneid_HTMLFont = 'HFNT';
const	PaneIDT		paneid_HTMLSize = 'HSIZ';
const	PaneIDT		paneid_MinimumSize = 'MINS';
const	PaneIDT		paneid_AAText = 'AATX';

// Messages


// Classes
class CFontPopup;
class CSizePopup;
class LCheckBox;

class CPrefsFormatting : public CPrefsPanel
{
private:
	CFontPopup*		mListFontMenu;
	CSizePopup*		mListSizeMenu;
	CFontPopup*		mDisplayFontMenu;
	CSizePopup*		mDisplaySizeMenu;
	CFontPopup*		mPrintFontMenu;
	CSizePopup*		mPrintSizeMenu;
	CFontPopup*		mCaptionFontMenu;
	CSizePopup*		mCaptionSizeMenu;
	CFontPopup*		mFixedFont;
	CSizePopup*		mFixedSize;
	CFontPopup*		mHTMLFont;
	CSizePopup*		mHTMLSize;
	CSizePopup*		mMinimumSize;
	LCheckBox*		mUseStyles;
	LCheckBox*		mAntiAliasFont;
	
public:
	enum { class_ID = 'Pfor' };
	
						CPrefsFormatting();
						CPrefsFormatting(LStream *inStream);
	virtual				~CPrefsFormatting();
	
	
protected:
	virtual void 		FinishCreateSelf(void);
	
public:
	virtual void 		ToggleICDisplay(void);
	virtual void		SetPrefs(CPreferences *copyPrefs);
	virtual void 		UpdatePrefs(void);
	
};

#endif
