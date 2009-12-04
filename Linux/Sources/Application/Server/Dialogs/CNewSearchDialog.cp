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


// Source for CNewSearchDialog class

#include "CNewSearchDialog.h"

#include "CINETCommon.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextField.h"

#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>
#include <string.h>

static const JCharacter *criteriaMenu = 
" whose name starts with %r | whose name ends with %r |"
" whose name contains %r | whose name is %r | in sub-hierarchy %r |"
" in entire hierarchy %r ";

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CNewSearchDialog::CNewSearchDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}


// Default destructor
CNewSearchDialog::~CNewSearchDialog()
{
}

void CNewSearchDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,205, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,205);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Search in account", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 115,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("for mailboxes", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,45, 110,20);
    assert( obj3 != NULL );

    mAccount =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,10, 170,25);
    assert( mAccount != NULL );

    mCriteria =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,40, 170,25);
    assert( mCriteria != NULL );

    mText =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,75, 320,20);
    assert( mText != NULL );

    mWhichHierarchy =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,105, 275,50);
    assert( mWhichHierarchy != NULL );

    mFullHierarchy =
        new JXTextRadioButton(eFullHierarchy, "Display Full Hierarchy", mWhichHierarchy,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,5, 190,20);
    assert( mFullHierarchy != NULL );

    mTopHierarchy =
        new JXTextRadioButton(eTopHierarchy, "Display Top Level of Hierarchy Only", mWhichHierarchy,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 245,20);
    assert( mTopHierarchy != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,170, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,170, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

// end JXLayout
	window->SetTitle("Display Hierarchy");
	SetButtons(mOKBtn, mCancelBtn);
	mWhichHierarchy->SelectItem(eTopHierarchy);

	mCriteria->SetMenuItems(criteriaMenu);
	mCriteria->SetToPopupChoice(kTrue, kCriteriaSubHierarchy);
	mCriteria->SetUpdateAction(JXMenu::kDisableNone);
	ListenTo(mCriteria);

	mAccount->SetUpdateAction(JXMenu::kDisableNone);
	InitAccountMenu();
	ListenTo(mAccount);
}

void CNewSearchDialog::InitAccountMenu(void)
{
	mAccount->RemoveAllItems();

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = 
				CPreferences::sPrefs-> mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); 
			++iter, ++menu_pos)
		//Append it as a radio button
		mAccount->AppendItem((*iter)->GetName(), kFalse, kTrue);
	mAccount->SetToPopupChoice(kTrue, 1);
}

// Set the details
void CNewSearchDialog::SetDetails(bool single, unsigned long index)
{
	if (single)
	{
		// Disable all menu items except for relevant one
		mAccount->DisableAll();
		mAccount->EnableItem(index);
	}
	else
		mAccount->EnableAll();
	
	mAccount->SetValue(index ? index : 1);
}


void CNewSearchDialog::GetDetails(cdstring& wd, unsigned long& index)
{
	index = mAccount->GetValue();
	JIndex criteria = mCriteria->GetValue();
	cdstring copyStr = mText->GetText();
	bool full = mWhichHierarchy->GetSelectedItem() == eFullHierarchy;
	
	switch (criteria)
	{
	case kCriteriaEndsWith:
	case kCriteriaContains:
		wd += cWILDCARD;
		break;
	case kCriteriaStartsWith:
	case kCriteriaIs:
	case kCriteriaSubHierarchy:
	case kCriteriaFullHierarchy:
		break;
	}

	// Only if not full
	if (criteria != kCriteriaFullHierarchy)
		wd += copyStr;

	// Set trailing criteria value
	switch(criteria)
	{
	case kCriteriaStartsWith:
	case kCriteriaContains:
		wd += cWILDCARD;
		break;
	case kCriteriaEndsWith:
	case kCriteriaIs:
		break;
	case kCriteriaSubHierarchy:
		{
			char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue()[index - 1]->GetDirDelim();
			// Append dir delim if not already
			if (dir_delim && wd.length() && 
					(wd.c_str()[wd.length() - 1] != dir_delim))
				wd += dir_delim;
		}
		// Append full wildcard
		if (full)
			wd += cWILDCARD;
		break;

	case kCriteriaFullHierarchy:
		// Append appropriate wildcard
		if (full)
			wd += cWILDCARD;
		else
			wd += cWILDCARD_NODIR;
		break;
	}
}

void CNewSearchDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected) && sender == mCriteria)
	{
    	const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		switch (is->GetIndex())
		{
		case kCriteriaStartsWith:
		case kCriteriaEndsWith:
		case kCriteriaContains:
		case kCriteriaIs:
			mFullHierarchy->Deactivate();
			mTopHierarchy->Deactivate();
			break;
		case kCriteriaSubHierarchy:
			mFullHierarchy->Activate();
			mTopHierarchy->Activate();
			mText->Activate();
			break;
		case kCriteriaFullHierarchy:
			mFullHierarchy->Activate();
			mTopHierarchy->Activate();
			mText->Deactivate();
			break;
		}
	}
	else
		CDialogDirector::Receive(sender,message);
}

bool CNewSearchDialog::PoseDialog(bool single, unsigned long& index, cdstring& wd)
{
	bool result = false;

	CNewSearchDialog* dlog = new CNewSearchDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(single, index);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(wd, index);
		result = true;
		dlog->Close();
	}

	return result;
}
