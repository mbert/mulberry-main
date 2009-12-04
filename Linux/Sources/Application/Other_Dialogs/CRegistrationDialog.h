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


// Header for CRegistrationDialog class

#ifndef __CREGISTRATIONDIALOG__MULBERRY__
#define __CREGISTRATIONDIALOG__MULBERRY__

#include "CDialogDirector.h"

// Classes
class JXTextButton;
class CTextInputField;
class JXImageWidget;

class CRegistrationDialog : public CDialogDirector
{
public:
			CRegistrationDialog(JXDirector* supervisor);

	static bool	DoRegistration(bool initial);			// Try to register

protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

// begin JXLayout

    JXTextButton*    mOKBtn;
    JXTextButton*    mDemoBtn;
    JXTextButton*    mCancelBtn;
    JXImageWidget*   mLogo;
    CTextInputField* mLicensee;
    CTextInputField* mOrganisation;
    CTextInputField* mSerialNumber;

// end JXLayout
// begin JXLayout2

    CTextInputField* mRegKey1;
    CTextInputField* mRegKey2;
    CTextInputField* mRegKey3;
    CTextInputField* mRegKey4;

// end JXLayout2
	bool mAllowDemo;

};

#endif
