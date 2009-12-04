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


// Header for CPrefsMessageGeneral class

#ifndef __CPREFSMESSAGEGENERAL__MULBERRY__
#define __CPREFSMESSAGEGENERAL__MULBERRY__

#include "CPrefsTabSubPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsMessageGeneral = 5075;
const	PaneIDT		paneid_SaveCreator = 'CREA';
const	PaneIDT		paneid_ChooseCreator = 'GETF';
const	PaneIDT		paneid_NoLimit = 'NOSZ';
const	PaneIDT		paneid_SizeLimit = 'LSIZ';
const	PaneIDT		paneid_WarnMessageSize = 'MAXK';
const	PaneIDT		paneid_OptionKeyDown = 'OPDW';
const	PaneIDT		paneid_OptionKeyUp = 'OPUP';
const	PaneIDT		paneid_ForwardChoice = 'FCHO';
const	PaneIDT		paneid_ForwardOptions = 'FOPT';
const	PaneIDT		paneid_ForwardQuoteOriginal = 'FQUT';
const	PaneIDT		paneid_ForwardHeaders = 'FHDR';
const	PaneIDT		paneid_ForwardAttachment = 'FATC';
const	PaneIDT		paneid_ForwardRFC822 = 'FRFC';

// Resources
const	ResIDT		RidL_CPrefsMessageGeneralBtns = 5075;

// Mesages
const	MessageT	msg_ChooseCreator = 5140;
const	MessageT	msg_NoLimit = 5141;
const	MessageT	msg_SizeLimit = 5142;
const	MessageT	msg_EditPrintCaption = 'PCAP';
const	MessageT	msg_ForwardChoice = 'FCHO';
const	MessageT	msg_ForwardOptions = 'FOPT';
const	MessageT	msg_ForwardQuoteOriginal = 'FQUT';
const	MessageT	msg_ForwardAttachment = 'FATC';

// Classes
class CPreferences;
class CTextFieldX;
class LCheckBox;
class LPushButton;
class LRadioButton;
class cdstring;
template<class T> class CPreferenceValueMap;

class	CPrefsMessageGeneral : public CPrefsTabSubPanel,
						public LListener {

private:
	CTextFieldX*		mSaveCreator;
	LPushButton*		mChooseCreator;
	LRadioButton*		mNoLimit;
	LRadioButton*		mSizeLimit;
	CTextFieldX*		mWarnMessageSize;
	LRadioButton*		mOptionKeyDown;
	LRadioButton*		mOptionKeyUp;
	LRadioButton*		mForwardChoice;
	LRadioButton*		mForwardOptions;
	LCheckBox*			mForwardQuoteOriginal;
	LCheckBox*			mForwardHeaders;
	LCheckBox*			mForwardAttachment;
	LCheckBox*			mForwardRFC822;
	
	CPreferences*		mCopyPrefs;

public:
	enum { class_ID = 'Mgen' };

					CPrefsMessageGeneral();
					CPrefsMessageGeneral(LStream *inStream);
	virtual 		~CPrefsMessageGeneral();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);					// Toggle display of IC - pure virtual
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

	virtual void	DoChooseCreator(void);					// Choose creator app
	virtual void	EditCaption(CPreferenceValueMap<cdstring>* text1,
								CPreferenceValueMap<cdstring>* text2,
								CPreferenceValueMap<cdstring>* text3,
								CPreferenceValueMap<cdstring>* text4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* box1,
								CPreferenceValueMap<bool>* box2,
								CPreferenceValueMap<bool>* summary);	// Edit a caption
};

#endif
