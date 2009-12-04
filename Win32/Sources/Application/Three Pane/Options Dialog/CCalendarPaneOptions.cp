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


// Source for CCalendarPaneOptions class

#include "CCalendarPaneOptions.h"

#include "CCalendarViewOptions.h"

/////////////////////////////////////////////////////////////////////////////
// CCalendarPaneOptions property page

IMPLEMENT_DYNCREATE(CCalendarPaneOptions, CCommonViewOptions)

CCalendarPaneOptions::CCalendarPaneOptions() : CCommonViewOptions(CCalendarPaneOptions::IDD)
{
	//{{AFX_DATA_INIT(CCalendarPaneOptions)
	//}}AFX_DATA_INIT
}

CCalendarPaneOptions::~CCalendarPaneOptions()
{
}

void CCalendarPaneOptions::DoDataExchange(CDataExchange* pDX)
{
	CCommonViewOptions::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCalendarPaneOptions)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCalendarPaneOptions, CCommonViewOptions)
	//{{AFX_MSG_MAP(CCalendarPaneOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCalendarPaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CCalendarViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);
}

void CCalendarPaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CCalendarViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);
}
