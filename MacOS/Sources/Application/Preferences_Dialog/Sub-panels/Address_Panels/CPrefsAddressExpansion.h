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


// Header for CPrefsAddressExpansion class

#ifndef __CPREFSADDRESSEXPANSION__MULBERRY__
#define __CPREFSADDRESSEXPANSION__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressExpansion = 5090;
const 	PaneIDT 	paneid_AEExpandNoNicknames = 'EALL';
const 	PaneIDT 	paneid_AEExpandFailedNicknames = 'FAIL';
const 	PaneIDT 	paneid_AEExpandFullName = 'NAME';
const 	PaneIDT 	paneid_AEExpandNickName = 'NICK';
const 	PaneIDT 	paneid_AEExpandEmail = 'MAIL';
const 	PaneIDT 	paneid_AESkipLDAP = 'SKIP';

// Mesages
const	MessageT	msg_AEExpandNoNicknames = 'EALL';

// Resources
const	ResIDT		RidL_CPrefsAddressExpansionBtns = 5090;

// Classes
class LCheckBox;

class CPrefsAddressExpansion : public CPrefsTabSubPanel,
									public LListener
{
private:
	LCheckBox*	mExpandNoNicknames;
	LCheckBox*	mExpandFailedNicknames;
	LCheckBox*	mExpandFullName;
	LCheckBox*	mExpandNickName;
	LCheckBox*	mExpandEmail;
	LCheckBox*	mSkipLDAP;

public:
	enum { class_ID = 'Aexp' };

					CPrefsAddressExpansion();
					CPrefsAddressExpansion(LStream *inStream);
	virtual 		~CPrefsAddressExpansion();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
