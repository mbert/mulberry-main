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


// Header for CAcceptCertDialog class

#ifndef __CACCEPTCERTDIALOG__MULBERRY__
#define __CACCEPTCERTDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_AcceptCertDialog = 22100;
const	PaneIDT		paneid_AcceptCertErrors = 'TABL';
const	PaneIDT		paneid_AcceptCertCert = 'CERT';

// Message
const	MessageT	msg_AcceptCertAcceptSave = 'SAVE';

// Resources
const	PaneIDT		RidL_CAcceptCertDialogBtns = 22100;

class CAcceptCertDialog : public LDialogBox
{
public:
	enum { class_ID = 'Cert' };

	enum
	{
		eAcceptOnce = 1,
		eNoAccept,
		eAcceptSave
	};

					CAcceptCertDialog();
					CAcceptCertDialog(LStream *inStream);
	virtual 		~CAcceptCertDialog();

	virtual	void	SetUpDetails(const char* certificate, const cdstrvect& errors);

	static int PoseDialog(const char* certificate, const cdstrvect& errors);

protected:
	virtual void	FinishCreateSelf(void);
};

#endif
