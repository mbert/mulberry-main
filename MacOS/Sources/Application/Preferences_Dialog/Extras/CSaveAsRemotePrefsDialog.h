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


// Header for CSaveAsRemotePrefsDialog class

#ifndef __CSAVEASREMOTEPREFSDIALOG__MULBERRY__
#define __CSAVEASREMOTEPREFSDIALOG__MULBERRY__

#include <LGADialogBox.h>

#include "cdstring.h"

// Panes
const	PaneIDT		paneid_SaveAsRemotePrefsDialog = 5151;
const	PaneIDT		paneid_SaveAsRemotePrefsOKBtn = 'OKBT';
const	PaneIDT		paneid_SaveAsRemotePrefsList = 'RemP';
const	PaneIDT		paneid_SaveAsRemotePrefsName = 'SAVA';

// Messages
const	MessageT	msg_SaveAsRemotePrefsListClick = 'Rem1';
const	MessageT	msg_SaveAsRemotePrefsListDblClick = 'Rem2';

// Classes
class CTextTable;
class CTextField;
class LGAPushButton;

class	CSaveAsRemotePrefsDialog : public LGADialogBox
{
public:
	enum { class_ID = 'RmPS' };

					CSaveAsRemotePrefsDialog();
					CSaveAsRemotePrefsDialog(LStream *inStream);
	virtual 		~CSaveAsRemotePrefsDialog();

	virtual void		SetName(const char* name);			// Set initial name
	virtual cdstring	GetDetails(void);					// Get details from dialog

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Check for valid OK

private:
	LGAPushButton*	mOKBtn;
	CTextTable*		mRemoteList;
	CTextField*		mName;
};

#endif
