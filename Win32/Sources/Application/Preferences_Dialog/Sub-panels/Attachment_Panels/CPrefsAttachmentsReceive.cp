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


// CPrefsAttachmentsReceive.cpp : implementation file
//


#include "CPrefsAttachmentsReceive.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditMappings.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsReceive property page

IMPLEMENT_DYNCREATE(CPrefsAttachmentsReceive, CTabPanel)

CPrefsAttachmentsReceive::CPrefsAttachmentsReceive() : CTabPanel(CPrefsAttachmentsReceive::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAttachmentsReceive)
	mViewDoubleClick = -1;
	mAskDownload = -1;
	mAppLaunch = -1;
	mLaunchText = FALSE;
	//}}AFX_DATA_INIT
}

CPrefsAttachmentsReceive::~CPrefsAttachmentsReceive()
{
	// Must unsubclass buttons
	mExplicitMappingBtnCtrl.Detach();
}

void CPrefsAttachmentsReceive::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAttachmentsReceive)
	DDX_Control(pDX, IDC_DOWNLOADDIR, mDefaultDownloadCtrl);
	DDX_Control(pDX, IDC_CHOOSEDOWNLOADDIR, mDefaultChooseBtnCtrl);

	DDX_Radio(pDX, IDC_VIEWDOUBLECLICK, mViewDoubleClick);
	DDX_Radio(pDX, IDC_ASKDOWNLOAD, mAskDownload);
	DDX_UTF8Text(pDX, IDC_DOWNLOADDIR, mDefaultDownload);
	DDX_Radio(pDX, IDC_ALWAYSLAUNCH, mAppLaunch);
	DDX_Check(pDX, IDC_LAUNCHTEXT, mLaunchText);
	DDX_UTF8Text(pDX, IDC_SHELLLAUNCH, mShellLaunch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAttachmentsReceive, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAttachmentsReceive)
	ON_BN_CLICKED(IDC_CHOOSEDOWNLOADDIR, OnChooseDefaultDownload)
	ON_BN_CLICKED(IDC_EXPLICITMAPPING, OnExplicitMapping)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsAttachmentsReceive::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	mViewDoubleClick = !copyPrefs->mViewDoubleClick.GetValue();
	mAskDownload = !copyPrefs->mAskDownload.GetValue();
	mDefaultDownload = copyPrefs->mDefaultDownload.GetValue();
	mAppLaunch = copyPrefs->mAppLaunch.GetValue();
	mLaunchText = copyPrefs->mLaunchText.GetValue();
	mShellLaunch = copyPrefs->mShellLaunch.GetValue();
	
	mMappings = copyPrefs->mMIMEMappings.GetValue();
	mMapChange = false;

	// Subclass buttons
	mExplicitMappingBtnCtrl.SubclassDlgItem(IDC_EXPLICITMAPPING, this, IDI_EXPLICITMAPPING);
}

// Get params from DDX
bool CPrefsAttachmentsReceive::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	copyPrefs->mViewDoubleClick.SetValue(!mViewDoubleClick);
	copyPrefs->mAskDownload.SetValue(!mAskDownload);
	copyPrefs->mDefaultDownload.SetValue(mDefaultDownload);
	copyPrefs->mAppLaunch.SetValue((EAppLaunch) mAppLaunch);
	copyPrefs->mLaunchText.SetValue(mLaunchText);
	if (mMapChange)
		copyPrefs->mMIMEMappings.SetValue(mMappings);
	copyPrefs->mShellLaunch.SetValue(mShellLaunch);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsReceive message handlers

void CPrefsAttachmentsReceive::OnChooseDefaultDownload()
{
	cdstring temp;
	if (BrowseForFolder("Choose a Download directory", temp, this))
		CUnicodeUtils::SetWindowTextUTF8(&mDefaultDownloadCtrl, temp);
}

void CPrefsAttachmentsReceive::OnExplicitMapping()
{
	// Local copy for modification
	CMIMEMapVector change = mMappings;

	// Create dialog
	CPrefsEditMappings dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetMap(&change);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		mMapChange = true;
		mMappings = change;
	}
}
