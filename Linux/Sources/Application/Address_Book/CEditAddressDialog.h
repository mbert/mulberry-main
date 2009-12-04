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


// CEditAddressDialog.h : header file
//

#ifndef __CEDITADDRESSDIALOG__MULBERRY__
#define __CEDITADDRESSDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog dialog

class CAdbkAddress;
class CTextInputDisplay;
class CTextInputField;
class JXTextButton;

class CEditAddressDialog : public CDialogDirector
{
// Construction
public:
	CEditAddressDialog(JXDirector* supervisor);

	static bool PoseDialog(CAdbkAddress* addr, bool allow_edit = true);

protected:
// begin JXLayout

    CTextInputField*   mFullName;
    CTextInputField*   mNickName;
    CTextInputField*   mEmail;
    CTextInputField*   mCalendar;
    CTextInputField*   mCompany;
    CTextInputField*   mPhoneWork;
    CTextInputField*   mPhoneHome;
    CTextInputField*   mFax;
    CTextInputDisplay* mAddress;
    CTextInputDisplay* mURL;
    CTextInputDisplay* mNotes;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mOKBtn;

// end JXLayout

	virtual void OnCreate();

			bool GetFields(CAdbkAddress* addr);
			void SetFields(CAdbkAddress* addr, bool allow_edit);
};

#endif
