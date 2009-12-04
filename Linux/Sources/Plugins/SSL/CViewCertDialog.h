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


// CViewCertDialog.h : header file
//

#ifndef __CVIEWCERTDIALOG__MULBERRY__
#define __CVIEWCERTDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CViewCertDialog dialog

class cdstring;
class CTextDisplay;
class JXTextButton;

class CViewCertDialog : public CDialogDirector
{
// Construction
public:
	CViewCertDialog(JXDirector* supervisor);   // standard constructor

	static void PoseDialog(const cdstring& certificate);

protected:
// begin JXLayout

    CTextDisplay* mCertificate;
    JXTextButton* mOKBtn;

// end JXLayout

	void OnCreate();
};
#endif
