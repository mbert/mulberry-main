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


// Header for CPrefsAuthPlainText class

#ifndef __CPREFSAUTHPLAINTEXT__MULBERRY__
#define __CPREFSAUTHPLAINTEXT__MULBERRY__

#include "CPrefsAuthPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAuthPlainText = 5050;
const	PaneIDT		paneid_AuthPlainTextUID = 'IUID';
const	PaneIDT		paneid_AuthPlainTextSaveUser = 'SAVU';
const	PaneIDT		paneid_AuthPlainTextSavePswd = 'SAVP';

// Mesages

// Resources

// Classes
class CAuthenticator;
class CTextFieldX;
class LCheckBox;

class	CPrefsAuthPlainText : public CPrefsAuthPanel
{
private:
	CTextFieldX*	mUID;
	LCheckBox*		mSaveUser;
	LCheckBox*		mSavePswd;

public:
	enum { class_ID = 'Atxt' };

					CPrefsAuthPlainText();
					CPrefsAuthPlainText(LStream *inStream);
	virtual 		~CPrefsAuthPlainText();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ToggleICDisplay(bool IC_on);		// Toggle display of IC
	virtual void	SetAuth(CAuthenticator* auth);		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* auth);	// Force update of authenticator
	virtual void	UpdateItems(bool enable);					// Update item entry
};

#endif
