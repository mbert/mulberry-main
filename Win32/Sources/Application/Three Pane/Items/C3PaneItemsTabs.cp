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


// Source for C3PaneItemsTabs class

#include "C3PaneItemsTabs.h"

#include "CMulberryCommon.h"
#include "C3PaneItems.h"

BEGIN_MESSAGE_MAP(C3PaneItemsTabs, CTabCtrl)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

int C3PaneItemsTabs::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create image list
	mImages = new CImageList;
	mImages->Create(16, 16, ILC_MASK, 1, 1);
	mImages->Add((HICON) ::LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_TAB_UNSEEN), RT_GROUP_ICON), MAKEINTRESOURCE(IDI_TAB_UNSEEN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
	SetImageList(mImages);

	return 0;
}

void C3PaneItemsTabs::MoveTab(unsigned long oldindex, unsigned long newindex)
{
	// Temporarily stop screen drawing
	StNoRedraw _noredraw(this);

	// Check current selected item
	unsigned long selected = GetCurSel();

	// Get tab details for the one being removed
	CString buffer;
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT | TCIF_IMAGE;
	tabs.pszText = buffer.GetBuffer(1024);
	tabs.cchTextMax = 1024;
	GetItem(oldindex, &tabs);
	buffer.ReleaseBuffer(-1);
	
	// Delete from old position and insert at new position
	DeleteItem(oldindex);
	InsertItem(newindex, &tabs);

	// Now change the value if it moved
	if ((selected >= min(oldindex, newindex)) && (selected <= max(oldindex, newindex)))
	{
		if (selected == oldindex)
			SetCurSel(newindex);
		else
			SetCurSel(selected - (newindex > oldindex ? 1 : -1));
	}
}

// Look for shift-click
void C3PaneItemsTabs::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (nFlags & MK_SHIFT)
	{
		TC_HITTESTINFO info;
		info.pt = point;
		info.flags = 0;
		mLastClicked = HitTest(&info);
		if (mLastClicked >= 0)
			GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
	}
	else
		CTabCtrl::OnLButtonDown(nFlags, point);
}

// Do context menu
void C3PaneItemsTabs::OnRButtonDown(UINT nFlags, CPoint point)
{
	TC_HITTESTINFO info;
	info.pt = point;
	info.flags = 0;
	mLastClicked = HitTest(&info);
	bool in_button = (mLastClicked >= 0) && IsActive();

	CMenu popup;
	CPoint global = point;

	ClientToScreen(&global);

	if (popup.LoadMenu(IDR_POPUP_3PANE_ITEMSTAB))
	{
		// Track the popup
		CMenu* pPopup = popup.GetSubMenu(0);
		
		// Enable/disable items
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_MOVELEFT, ((in_button && (mLastClicked > 0)) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_MOVERIGHT, ((in_button && (mLastClicked < GetItemCount() - 1)) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_MOVESTART, ((in_button && (mLastClicked > 0)) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_MOVEEND, ((in_button && (mLastClicked < GetItemCount() - 1)) ? MF_ENABLED : MF_GRAYED) | MF_BYCOMMAND);

		if (in_button)
		{
			// Determine if locked
			bool is_locked = mOwner && mOwner->IsSubstituteLocked(mLastClicked);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSE, (is_locked ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
			pPopup->CheckMenuItem(IDM_3PANEITEMSTAB_LOCK, (is_locked ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);

			// Determine if dynamic
			bool is_dynamic = mOwner && mOwner->IsSubstituteDynamic(mLastClicked);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_DYNAMIC, (is_locked ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
			pPopup->CheckMenuItem(IDM_3PANEITEMSTAB_DYNAMIC, (is_dynamic ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
		}
		else
		{
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSE, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSEALL, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSEOTHERS, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_RENAME, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_LOCK, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_LOCKALL, MF_GRAYED | MF_BYCOMMAND);
			pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_DYNAMIC, MF_GRAYED | MF_BYCOMMAND);
		}

		pPopup->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, global.x, global.y, GetParent());
		
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSE, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSEALL, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_CLOSEOTHERS, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_RENAME, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_LOCK, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_LOCKALL, MF_ENABLED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_3PANEITEMSTAB_DYNAMIC, MF_ENABLED | MF_BYCOMMAND);
	}
}

// Do context menu
LRESULT C3PaneItemsTabs::OnNcHitTest(CPoint point)
{
	// Always pretend we are in client area so a right-click anywhere will
	// result in the popup appearing even when outside a tab-button
	return HTCLIENT;
}
