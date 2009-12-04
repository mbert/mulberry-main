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

// Classes
class CTextInputField;
class JXTextCheckbox;

class CPrefsAuthPlainText : public CPrefsAuthPanel
{
public:
	CPrefsAuthPlainText(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CPrefsAuthPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void OnCreate();

	virtual void	SetAuth(CAuthenticator* auth);		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* auth);	// Force update of authenticator
	virtual void	UpdateItems(bool enable);					// Update item entry

protected:
// begin JXLayout1

    CTextInputField* mUID;
    JXTextCheckbox*  mSaveUser;
    JXTextCheckbox*  mSavePswd;

// end JXLayout1
};

#endif
