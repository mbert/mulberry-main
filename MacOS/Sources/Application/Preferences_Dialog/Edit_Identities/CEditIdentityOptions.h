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


// Header for CEditIdentityOptions class

#ifndef __CEDITIDENTITYOPTIONS__MULBERRY__
#define __CEDITIDENTITYOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "cdstring.h"

#include <LListener.h>

// Constants

// Panes
const	PaneIDT		paneid_EditIdentityOptions = 5108;
const	PaneIDT		paneid_EditIdentityUseCopyTo = 'CBT1';
const	PaneIDT		paneid_EditIdentityCopyToGroup = 'GRP1';
const	PaneIDT		paneid_EditIdentityCopyToNone = 'CBT2';
const	PaneIDT		paneid_EditIdentityCopyToChoose = 'CBT3';
const	PaneIDT		paneid_EditIdentityCopyTo = 'COPY';
const	PaneIDT		paneid_EditIdentityCopyToPopup = 'COPM';
const	PaneIDT		paneid_EditIdentityCopyToActive = 'COPA';
const	PaneIDT		paneid_EditIdentityCopyReplied = 'ALSO';
const	PaneIDT		paneid_EditIdentityHeader = 'HEAD';
const	PaneIDT		paneid_EditIdentityHeaderActive = 'HEAA';
const	PaneIDT		paneid_EditIdentityHeaderGroup = 'GRP2';
const	PaneIDT		paneid_EditIdentityFooter = 'FOOT';
const	PaneIDT		paneid_EditIdentityFooterActive = 'FOOA';
const	PaneIDT		paneid_EditIdentityFooterGroup = 'GRP3';

// Mesages
const	MessageT	msg_IdentityHeader = 5150;
const	MessageT	msg_IdentityFooter = 5151;
const	MessageT	msg_IdentityServerDefault = 'SERA';
const	MessageT	msg_IdentityUseCopyTo = 'CBT1';
const	MessageT	msg_IdentityCopyToNone = 'CBT2';
const	MessageT	msg_IdentityCopyToChoose = 'CBT3';
const	MessageT	msg_IdentityCopyToPopup = 'COPM';
const	MessageT	msg_IdentityCopyToActive = 'COPA';
const	MessageT	msg_IdentityHeaderActive = 'HEAA';
const	MessageT	msg_IdentityFooterActive = 'FOOA';

// Resources
const	ResIDT		RidL_CEditIdentityOptionsBtns = 5108;

// Classes
class LCheckBoxGroupBox;
class LCheckBox;
class CMailboxPopup;
class CTextFieldX;
class LRadioButton;
class LBevelButton;

class	CEditIdentityOptions : public CPrefsTabSubPanel,
								public LListener
{
private:
	LCheckBoxGroupBox*	mCopyToActive;
	LRadioButton*		mUseCopyTo;
	LRadioButton*		mCopyToNone;
	LRadioButton*		mCopyToChoose;
	CTextFieldX*		mCopyTo;
	CMailboxPopup*		mCopyToPopup;
	LCheckBox*			mCopyReplied;
	LCheckBoxGroupBox*	mHeaderActive;
	LBevelButton*		mHeaderBtn;
	LCheckBoxGroupBox*	mFooterActive;
	LBevelButton*		mFooterBtn;
	cdstring			mHeader;
	cdstring			mFooter;

public:
	enum { class_ID = 'Ieop' };

					CEditIdentityOptions();
					CEditIdentityOptions(LStream *inStream);
	virtual 		~CEditIdentityOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons

	virtual void	ToggleICDisplay(bool IC_on);	// Toggle display of IC
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

private:
	virtual void	EditHeadFoot(cdstring* text,
									bool header);		// Edit header or footer
};

#endif
