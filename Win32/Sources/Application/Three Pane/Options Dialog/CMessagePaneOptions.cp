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


// Source for CMessagePaneOptions class

#include "CMessagePaneOptions.h"

#include "CMailViewOptions.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMessagePaneOptions property page

IMPLEMENT_DYNCREATE(CMessagePaneOptions, CCommonViewOptions)

CMessagePaneOptions::CMessagePaneOptions() : CCommonViewOptions(CMessagePaneOptions::IDD)
{
	//{{AFX_DATA_INIT(CMessagePaneOptions)
	mMessageAddress = FALSE;
	mMessageSummary = FALSE;
	mMessageParts = FALSE;
	//}}AFX_DATA_INIT
}

CMessagePaneOptions::~CMessagePaneOptions()
{
}

void CMessagePaneOptions::DoDataExchange(CDataExchange* pDX)
{
	CCommonViewOptions::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessagePaneOptions)
	DDX_Check(pDX, IDC_3PANE_MAILOPTIONS_USETABS, mUseTabs);
	DDX_Check(pDX, IDC_3PANE_MAILOPTIONS_RESTORETABS, mRestoreTabs);
	DDX_Check(pDX, IDC_3PANE_MAILOPTIONS_SHOWADDRESS, mMessageAddress);
	DDX_Check(pDX, IDC_3PANE_MAILOPTIONS_SHOWSUMMARY, mMessageSummary);
	DDX_Check(pDX, IDC_3PANE_MAILOPTIONS_SHOWPARTS, mMessageParts);
	DDX_Radio(pDX, IDC_3PANE_MAILOPTIONS_MARKSEEN, mMarkSeen);
	DDX_UTF8Text(pDX, IDC_3PANE_MAILOPTIONS_DELAYSECS, mMarkSeenDelay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessagePaneOptions, CCommonViewOptions)
	//{{AFX_MSG_MAP(CMessagePaneOptions)
	ON_BN_CLICKED(IDC_3PANE_MAILOPTIONS_USETABS, OnUseTabs)
	ON_BN_CLICKED(IDC_3PANE_MAILOPTIONS_MARKSEEN, OnDelayDisable)
	ON_BN_CLICKED(IDC_3PANE_MAILOPTIONS_DELAYSEEN, OnDelayEnable)
	ON_BN_CLICKED(IDC_3PANE_MAILOPTIONS_NOMARKSEEN, OnDelayDisable)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMessagePaneOptions::OnInitDialog()
{
	BOOL result = CCommonViewOptions::OnInitDialog();

	// Disable items if appropriate
	if (!mUseTabs)
		OnUseTabs();
	if (mMarkSeen != 1)
		OnDelayDisable();
	
	return result;
}

void CMessagePaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CMailViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);

	mUseTabs = options.GetUseTabs();
	mRestoreTabs = options.GetRestoreTabs();

	mMessageAddress = options.GetShowAddressPane();
	mMessageSummary = options.GetShowSummary();
	mMessageParts = options.GetShowParts();
	
	mMarkSeen = options.GetPreviewFlagging();
	mMarkSeenDelay = options.GetPreviewDelay();
}

void CMessagePaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CMailViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);

	options.SetUseTabs(mUseTabs);
	options.SetRestoreTabs(mRestoreTabs);

	options.SetShowAddressPane(mMessageAddress);
	options.SetShowSummary(mMessageSummary);
	options.SetShowParts(mMessageParts);
	
	options.SetPreviewFlagging(static_cast<CMailViewOptions::EPreviewFlagging>(mMarkSeen));
	options.SetPreviewDelay(mMarkSeenDelay);
}

void CMessagePaneOptions::OnUseTabs()
{
	// TODO: Add your control notification handler code here
	CButton* cb = static_cast<CButton*>(GetDlgItem(IDC_3PANE_MAILOPTIONS_USETABS));
	GetDlgItem(IDC_3PANE_MAILOPTIONS_RESTORETABS)->EnableWindow(cb->GetCheck());
}

void CMessagePaneOptions::OnDelayEnable()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_3PANE_MAILOPTIONS_DELAYSECS)->EnableWindow(true);
	GetDlgItem(IDC_3PANE_MAILOPTIONS_DELAYSECS)->SetFocus();
}

void CMessagePaneOptions::OnDelayDisable()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_3PANE_MAILOPTIONS_DELAYSECS)->EnableWindow(false);
}

