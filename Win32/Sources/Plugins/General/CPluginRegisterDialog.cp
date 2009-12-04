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

// CPluginRegisterDialog.cpp : implementation file
//

#include "CPluginRegisterDialog.h"

#include "CPlugin.h"
#include "CUnicodeUtils.h"
#include "CUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog dialog


CPluginRegisterDialog::CPluginRegisterDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CPluginRegisterDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPluginRegisterDialog)
	//}}AFX_DATA_INIT
}


void CPluginRegisterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPluginRegisterDialog)
	DDX_Control(pDX, IDOK, mRegisterBtn);
	DDX_Control(pDX, IDCANCEL, mCancelBtn);
	DDX_Control(pDX, IDC_PLUGINREGISTER_REMOVE, mRemoveBtn);
	DDX_Control(pDX, IDC_PLUGINREGISTER_RUNDEMO, mRunDemoBtn);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_NAME, mName);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_VERSION, mVersion);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_TYPE, mType);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_MANUFACTURER, mManufacturer);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_DESCRIPTION, mDescription);
	DDX_UTF8Text(pDX, IDC_PLUGINREGISTER_REGKEY, mRegKey);
	DDV_UTF8MaxChars(pDX, mRegKey, 8);
	DDX_Control(pDX, IDC_PLUGINREGISTER_REGKEY, mRegKeyCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPluginRegisterDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CPluginRegisterDialog)
	ON_COMMAND(IDC_PLUGINREGISTER_REMOVE, OnRemove)
	ON_COMMAND(IDC_PLUGINREGISTER_RUNDEMO, OnRunAsDemo)
	ON_EN_CHANGE(IDC_PLUGINREGISTER_REGKEY, OnChangeEntry)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog message handlers

void CPluginRegisterDialog::SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete)
{
	mName = plugin.GetName();
	mVersion = ::GetVersionText(plugin.GetVersion());
	mType = rsrc::GetIndexedString("Alerts::Plugins::Type", plugin.GetType());
	mManufacturer = plugin.GetManufacturer();
	mDescription = plugin.GetDescription();

	mAllowDemo = allow_demo;
	mAllowDelete = allow_delete;
}

// Called during startup
BOOL CPluginRegisterDialog::OnInitDialog(void)
{
	CDialog::OnInitDialog();

	// Prevent demo button if required
	mRunDemoBtn.ShowWindow(mAllowDemo ? SW_SHOW : SW_HIDE);
	mRemoveBtn.EnableWindow(mAllowDemo);
	
	mCancelBtn.ShowWindow(mAllowDelete ? SW_HIDE : SW_SHOW);
	mCancelBtn.EnableWindow(!mAllowDelete);
	mRemoveBtn.ShowWindow(mAllowDelete ? SW_SHOW : SW_HIDE);
	mRemoveBtn.EnableWindow(mAllowDelete);

	return TRUE;
}

void CPluginRegisterDialog::OnRemove()
{
	EndDialog(IDC_PLUGINREGISTER_REMOVE);
}

void CPluginRegisterDialog::OnRunAsDemo()
{
	EndDialog(IDC_PLUGINREGISTER_RUNDEMO);
}

void CPluginRegisterDialog::OnChangeEntry()
{
	cdstring reg_key = CUnicodeUtils::GetWindowTextUTF8(&mRegKeyCtrl);

	if (reg_key.empty())
		mRegisterBtn.EnableWindow(false);
	else
		mRegisterBtn.EnableWindow(true);

}
