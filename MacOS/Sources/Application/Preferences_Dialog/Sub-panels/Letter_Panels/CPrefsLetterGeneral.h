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


// Header for CPrefsLetterGeneral class

#ifndef __CPREFSLETTERGENERAL__MULBERRY__
#define __CPREFSLETTERGENERAL__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsLetterGeneral = 5070;
const	PaneIDT		paneid_LGMailDomain = 'DOMA';
const	PaneIDT		paneid_LGSpacesPerTab = 'STAB';
const	PaneIDT		paneid_LGTabSpace = 'TABS';
const	PaneIDT		paneid_LGIncludeChar = 'INCL';
const	PaneIDT		paneid_LGForwardQuoteChar = 'FRWD';
const	PaneIDT		paneid_LGForwardSubject = 'FSUB';
const	PaneIDT		paneid_LGWrapLength = 'WRAP';
const	PaneIDT		paneid_LGSeparateBCC = 'BCCC';
const	PaneIDT		paneid_LGBCCCaption = 'BCAP';
const	PaneIDT		paneid_LGSaveToFile = 'SFIL';
const	PaneIDT		paneid_LGSaveToMailbox = 'SMBX';
const	PaneIDT		paneid_LGSaveChoose = 'SCHO';
const	PaneIDT		paneid_LGSaveToMailboxName = 'SAVN';
const	PaneIDT		paneid_LGSaveToMailboxPopup = 'SAVM';
const	PaneIDT		paneid_LGAutoSaveDrafts = 'AUTS';
const	PaneIDT		paneid_LGAutoSaveDraftsInterval = 'SECS';

// Mesages
const	MessageT	msg_LGEditReplySet = 'RCP1';
const	MessageT	msg_LGEditForwardSet = 'JCP1';
const	MessageT	msg_LGEditLtrPrintCaption = 'PCAP';
const	MessageT	msg_LGSeparateBCC = 'BCCC';
const	MessageT	msg_LGEditBCCCaption = 'BCAP';
const	MessageT	msg_LGSaveToFile = 'SFIL';
const	MessageT	msg_LGSaveToMailbox = 'SMBX';
const	MessageT	msg_LGSaveChoose = 'SCHO';
const	MessageT	msg_LGSaveToMailboxPopup = 'SAVM';
const	MessageT	msg_LGAutoSaveDrafts = 'AUTS';

// Resources
const	ResIDT		RidL_CPrefsLetterGeneralBtns = 5070;

// Classes
class CMailboxPopup;
class CPreferences;
class CTextFieldX;
template<class T> class CPreferenceValueMap;
class LCheckBox;
class LPushButton;
class LRadioButton;

class	CPrefsLetterGeneral : public CPrefsTabSubPanel,
									public LListener
{
private:
	CTextFieldX*		mMailDomain;
	CTextFieldX*		mSpacesPerTab;
	LCheckBox*			mTabSpace;
	CTextFieldX*		mReplyQuoteChar;
	CTextFieldX*		mForwardQuoteChar;
	CTextFieldX*		mForwardSubject;
	CTextFieldX*		mWrapLength;
	LCheckBox*			mSeparateBCC;
	LPushButton*		mBCCCaption;
	LRadioButton*		mSaveToFile;
	LRadioButton*		mSaveToMailbox;
	LRadioButton*		mSaveChoose;
	CTextFieldX*		mSaveToMailboxName;
	CMailboxPopup*		mSaveToMailboxPopup;
	LCheckBox*			mAutoSaveDrafts;
	CTextFieldX*		mAutoSaveDraftsInterval;

public:
	enum { class_ID = 'Lgen' };

					CPrefsLetterGeneral();
					CPrefsLetterGeneral(LStream *inStream);
	virtual 		~CPrefsLetterGeneral();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ToggleICDisplay(bool IC_on);				// Toggle display of IC
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

protected:
	CPreferences*	mCopyPrefs;

			void	EditCaption(CPreferenceValueMap<cdstring>* text1,
								CPreferenceValueMap<cdstring>* text2,
								CPreferenceValueMap<cdstring>* text3,
								CPreferenceValueMap<cdstring>* text4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* box1,
								CPreferenceValueMap<bool>* box2,
								CPreferenceValueMap<bool>* summary);	// Edit a caption
			void	EditBCCCaption();
};

#endif
