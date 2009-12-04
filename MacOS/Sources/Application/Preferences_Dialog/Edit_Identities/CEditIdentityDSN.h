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


// Header for CEditIdentityDSN class

#ifndef __CEDITIDENTITYDSN__MULBERRY__
#define __CEDITIDENTITYDSN__MULBERRY__

#include "CPrefsTabSubPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_EditIdentityDSN = 5112;
const	PaneIDT		paneid_EditIdentityDSNActive = 'ACTV';
const	PaneIDT		paneid_EditIdentityDSNGroup1 = 'GRP1';
const	PaneIDT		paneid_EditIdentityDSNUse = 'UDSN';
const	PaneIDT		paneid_EditIdentityDSNSuccess = 'SDSN';
const	PaneIDT		paneid_EditIdentityDSNFailure = 'FDSN';
const	PaneIDT		paneid_EditIdentityDSNDelay = 'DDSN';
const	PaneIDT		paneid_EditIdentityDSNFull = 'MDSN';
const	PaneIDT		paneid_EditIdentityDSNHeaders = 'HDSN';

// Mesages
const	MessageT	msg_EditIdentityDSNActive = 'ACTV';

// Resources
const	ResIDT		RidL_CEditIdentityDSNBtns = 5112;

// Classes
class LCheckBoxGroupBox;
class LCheckBox;
class LRadioButton;

class	CEditIdentityDSN : public CPrefsTabSubPanel
{
private:
	LCheckBoxGroupBox*	mActive;
	LCheckBox*			mUseDSN;
	LCheckBox*			mSuccess;
	LCheckBox*			mFailure;
	LCheckBox*			mDelay;
	LRadioButton*		mFull;
	LRadioButton*		mHeaders;

public:
	enum { class_ID = 'Ieds' };

					CEditIdentityDSN();
					CEditIdentityDSN(LStream *inStream);
	virtual 		~CEditIdentityDSN();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
