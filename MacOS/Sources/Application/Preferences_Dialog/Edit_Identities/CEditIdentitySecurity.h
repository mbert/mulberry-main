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


// Header for CEditIdentitySecurity class

#ifndef __CEDITIDENTITYSECURITY__MULBERRY__
#define __CEDITIDENTITYSECURITY__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "cdstring.h"

#include <LListener.h>

// Constants

// Panes
const	PaneIDT		paneid_EditIdentitySecurity = 5111;
const	PaneIDT		paneid_EditIdentitySecurityActive = 'ACTI';
const	PaneIDT		paneid_EditIdentitySecurityGroup1 = 'GRP1';
const	PaneIDT		paneid_EditIdentitySign = 'SIGN';
const	PaneIDT		paneid_EditIdentityEncrypt = 'ENCR';
const	PaneIDT		paneid_EditIdentitySignWith = 'WITH';
const	PaneIDT		paneid_EditIdentitySignOther = 'OTHE';

// Mesages
const	MessageT	msg_EditIdentitySecurityActive = 'ACTI';
const	MessageT	msg_EditIdentitySignWith = 'WITH';

// Resources
const	ResIDT		RidL_CEditIdentitySecurityBtns = 5111;

// Classes
class LCheckBox;
class LCheckBoxGroupBox;
class LPopupButton;
class CTextFieldX;

class	CEditIdentitySecurity : public CPrefsTabSubPanel,
								public LListener
{
private:
	LCheckBoxGroupBox*	mActive;
	LCheckBox*			mSign;
	LCheckBox*			mEncrypt;
	LPopupButton*		mSignWithPopup;
	CTextFieldX*			mSignOther;

public:
	enum { class_ID = 'Iese' };

					CEditIdentitySecurity();
					CEditIdentitySecurity(LStream *inStream);
	virtual 		~CEditIdentitySecurity();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
