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


// Source for CMatchOptionsDialog class

#include "CMatchOptionsDialog.h"

#include "CCheckboxTable.h"
#include "CDivider.h"
#include "CMatchItem.h"
#include "CPreferences.h"
#include "CScrollbarSet.h"

#include <JXEngravedRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>
#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMatchOptionsDialog::CMatchOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CMatchOptionsDialog::~CMatchOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMatchOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,460, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,460);
    assert( obj1 != NULL );

    mViewGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 270,80);
    assert( mViewGroup != NULL );

    JXStaticText* obj2 =
        new JXStaticText("View:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,10, 40,15);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(eViewAllNoHilight, "All Messages without Match Hilights", mViewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 235,20);
    assert( obj3 != NULL );

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(eViewAllHilight, "All Messages with Match Hilights", mViewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 235,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(eViewMatch, "Matched Messages Only", mViewGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 235,20);
    assert( obj5 != NULL );

    mMatchUsingGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,20, 200,80);
    assert( mMatchUsingGroup != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Match using:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 295,10, 80,15);
    assert( obj6 != NULL );

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(eMatchUsingMatch, "Selected Criteria", mMatchUsingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 185,20);
    assert( obj7 != NULL );

    JXTextRadioButton* obj8 =
        new JXTextRadioButton(eMatchUsingSearch, "Last Search Results", mMatchUsingGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 185,20);
    assert( obj8 != NULL );

    CDivider* obj9 =
        new CDivider(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 480,2);
    assert( obj9 != NULL );

    mMatchIfGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 160,140);
    assert( mMatchIfGroup != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Match if:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 15,120, 60,15);
    assert( obj10 != NULL );

    JXTextRadioButton* obj11 =
        new JXTextRadioButton(eMatchOr, "any Criteria met", mMatchIfGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 140,20);
    assert( obj11 != NULL );

    JXTextRadioButton* obj12 =
        new JXTextRadioButton(eMatchAnd, "all Criteria met", mMatchIfGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 140,20);
    assert( obj12 != NULL );

    JXTextRadioButton* obj13 =
        new JXTextRadioButton(eMatchNor, "no Criteria met", mMatchIfGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 140,20);
    assert( obj13 != NULL );

    JXTextRadioButton* obj14 =
        new JXTextRadioButton(eMatchNand, "not all Criteria met", mMatchIfGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 140,20);
    assert( obj14 != NULL );

    JXEngravedRect* obj15 =
        new JXEngravedRect(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 180,130, 100,140);
    assert( obj15 != NULL );

    JXStaticText* obj16 =
        new JXStaticText("Flag Criteria:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 185,120, 80,15);
    assert( obj16 != NULL );

    mNew =
        new JXTextCheckbox("New", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 90,20);
    assert( mNew != NULL );

    mUnseen =
        new JXTextCheckbox("Unseen", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 90,20);
    assert( mUnseen != NULL );

    mImportant =
        new JXTextCheckbox("Important", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 90,20);
    assert( mImportant != NULL );

    mAnswered =
        new JXTextCheckbox("Answered", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 90,20);
    assert( mAnswered != NULL );

    mDeleted =
        new JXTextCheckbox("Deleted", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,90, 90,20);
    assert( mDeleted != NULL );

    mDraft =
        new JXTextCheckbox("Draft", obj15,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,110, 90,20);
    assert( mDraft != NULL );

    JXEngravedRect* obj17 =
        new JXEngravedRect(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 290,130, 200,140);
    assert( obj17 != NULL );

    JXStaticText* obj18 =
        new JXStaticText("Date Criteria:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 295,120, 80,15);
    assert( obj18 != NULL );

    mSentToday =
        new JXTextCheckbox("Sent Today", obj17,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 160,20);
    assert( mSentToday != NULL );

    mSentSinceYesterday =
        new JXTextCheckbox("Sent since Yesterday", obj17,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 160,20);
    assert( mSentSinceYesterday != NULL );

    mSentThisWeek =
        new JXTextCheckbox("Sent this Week", obj17,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 160,20);
    assert( mSentThisWeek != NULL );

    mSentWithin7Days =
        new JXTextCheckbox("Sent within 7 Days", obj17,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 160,20);
    assert( mSentWithin7Days != NULL );

    JXUpRect* obj19 =
        new JXUpRect(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,280, 160,25);
    assert( obj19 != NULL );

    JXStaticText* obj20 =
        new JXStaticText("Search Sets", obj19,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 40,4, 80,17);
    assert( obj20 != NULL );

    CScrollbarSet* sbs =
        new CScrollbarSet(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,300, 160,110);
    assert( sbs != NULL );

    JXEngravedRect* obj21 =
        new JXEngravedRect(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 180,290, 100,120);
    assert( obj21 != NULL );

    JXStaticText* obj22 =
        new JXStaticText("Sent:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 185,280, 40,15);
    assert( obj22 != NULL );

    mSentByMe =
        new JXTextCheckbox("by Me", obj21,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 90,20);
    assert( mSentByMe != NULL );

    mSentToMe =
        new JXTextCheckbox("to Me", obj21,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 90,20);
    assert( mSentToMe != NULL );

    mSentToList =
        new JXTextCheckbox("to List", obj21,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 90,20);
    assert( mSentToList != NULL );

    JXEngravedRect* obj23 =
        new JXEngravedRect(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 290,290, 200,120);
    assert( obj23 != NULL );

    JXStaticText* obj24 =
        new JXStaticText("Selected:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 295,280, 60,15);
    assert( obj24 != NULL );

    mSelectedTo =
        new JXTextCheckbox("To Addresses", obj23,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,10, 160,20);
    assert( mSelectedTo != NULL );

    mSelectedFrom =
        new JXTextCheckbox("From Addresses", obj23,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 160,20);
    assert( mSelectedFrom != NULL );

    mSelectedCorrespondents =
        new JXTextCheckbox("Correspondents", obj23,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 160,20);
    assert( mSelectedCorrespondents != NULL );

    mSelectedSubjects =
        new JXTextCheckbox("Subjects", obj23,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,70, 160,20);
    assert( mSelectedSubjects != NULL );

    mSelectedDates =
        new JXTextCheckbox("Dates", obj23,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,90, 160,20);
    assert( mSelectedDates != NULL );

    mClearBtn =
        new JXTextButton("Clear", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 180,425, 70,25);
    assert( mClearBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 320,425, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 410,425, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	mTable = new CCheckboxTable(sbs,sbs->GetScrollEnclosure(),
															JXWidget::kHElastic,
															JXWidget::kVElastic,
															0,0, 10, 10);
	mTable->OnCreate();

	window->SetTitle("Match Options");
	SetButtons(mOKBtn, mCancelBtn);
	ListenTo(mClearBtn);
}

void CMatchOptionsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mClearBtn)
		{
			ClearItems();
			return;
		}
	}
		
	CDialogDirector::Receive(sender, message);
	return;
}

void CMatchOptionsDialog::ClearItems()
{
	// Just set an empty spec
	CMatchItem empty;
	SetMatchItems((mViewGroup->GetSelectedItem() == eViewMatch) ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All,
					empty, mMatchUsingGroup->GetSelectedItem() == eMatchUsingSearch);
}

void CMatchOptionsDialog::SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search)
{
	// View mode
	if (mbox_mode == NMbox::eViewMode_All)
		mViewGroup->SelectItem(eViewAllNoHilight);
	else if (mbox_mode == NMbox::eViewMode_AllMatched)
		mViewGroup->SelectItem(eViewAllHilight);
	else
		mViewGroup->SelectItem(eViewMatch);

	// Match or search
	if (!last_search)
		mMatchUsingGroup->SelectItem(eMatchUsingMatch);
	else
		mMatchUsingGroup->SelectItem(eMatchUsingSearch);

	// Copy Or/And/Nor/Nand
	if (match.GetNegate())
	{
		if (match.GetUseOr())
			mMatchIfGroup->SelectItem(eMatchNor);
		else
			mMatchIfGroup->SelectItem(eMatchNand);
	}
	else
	{
		if (match.GetUseOr())
			mMatchIfGroup->SelectItem(eMatchOr);
		else
			mMatchIfGroup->SelectItem(eMatchAnd);
	}

	// Do all other bits
	mNew->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eNew]));
	mUnseen->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eUnseen]));
	mImportant->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eFlagged]));
	mAnswered->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eAnswered]));
	mDeleted->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eDeleted]));
	mDraft->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eDraft]));
	mSentToday->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentToday]));
	mSentSinceYesterday->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentSinceYesterday]));
	mSentThisWeek->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentThisWeek]));
	mSentWithin7Days->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentWithin7Days]));
	mSentByMe->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentByMe]));
	mSentToMe->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentToMe]));
	mSentToList->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSentToList]));
	mSelectedTo->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSelectedTo]));
	mSelectedFrom->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSelectedFrom]));
	mSelectedCorrespondents->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSelectedSmart]));
	mSelectedSubjects->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSelectedSubject]));
	mSelectedDates->SetState(JBoolean(match.GetBitsSet()[CMatchItem::eSelectedDate]));

	// Add all search styles
	const cdstrvect& sets = match.GetSearchSet();
	cdstrvect names;
	boolvector selected;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
	{
		names.push_back((*iter)->GetName());
		bool select =  (std::find(sets.begin(), sets.end(), (*iter)->GetName()) != sets.end());
		selected.push_back(select);
	}
	mTable->SetContents(names);
	mTable->SetSelection(selected);
}

