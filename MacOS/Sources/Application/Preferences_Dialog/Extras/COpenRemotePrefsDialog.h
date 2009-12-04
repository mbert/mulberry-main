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


// Header for COpenRemotePrefsDialog class

#ifndef __COPENREMOTEPREFSDIALOG__MULBERRY__
#define __COPENREMOTEPREFSDIALOG__MULBERRY__

#include <LGADialogBox.h>

#include "cdstring.h"

// Panes
const	PaneIDT		paneid_OpenRemotePrefsDialog = 5150;
const	PaneIDT		paneid_OpenRemotePrefsList = 'RemP';
const	PaneIDT		paneid_OpenRemotePrefsOpenBtn = 'OKBT';

// Mesages
const	MessageT	msg_SelectRemotePrefs = 'RemP';

// Resources
const	ResIDT		RidL_COpenRemotePrefsDialogBtns = 5150;

// Classes
class CTextTable;
class LGAPushButton;

class	COpenRemotePrefsDialog : public LGADialogBox
{
public:
	enum { class_ID = 'RmPO' };

					COpenRemotePrefsDialog();
					COpenRemotePrefsDialog(LStream *inStream);
	virtual 		~COpenRemotePrefsDialog();

	virtual cdstring	GetDetails(void);					// Get details from dialog

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	CTextTable*		mRemoteList;
	LGAPushButton*	mOpenBtn;
};

#endif
