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


// Header for CPrefsIdentities class

#ifndef __CPREFSIDENTITIES__MULBERRY__
#define __CPREFSIDENTITIES__MULBERRY__

#include "CPrefsPanel.h"
#include "CListener.h"

#include "templs.h"

// Classes
class CTextTable;
class CTextDisplay;
class JXTextButton;
class JXTextCheckbox;

class CPrefsIdentities : public CPrefsPanel,
							public CListener
{
// Construction
public:
	CPrefsIdentities(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

	virtual void	ListenTo_Message(long msg, void* param);

protected:
// begin JXLayout1

    JXTextButton*   mAddBtn;
    JXTextButton*   mChangeBtn;
    JXTextButton*   mDuplicateBtn;
    JXTextButton*   mDeleteBtn;
    JXTextCheckbox* mContextTied;
    JXTextCheckbox* mMsgTied;
    JXTextCheckbox* mTiedMboxInherit;
    CTextDisplay*   mSmartAddressText;

// end JXLayout1
	CTextTable*		mIdentitiesList;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	InitIdentitiesList();				// Initialise the list
	
			void	DoAddIdentities();					// Set identities
			void	DoEditIdentities();					// Set identities
			void	DoDuplicateIdentities();			// Duplicate identities
			void	DoDeleteIdentities();				// Clear identities
			void	DoMoveIdentities(const ulvector& from,	// Move identities
										unsigned long to);
};

#endif
