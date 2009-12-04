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


// Header for CPrefsAccountExtras class

#ifndef __CPREFSACCOUNTEXTRAS__MULBERRY__
#define __CPREFSACCOUNTEXTRAS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountExtras = 5029;
const	PaneIDT		paneid_AccountExtrasTieIdentity = 'FIRS';
const	PaneIDT		paneid_AccountExtrasIdentityPopup = 'IDEN';

// Mesages
const	MessageT	msg_AccountExtrasTieIdentity = 'FIRS';
const	MessageT	msg_AccountExtrasIdentityPopup = 'IDEN';

// Resources
const	ResIDT		RidL_CPrefsAccountExtrasBtns = 5029;

// Classes
class CIdentityPopup;
class LCheckBox;

class	CPrefsAccountExtras : public CPrefsTabSubPanel, public LListener
{
private:
	LCheckBox*			mTieIdentity;
	CIdentityPopup*		mIdentityPopup;

public:
	enum { class_ID = 'Axtr' };

					CPrefsAccountExtras();
					CPrefsAccountExtras(LStream *inStream);
	virtual 		~CPrefsAccountExtras();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
