/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

// CMatchOptionsDialog.cp : implementation file
//

#include "CMatchOptionsDialog.h"

#include "CMatchItem.h"
#include "CPreferences.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CMatchOptionsDialog dialog


CMatchOptionsDialog::CMatchOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CMatchOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMatchOptionsDialog)
	mViewAllNoHilight = -1;
	mMatch = -1;
	mOr = -1;
	mNew = FALSE;
	mUnseen = FALSE;
	mImportant = FALSE;
	mAnswered = FALSE;
	mDeleted = FALSE;
	mDraft = FALSE;
	mSentToday = FALSE;
	mSentYesterday = FALSE;
	mSentThisWeek = FALSE;
	mSent7Days = FALSE;
	mSentByMe = FALSE;
	mSentToMe = FALSE;
	mSentToList = FALSE;
	mSelectedTo = FALSE;
	mSelectedFrom = FALSE;
	mSelectedSmart = FALSE;
	mSelectedSubject = FALSE;
	mSelectedDate = FALSE;
	//}}AFX_DATA_INIT
}


void CMatchOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMatchOptionsDialog)
	DDX_Radio(pDX, IDC_MATCHOPTIONS_VIEWALLNOHILIGHT, mViewAllNoHilight);
	DDX_Radio(pDX, IDC_MATCHOPTIONS_CRITERIA, mMatch);
	DDX_Radio(pDX, IDC_MATCHOPTIONS_OR, mOr);
	DDX_Check(pDX, IDC_MATCHOPTIONS_NEW, mNew);
	DDX_Check(pDX, IDC_MATCHOPTIONS_UNSEEN, mUnseen);
	DDX_Check(pDX, IDC_MATCHOPTIONS_IMPORTANT, mImportant);
	DDX_Check(pDX, IDC_MATCHOPTIONS_ANSWERED, mAnswered);
	DDX_Check(pDX, IDC_MATCHOPTIONS_DELETED, mDeleted);
	DDX_Check(pDX, IDC_MATCHOPTIONS_DRAFT, mDraft);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTTODAY, mSentToday);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTYESTERDAY, mSentYesterday);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTWEEK, mSentThisWeek);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENT7DAYS, mSent7Days);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTBYME, mSentByMe);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTTOME, mSentToMe);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SENTTOLIST, mSentToList);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SELECTEDTO, mSelectedTo);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SELECTEDFROM, mSelectedFrom);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SELECTEDSMART, mSelectedSmart);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SELECTEDSUBJECT, mSelectedSubject);
	DDX_Check(pDX, IDC_MATCHOPTIONS_SELECTEDDATE, mSelectedDate);
	//}}AFX_DATA_MAP
	
	// Must grab the checkbox list info here before the control is destroyed
	if (pDX->m_bSaveAndValidate)
	{
		unsigned long index = 0;
		for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
				iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++, index++)
		{
			mSelected[index] = mTable.GetCheck(index);
		}
	}
	else
	{
		mTable.ResetContent();
		unsigned long index = 0;
		for(cdstrvect::const_iterator iter = mNames.begin(); iter != mNames.end(); iter++, index++)
		{
			mTable.AddString((*iter).win_str());
			mTable.SetCheck(index, mSelected[index]);
		}
	}
}


