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


// Header for CSearchOptionsDialog class

#ifndef __CSEARCHOPTIONSDIALOG__MULBERRY__
#define __CSEARCHOPTIONSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_SearchOptionsDialog = 1015;
const	PaneIDT		paneid_SearchOptionsUseMultiple = 'MULT';
const	PaneIDT		paneid_SearchOptionsMaxMultiple = 'MAXN';
const	PaneIDT		paneid_SearchOptionsLoadBalance = 'LOAD';
const	PaneIDT		paneid_SearchOptionsOpenFirst = 'OPEN';

// Mesages
const	MessageT	msg_SearchOptionsUseMultiple = 'MULT';

// Resources
const	PaneIDT		RidL_CSearchOptionsDialog = 1015;

class CTextFieldX;
class LCheckBox;
class LCheckBoxGroupBox;

class CSearchOptionsDialog : public LDialogBox
{
public:
	enum { class_ID = 'Sopt' };

					CSearchOptionsDialog();
					CSearchOptionsDialog(LStream *inStream);
	virtual 		~CSearchOptionsDialog();

	virtual void ListenToMessage(MessageT inMessage, void* ioParam );				// ¥ OVERRIDE

	void SetItems();
	void GetItems();

protected:
	LCheckBoxGroupBox* mMultiple;
	CTextFieldX* mMaximum;
	LCheckBox* mLoadBalance;
	LCheckBox* mOpenFirst;

	virtual void	FinishCreateSelf(void);
};

#endif
