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


// CPrefsMailboxAdvanced.cpp : implementation file
//

#include "CPrefsMailboxAdvanced.h"

#include "CCopyToMenu.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxAdvanced dialog

IMPLEMENT_DYNAMIC(CPrefsMailboxAdvanced, CTabPanel)

CPrefsMailboxAdvanced::CPrefsMailboxAdvanced()
	: CTabPanel(CPrefsMailboxAdvanced::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailboxAdvanced)
	mNewMessage = -1;

	mUseCopyToCabinet = FALSE;
	mUseAppendToCabinet = FALSE;
	mMaximumRecent = -1;

	mRLoCache = 0;
	mRNoHiCache = -1;
	mRHiCache = 0;
	mRAutoIncrement = -1;
	mRCacheIncrement = 0;
	mRSortCache = 0;

	mLLoCache = 0;
	mLNoHiCache = -1;
	mLHiCache = 0;
	mLAutoIncrement = -1;
	mLCacheIncrement = 0;
	mLSortCache = 0;
	//}}AFX_DATA_INIT
}


void CPrefsMailboxAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailboxAdvanced)
	DDX_Control(pDX, IDC_RHICACHE, mRHiCacheCtrl);
	DDX_Control(pDX, IDC_RCACHEINCREMENT, mRCacheIncrementCtrl);
	DDX_Control(pDX, IDC_LHICACHE, mLHiCacheCtrl);
	DDX_Control(pDX, IDC_LCACHEINCREMENT, mLCacheIncrementCtrl);

	DDX_Radio(pDX, IDC_UNSEENNEW, mNewMessage);

	DDX_Check(pDX, IDC_USECOPYTOCABINET, mUseCopyToCabinet);
	DDX_Control(pDX, IDC_USECOPYTOCABINET, mUseCopyToCabinetCtrl);
	DDX_Check(pDX, IDC_USEAPPENDTOCABINET, mUseAppendToCabinet);
	DDX_Control(pDX, IDC_USEAPPENDTOCABINET, mUseAppendToCabinetCtrl);
	DDX_UTF8Text(pDX, IDC_MAXIMUMRECENT, mMaximumRecent);
	DDV_MinMaxInt(pDX, mMaximumRecent, 0, 32767);

	DDX_UTF8Text(pDX, IDC_RLOCACHE, mRLoCache);
	DDV_MinMaxInt(pDX, mRLoCache, 0, 32767);
	DDX_Radio(pDX, IDC_RNOCACHELIMIT, mRNoHiCache);
	DDX_UTF8Text(pDX, IDC_RHICACHE, mRHiCache);
	DDV_MinMaxInt(pDX, mRHiCache, 0, 32767);
	DDX_Radio(pDX, IDC_RAUTOCACHEINCREMENT, mRAutoIncrement);
	DDX_UTF8Text(pDX, IDC_RCACHEINCREMENT, mRCacheIncrement);
	DDV_MinMaxInt(pDX, mRCacheIncrement, 0, 32767);
	DDX_UTF8Text(pDX, IDC_RSORTCACHE, mRSortCache);
	DDV_MinMaxInt(pDX, mRSortCache, 0, 32767);

	DDX_UTF8Text(pDX, IDC_LLOCACHE, mLLoCache);
	DDV_MinMaxInt(pDX, mLLoCache, 0, 32767);
	DDX_Radio(pDX, IDC_LNOCACHELIMIT, mLNoHiCache);
	DDX_UTF8Text(pDX, IDC_LHICACHE, mLHiCache);
	DDV_MinMaxInt(pDX, mLHiCache, 0, 32767);
	DDX_Radio(pDX, IDC_LAUTOCACHEINCREMENT, mLAutoIncrement);
	DDX_UTF8Text(pDX, IDC_LCACHEINCREMENT, mLCacheIncrement);
	DDV_MinMaxInt(pDX, mLCacheIncrement, 0, 32767);
	DDX_UTF8Text(pDX, IDC_LSORTCACHE, mLSortCache);
	DDV_MinMaxInt(pDX, mLSortCache, 0, 32767);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailboxAdvanced, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMailboxAdvanced)
	ON_BN_CLICKED(IDC_USECOPYTOCABINET, OnForceMailboxListReset)
	ON_BN_CLICKED(IDC_USEAPPENDTOCABINET, OnForceMailboxListReset)

	ON_BN_CLICKED(IDC_RNOCACHELIMIT, OnRNoHiCache)
	ON_BN_CLICKED(IDC_RUSEHICACHE, OnRUseHiCache)
	ON_BN_CLICKED(IDC_RAUTOCACHEINCREMENT, OnRAutoIncrement)
	ON_BN_CLICKED(IDC_RINCREMENTBY, OnRUseCacheIncrement)

	ON_BN_CLICKED(IDC_LNOCACHELIMIT, OnLNoHiCache)
	ON_BN_CLICKED(IDC_LUSEHICACHE, OnLUseHiCache)
	ON_BN_CLICKED(IDC_LAUTOCACHEINCREMENT, OnLAutoIncrement)
	ON_BN_CLICKED(IDC_LINCREMENTBY, OnLUseCacheIncrement)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxAdvanced message handlers

