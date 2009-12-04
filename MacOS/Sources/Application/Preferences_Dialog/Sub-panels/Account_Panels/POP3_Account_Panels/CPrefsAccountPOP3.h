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


// Header for CPrefsAccountPOP3 class

#ifndef __CPREFSACCOUNTPOP3__MULBERRY__
#define __CPREFSACCOUNTPOP3__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountPOP3 = 5028;
const	PaneIDT		paneid_AccountPOP3LeaveOnServer = 'LEAV';
const	PaneIDT		paneid_AccountPOP3DeleteAfter = 'AFTE';
const	PaneIDT		paneid_AccountPOP3DeleteDays = 'DAYS';
const	PaneIDT		paneid_AccountPOP3DeleteExpunged = 'EXPU';
const	PaneIDT		paneid_AccountPOP3UseMaxSize = 'LARG';
const	PaneIDT		paneid_AccountPOP3MaxSize = 'SIZE';
const	PaneIDT		paneid_AccountPOP3UseAPOP = 'APOP';

// Mesages
const	MessageT	msg_AccountPOP3LeaveOnServer = 'LEAV';
const	MessageT	msg_AccountPOP3DeleteAfter = 'AFTE';
const	MessageT	msg_AccountPOP3UseMaxSize = 'LARG';

// Resources
const	ResIDT		RidL_CPrefsAccountPOP3Btns = 5028;

// Classes
class CTextFieldX;
class LCheckBoxGroupBox;
class LCheckBox;

class	CPrefsAccountPOP3 : public CPrefsTabSubPanel, public LListener
{
private:
	LCheckBoxGroupBox*	mLeaveOnServer;
	LCheckBox*			mDeleteAfter;
	CTextFieldX*		mDeleteDays;
	LCheckBox*			mDeleteExpunged;
	LCheckBox*			mUseMaxSize;
	CTextFieldX*		mMaxSize;
	LCheckBox*			mUseAPOP;

public:
	enum { class_ID = 'AlP3' };

					CPrefsAccountPOP3();
					CPrefsAccountPOP3(LStream *inStream);
	virtual 		~CPrefsAccountPOP3();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data

private:
	virtual void	SetLeaveOnServer(bool on);
};

#endif
