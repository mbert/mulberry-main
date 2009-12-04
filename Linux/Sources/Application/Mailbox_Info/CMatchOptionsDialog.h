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


// Header for CMatchOptionsDialog class

#ifndef __CMATCHOPTIONSDIALOG__MULBERRY__
#define __CMATCHOPTIONSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CMboxFwd.h"

// Classes
class CCheckboxTable;
class CMatchItem;
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;

class CMatchOptionsDialog : public CDialogDirector
{
public:
					CMatchOptionsDialog(JXDirector* supervisor);
	virtual 		~CMatchOptionsDialog();

	static bool		PoseDialog(NMbox::EViewMode& mbox_mode, CMatchItem& match, bool& last_search);

protected:
	enum {eViewAllNoHilight, eViewAllHilight, eViewMatch};
	enum {eMatchUsingMatch, eMatchUsingSearch};
	enum {eMatchOr, eMatchAnd, eMatchNor, eMatchNand};

// begin JXLayout

    JXRadioGroup*   mViewGroup;
    JXRadioGroup*   mMatchUsingGroup;
    JXRadioGroup*   mMatchIfGroup;
    JXTextCheckbox* mNew;
    JXTextCheckbox* mUnseen;
    JXTextCheckbox* mImportant;
    JXTextCheckbox* mAnswered;
    JXTextCheckbox* mDeleted;
    JXTextCheckbox* mDraft;
    JXTextCheckbox* mSentToday;
    JXTextCheckbox* mSentSinceYesterday;
    JXTextCheckbox* mSentThisWeek;
    JXTextCheckbox* mSentWithin7Days;
    JXTextCheckbox* mSentByMe;
    JXTextCheckbox* mSentToMe;
    JXTextCheckbox* mSentToList;
    JXTextCheckbox* mSelectedTo;
    JXTextCheckbox* mSelectedFrom;
    JXTextCheckbox* mSelectedCorrespondents;
    JXTextCheckbox* mSelectedSubjects;
    JXTextCheckbox* mSelectedDates;
    JXTextButton*   mClearBtn;
    JXTextButton*   mCancelBtn;
    JXTextButton*   mOKBtn;

// end JXLayout

	CCheckboxTable*	mTable;

	virtual void	OnCreate();					// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	void SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search);
	CMatchItem GetMatchItems(NMbox::EViewMode& mbox_mode, bool& last_search) const;

	void ClearItems();
};

#endif
