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


// Header for CNewSearchDialog class

#ifndef __CNEWSEARCHDIALOG__MULBERRY__
#define __CNEWSEARCHDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "HPopupMenu.h"

class cdstring;
class JXTextButton;
class CTextInputField;
class JXTextRadioButton;
class JXRadioGroup;

class CNewSearchDialog : public CDialogDirector
{
 public:
	enum {eFullHierarchy, eTopHierarchy};
	enum {
		kCriteriaStartsWith = 1,
		kCriteriaEndsWith,
		kCriteriaContains,
		kCriteriaIs,
		kCriteriaSubHierarchy,
		kCriteriaFullHierarchy
	};

	CNewSearchDialog(JXDirector* supervisor);
	virtual ~CNewSearchDialog();

	static bool PoseDialog(bool single, unsigned long& index, cdstring& wd);

	virtual void	SetDetails(bool single, unsigned long index);
	virtual void	GetDetails(cdstring& wd, unsigned long& index);

protected:
	void OnCreate();
	void InitAccountMenu();

// begin JXLayout

    HPopupMenu*        mAccount;
    HPopupMenu*        mCriteria;
    CTextInputField*   mText;
    JXRadioGroup*      mWhichHierarchy;
    JXTextRadioButton* mFullHierarchy;
    JXTextRadioButton* mTopHierarchy;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;

// end JXLayout

protected:
	virtual void Receive(JBroadcaster* sender, const Message& message);
	 
};

#endif
