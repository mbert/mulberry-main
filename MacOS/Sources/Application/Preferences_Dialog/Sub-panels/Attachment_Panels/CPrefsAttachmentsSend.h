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


// Header for CPrefsAttachmentsSend class

#ifndef __CPREFSATTACHMENTSSEND__MULBERRY__
#define __CPREFSATTACHMENTSSEND__MULBERRY__

#include "CPrefsTabSubPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAttachmentsSend = 5095;
const	PaneIDT		paneid_EncodingMenu = 'ENCD';
const	PaneIDT		paneid_EncodingAlways = 'ALWY';
const	PaneIDT		paneid_EncodingWhenNeeded = 'ONLY';
const	PaneIDT		paneid_CheckDefaultMailClient = 'MLTO';
const	PaneIDT		paneid_WarnMailtoFiles = 'WMFA';
const	PaneIDT		paneid_CheckDefaultWebcalClient = 'WEBC';
const	PaneIDT		paneid_WarnMissingAttachments = 'WARN';
const	PaneIDT		paneid_MissingAttachmentSubject = 'SUBJ';
const	PaneIDT		paneid_MissingAttachmentWords = 'WORD';

// Mesages

// Resources
enum
{
	menu_PrefsAttachmentsAS = 1,
	menu_PrefsAttachmentsAD,
	menu_PrefsAttachmentsBinHex,
	menu_PrefsAttachmentsUU
};

// Classes
class CTextDisplay;
class LCheckBox;
class LCheckBoxGroupBox;
class LRadioButton;
class LPopupButton;

class	CPrefsAttachmentsSend : public CPrefsTabSubPanel
{
private:
	LPopupButton*		mDefaultEncoding;
	LRadioButton*		mEncodingAlways;
	LRadioButton*		mEncodingWhenNeeded;
	LCheckBox*			mCheckDefaultMailClient;
	LCheckBox*			mWarnMailtoFiles;
	LCheckBox*			mCheckDefaultWebcalClient;
	LCheckBoxGroupBox*	mWarnMissingAttachments;
	LCheckBox*			mMissingAttachmentSubject;
	CTextDisplay*		mMissingAttachmentWords;

public:
	enum { class_ID = 'Asen' };

					CPrefsAttachmentsSend();
					CPrefsAttachmentsSend(LStream *inStream);
	virtual 		~CPrefsAttachmentsSend();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