CMatchItem CMatchOptionsDialog::GetMatchItems(NMbox::EViewMode& mbox_mode, bool& last_search) const
{
	// Create new match item
	CMatchItem match;

	// Create appropriate store
	boolvector new_bits(CMatchItem::eMatchLast + 1);
	cdstrvect sets;

	// View mode
	if (mViewGroup->GetSelectedItem() == eViewAllNoHilight)
		mbox_mode = NMbox::eViewMode_All;
	else if (mViewGroup->GetSelectedItem() == eViewAllHilight)
		mbox_mode = NMbox::eViewMode_AllMatched;
	else
		mbox_mode = NMbox::eViewMode_ShowMatch;

	last_search = (mMatchUsingGroup->GetSelectedItem() == eMatchUsingSearch);

	// Copy Not/Or/And
	match.SetNegate((mMatchIfGroup->GetSelectedItem() == eMatchNor) || (mMatchIfGroup->GetSelectedItem() == eMatchNand));
	match.SetUseOr((mMatchIfGroup->GetSelectedItem() == eMatchOr) || (mMatchIfGroup->GetSelectedItem() == eMatchNor));

	// Do all other bits
	new_bits[CMatchItem::eNew] = mNew->IsChecked();
	new_bits[CMatchItem::eUnseen] = mUnseen->IsChecked();
	new_bits[CMatchItem::eFlagged] = mImportant->IsChecked();
	new_bits[CMatchItem::eAnswered] = mAnswered->IsChecked();
	new_bits[CMatchItem::eDeleted] = mDeleted->IsChecked();
	new_bits[CMatchItem::eDraft] = mDraft->IsChecked();
	new_bits[CMatchItem::eSentToday] = mSentToday->IsChecked();
	new_bits[CMatchItem::eSentSinceYesterday] = mSentSinceYesterday->IsChecked();
	new_bits[CMatchItem::eSentThisWeek] = mSentThisWeek->IsChecked();
	new_bits[CMatchItem::eSentWithin7Days] = mSentWithin7Days->IsChecked();
	new_bits[CMatchItem::eSentByMe] = mSentByMe->IsChecked();
	new_bits[CMatchItem::eSentToMe] = mSentToMe->IsChecked();
	new_bits[CMatchItem::eSentToList] = mSentToList->IsChecked();
	new_bits[CMatchItem::eSelectedTo] = mSelectedTo->IsChecked();
	new_bits[CMatchItem::eSelectedFrom] = mSelectedFrom->IsChecked();
	new_bits[CMatchItem::eSelectedSmart] = mSelectedCorrespondents->IsChecked();
	new_bits[CMatchItem::eSelectedSubject] = mSelectedSubjects->IsChecked();
	new_bits[CMatchItem::eSelectedDate] = mSelectedDates->IsChecked();

	// Add all search styles
	boolvector selected;
	mTable->GetSelection(selected);
	unsigned long index = 0;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++, index++)
	{
		if (selected.at(index))
			sets.push_back((*iter)->GetName());
	}

	match.SetBitsSet(new_bits);
	match.SetSearchSet(sets);

	return match;
}

bool CMatchOptionsDialog::PoseDialog(NMbox::EViewMode& mbox_mode, CMatchItem& match, bool& last_search)
{
	bool result = false;

	// Create the dialog
	CMatchOptionsDialog* dlog = new CMatchOptionsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetMatchItems(mbox_mode, match, last_search);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		match = dlog->GetMatchItems(mbox_mode, last_search);
		result = true;
		dlog->Close();
	}
	
	return result;
}
