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


// Header for CPrefsAddressLDAP1 class

#ifndef __CPREFSADDRESSLDAP1__MULBERRY__
#define __CPREFSADDRESSLDAP1__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressLDAP1 = 5025;
const	PaneIDT		paneid_LDAPAccountLDAPRoot = 'ROOT';
const	PaneIDT		paneid_LDAPAccountLDAPName = 'LNAM';
const	PaneIDT		paneid_LDAPAccountLDAPEmail = 'LEMA';
const	PaneIDT		paneid_LDAPAccountLDAPNickName = 'LNIC';
const	PaneIDT		paneid_LDAPAccountLDAPAddress = 'LADR';
const	PaneIDT		paneid_LDAPAccountLDAPCompany = 'LCOM';

// Mesages

// Resources

// Classes
class CTextFieldX;

class	CPrefsAddressLDAP1 : public CPrefsTabSubPanel
{
private:
	CTextFieldX*		mLDAPRoot;
	CTextFieldX*		mLDAPName;
	CTextFieldX*		mLDAPEmail;
	CTextFieldX*		mLDAPNickName;
	CTextFieldX*		mLDAPAddress;
	CTextFieldX*		mLDAPCompany;

public:
	enum { class_ID = 'Ald1' };

					CPrefsAddressLDAP1();
					CPrefsAddressLDAP1(LStream *inStream);
	virtual 		~CPrefsAddressLDAP1();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
