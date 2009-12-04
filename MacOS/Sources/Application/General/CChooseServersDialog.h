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


// Header for CChooseServersDialog class

#ifndef __CCHOOSESERVERSDIALOG__MULBERRY__
#define __CCHOOSESERVERSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_ChooseServerDialog = 5202;
const	PaneIDT		paneid_ChooseServerMail = 'MAIL';
const	PaneIDT		paneid_ChooseServerSMTP = 'SMTP';

// Resources

class LCommander;

class CChooseServersDialog : public LDialogBox
{
public:
	enum { class_ID = 'ChSv' };

					CChooseServersDialog();
					CChooseServersDialog(LStream *inStream);
	virtual 		~CChooseServersDialog();

	static bool	PoseDialog(cdstring& mail, cdstring& smtp);

protected:
	virtual void	FinishCreateSelf(void);
};

#endif
