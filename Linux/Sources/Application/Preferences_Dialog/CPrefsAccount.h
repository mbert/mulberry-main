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


// Header for CPrefsAccount class

#ifndef __CPREFSACCOUNT__MULBERRY__
#define __CPREFSACCOUNT__MULBERRY__

#include "CPrefsPanel.h"

#include "CAuthenticator.h"
#include "CINETAccount.h"

#include "HPopupMenu.h"

// Classes
class CPrefsAccountPanel;

class JXCardFile;
class JXStaticText;
class JXTextCheckbox;
class CTextInputField;

class CPrefsAccount : public CPrefsPanel
{
// Construction
public:
	CPrefsAccount(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);
	virtual ~CPrefsAccount();

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    HPopupMenu*      mAccountPopup;
    JXStaticText*    mServerType;
    JXStaticText*    mServerIPTitle;
    CTextInputField* mServerIP;
    JXTextCheckbox*  mMatchUIDPswd;
    JXCardFile*      mPanels;

// end JXLayout1
	int					mAccountValue;
	bool				mIsSMTP;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	AddPanel(CPrefsAccountPanel* panel);
			void	InitAccountMenu();								// Set up account menu
			void	DoNewAccount();									// Add new account
			void	DoRenameAccount();								// Rename account
			void	DoDeleteAccount();								// Delete accoount
			void	UpdateItems(bool enable);						// Update item entry
			void	SetAccount(const CINETAccount* account);		// Set account details
			void	UpdateAccount();								// Update current account
	
			void	GetCurrentAccount(CINETAccount*& acct, 
										long& index,
										CINETAccountList*& list);	// Get current account details

			void	SetPanel(const CINETAccount* account);			// Set panel
};

#endif
