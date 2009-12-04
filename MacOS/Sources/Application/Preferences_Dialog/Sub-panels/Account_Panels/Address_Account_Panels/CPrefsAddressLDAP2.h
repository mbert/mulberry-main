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


// Header for CPrefsAddressLDAP2 class

#ifndef __CPREFSADDRESSLDAP2__MULBERRY__
#define __CPREFSADDRESSLDAP2__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressLDAP2 = 5026;
const	PaneIDT		paneid_LDAPAccountLDAPPhoneWork = 'LPHW';
const	PaneIDT		paneid_LDAPAccountLDAPPhoneHome = 'LPHH';
const	PaneIDT		paneid_LDAPAccountLDAPFax = 'LFAX';
const	PaneIDT		paneid_LDAPAccountLDAPURLs = 'LURL';
const	PaneIDT		paneid_LDAPAccountLDAPNotes = 'LNOT';
const	PaneIDT		paneid_LDAPAccountLDAPPut = 'LPUT';

// Mesages
const	MessageT	msg_LDAPAccountLDAPPut = 'LPUT';

// Resources
const	ResIDT		RidL_CPrefsAddressLDAP2Btns = 5026;

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsAddressLDAP2 : public CPrefsTabSubPanel,
							public LListener
{
private:
	CTextFieldX*		mLDAPPhoneWork;
	CTextFieldX*		mLDAPPhoneHome;
	CTextFieldX*		mLDAPFax;
	CTextFieldX*		mLDAPURLs;
	CTextFieldX*		mLDAPNotes;
	LCheckBox*			mLDAPPut;

public:
	enum { class_ID = 'Ald2' };

					CPrefsAddressLDAP2();
					CPrefsAddressLDAP2(LStream *inStream);
	virtual 		~CPrefsAddressLDAP2();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
