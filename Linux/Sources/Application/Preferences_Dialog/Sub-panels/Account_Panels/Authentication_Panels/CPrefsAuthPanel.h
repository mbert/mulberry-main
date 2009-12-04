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

#include <JXWidgetSet.h>

// Classes
class CAuthenticator;

class CPrefsAuthPanel : public JXWidgetSet
{
public:
	CPrefsAuthPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void	OnCreate() = 0;

	virtual void	SetAuth(CAuthenticator* copyAuth) = 0;		// Set authenticator - pure virtual
	virtual void	UpdateAuth(CAuthenticator* copyAuth) = 0;	// Force update of authenticator
	virtual void	UpdateItems(bool enable) = 0;				// Update item entry
};

#endif
