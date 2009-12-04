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


// Header for CNewWildcardDialog class

#ifndef __CNEWWILDCARDDIALOG__MULBERRY__
#define __CNEWWILDCARDDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_NewWildcardDialog = 4003;
const	PaneIDT		paneid_NewWildcardAccount = 'ACCT';
const	PaneIDT		paneid_NewWildcardCriteria = 'CRIT';
const	PaneIDT		paneid_NewWildcardText = 'TEXT';

// Resources
const	PaneIDT		RidL_CNewWildcardDialogBtns = 4002;

class CTextFieldX;
class LPopupButton;

class CNewWildcardDialog : public LDialogBox
{
public:
	enum { class_ID = 'NewW' };

					CNewWildcardDialog();
					CNewWildcardDialog(LStream *inStream);
	virtual 		~CNewWildcardDialog();

	static bool		PoseDialog(cdstring& name);

	virtual	void	GetDetails(cdstring& name);

protected:
	LPopupButton* mAccount;
	LPopupButton* mCriteria;
	CTextFieldX* mText;

	virtual void	FinishCreateSelf(void);
	
private:
	void InitAccountMenu(void);
};

#endif
