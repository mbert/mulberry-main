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


// CChooseServersDialog.h : header file
//

#ifndef __CCHOOSESERVERSDIALOG__MULBERRY__
#define __CCHOOSESERVERSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CChooseServersDialog dialog

class CTextInputField;

class CChooseServersDialog : public CDialogDirector
{
// Construction
public:
	CChooseServersDialog(JXDirector* supervisor);   // standard constructor

	static bool PoseDialog(cdstring& mail, cdstring& smtp);

protected:
// begin JXLayout

    CTextInputField* mMailServer;
    CTextInputField* mSMTPServer;

// end JXLayout

	virtual void OnCreate();
};

#endif
