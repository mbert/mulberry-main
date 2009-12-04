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


// Header for CPrefsAddressLDAPOptions class

#ifndef __CPREFSADDRESSLDAPOPTIONS__MULBERRY__
#define __CPREFSADDRESSLDAPOPTIONS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressLDAPOptions = 5038;
const	PaneIDT		paneid_LDAPAccountLDAPUseExpansion = 'LEXP';
const	PaneIDT		paneid_LDAPAccountLDAPSearch = 'LSRC';

// Mesages

// Resources
const	ResIDT		RidL_CPrefsAddressLDAPOptionsBtns = 5038;

// Classes
class LCheckBox;

class	CPrefsAddressLDAPOptions : public CPrefsTabSubPanel
{
private:
	LCheckBox*			mLDAPUseExpansion;
	LCheckBox*			mLDAPSearch;

public:
	enum { class_ID = 'Aldo' };

					CPrefsAddressLDAPOptions();
					CPrefsAddressLDAPOptions(LStream *inStream);
	virtual 		~CPrefsAddressLDAPOptions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
