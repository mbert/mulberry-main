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


// Header for CPrefsAuthPanel class

#ifndef __CPREFSAUTHPANEL__MULBERRY__
#define __CPREFSAUTHPANEL__MULBERRY__


// Constants

// Classes
class CAuthenticator;

class CPrefsAuthPanel : public LView
{
public:
	enum { class_ID = 'Apan' };

					CPrefsAuthPanel() {}
					CPrefsAuthPanel(LStream *inStream) : LView(inStream) {}
	virtual 		~CPrefsAuthPanel() {}

	virtual void	ToggleICDisplay(bool IC_on) = 0;			// Toggle display of IC - pure virtual
	virtual void	SetAuth(CAuthenticator* copyAuth) = 0;		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* copyAuth) = 0;	// Force update of authenticator
	virtual void	UpdateItems(bool enable) = 0;				// Update item entry

	virtual void	SavePlace(LStream *outPlace)
		{ /*LPane::SavePlace(outPlace);*/ }
	virtual void	RestorePlace(LStream *inPlace)
		{ /*LPane::RestorePlace(inPlace);*/ }
};

#endif
