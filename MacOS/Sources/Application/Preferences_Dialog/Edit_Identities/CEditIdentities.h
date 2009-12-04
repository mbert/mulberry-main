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


// Header for CEditIdentities class

#ifndef __CEDITIDENTITIES__MULBERRY__
#define __CEDITIDENTITIES__MULBERRY__

#include <LDialogBox.h>

#include "CIdentity.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsEditIdentity = 5106;
const	PaneIDT		paneid_IdentityName = 'IDTY';
const	PaneIDT		paneid_IdentityInherit = 'INHE';
const	PaneIDT		paneid_IdentityServer = 'SERV';
const	PaneIDT		paneid_IdentityServerInherit = 'SINH';
const	PaneIDT		paneid_IdentityTabs = 'TABS';

// Mesages
const	MessageT	msg_ServerInherit = 'SINH';
const	MessageT	msg_IdentityTabs = 'TABS';

// Resources
const	ResIDT		RidL_CEditIdentitiesBtns = 5106;

// Classes

class CIdentity;
class CTextFieldX;
class CPrefsSubTab;
class CPreferences;
class LCheckBox;
class LPopupButton;

class	CEditIdentities : public LDialogBox
{

public:
	enum { class_ID = 'EdId' };

					CEditIdentities();
					CEditIdentities(LStream *inStream);
	virtual 		~CEditIdentities();


	virtual void	SetIdentity(const CPreferences* prefs, const CIdentity* identity);
	virtual void	GetIdentity(CIdentity* identity);

	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	InitServerPopup(const CPreferences* prefs);
	virtual void	InitInheritPopup(const CPreferences* prefs, const CIdentity* id);

private:
	CTextFieldX*		mName;
	LPopupButton*		mInherit;
	LPopupButton*		mServer;
	LCheckBox*			mServerInherit;
	CPrefsSubTab*		mTabs;
	CIdentity			mCopyIdentity;

	bool				mFromEnabled;
	bool				mReplyToEnabled;
	bool				mSenderEnabled;
	bool				mCalendarEnabled;
};

#endif
