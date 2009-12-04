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


// Header for CPrefsAccountIMAP class

#ifndef __CPREFSACCOUNTIMAP__MULBERRY__
#define __CPREFSACCOUNTIMAP__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountIMAP = 5021;
const	PaneIDT		paneid_AccountIMAPLoginAtStart = 'LOGO';
const	PaneIDT		paneid_AccountIMAPDirSeparator = 'SEPR';
const	PaneIDT		paneid_AccountIMAPAutoNamespace = 'AUTO';
const	PaneIDT		paneid_AccountIMAPDisconnected = 'DISC';

// Mesages
const	MessageT	msg_AccountIMAPDisconnected = 'DISC';

// Resources
const	ResIDT		RidL_CPrefsAccountIMAPBtns = 5021;

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsAccountIMAP : public CPrefsTabSubPanel
{
private:
	LCheckBox*			mLogonAtStartup;
	CTextFieldX*		mDirectorySeparator;
	LCheckBox*			mAutoNamespace;
	LCheckBox*			mDisconnected;

public:
	enum { class_ID = 'Aimp' };

					CPrefsAccountIMAP();
					CPrefsAccountIMAP(LStream *inStream);
	virtual 		~CPrefsAccountIMAP();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
