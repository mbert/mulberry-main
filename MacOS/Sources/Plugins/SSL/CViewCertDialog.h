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


// Header for CViewCertDialog class

#ifndef __CVIEWCERTDIALOG__MULBERRY__
#define __CVIEWCERTDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_ViewCertDialog = 22111;
const	PaneIDT		paneid_ViewCertCert = 'CERT';

// Message

// Resources

class CViewCertDialog : public LDialogBox
{
public:
	enum { class_ID = 'Vcer' };

					CViewCertDialog();
					CViewCertDialog(LStream *inStream);
	virtual 		~CViewCertDialog();

	static void PoseDialog(const cdstring& cert);

protected:
		void	SetUpDetails(const cdstring& cert);
};

#endif
