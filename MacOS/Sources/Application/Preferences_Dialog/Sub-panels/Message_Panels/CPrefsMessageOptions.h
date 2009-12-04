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


// Header for CPrefsMessageOptions class

#ifndef __CPREFSMESSAGEOPTIONS__MULBERRY__
#define __CPREFSMESSAGEOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsMessageOptions = 5076;
const	PaneIDT		paneid_ShowHeader = 'SHOW';
const	PaneIDT		paneid_SaveHeader = 'SAVE';
const	PaneIDT		paneid_DeleteAfterCopy = 'CDEL';
const	PaneIDT		paneid_OpenDeleted = 'DELM';
const	PaneIDT		paneid_CloseDeleted = 'DELC';
const	PaneIDT		paneid_OpenReuse = 'RUSE';
const	PaneIDT		paneid_QuoteSelection = 'QSEL';
const	PaneIDT		paneid_AlwaysQuote = 'QUOT';
const	PaneIDT		paneid_AutoDigest = 'DIGS';
const	PaneIDT		paneid_ExpandHeader = 'EXPH';
const	PaneIDT		paneid_ExpandParts = 'EXPA';
const 	PaneIDT		paneid_ShowStyled = 'STYL';
const 	PaneIDT		paneid_MDNAlwaysSend = 'MDNA';
const 	PaneIDT		paneid_MDNNeverSend = 'MDNN';
const 	PaneIDT		paneid_MDNPromptSend = 'MDNP';

// Resources
const	ResIDT		RidL_CPrefsMessageOptionsBtns = 5076;

// Mesages

// Classes
class LCheckBox;
class LRadioButton;

class	CPrefsMessageOptions : public CPrefsTabSubPanel
{
private:
	LCheckBox*		mShowHeader;
	LCheckBox*		mSaveHeader;
	LCheckBox*		mDeleteAfterCopy;
	LCheckBox*		mOpenDeleted;
	LCheckBox*		mCloseDeleted;
	LCheckBox*		mOpenReuse;
	LCheckBox*		mQuoteSelection;
	LCheckBox*		mAlwaysQuote;
	LCheckBox*		mAutoDigest;
	LCheckBox*		mExpandHeader;
	LCheckBox*		mExpandParts;
	LCheckBox*		mShowStyled;
	LRadioButton*	mMDNAlwaysSend;
	LRadioButton*	mMDNNeverSend;
	LRadioButton*	mMDNPromptSend;

public:
	enum { class_ID = 'Mopt' };

					CPrefsMessageOptions();
					CPrefsMessageOptions(LStream *inStream);
	virtual 		~CPrefsMessageOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
