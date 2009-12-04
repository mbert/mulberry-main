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


// Source for CNewWildcardDialog class

#include "CNewWildcardDialog.h"

#include "CINETCommon.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include <JXTextButton.h>
#include <JXStaticText.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>

static const JCharacter *criteriaMenu = 
" whose name starts with %r | whose name ends with %r |"
" whose name contains %r | whose name is %r | anywhere in hierarchy %r |"
" at top of hierarchy%r ";

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

enum
{
	kAccountAll = 1,
	kAccountFirst
};

enum {
	kCriteriaStartsWith = 1,
	kCriteriaEndsWith,
	kCriteriaContains,
	kCriteriaIs,
	kCriteriaFullHierarchy,
	kCriteriaTopHierarchy
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewWildcardDialog::CNewWildcardDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CNewWildcardDialog::~CNewWildcardDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewWildcardDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,150, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,150);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Match mailboxes in account", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 170,20);
    assert( obj2 != NULL );

    mAccount =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,10, 145,25);
    assert( mAccount != NULL );

    mCriteria =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,40, 215,25);
    assert( mCriteria != NULL );

    mText =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,75, 320,20);
    assert( mText != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,115, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,115, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	window->SetTitle("Cabinet Entry List");
	SetButtons(mOKBtn, mCancelBtn);

	mCriteria->SetMenuItems(criteriaMenu);
	mCriteria->SetToPopupChoice(kTrue, kCriteriaFullHierarchy);
	mCriteria->SetUpdateAction(JXMenu::kDisableNone);
	ListenTo(mCriteria);

	mAccount->SetUpdateAction(JXMenu::kDisableNone);
	InitAccountMenu();
	ListenTo(mAccount);
}

// Called during idle
void CNewWildcardDialog::GetDetails(cdstring& change)
{
	// Add account name
	if (mAccount->GetValue() == kAccountAll)
		change += cWILDCARD;
	else
		change += mAccount->GetItemText(mAccount->GetValue());
	change += cMailAccountSeparator;

	// Get criteria and name
	JIndex criteria = mCriteria->GetValue();
	cdstring copyStr = mText->GetText();

	// Set leading criteria value
	switch(criteria)
	{
	case kCriteriaEndsWith:
	case kCriteriaContains:
		change += cWILDCARD;
		break;
	case kCriteriaStartsWith:
	case kCriteriaIs:
	case kCriteriaFullHierarchy:
	case kCriteriaTopHierarchy:
		break;
	}

	change += copyStr;

	// Set trailing criteria value
	switch(criteria)
	{
	case kCriteriaStartsWith:
	case kCriteriaContains:
		change += cWILDCARD;
		break;
	case kCriteriaEndsWith:
	case kCriteriaIs:
		break;
	case kCriteriaFullHierarchy:
	case kCriteriaTopHierarchy:
		{
			short acct_num = mAccount->GetValue() - kAccountFirst;
			if (acct_num < 0)
				acct_num = 0;
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

			// Append dir delim if not already
			if (dir_delim && change.length() && (change.c_str()[change.length() - 1] != dir_delim) && !copyStr.empty())
				change += dir_delim;

			// Append appropriate wildcard
			if (criteria == kCriteriaFullHierarchy)
				change += cWILDCARD;
			else
				change += cWILDCARD_NODIR;
		}
		break;
	}

}

// Called during idle
void CNewWildcardDialog::InitAccountMenu(void)
{
	mAccount->RemoveAllItems();

	mAccount->AppendMenuItems("All %r%l");

	// Add each mail account
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		mAccount->AppendItem((*iter)->GetName(), kFalse, kTrue);

	mAccount->SetToPopupChoice(kTrue, 1);
}

bool CNewWildcardDialog::PoseDialog(cdstring& name)
{
	bool result = false;

	CNewWildcardDialog* dlog = new CNewWildcardDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		dlog->GetDetails(name);
		result = true;
		dlog->Close();
	}

	return result;
}
