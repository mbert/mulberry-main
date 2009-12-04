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


// CEditGroupDialog.h : header file
//

#ifndef __CEDITGROUPDIALOG__MULBERRY__
#define __CEDITGROUPDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDialog dialog

class CGroup;
class CTextInputField;
class CTextInputDisplay;
class JXTextButton;

class CEditGroupDialog : public CDialogDirector
{
// Construction
public:
	CEditGroupDialog(JXDirector* supervisor);

	static bool PoseDialog(CGroup* grp);

protected:
// begin JXLayout

    CTextInputField*   mGroupName;
    CTextInputField*   mNickName;
    CTextInputDisplay* mAddressList;
    JXTextButton*      mSortBtn;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mOKBtn;

// end JXLayout

	cdstring mOriginalListText;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void SetFields(const CGroup* grp);
			bool GetFields(CGroup* grp);
			void OnGroupEditSort();
};

#endif
