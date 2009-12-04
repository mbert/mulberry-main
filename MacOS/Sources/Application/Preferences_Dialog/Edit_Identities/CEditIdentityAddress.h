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


// Header for CEditIdentityAddress class

#ifndef __CEDITIDENTITYADDRESS__MULBERRY__
#define __CEDITIDENTITYADDRESS__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include <LListener.h>

// Constants

// Panes
const	PaneIDT		paneid_EditIdentityAddress = 5107;
const	PaneIDT		paneid_EditIdentityAddressOutgoing = 5110;
const	PaneIDT		paneid_EditIdentityActive = 'ACTV';
const	PaneIDT		paneid_EditIdentityGroup1 = 'GRP1';
const	PaneIDT		paneid_EditIdentitySingle = 'SING';
const	PaneIDT		paneid_EditIdentityMultiple = 'MULT';
const	PaneIDT		paneid_EditIdentityPanel1 = 'PAN1';
const	PaneIDT		paneid_EditIdentityName = 'NAME';
const	PaneIDT		paneid_EditIdentityEmail = 'EMAL';
const	PaneIDT		paneid_EditIdentityFocus = 'MFOC';
const	PaneIDT		paneid_EditIdentityScroller = 'MSCR';
const	PaneIDT		paneid_EditIdentityText = 'MTXT';

// Mesages
const	MessageT	msg_EditIdentityActive = 'ACTV';
const	MessageT	msg_EditIdentityUseSingle = 'SING';
const	MessageT	msg_EditIdentityUseFull = 'MULT';

// Resources
const	ResIDT		RidL_CEditIdentityAddressBtns = 5107;
const	ResIDT		RidL_CEditIdentityAddressOutgoingBtns = 5110;

// Classes
class cdstring;
class CTextDisplay;
class CTextFieldX;
class LCheckBoxGroupBox;
class LRadioButton;

class CEditIdentityAddress : public CPrefsTabSubPanel,
								public LListener
{
private:
	LCheckBoxGroupBox*	mActive;
	LRadioButton*		mSingle;
	LRadioButton*		mMultiple;
	LView*				mPanel;
	CTextFieldX*		mName;
	CTextFieldX*		mEmail;
	LView*				mFocus;
	CTextDisplay*		mText;

	bool				mFrom;
	bool				mReplyTo;
	bool				mSender;
	bool				mTo;
	bool				mCC;
	bool				mBcc;
	bool				mCalendar;

public:
	enum { class_ID = 'Iead' };

					CEditIdentityAddress();
					CEditIdentityAddress(LStream *inStream);
	virtual 		~CEditIdentityAddress();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetAddressType(bool from, bool reply_to, bool sender, bool to, bool cc, bool bcc, bool calendar);
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);	// Toggle display of IC
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

private:
	virtual void	SetSingle(bool single);
	virtual void	SetItemData(bool active, const cdstring& address);
	virtual void	GetItemData(bool& active, cdstring& address);
};

#endif