BEGIN_MESSAGE_MAP(CMatchOptionsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CMatchOptionsDialog)
	ON_BN_CLICKED(IDC_MATCHOPTIONS_CLEAR, OnClear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMatchOptionsDialog::OnInitDialog() 
{
	// Subclass the control before init'ing
	mTable.SubclassDlgItem(IDC_MATCHOPTIONS_SEARCHSETS, this);
	
	CHelpDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CMatchOptionsDialog message handlers

void CMatchOptionsDialog::OnClear() 
{
	// Just set an empty spec
	CMatchItem empty;
	bool viewmatch = static_cast<CButton*>(GetDlgItem(IDC_MATCHOPTIONS_VIEWMATCH))->GetCheck();
	bool lastsearch = static_cast<CButton*>(GetDlgItem(IDC_MATCHOPTIONS_LASTSEARCH))->GetCheck();
	SetMatchItems(viewmatch ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All, empty, lastsearch);
	
	// Now force dialog update
	UpdateData(false);
}

void CMatchOptionsDialog::SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search)
{
	// View mode
	mViewAllNoHilight = mbox_mode;

	// Matcvh or search
	mMatch = last_search ? 1 : 0;

	// Copy Or/And/Nor/Nand
	mOr = (match.GetUseOr() ? 0 : 1) + (match.GetNegate() ? 2 : 0);

	// Do all other bits
	mNew = match.GetBitsSet()[CMatchItem::eNew];
	mUnseen = match.GetBitsSet()[CMatchItem::eUnseen];
	mImportant = match.GetBitsSet()[CMatchItem::eFlagged];
	mAnswered = match.GetBitsSet()[CMatchItem::eAnswered];
	mDeleted = match.GetBitsSet()[CMatchItem::eDeleted];
	mDraft = match.GetBitsSet()[CMatchItem::eDraft];
	mSentToday = match.GetBitsSet()[CMatchItem::eSentToday];
	mSentYesterday = match.GetBitsSet()[CMatchItem::eSentSinceYesterday];
	mSentThisWeek = match.GetBitsSet()[CMatchItem::eSentThisWeek];
	mSent7Days = match.GetBitsSet()[CMatchItem::eSentWithin7Days];
	mSentByMe = match.GetBitsSet()[CMatchItem::eSentByMe];
	mSentToMe = match.GetBitsSet()[CMatchItem::eSentToMe];
	mSentToList = match.GetBitsSet()[CMatchItem::eSentToList];
	mSelectedTo = match.GetBitsSet()[CMatchItem::eSelectedTo];
	mSelectedFrom = match.GetBitsSet()[CMatchItem::eSelectedFrom];
	mSelectedSmart = match.GetBitsSet()[CMatchItem::eSelectedSmart];
	mSelectedSubject = match.GetBitsSet()[CMatchItem::eSelectedSubject];
	mSelectedDate = match.GetBitsSet()[CMatchItem::eSelectedDate];

	// Add all search styles - this may get called more than once
	// so clear out any existing data first
	mNames.clear();
	mSelected.clear();
	const cdstrvect& sets = match.GetSearchSet();
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
	{
		mNames.push_back((*iter)->GetName());
		bool select =  (std::find(sets.begin(), sets.end(), (*iter)->GetName()) != sets.end());
		mSelected.push_back(select);
	}
}

CMatchItem CMatchOptionsDialog::GetMatchItems(NMbox::EViewMode& mbox_mode, bool& last_search) const
{
	// Create new match item
	CMatchItem match;

	// Create appropriate store
	boolvector new_bits(CMatchItem::eMatchLast + 1);
	cdstrvect sets;

	// View mode
	if (mViewAllNoHilight == 0)
		mbox_mode = NMbox::eViewMode_All;
	else if (mViewAllNoHilight == 1)
		mbox_mode = NMbox::eViewMode_AllMatched;
	else
		mbox_mode = NMbox::eViewMode_ShowMatch;

	// Check search state
	last_search = (mMatch == 1);

	// Copy Not/Or/And
	match.SetNegate(mOr >= 2);
	match.SetUseOr((mOr == 0) || (mOr == 2));

	// Do all other bits
	new_bits[CMatchItem::eNew] = mNew;
	new_bits[CMatchItem::eUnseen] = mUnseen;
	new_bits[CMatchItem::eFlagged] = mImportant;
	new_bits[CMatchItem::eAnswered] = mAnswered;
	new_bits[CMatchItem::eDeleted] = mDeleted;
	new_bits[CMatchItem::eDraft] = mDraft;
	new_bits[CMatchItem::eSentToday] = mSentToday;
	new_bits[CMatchItem::eSentSinceYesterday] = mSentYesterday;
	new_bits[CMatchItem::eSentThisWeek] = mSentThisWeek;
	new_bits[CMatchItem::eSentWithin7Days] = mSent7Days;
	new_bits[CMatchItem::eSentByMe] = mSentByMe;
	new_bits[CMatchItem::eSentToMe] = mSentToMe;
	new_bits[CMatchItem::eSentToList] = mSentToList;
	new_bits[CMatchItem::eSelectedTo] = mSelectedTo;
	new_bits[CMatchItem::eSelectedFrom] = mSelectedFrom;
	new_bits[CMatchItem::eSelectedSmart] = mSelectedSmart;
	new_bits[CMatchItem::eSelectedSubject] = mSelectedSubject;
	new_bits[CMatchItem::eSelectedDate] = mSelectedDate;

	// Add all search styles
	unsigned long index = 0;
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++, index++)
	{
		if (mSelected[index])
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
	CMatchOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetMatchItems(mbox_mode, match, last_search);

	// Run modal loop and look for OK
	if (dlog.DoModal() == msg_OK)
	{
		match = dlog.GetMatchItems(mbox_mode, last_search);
		result = true;
	}
	
	return result;
}

