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

// CAboutPluginsDialog.cpp : implementation file
//

#include "CAboutPluginsDialog.h"

#include "CPluginManager.h"
#include "CUnicodeUtils.h"
#include "CUtils.h"
#include "CXStringResources.h"

#include <WIN_LTableMonoGeometry.h>
#include <WIN_LTableSingleSelector.h>

/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog dialog


CAboutPluginsDialog::CAboutPluginsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CAboutPluginsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAboutPluginsDialog)
	//}}AFX_DATA_INIT
}


void CAboutPluginsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutPluginsDialog)
	DDX_Control(pDX, IDC_ABOUTPLUGINS_REGISTER, mRegisterBtn);
	//DDX_Control(pDX, IDC_ABOUTPLUGINS_LIST, mList);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_NAME, mName);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_VERSION, mVersion);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_TYPE, mType);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_MANUFACTURER, mManufacturer);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_DESCRIPTION, mDescription);
	DDX_Control(pDX, IDC_ABOUTPLUGINS_MODE, mMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAboutPluginsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CAboutPluginsDialog)
	ON_BN_CLICKED(IDC_ABOUTPLUGINS_REGISTER, OnAboutPluginsRegister)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAboutPluginsDialog message handlers

// Called during startup
BOOL CAboutPluginsDialog::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mList.SubclassDlgItem(IDC_ABOUTPLUGINS_LIST, this);
	mList.SetDlog(this);

	// Create columns and adjust flag rect
	mList.InsertCols(1, 1);

	CRect client;
	mList.GetClientRect(client);
	int cx = client.Width() - 16;

	// List column has variable width
	mList.SetColWidth(cx, 1, 1);

	// Add types to list
	int parent = 0;
	const CPluginList& plugins = CPluginManager::sPluginManager.GetPlugins();

	for(int i = CPlugin::ePluginUnknown; i < CPlugin::ePluginLast; i++)
	{
		cdstring title = rsrc::GetIndexedString("Alerts::Plugins::Type", i);

		parent = mList.InsertSiblingRows(1, parent, NULL, 0, true);
		mList.SetCellString(parent, title);
		mHierPlugins.push_back(NULL);

		// Add suitable children
		for(CPluginList::const_iterator iter = plugins.begin(); iter != plugins.end(); iter++)
		{
			if ((*iter)->GetType() == (CPlugin::EPluginType) i)
			{
				int child = mList.AddLastChildRow(parent, NULL, 0);
				mList.SetCellString(child, (*iter)->GetName());
				mHierPlugins.push_back(*iter);
			}
		}
		
		// Always expanded
		mList.ExpandRow(parent);
	}

	return true;
}

void CAboutPluginsDialog::OnAboutPluginsRegister() 
{
	STableCell aCell(0, 0);
	if (mList.GetNextSelectedCell(aCell))
	{
		CPlugin* plugin = mHierPlugins.at(mList.GetWideOpenIndex(aCell.row) - 1);
		StLoadPlugin load(plugin);
		plugin->DoRegistration(false, false);
		ShowPluginInfo(plugin);
	}
}

void CAboutPluginsDialog::OnClickAboutPluginsList() 
{
	STableCell aCell(0, 0);
	if (mList.GetNextSelectedCell(aCell))
		ShowPluginInfo(mHierPlugins.at(mList.GetWideOpenIndex(aCell.row) - 1));
	else
		ShowPluginInfo(NULL);
}

void CAboutPluginsDialog::ShowPluginInfo(const CPlugin* plugin)
{
	if (plugin)
	{
		CUnicodeUtils::SetWindowTextUTF8(mName, plugin->GetName());
		CUnicodeUtils::SetWindowTextUTF8(mVersion, ::GetVersionText(plugin->GetVersion()));

		cdstring s = rsrc::GetIndexedString("Alerts::Plugins::Type", plugin->GetType());
		CUnicodeUtils::SetWindowTextUTF8(mType, s);
		CUnicodeUtils::SetWindowTextUTF8(mManufacturer, plugin->GetManufacturer());
		CUnicodeUtils::SetWindowTextUTF8(mDescription, plugin->GetDescription());
		
		if (plugin->IsRegistered())
			s = rsrc::GetString("Alerts::Plugins::ModeRegistered");
		else if (plugin->IsDemo())
			s = rsrc::GetString("Alerts::Plugins::ModeDemo");
		else
			s = rsrc::GetString("Alerts::Plugins::ModeUnregistered");
		CUnicodeUtils::SetWindowTextUTF8(mMode, s);
		
		if (plugin->IsDemo())
			mRegisterBtn.EnableWindow(true);
		else
			mRegisterBtn.EnableWindow(false);
	}
	else
	{
		CUnicodeUtils::SetWindowTextUTF8(mName, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mVersion, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mType, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mManufacturer, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mDescription, cdstring::null_str);
		CUnicodeUtils::SetWindowTextUTF8(mMode, cdstring::null_str);
		mRegisterBtn.EnableWindow(false);
	}
}

#pragma mark ____________________________CAboutPluginsTable

CAboutPluginsTable::CAboutPluginsTable()
{
	mDlog = NULL;
	SetTableGeometry(new LTableMonoGeometry(this, 175, 16));
	SetTableSelector(new LTableSingleSelector(this));
	
	mHierarchyCol = 1;
}

void CAboutPluginsTable::DoSelectionChanged()
{ 
	if (mDlog)
		mDlog->OnClickAboutPluginsList();
}
