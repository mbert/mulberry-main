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


// CPrefsAddress.cpp : implementation file
//


#include "CPrefsAddress.h"

#include "CPreferences.h"
#include "CPrefsLocalOptions.h"
#include "CPrefsAddressExpansion.h"
#include "CPrefsAddressCapture.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddress property page

IMPLEMENT_DYNCREATE(CPrefsAddress, CPrefsPanel)

CPrefsAddress::CPrefsAddress() : CPrefsPanel(CPrefsAddress::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddress)
	mOptionKeyAddressDialog = -1;
	//}}AFX_DATA_INIT
}

CPrefsAddress::~CPrefsAddress()
{
}

void CPrefsAddress::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddress)
	DDX_Radio(pDX, IDC_ADDRESSALTDOWN, mOptionKeyAddressDialog);
	DDX_Control(pDX, IDC_ADDRESSALTDOWN, mAltDownCtrl);
	DDX_Control(pDX, IDC_ADDRESSALTUP, mAltUpCtrl);
	mTabs.DoDataExchange(pDX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddress, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsAddress)
	ON_BN_CLICKED(IDC_ADDRESSALTDOWN, OnAltDown)
	ON_BN_CLICKED(IDC_ADDRESSALTUP, OnAltUp)
	ON_NOTIFY(TCN_SELCHANGE, IDC_ADDRESS_TABS, OnSelChangeAddressTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsAddress::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mOptionKeyAddressDialog = !prefs->mOptionKeyAddressDialog.GetValue();
	mTabs.SetContent(mCopyPrefs);
}

// Get params from DDX
void CPrefsAddress::UpdatePrefs(CPreferences* prefs)
{
	prefs->mOptionKeyAddressDialog.SetValue(mOptionKeyAddressDialog == 0);
	mTabs.UpdateContent(mCopyPrefs);
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddress message handlers

void CPrefsAddress::InitControls(void)
{
	// Subclass buttons
	mTabs.SubclassDlgItem(IDC_ADDRESS_TABS, this);

	// Create tab panels
	CPrefsAddressExpansion* expansion = new CPrefsAddressExpansion;
	mTabs.AddPanel(expansion);
	CPrefsAddressCapture* capture = new CPrefsAddressCapture;
	mTabs.AddPanel(capture);
	CPrefsLocalOptions* local = new CPrefsLocalOptions;
	local->SetLocalAddress();
	mTabs.AddPanel(local);
}

void CPrefsAddress::SetControls(void)
{
	// Give data to tab panels
	mTabs.SetContent(mCopyPrefs);
	mTabs.SetPanel(0);
}

void CPrefsAddress::OnAltDown()
{
	if (!mAltDownCtrl.GetCheck())
	{
		mAltUpCtrl.SetCheck(0);
		mAltDownCtrl.SetCheck(1);
	}
}

void CPrefsAddress::OnAltUp()
{
	if (!mAltUpCtrl.GetCheck())
	{
		mAltDownCtrl.SetCheck(0);
		mAltUpCtrl.SetCheck(1);
	}
}

void CPrefsAddress::OnSelChangeAddressTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}


