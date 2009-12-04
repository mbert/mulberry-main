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


// Header for CPrefsAuthKerberos class

#ifndef __CPREFSAUTHKERBEROS__MULBERRY__
#define __CPREFSAUTHKERBEROS__MULBERRY__

#include "CPrefsAuthPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAuthKerberos = 5051;
const	PaneIDT		paneid_AuthKerberosDefaultPrincipal = 'DSRV';
const	PaneIDT		paneid_AuthKerberosPrincipal = 'RELM';
const	PaneIDT		paneid_AuthKerberosDefaultUID = 'DUID';
const	PaneIDT		paneid_AuthKerberosUID = 'SUID';

// Mesages
const	MessageT	msg_AuthKerberosDefaultPrincipal = 'DSRV';
const	MessageT	msg_AuthKerberosDefaultUID = 'DUID';

// Resources
const	ResIDT		RidL_CPrefsAuthKerberosBtns = 5051;


// Classes
class CAuthenticator;
class CTextFieldX;
class LCheckBox;

class	CPrefsAuthKerberos : public CPrefsAuthPanel,
								public LListener
{
private:
	LCheckBox*			mDefaultPrincipal;
	CTextFieldX*		mPrincipal;
	LCheckBox*			mDefaultUID;
	CTextFieldX*		mUID;

public:
	enum { class_ID = 'Akrb' };

					CPrefsAuthKerberos();
					CPrefsAuthKerberos(LStream *inStream);
	virtual 		~CPrefsAuthKerberos();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);		// Toggle display of IC
	virtual void	SetAuth(CAuthenticator* auth);		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* auth);	// Force update of authenticator
	virtual void	UpdateItems(bool enable);					// Update item entry
};

#endif
