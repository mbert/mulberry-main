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


// Header for CPrefsLetterOptions class

#ifndef __CPREFSLETTEROPTIONS__MULBERRY__
#define __CPREFSLETTEROPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsLetterOptions = 5072;
const	PaneIDT		paneid_LOAutoInsertSigBtn = 'ASIG';
const	PaneIDT		paneid_LOSmartURLPaste = 'SURL';
const	PaneIDT		paneid_LOWindowWrap = 'WIND';
const	PaneIDT		paneid_LOSigEmptyLineBtn = 'BLNK';
const	PaneIDT		paneid_LOSigDashesBtn = 'SIGD';
const	PaneIDT		paneid_LONoSubjectWarnBtn = 'SUBJ';
const	PaneIDT		paneid_LODisplayAttachments = 'ATTC';
const	PaneIDT		paneid_LOAppendDraft = 'DRFT';
const	PaneIDT		paneid_LOInboxAppend = 'APPR';
const	PaneIDT		paneid_LOGenerateSender = 'SNDR';
const	PaneIDT		paneid_LODeleteOriginalDraft = 'DELO';
const	PaneIDT		paneid_LOTemplateDrafts = 'IMPT';
const	PaneIDT		paneid_LOReplyNoSignature = 'RNOS';
const	PaneIDT		paneid_LOWarnReplySubject = 'RSUB';
const	PaneIDT		paneid_LOShowCC = 'ShwC';
const	PaneIDT		paneid_LOShowBCC = 'ShwB';
const	PaneIDT		paneid_LOAlwaysUnicode = 'UTF8';
const	PaneIDT		paneid_LODisplayIdentityFrom = 'IFRM';

// Mesages
const	MessageT	msg_LODeleteOriginalDraft = 'DELO';

// Resources
const	ResIDT		RidL_CPrefsLetterOptionsBtns = 5072;

// Classes
class CTextFieldX;
class LCheckBox;
class CPreferences;

class CPrefsLetterOptions : public CPrefsTabSubPanel,
									public LListener
{
private:
	LCheckBox*		mAutoInsertSigBtn;
	LCheckBox*		mSigEmptyLineBtn;
	LCheckBox*		mSigDashesBtn;
	LCheckBox*		mNoSubjectWarnBtn;
	LCheckBox*		mDisplayAttachments;
	LCheckBox*		mAppendDraft;
	LCheckBox*		mInboxAppend;
	LCheckBox*		mSmartURLPaste;
	LCheckBox*		mComposeWrap;
	LCheckBox*		mDeleteOriginalDraft;
	LCheckBox*		mTemplateDrafts;
	LCheckBox*		mReplyNoSignature;
	LCheckBox*		mWarnReplySubject;
	LCheckBox*		mShowCC;
	LCheckBox*		mShowBCC;
	LCheckBox*		mAlwaysUnicode;
	LCheckBox*		mDisplayIdentityFrom;

public:
	enum { class_ID = 'Lopt' };

					CPrefsLetterOptions();
					CPrefsLetterOptions(LStream *inStream);
	virtual 		~CPrefsLetterOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

protected:
	CPreferences*	mCopyPrefs;

};

#endif
