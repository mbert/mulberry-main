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


// Header for CCommonViewOptions class

#include "CCommonViewOptions.h"

#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCommonViewOptions property page

IMPLEMENT_DYNCREATE(CCommonViewOptions, CHelpPropertyPage)

CCommonViewOptions::CCommonViewOptions()
{
}

CCommonViewOptions::CCommonViewOptions(UINT nID) : CHelpPropertyPage(nID)
{
}

CCommonViewOptions::~CCommonViewOptions()
{
}

void CCommonViewOptions::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCommonViewOptions)
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN1, mListPreview.mSelect);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN2, mListPreview.mSingleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN3, mListPreview.mDoubleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN4, mListPreview.mUseKey);
	DDX_UTF8Text(pDX, IDC_3PANE_OPTIONS_KEY1, mListPreview.mKey);
	DDX_Control(pDX, IDC_3PANE_OPTIONS_KEY1, mListPreview.mKeyCtrl);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN5, mListFullView.mSelect);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN6, mListFullView.mSingleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN7, mListFullView.mDoubleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN8, mListFullView.mUseKey);
	DDX_UTF8Text(pDX, IDC_3PANE_OPTIONS_KEY2, mListFullView.mKey);
	DDX_Control(pDX, IDC_3PANE_OPTIONS_KEY2, mListFullView.mKeyCtrl);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN9, mItemsPreview.mSelect);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN10, mItemsPreview.mSingleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN11, mItemsPreview.mDoubleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN12, mItemsPreview.mUseKey);
	DDX_UTF8Text(pDX, IDC_3PANE_OPTIONS_KEY3, mItemsPreview.mKey);
	DDX_Control(pDX, IDC_3PANE_OPTIONS_KEY3, mItemsPreview.mKeyCtrl);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN13, mItemsFullView.mSelect);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN14, mItemsFullView.mSingleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN15, mItemsFullView.mDoubleClick);
	DDX_Check(pDX, IDC_3PANE_OPTIONS_BTN16, mItemsFullView.mUseKey);
	DDX_UTF8Text(pDX, IDC_3PANE_OPTIONS_KEY4, mItemsFullView.mKey);
	DDX_Control(pDX, IDC_3PANE_OPTIONS_KEY4, mItemsFullView.mKeyCtrl);
	//}}AFX_DATA_MAP
	
	// Disable items if 3-pane
	if (!pDX->m_bSaveAndValidate && !mIs3Pane)
		DisableItems();
}


BEGIN_MESSAGE_MAP(CCommonViewOptions, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CCommonViewOptions)
	ON_BN_CLICKED(IDC_3PANE_OPTIONS_CHOOSE1, OnChoose1)
	ON_BN_CLICKED(IDC_3PANE_OPTIONS_CHOOSE2, OnChoose2)
	ON_BN_CLICKED(IDC_3PANE_OPTIONS_CHOOSE3, OnChoose3)
	ON_BN_CLICKED(IDC_3PANE_OPTIONS_CHOOSE4, OnChoose4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCommonViewOptions::SetCommonViewData(const CUserAction& listPreview,
											const CUserAction& listFullView,
											const CUserAction& itemsPreview,
											const CUserAction& itemsFullView,
											bool is3pane)
{
	mListPreview.SetData(listPreview);
	mListFullView.SetData(listFullView);
	mItemsPreview.SetData(itemsPreview);
	mItemsFullView.SetData(itemsFullView);
	
	mIs3Pane = is3pane;
}

void CCommonViewOptions::GetCommonViewData(CUserAction& listPreview,
											CUserAction& listFullView,
											CUserAction& itemsPreview,
											CUserAction& itemsFullView)
{
	mListPreview.GetData(listPreview);
	mListFullView.GetData(listFullView);
	mItemsPreview.GetData(itemsPreview);
	mItemsFullView.GetData(itemsFullView);
}

void CCommonViewOptions::DisableItems()
{
	GetDlgItem(IDC_3PANE_OPTIONS_BTN1)->EnableWindow(false);
	GetDlgItem(IDC_3PANE_OPTIONS_BTN2)->EnableWindow(false);
	GetDlgItem(IDC_3PANE_OPTIONS_BTN3)->EnableWindow(false);
	GetDlgItem(IDC_3PANE_OPTIONS_BTN4)->EnableWindow(false);
	GetDlgItem(IDC_3PANE_OPTIONS_KEY1)->EnableWindow(false);
	GetDlgItem(IDC_3PANE_OPTIONS_CHOOSE1)->EnableWindow(false);
}


/////////////////////////////////////////////////////////////////////////////
// CCommonViewOptions message handlers

void CCommonViewOptions::OnChoose1() 
{
	mListPreview.GetKey();
}

void CCommonViewOptions::OnChoose2() 
{
	mListFullView.GetKey();
}

void CCommonViewOptions::OnChoose3() 
{
	mItemsPreview.GetKey();
}

void CCommonViewOptions::OnChoose4() 
{
	mItemsFullView.GetKey();
}
