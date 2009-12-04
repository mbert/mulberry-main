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


// Header for CDeleteIdentityDialog class

#ifndef __CDELETEIDENTITYDIALOG__MULBERRY__
#define __CDELETEIDENTITYDIALOG__MULBERRY__

#include <LGADialogBox.h>

#include "templs.h"

// Panes
const	PaneIDT		paneid_DeleteIdentityDialog = 3005;
const	PaneIDT		paneid_DeleteIdentityList = 'IDEN';
const	PaneIDT		paneid_DeleteIdentityDeleteBtn = 'OKBT';

// Mesages
const	MessageT	msg_SelectIdentity = 'IDEN';

// Resources
const	ResIDT		RidL_CDeleteIdentityDialogBtns = 3005;

// Classes
class CTextTable;
class LGAPushButton;
class CIdentityList;

class	CDeleteIdentityDialog : public LGADialogBox
{
public:
	enum { class_ID = 'IdsD' };

					CDeleteIdentityDialog();
					CDeleteIdentityDialog(LStream *inStream);
	virtual 		~CDeleteIdentityDialog();

	virtual void		SetList(const CIdentityList* aList);
	virtual ulvector	GetDetails(void);					// Get details from dialog

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	CTextTable*		mIdentityList;
	LGAPushButton*	mDeleteBtn;
};

#endif