BOOL CPrefsMailboxAdvanced::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsMailboxAdvanced::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	if (copyPrefs->mUnseenNew.GetValue() && !copyPrefs->mRecentNew.GetValue())
		mNewMessage = 0;
	else if (!copyPrefs->mUnseenNew.GetValue() && copyPrefs->mRecentNew.GetValue())
		mNewMessage = 1;
	else if (copyPrefs->mUnseenNew.GetValue() && copyPrefs->mRecentNew.GetValue())
		mNewMessage = 2;

	mUseCopyToCabinet = copyPrefs->mUseCopyToMboxes.GetValue();
	mUseAppendToCabinet = copyPrefs->mUseAppendToMboxes.GetValue();
	mMaximumRecent = copyPrefs->mMRUMaximum.GetValue();

	mRLoCache = copyPrefs->mRLoCache.GetValue();
	mRNoHiCache = copyPrefs->mRUseHiCache.GetValue();
	mRHiCache = copyPrefs->mRHiCache.GetValue();
	mRAutoIncrement = !copyPrefs->mRAutoCacheIncrement.GetValue();
	mRCacheIncrement = copyPrefs->mRCacheIncrement.GetValue();
	mRSortCache = copyPrefs->mRSortCache.GetValue();

	// set initial control states
	mRHiCacheCtrl.EnableWindow(mRNoHiCache);
	mRCacheIncrementCtrl.EnableWindow(mRAutoIncrement);

	mLLoCache = copyPrefs->mLLoCache.GetValue();
	mLNoHiCache = copyPrefs->mLUseHiCache.GetValue();
	mLHiCache = copyPrefs->mLHiCache.GetValue();
	mLAutoIncrement = !copyPrefs->mLAutoCacheIncrement.GetValue();
	mLCacheIncrement = copyPrefs->mLCacheIncrement.GetValue();
	mLSortCache = copyPrefs->mLSortCache.GetValue();

	// set initial control states
	mLHiCacheCtrl.EnableWindow(mLNoHiCache);
	mLCacheIncrementCtrl.EnableWindow(mLAutoIncrement);
}

// Force update of data
bool CPrefsMailboxAdvanced::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->mUnseenNew.SetValue((mNewMessage == 0) || (mNewMessage == 2));
	copyPrefs->mRecentNew.SetValue((mNewMessage == 1) || (mNewMessage == 2));

	copyPrefs->mUseCopyToMboxes.SetValue(mUseCopyToCabinet);
	copyPrefs->mUseAppendToMboxes.SetValue(mUseAppendToCabinet);
	copyPrefs->mMRUMaximum.SetValue(mMaximumRecent);

	copyPrefs->mRLoCache.SetValue(mRLoCache);
	copyPrefs->mRUseHiCache.SetValue(mRNoHiCache);
	copyPrefs->mRHiCache.SetValue(mRHiCache);
	copyPrefs->mRAutoCacheIncrement.SetValue(!mRAutoIncrement);
	copyPrefs->mRCacheIncrement.SetValue(mRCacheIncrement);
	copyPrefs->mRSortCache.SetValue(mRSortCache);

	copyPrefs->mLLoCache.SetValue(mLLoCache);
	copyPrefs->mLUseHiCache.SetValue(mLNoHiCache);
	copyPrefs->mLHiCache.SetValue(mLHiCache);
	copyPrefs->mLAutoCacheIncrement.SetValue(!mLAutoIncrement);
	copyPrefs->mLCacheIncrement.SetValue(mLCacheIncrement);
	copyPrefs->mLSortCache.SetValue(mLSortCache);
	
	return true;
}

void CPrefsMailboxAdvanced::OnForceMailboxListReset()
{
	// Must force reset of menu lists
	CCopyToMenu::SetMenuOptions(mUseCopyToCabinetCtrl.GetCheck(),
								mUseAppendToCabinetCtrl.GetCheck());
			
	
	// Make sure menus are rest when dialog is exited (OK or Cancel)
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetParentOwner();
	prefs_dlog->SetForceMenuListReset(true);
}

void CPrefsMailboxAdvanced::OnRNoHiCache()
{
	// TODO: Add your control notification handler code here
	mRHiCacheCtrl.EnableWindow(false);
}

void CPrefsMailboxAdvanced::OnRUseHiCache()
{
	// TODO: Add your control notification handler code here
	mRHiCacheCtrl.EnableWindow(true);
}

void CPrefsMailboxAdvanced::OnRAutoIncrement()
{
	// TODO: Add your control notification handler code here
	mRCacheIncrementCtrl.EnableWindow(false);
}

void CPrefsMailboxAdvanced::OnRUseCacheIncrement()
{
	// TODO: Add your control notification handler code here
	mRCacheIncrementCtrl.EnableWindow(true);
}

void CPrefsMailboxAdvanced::OnLNoHiCache()
{
	// TODO: Add your control notification handler code here
	mLHiCacheCtrl.EnableWindow(false);
}

void CPrefsMailboxAdvanced::OnLUseHiCache()
{
	// TODO: Add your control notification handler code here
	mLHiCacheCtrl.EnableWindow(true);
}

void CPrefsMailboxAdvanced::OnLAutoIncrement()
{
	// TODO: Add your control notification handler code here
	mLCacheIncrementCtrl.EnableWindow(false);
}

void CPrefsMailboxAdvanced::OnLUseCacheIncrement()
{
	// TODO: Add your control notification handler code here
	mLCacheIncrementCtrl.EnableWindow(true);
}
