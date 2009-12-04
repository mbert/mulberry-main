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


// Header for CCreateAccountDialog class

#ifndef __CCREATEACCOUNTDIALOG__MULBERRY__
#define __CCREATEACCOUNTDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_CreateAccountDialog = 5105;
const	PaneIDT		paneid_CreateAccountName = 'NAME';
const	PaneIDT		paneid_CreateAccountType = 'TYPE';

// Resources
const	PaneIDT		RidL_CCreateAccountDialogBtns = 10002;

class CTextFieldX;
class LPopupButton;

class CCreateAccountDialog : public LDialogBox
{
public:
	enum { class_ID = 'NewA' };

					CCreateAccountDialog();
					CCreateAccountDialog(LStream *inStream);
	virtual 		~CCreateAccountDialog();

	virtual	void	GetDetails(cdstring& name, short& type);

protected:
	CTextFieldX* mText;
	LPopupButton* mType;

	virtual void	FinishCreateSelf(void);
};

#endif
