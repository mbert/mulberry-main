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


// CAcceptCertDialog.h : header file
//

#ifndef __CACCEPTCERTDIALOG__MULBERRY__
#define __CACCEPTCERTDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CAcceptCertDialog dialog

class CTextDisplay;
class CTextTable;
class JXTextButton;

class CAcceptCertDialog : public CDialogDirector
{
// Construction
public:
	enum
	{
		eAcceptOnce = 1,
		eNoAccept,
		eAcceptSave
	};

	CAcceptCertDialog(JXDirector* supervisor);   // standard constructor

	static int PoseDialog(const char* certificate, const cdstrvect& errors);

protected:
// begin JXLayout

    CTextDisplay* mCertificate;
    JXTextButton* mAcceptOnceBtn;
    JXTextButton* mCancelBtn;
    JXTextButton* mAcceptSaveBtn;

// end JXLayout
	CTextTable*		mErrors;

	void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
};
#endif
