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


// Header for CPrefsAuthAnonymous class

#ifndef __CPREFSAUTHANONYMOUS__MULBERRY__
#define __CPREFSAUTHANONYMOUS__MULBERRY__

#include "CPrefsAuthPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAuthAnonymous = 5052;

// Classes
class CAuthenticator;

class	CPrefsAuthAnonymous : public CPrefsAuthPanel
{
public:
	enum { class_ID = 'Anon' };

					CPrefsAuthAnonymous();
					CPrefsAuthAnonymous(LStream *inStream);
	virtual 		~CPrefsAuthAnonymous();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ToggleICDisplay(bool IC_on);		// Toggle display of IC
	virtual void	SetAuth(CAuthenticator* auth);		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* auth);	// Force update of authenticator
	virtual void	UpdateItems(bool enable);			// Update item entry
};

#endif
