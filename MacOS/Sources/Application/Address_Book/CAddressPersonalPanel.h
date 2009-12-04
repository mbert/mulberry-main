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


// Header for CAddressPersonalPanel class

#ifndef __CADDRESSPERSONALPANEL__MULBERRY__
#define __CADDRESSPERSONALPANEL__MULBERRY__

#include "CAddressPanelBase.h"

// Constants
const	PaneIDT		paneid_AddressPersonal = 1735;
const	PaneIDT		paneid_AddressPersonalNickName = 'NICK';
const	PaneIDT		paneid_AddressPersonalFullName = 'FULL';
const	PaneIDT		paneid_AddressPersonalEmail = 'ADDR';
const	PaneIDT		paneid_AddressPersonalCalendar = 'CALA';
const	PaneIDT		paneid_AddressPersonalCompany = 'COMP';

// Messages

// Classes
class CTextFieldX;

class CAddressPersonalPanel : public CAddressPanelBase
{
public:
	enum { class_ID = 'Aper' };

					CAddressPersonalPanel(LStream *inStream);
	virtual 		~CAddressPersonalPanel();

	virtual void	Focus();

protected:
	CTextFieldX*		mFullName;
	CTextFieldX*		mNickName;
	CTextFieldX*		mEmail;
	CTextFieldX*		mCalendar;
	CTextFieldX*		mCompany;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	SetFields(const CAdbkAddress* addr);	// Set fields in dialog
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
};

#endif
