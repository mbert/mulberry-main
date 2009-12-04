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

#include "CBalloonDialog.h"
#include "CCheckboxTable.h"
#include "CMatchItem.h"
#include "CMulberryApp.h"
#include "CPreferences.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMatchOptionsDialog::CMatchOptionsDialog()
{
}

// Constructor from stream
CMatchOptionsDialog::CMatchOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CMatchOptionsDialog::~CMatchOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMatchOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mViewAllNoHilight = (LRadioButton*) FindPaneByID(paneid_MatchOptionsViewAllNoHilight);
	mViewAllHilight = (LRadioButton*) FindPaneByID(paneid_MatchOptionsViewAllHilight);
	mViewMatch = (LRadioButton*) FindPaneByID(paneid_MatchOptionsViewMatch);
	mMatch = (LRadioButton*) FindPaneByID(paneid_MatchOptionsMatch);
	mLastSearch = (LRadioButton*) FindPaneByID(paneid_MatchOptionsLastSearch);
	mOr = (LRadioButton*) FindPaneByID(paneid_MatchOptionsOr);
	mAnd = (LRadioButton*) FindPaneByID(paneid_MatchOptionsAnd);
	mNor = (LRadioButton*) FindPaneByID(paneid_MatchOptionsNor);
	mNand = (LRadioButton*) FindPaneByID(paneid_MatchOptionsNand);
	mNew = (LCheckBox*) FindPaneByID(paneid_MatchOptionsNew);
	mUnseen = (LCheckBox*) FindPaneByID(paneid_MatchOptionsUnseen);
	mImportant = (LCheckBox*) FindPaneByID(paneid_MatchOptionsImportant);
	mAnswered = (LCheckBox*) FindPaneByID(paneid_MatchOptionsAnswered);
	mDeleted = (LCheckBox*) FindPaneByID(paneid_MatchOptionsDeleted);
	mDraft = (LCheckBox*) FindPaneByID(paneid_MatchOptionsDraft);
	mSentToday = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentToday);
	mSentSinceYesterday = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentSinceYesterday);
	mSentThisWeek = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentThisWeek);
	mSentWithin7Days = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentWithSevenDays);
	mSentByMe = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentByMe);
	mSentToMe = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentToMe);
	mSentToList = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSentToList);
	mSelectedTo = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSelectedTo);
	mSelectedFrom = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSelectedFrom);
	mSelectedCorrespondents = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSelectedCorrespondents);
	mSelectedSubjects = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSelectedSubject);
	mSelectedDates = (LCheckBox*) FindPaneByID(paneid_MatchOptionsSelectedDates);

	mTable = (CCheckboxTable*) FindPaneByID(paneid_MatchOptionsSets);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CMatchOptionsDialogBtns);

}

// Handle OK button
void CMatchOptionsDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_MatchOptionsClear:
		ClearItems();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CMatchOptionsDialog::ClearItems()
{
	// Just set an empty spec
	CMatchItem empty;
	SetMatchItems(mViewMatch->GetValue() ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All, empty, mLastSearch->GetValue());
}

