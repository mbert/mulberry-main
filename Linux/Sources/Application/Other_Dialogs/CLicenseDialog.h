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


// CLicenseDialog.h : header file
//

#ifndef __CLICENSEDIALOG__MULBERRY__
#define __CLICENSEDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CLicenseDialog dialog

// classes
class JXTextButton;
class CTextDisplay;
class JXImageWidget;

class CLicenseDialog : public CDialogDirector
{
// Construction
public:
	CLicenseDialog(JXDirector* supervisor);

protected:
	virtual void OnCreate();

// begin JXLayout

    JXTextButton*  mOKBtn;
    JXTextButton*  mCancelBtn;
    CTextDisplay*  mLicenseText;
    JXImageWidget* mLogo;

// end JXLayout
};

#endif
