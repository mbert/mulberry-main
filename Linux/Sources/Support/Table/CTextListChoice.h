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

#ifndef __CTEXTLISTCHOICE__MULBERRY__
#define __CTEXTLISTCHOICE__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"
#include "templs.h"

class JXTextButton;
class CTextInputField;
class JXStaticText;
class CTextTable;

class CTextListChoice : public CDialogDirector
{
public:
	CTextListChoice(JXDirector* supervisor);

	static bool PoseDialog(const char* title, const char* description, const char* item, bool use_entry,
							bool single_selection, bool no_selection, bool select_first,
							cdstrvect& items, cdstring& text, ulvector& selection, const char* btn = NULL);

protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

// begin JXLayout

    JXStaticText*    mListTitle;
    JXStaticText*    mEnterTitle;
    CTextInputField* mEnterCtrl;
    JXTextButton*    mOKBtn;
    JXTextButton*    mCancelBtn;

// end JXLayout

public:
	CTextTable*				   mList;
	cdstring			mButtonTitle;
	cdstring			mTextListDescription;
	cdstring			mTextListEnter;
	cdstring			mTextListEnterDescription;
	//}}AFX_DATA
	bool			mUseEntry;
	bool			mSingleSelection;
	bool			mNoSelection;
	bool			mSelectFirst;
	cdstrvect		mItems;
	ulvector		mSelection;
};
#endif
