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


// Header for CPrefsAttachmentsSave class

#ifndef __CPREFSATTACHMENTSSAVE__MULBERRY__
#define __CPREFSATTACHMENTSSAVE__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "CMIMEMap.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAttachmentsSave = 5096;
const	PaneIDT		paneid_ViewDoubleClick = 'VIEW';
const	PaneIDT		paneid_SaveDoubleClick = 'SAVE';
const	PaneIDT		paneid_AskUserSave = 'ASKS';
const	PaneIDT		paneid_DefaultFolderSave = 'USED';
const	PaneIDT		paneid_DefaultFolder = 'DEFF';
const	PaneIDT		paneid_ChooseDefaultFolder = 'CHDF';
const	PaneIDT		paneid_AlwaysOpen = 'ALWO';
const	PaneIDT		paneid_AskOpen = 'ASKO';
const	PaneIDT		paneid_NeverOpen = 'NEVO';
const	PaneIDT		paneid_IncludeText = 'TEXT';
const	PaneIDT		paneid_EditMappingsBtn = 'EMAP';

// Mesages
const	MessageT	msg_ChooseDefaultFolder = 5170;
const	MessageT	msg_EditMappings = 5171;
const	MessageT	msg_AskUserSave = 5172;
const	MessageT	msg_DefaultFolderSave = 5173;

// Resources
const	ResIDT		RidL_CPrefsAttachmentsSaveBtns = 5096;

// Classes
class CTextFieldX;
class LRadioButton;
class LPushButton;

class	CPrefsAttachmentsSave : public CPrefsTabSubPanel,
						public LListener {

private:
	LRadioButton*		mViewDoubleClick;
	LRadioButton*		mSaveDoubleClick;
	LRadioButton*		mAskUserSave;
	LRadioButton*		mDefaultFolderSave;
	CTextFieldX*		mDefaultFolder;
	LPushButton*		mChooseDefaultFolder;
	LRadioButton*		mAlwaysOpen;
	LRadioButton*		mAskOpen;
	LRadioButton*		mNeverOpen;
	LRadioButton*		mIncludeText;
	CMIMEMapVector	 	mMappings;
	bool				mMapChange;

public:
	enum { class_ID = 'Asav' };

					CPrefsAttachmentsSave();
					CPrefsAttachmentsSave(LStream *inStream);
	virtual 		~CPrefsAttachmentsSave();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

private:
	virtual void	DoChooseDefaultFolder(void);				// Choose default folder using browser
	virtual void	EditMappings(void);						// Edit mappings

};

#endif
