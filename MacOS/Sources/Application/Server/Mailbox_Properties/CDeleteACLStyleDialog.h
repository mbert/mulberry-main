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


// Header for CDeleteACLStyleDialog class

#ifndef __CDELETEACLSTYLEDIALOG__MULBERRY__
#define __CDELETEACLSTYLEDIALOG__MULBERRY__

#include <LGADialogBox.h>

#include "CACL.h"

#include "templs.h"

// Panes
const	PaneIDT		paneid_DeleteACLStyleDialog = 4101;
const	PaneIDT		paneid_DeleteACLStyleList = 'STYL';
const	PaneIDT		paneid_DeleteACLStyleDeleteBtn = 'OKBT';

// Mesages
const	MessageT	msg_SelectStyle = 'STYS';

// Resources
const	ResIDT		RidL_CDeleteACLStyleDialogBtns = 4101;

// Classes
class CTextTable;
class LGAPushButton;

class	CDeleteACLStyleDialog : public LGADialogBox
{
public:
	enum { class_ID = 'AcDS' };

					CDeleteACLStyleDialog();
					CDeleteACLStyleDialog(LStream *inStream);
	virtual 		~CDeleteACLStyleDialog();

	virtual void		SetList(const SACLStyleList* aList);
	virtual ulvector	GetDetails(void);					// Get details from dialog

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	CTextTable*		mStyleList;
	LGAPushButton*	mDeleteBtn;
};

#endif
