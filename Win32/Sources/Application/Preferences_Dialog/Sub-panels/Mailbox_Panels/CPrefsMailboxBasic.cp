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


// CPrefsMailboxBasic.cpp : implementation file
//

#include "CPrefsMailboxBasic.h"

#include "CMbox.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxBasic dialog

IMPLEMENT_DYNAMIC(CPrefsMailboxBasic, CTabPanel)

CPrefsMailboxBasic::CPrefsMailboxBasic()
	: CTabPanel(CPrefsMailboxBasic::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailboxBasic)
	openAtFirst = -1;
	mOldestToNewest = -1;
	expungeOnClose = FALSE;
	warnOnExpunge = FALSE;

	mWarnPuntUnseen = FALSE;

	mDoRollover = FALSE;
	mRolloverWarn = FALSE;

	mDoMailboxClear = FALSE;
	clear_warning = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsMailboxBasic::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailboxBasic)
	DDX_Control(pDX, IDC_MOVEREAD, mDoMailboxClearCtrl);
	DDX_Control(pDX, IDC_WARNMOVE, mWarnMoveCtrl);
	DDX_Control(pDX, IDC_MOVEMAILBOX, mMoveMailboxCtrl);

	DDX_Radio(pDX, IDC_OPENOLDEST, openAtFirst);
	DDX_Check(pDX, IDC_NOOPENPREVIEW, mNoOpenPreview);
	DDX_Radio(pDX, IDC_OLDESTTONEWEST, mOldestToNewest);
	DDX_Check(pDX, IDC_EXPUNGECLOSE, expungeOnClose);
	DDX_Check(pDX, IDC_WARNEXPUNGE, warnOnExpunge);
	DDX_Check(pDX, IDC_WARNPUNTUNSEEN, mWarnPuntUnseen);
	DDX_Check(pDX, IDC_DOROLLOVER, mDoRollover);
	DDX_Check(pDX, IDC_ROLLOVERWARN, mRolloverWarn);
	DDX_Check(pDX, IDC_SCROLLFORUNSEEN, mScrollForUnseen);
	DDX_UTF8Text(pDX, IDC_MOVEMAILBOX, clear_mailbox);
	DDX_Check(pDX, IDC_MOVEREAD, mDoMailboxClear);
	DDX_Check(pDX, IDC_WARNMOVE, clear_warning);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailboxBasic, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMailboxBasic)
	ON_BN_CLICKED(IDC_MOVEREAD, OnMoveRead)
	ON_COMMAND_RANGE(IDM_CopyToPopupNone, IDM_CopyToPopupEnd, OnChangeMoveTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxBasic message handlers

BOOL CPrefsMailboxBasic::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mMoveToPopup.SubclassDlgItem(IDC_MOVETOPOPUP, this, IDI_POPUPBTN);
	mMoveToPopup.SetButtonText(false);
	mMoveToPopup.SetCopyTo(true);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsMailboxBasic::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	openAtFirst = (copyPrefs->openAtFirst.GetValue() ? 0 : (copyPrefs->openAtLast.GetValue() ? 1 : (copyPrefs->openAtFirstNew.GetValue() ? 2 : 0)));
	mNoOpenPreview = copyPrefs->mNoOpenPreview.GetValue();
	mOldestToNewest = copyPrefs->mNextIsNewest.GetValue() ? 0 : 1;
	expungeOnClose = copyPrefs->expungeOnClose.GetValue();
	warnOnExpunge = copyPrefs->warnOnExpunge.GetValue();
	mWarnPuntUnseen = copyPrefs->mWarnPuntUnseen.GetValue();
	mDoRollover = copyPrefs->mDoRollover.GetValue();
	mRolloverWarn = copyPrefs->mRolloverWarn.GetValue();
	mScrollForUnseen = copyPrefs->mScrollForUnseen.GetValue();

	mDoMailboxClear = copyPrefs->mDoMailboxClear.GetValue();
	clear_mailbox = copyPrefs->clear_mailbox.GetValue();
	clear_warning = copyPrefs->clear_warning.GetValue();

	// set initial control states
	mMoveMailboxCtrl.EnableWindow(mDoMailboxClear);
	mMoveToPopup.EnableWindow(mDoMailboxClear);
	mWarnMoveCtrl.EnableWindow(mDoMailboxClear);
}

// Force update of data
bool CPrefsMailboxBasic::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->openAtFirst.SetValue((openAtFirst == 0));
	copyPrefs->openAtLast.SetValue((openAtFirst == 1));
	copyPrefs->openAtFirstNew.SetValue((openAtFirst == 2));
	copyPrefs->mNoOpenPreview.SetValue(mNoOpenPreview);
	copyPrefs->mNextIsNewest.SetValue(mOldestToNewest == 0);
	copyPrefs->expungeOnClose.SetValue(expungeOnClose);
	copyPrefs->warnOnExpunge.SetValue(warnOnExpunge);
	copyPrefs->mWarnPuntUnseen.SetValue(mWarnPuntUnseen);
	copyPrefs->mDoRollover.SetValue(mDoRollover);
	copyPrefs->mRolloverWarn.SetValue(mRolloverWarn);
	copyPrefs->mScrollForUnseen.SetValue(mScrollForUnseen);
	copyPrefs->mDoMailboxClear.SetValue(mDoMailboxClear);
	copyPrefs->clear_mailbox.SetValue(clear_mailbox);
	copyPrefs->clear_warning.SetValue(clear_warning);
	
	return true;
}

void CPrefsMailboxBasic::OnMoveRead()
{
	// TODO: Add your control notification handler code here
	bool set = mDoMailboxClearCtrl.GetCheck();
	mWarnMoveCtrl.EnableWindow(set);
	mMoveMailboxCtrl.EnableWindow(set);
	mMoveToPopup.EnableWindow(set);
}

// Change move to
void CPrefsMailboxBasic::OnChangeMoveTo(UINT nID)
{
	if (nID == IDM_CopyToPopupNone)
		CUnicodeUtils::SetWindowTextUTF8(&mMoveMailboxCtrl, cdstring::null_str);
	else
	{
		// Must set/reset control value to ensure selected mailbox is returned but
		// popup does not display check mark
		mMoveToPopup.SetValue(nID);
		cdstring mbox_name;
		if (mMoveToPopup.GetSelectedMboxName(mbox_name, true))
			CUnicodeUtils::SetWindowTextUTF8(&mMoveMailboxCtrl, mbox_name);
		mMoveToPopup.SetValue(-1);
	}
}