void CMatchOptionsDialog::SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search)
{
	// View mode
	mViewAllNoHilight->SetValue(mbox_mode == NMbox::eViewMode_All);
	mViewAllHilight->SetValue(mbox_mode == NMbox::eViewMode_AllMatched);
	mViewMatch->SetValue(mbox_mode == NMbox::eViewMode_ShowMatch);

	// Matcvh or search
	mMatch->SetValue(!last_search);
	mLastSearch->SetValue(last_search);

	// Copy Or/And/Nor/Nand
	if (match.GetNegate())
	{
		if (match.GetUseOr())
			mNor->SetValue(1);
		else
			mNand->SetValue(1);
	}
	else
	{
		if (match.GetUseOr())
			mOr->SetValue(1);
		else
			mAnd->SetValue(1);
	}

	// Do all other bits
	mNew->SetValue(match.GetBitsSet()[CMatchItem::eNew]);
	mUnseen->SetValue(match.GetBitsSet()[CMatchItem::eUnseen]);
	mImportant->SetValue(match.GetBitsSet()[CMatchItem::eFlagged]);
	mAnswered->SetValue(match.GetBitsSet()[CMatchItem::eAnswered]);
	mDeleted->SetValue(match.GetBitsSet()[CMatchItem::eDeleted]);
	mDraft->SetValue(match.GetBitsSet()[CMatchItem::eDraft]);
	mSentToday->SetValue(match.GetBitsSet()[CMatchItem::eSentToday]);
	mSentSinceYesterday->SetValue(match.GetBitsSet()[CMatchItem::eSentSinceYesterday]);
	mSentThisWeek->SetValue(match.GetBitsSet()[CMatchItem::eSentThisWeek]);
	mSentWithin7Days->SetValue(match.GetBitsSet()[CMatchItem::eSentWithin7Days]);
	mSentByMe->SetValue(match.GetBitsSet()[CMatchItem::eSentByMe]);
	mSentToMe->SetValue(match.GetBitsSet()[CMatchItem::eSentToMe]);
	mSentToList->SetValue(match.GetBitsSet()[CMatchItem::eSentToList]);
	mSelectedTo->SetValue(match.GetBitsSet()[CMatchItem::eSelectedTo]);
	mSelectedFrom->SetValue(match.GetBitsSet()[CMatchItem::eSelectedFrom]);
	mSelectedCorrespondents->SetValue(match.GetBitsSet()[CMatchItem::eSelectedSmart]);
	mSelectedSubjects->SetValue(match.GetBitsSet()[CMatchItem::eSelectedSubject]);
	mSelectedDates->SetValue(match.GetBitsSet()[CMatchItem::eSelectedDate]);

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
	if (mViewAllNoHilight->GetValue())
		mbox_mode = NMbox::eViewMode_All;
	else if (mViewAllHilight->GetValue())
		mbox_mode = NMbox::eViewMode_AllMatched;
	else
		mbox_mode = NMbox::eViewMode_ShowMatch;

	// Check search state
	last_search = mLastSearch->GetValue();

	// Copy Not/Or/And
	match.SetNegate(mNor->GetValue() || mNand->GetValue());
	match.SetUseOr(mOr->GetValue() || mNor->GetValue());

	// Do all other bits
	new_bits[CMatchItem::eNew] = mNew->GetValue();
	new_bits[CMatchItem::eUnseen] = mUnseen->GetValue();
	new_bits[CMatchItem::eFlagged] = mImportant->GetValue();
	new_bits[CMatchItem::eAnswered] = mAnswered->GetValue();
	new_bits[CMatchItem::eDeleted] = mDeleted->GetValue();
	new_bits[CMatchItem::eDraft] = mDraft->GetValue();
	new_bits[CMatchItem::eSentToday] = mSentToday->GetValue();
	new_bits[CMatchItem::eSentSinceYesterday] = mSentSinceYesterday->GetValue();
	new_bits[CMatchItem::eSentThisWeek] = mSentThisWeek->GetValue();
	new_bits[CMatchItem::eSentWithin7Days] = mSentWithin7Days->GetValue();
	new_bits[CMatchItem::eSentByMe] = mSentByMe->GetValue();
	new_bits[CMatchItem::eSentToMe] = mSentToMe->GetValue();
	new_bits[CMatchItem::eSentToList] = mSentToList->GetValue();
	new_bits[CMatchItem::eSelectedTo] = mSelectedTo->GetValue();
	new_bits[CMatchItem::eSelectedFrom] = mSelectedFrom->GetValue();
	new_bits[CMatchItem::eSelectedSmart] = mSelectedCorrespondents->GetValue();
	new_bits[CMatchItem::eSelectedSubject] = mSelectedSubjects->GetValue();
	new_bits[CMatchItem::eSelectedDate] = mSelectedDates->GetValue();

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
	CBalloonDialog theHandler(paneid_MatchOptionsDialog, CMulberryApp::sApp);
	CMatchOptionsDialog* dlog = static_cast<CMatchOptionsDialog*>(theHandler.GetDialog());
	dlog->SetMatchItems(mbox_mode, match, last_search);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();
		if (hitMessage == msg_OK)
		{
			match = dlog->GetMatchItems(mbox_mode, last_search);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
