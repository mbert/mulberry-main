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


// Header for CPrefsAddressIMSP class

#ifndef __CPREFSADDRESSIMSP__MULBERRY__
#define __CPREFSADDRESSIMSP__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressIMSP = 5024;
const	PaneIDT		paneid_AddressIMSPLoginAtStart = 'LOGO';
const	PaneIDT		paneid_AddressIMSPDisconnected = 'DISC';
const	PaneIDT		paneid_AddressIMSPBaseRURLText = 'RTXT';
const	PaneIDT		paneid_AddressIMSPBaseRURL = 'RURL';

// Mesages
const	MessageT	msg_AddressIMSPDisconnected = 'DISC';

// Resources
const	ResIDT		RidL_CPrefsAddressIMSPBtns = 5024;

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsAddressIMSP : public CPrefsTabSubPanel
{
private:
	LCheckBox*		mLogonAtStartup;
	LCheckBox*		mDisconnected;
	CTextFieldX*	mBaseRURL;

public:
	enum { class_ID = 'Aisa' };

					CPrefsAddressIMSP();
					CPrefsAddressIMSP(LStream *inStream);
	virtual 		~CPrefsAddressIMSP();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
