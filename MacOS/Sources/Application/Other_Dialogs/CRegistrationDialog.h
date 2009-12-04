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


// Header for CRegistrationDialog class

#ifndef __CREGISTRATIONDIALOG__MULBERRY__
#define __CREGISTRATIONDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_RegistrationV3Dialog = 5302;
const	PaneIDT		paneid_RegOKBtn = 'OKBT';
const	PaneIDT		paneid_DemoBtn = 'DEMO';
const	PaneIDT		paneid_RegLicenseeName = 'LNAM';
const	PaneIDT		paneid_RegSerialNumber = 'SNUM';
const	PaneIDT		paneid_RegOrganisation = 'ORGS';
const	PaneIDT		paneid_RegKey1 = 'RKY1';
const	PaneIDT		paneid_RegKey2 = 'RKY2';
const	PaneIDT		paneid_RegKey3 = 'RKY3';
const	PaneIDT		paneid_RegKey4 = 'RKY4';

const	PaneIDT		paneid_DemoLicenceDialog = 5301;

// Resources
const	ResIDT		STRx_RegisterDialogHelp = 5300;

// Messages
const	MessageT	msg_RunDemo = 5300;

// Classes

class	CTextFieldX;
class	LPushButton;
class	CHelpAttach;

class	CRegistrationDialog : public LDialogBox {

protected:
	LPushButton*	mOKBtn;
	LPushButton*	mDemoBtn;
	CTextFieldX*	mLicenseeName;
	CTextFieldX*	mSerialNumber;
	CTextFieldX*	mOrganisation;
	CTextFieldX*	mRegKey1;
	CTextFieldX*	mRegKey2;
	CTextFieldX*	mRegKey3;
	CTextFieldX*	mRegKey4;

public:
	enum { class_ID = 'Regi' };

					CRegistrationDialog();
					CRegistrationDialog(LStream *inStream);
	virtual 		~CRegistrationDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Check for valid OK

	static bool	DoRegistration(bool initial);				// Try to register
};

#endif
