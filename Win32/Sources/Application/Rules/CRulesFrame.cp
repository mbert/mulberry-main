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


// CRulesFrame.cpp : implementation of the CRulesFrame class
//


#include "CRulesFrame.h"

#include "CAdminLock.h"
#include "CDynamicMenu.h"
#include "CRulesWindow.h"

#pragma mark ____________________________

void CRulesFrame_InitToolTipMap(ulmap& map);
void CRulesFrame_InitToolTipMap(ulmap& map)
{
	map.insert(ulmap::value_type(IDC_RULESNEWBTN, IDC_RULESNEWBTN));
	map.insert(ulmap::value_type(IDC_RULESNEWTARGETBTN, IDC_RULESNEWTARGETBTN));
	map.insert(ulmap::value_type(IDC_RULESNEWSCRIPTSBTN, IDC_RULESNEWSCRIPTSBTN));
	map.insert(ulmap::value_type(IDC_RULESEDITBTN, IDC_RULESEDITBTN));
	map.insert(ulmap::value_type(IDC_RULESDELETEBTN, IDC_RULESDELETEBTN));
	map.insert(ulmap::value_type(IDC_RULESAPPLYBTN, IDC_RULESAPPLYBTN));
	map.insert(ulmap::value_type(IDC_RULESGENERATEBTN, IDC_RULESGENERATEBTN));
	map.insert(ulmap::value_type(IDC_RULESSHOWTRIGGERSBTN, IDC_RULESSHOWTRIGGERSBTN));
}

#pragma mark ____________________________CRulesFrameSDI
/////////////////////////////////////////////////////////////////////////////
// CRulesFrameSDI

IMPLEMENT_DYNCREATE(CRulesFrameSDI, CFrameWnd)

BEGIN_MESSAGE_MAP(CRulesFrameSDI, CSDIFrame)
	//{{AFX_MSG_MAP(CRulesFrameSDI)
		ON_WM_CREATE()
		ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulesFrameSDI message handlers

int CRulesFrameSDI::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CSDIFrame::OnCreate(lpCreateStruct);
	
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::CreateEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::CreateMailboxMenu(mbox_menu);
	
	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::CreateHelpMenu(help_menu);

	// Check for calendar and remove calendar menu items
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		GetMenu()->DeleteMenu(4, MF_BYPOSITION);
	}

	return 0;
}

void CRulesFrameSDI::OnDestroy(void)
{
	// Do not allow shared menu to be deleted
	CMenu* edit_menu = GetMenu()->GetSubMenu(1);
	if (edit_menu)
		CDynamicMenu::DestroyEditMenu(edit_menu);

	CMenu* mbox_menu = GetMenu()->GetSubMenu(2);
	if (mbox_menu)
		CDynamicMenu::DestroyMailboxMenu(mbox_menu);

	CMenu* help_menu = GetMenu()->GetSubMenu(6);
	if (help_menu)
		CDynamicMenu::DestroyHelpMenu(help_menu);

	CSDIFrame::OnDestroy();
}

void CRulesFrameSDI::InitToolTipMap()
{
	::CRulesFrame_InitToolTipMap(mToolTipMap);
}
