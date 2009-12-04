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


// Header for CChangePswdAcctDialog class

#ifndef __CCHANGEPSWDACCTDIALOG__MULBERRY__
#define __CCHANGEPSWDACCTDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_ChangePswdAcctDialog = 6002;
const	PaneIDT		paneid_ChangePswdAcctAcctPopup = 'ACCT';
const	PaneIDT		paneid_ChangePswdAcctPluginPopup = 'TYPE';

// Classes
class CINETAccount;
class CPswdChangePlugin;
class LPopupButton;

class CChangePswdAcctDialog : public LDialogBox
{
private:
	LPopupButton*		mAccountPopup;
	LPopupButton*		mPluginPopup;

public:
	enum { class_ID = 'AChP' };

					CChangePswdAcctDialog();
					CChangePswdAcctDialog(LStream *inStream);
	virtual 		~CChangePswdAcctDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void InitAccountMenu();
	void InitPluginsMenu();

public:
	void	GetDetails(CINETAccount*& acct, CPswdChangePlugin*& plugin); 					// Get details from dialog

	static void AcctPasswordChange();
	static bool PromptAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin,
											LCommander* cmdr);
	static bool DoAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin,
											LCommander* cmdr);
};

#endif
