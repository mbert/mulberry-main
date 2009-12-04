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


// Header for CPrefsEditCaption class

#ifndef __CPREFSEDITCAPTION__MULBERRY__
#define __CPREFSEDITCAPTION__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"


// Panes
const	PaneIDT		paneid_PrefsEditCaptionDialog = 5104;
const	PaneIDT		paneid_PrefsEditCaptionPage = 'PAGE';
const	PaneIDT		paneid_PrefsEditCaptionCursorTop = 'CTOP';
const	PaneIDT		paneid_PrefsEditCaptionButton1 = 'BTN1';
const	PaneIDT		paneid_PrefsEditCaptionButton2 = 'BTN2';
const	PaneIDT		paneid_PrefsEditCaptionBox1 = 'BOX1';
const	PaneIDT		paneid_PrefsEditCaptionSummary = 'SUMM';
const	PaneIDT		paneid_PrefsEditCaptionScroller1 = 'SCR1';
const	PaneIDT		paneid_PrefsEditCaptionText1 = 'TXT1';
const	PaneIDT		paneid_PrefsEditCaptionButton3 = 'BTN3';
const	PaneIDT		paneid_PrefsEditCaptionButton4 = 'BTN4';
const	PaneIDT		paneid_PrefsEditCaptionBox2 = 'BOX2';
const	PaneIDT		paneid_PrefsEditCaptionScroller2 = 'SCR2';
const	PaneIDT		paneid_PrefsEditCaptionText2 = 'TXT2';
const	PaneIDT		paneid_PrefsEditCaptionRevert = 'REVR';

// Resources
const	ResIDT		RidL_CPrefsEditCaptionBtns = 5104;

// Messages
const	MessageT	msg_PrefsEditCaptionMeName = 'MNAM';
const	MessageT	msg_PrefsEditCaptionMeEmail = 'MEMA';
const	MessageT	msg_PrefsEditCaptionMeFull = 'MFUL';
const	MessageT	msg_PrefsEditCaptionMeFirst = 'MFST';
const	MessageT	msg_PrefsEditCaptionSmartName = 'SNAM';
const	MessageT	msg_PrefsEditCaptionSmartEmail = 'SEMA';
const	MessageT	msg_PrefsEditCaptionSmartFull = 'SFUL';
const	MessageT	msg_PrefsEditCaptionSmartFirst = 'SFST';
const	MessageT	msg_PrefsEditCaptionFromName = 'FNAM';
const	MessageT	msg_PrefsEditCaptionFromEmail = 'FEMA';
const	MessageT	msg_PrefsEditCaptionFromFull = 'FFUL';
const	MessageT	msg_PrefsEditCaptionFromFirst = 'FFST';
const	MessageT	msg_PrefsEditCaptionToName = 'TNAM';
const	MessageT	msg_PrefsEditCaptionToEmail = 'TEMA';
const	MessageT	msg_PrefsEditCaptionToFull = 'TFUL';
const	MessageT	msg_PrefsEditCaptionToFirst = 'TFST';
const	MessageT	msg_PrefsEditCaptionCCName = 'CNAM';
const	MessageT	msg_PrefsEditCaptionCCEmail = 'CEMA';
const	MessageT	msg_PrefsEditCaptionCCFull = 'CFUL';
const	MessageT	msg_PrefsEditCaptionCCFirst = 'CFST';
const	MessageT	msg_PrefsEditCaptionSubject = 'SUBJ';
const	MessageT	msg_PrefsEditCaptionDateShort = 'DSHR';
const	MessageT	msg_PrefsEditCaptionDateLong = 'DLNG';
const	MessageT	msg_PrefsEditCaptionPage = 'PAGE';
const	MessageT	msg_PrefsEditCaptionNow = 'NOW ';
const	MessageT	msg_PrefsEditCaptionButton1 = 'BTN1';
const	MessageT	msg_PrefsEditCaptionButton2 = 'BTN2';
const	MessageT	msg_PrefsEditCaptionButton3 = 'BTN3';
const	MessageT	msg_PrefsEditCaptionButton4 = 'BTN4';
const	MessageT	msg_PrefsEditCaptionRevert = 'REVR';

// Classes

class LCheckBox;
class CTextDisplay;
template<class T> class CPreferenceValueMap;

class CPrefsEditCaption : public LDialogBox
{
private:
	LCheckBox*		mCursorTop;
	LCheckBox*		mUseBox1;
	LCheckBox*		mSummary;
	CTextDisplay*	mText1;
	LCheckBox*		mUseBox2;
	CTextDisplay*	mText2;
	
	bool showing1;
	bool showing3;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	cdstring text4;
	
	CPreferenceValueMap<cdstring>*		mText1Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText2Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText3Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText4Item;			// Prefs field
	CPreferenceValueMap<bool>*			mCursorTopItem;		// Cursor top item
	CPreferenceValueMap<bool>*			mBox1Item;			// Box state
	CPreferenceValueMap<bool>*			mBox2Item;			// Box state
	CPreferenceValueMap<bool>*			mSummaryItem;		// Summary state
	short								mWrap;				// Current wrap length

public:
	enum { class_ID = 'Capo' };

					CPrefsEditCaption();
					CPrefsEditCaption(LStream *inStream);
	virtual 		~CPrefsEditCaption();

	static bool PoseDialog(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary,
							unsigned long spaces);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetData(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary);		// Set text in editor
	virtual void	GetEditorText(void);								// Get text from editor
	virtual void	SetCurrentSpacesPerTab(short num);					// Set current set spaces per tab

private:
			void	SetDisplay1(bool show1);
			void	UpdateDisplay1();
			void	SetDisplay2(bool show2);
			void	UpdateDisplay2();
};

#endif
