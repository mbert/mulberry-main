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


// Header for CAddressContactPanel class

#ifndef __CADDRESSCONTACTPANEL__MULBERRY__
#define __CADDRESSCONTACTPANEL__MULBERRY__

#include "CAddressPanelBase.h"

// Constants
const	PaneIDT		paneid_AddressContact = 1736;
const	PaneIDT		paneid_AddressContactAddress = 'MADD';
const	PaneIDT		paneid_AddressContactPhoneWork = 'WORK';
const	PaneIDT		paneid_AddressContactPhoneHome = 'HOME';
const	PaneIDT		paneid_AddressContactFax = 'FAX ';
const	PaneIDT		paneid_AddressContactURL = 'URL ';

// Messages

// Classes
class CTextFieldX;
class CTextDisplay;

class CAddressContactPanel : public CAddressPanelBase
{
public:
	enum { class_ID = 'Acon' };

					CAddressContactPanel(LStream *inStream);
	virtual 		~CAddressContactPanel();

	virtual void	Focus();

protected:
	CTextDisplay*		mAddress;
	CTextFieldX*		mPhoneWork;
	CTextFieldX*		mPhoneHome;
	CTextFieldX*		mFax;
	CTextDisplay*		mURL;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
};

#endif
