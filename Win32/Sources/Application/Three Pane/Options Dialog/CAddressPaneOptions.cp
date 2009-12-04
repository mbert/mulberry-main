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


// Source for CAddressPaneOptions class

#include "CAddressPaneOptions.h"

#include "CAddressViewOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressPaneOptions property page

IMPLEMENT_DYNCREATE(CAddressPaneOptions, CCommonViewOptions)

CAddressPaneOptions::CAddressPaneOptions() : CCommonViewOptions(CAddressPaneOptions::IDD)
{
	//{{AFX_DATA_INIT(CAddressPaneOptions)
	mAddressSelect = FALSE;
	//}}AFX_DATA_INIT
}

CAddressPaneOptions::~CAddressPaneOptions()
{
}

void CAddressPaneOptions::DoDataExchange(CDataExchange* pDX)
{
	CCommonViewOptions::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddressPaneOptions)
	DDX_Check(pDX, IDC_3PANE_ADDRESSOPTIONS_SELECT, mAddressSelect);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddressPaneOptions, CCommonViewOptions)
	//{{AFX_MSG_MAP(CAddressPaneOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAddressPaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CAddressViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);

	mAddressSelect = options.GetSelectAddressPane();
}

void CAddressPaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CAddressViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);

	options.SetSelectAddressPane(mAddressSelect);
}
