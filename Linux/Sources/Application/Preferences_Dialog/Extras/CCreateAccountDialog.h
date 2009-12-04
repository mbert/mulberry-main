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


// Header for CCreateAccountDialog class

#ifndef __CCREATEACCOUNTDIALOG__MULBERRY__
#define __CCREATEACCOUNTDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

#include "HPopupMenu.h"

// Classes

class CTextInputField;
class JXTextButton;

class CCreateAccountDialog : public CDialogDirector
{
public:
	CCreateAccountDialog(JXDirector* supervisor);

	static bool PoseDialog(cdstring& name, short& type);

protected:
// begin JXLayout

    JXTextButton*    mOkBtn;
    JXTextButton*    mCancelBtn;
    CTextInputField* mText;
    HPopupMenu*      mType;

// end JXLayout

	virtual void	OnCreate();								// Do odds & ends

			void InitAccountTypePopup();
			void GetDetails(cdstring& name, short& type);

};

#endif
