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


// Source for CACLStylePopup class


#include "CACLStylePopup.h"

#include "CDeleteACLStyleDialog.h"
#include "CNewACLStyleDialog.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

IMPLEMENT_DYNCREATE(CACLStylePopup, CIconButton)

BEGIN_MESSAGE_MAP(CACLStylePopup, CIconButton)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C A C L S T Y L E P O P U P
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CACLStylePopup::CACLStylePopup()
{
	mPopupID = IDR_POPUP_ACL_STYLE;
}

// Default destructor
CACLStylePopup::~CACLStylePopup()
{
}

// Clicked somewhere
void CACLStylePopup::OnLButtonDown(UINT nFlags, CPoint point)
{
	CMenu popup;
	
	// Get buttons top-right corner as start pt
	CRect screen;
	GetWindowRect(screen);
	CPoint global(screen.right, screen.top);

	if (popup.LoadMenu(mPopupID))
	{
		// Push button
		SetPushed(true);

		// Track the popup
		CMenu* pPopup = popup.GetSubMenu(0);

		// Now add current items
		int menu_id = IDM_ACLStyleStart;
		for(SACLStyleList::const_iterator iter = mList->GetValue().begin(); iter != mList->GetValue().end(); iter++)
		{
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter).first);
			menu_id++;
		}

		CWnd* pWndPopupOwner = GetParent();
		while (pWndPopupOwner->GetStyle() & WS_CHILD)
			pWndPopupOwner = pWndPopupOwner->GetParent();
		int count = pPopup->GetMenuItemCount();
		pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_LEFTALIGN, global.x, global.y, GetParent());

		// Unpush button
		SetPushed(false);

	}
}

// Reset items
void CACLStylePopup::Reset(bool mbox)
{
	// Cache appropriate list
	mMbox = mbox;
	mList = (mbox ? &CPreferences::sPrefs->mMboxACLStyles : &CPreferences::sPrefs->mAdbkACLStyles);
}

// Add new style
void CACLStylePopup::DoNewStyle(SACLRight rights)
{
	// Create the dialog
	CNewACLStyleDialog dlog(CSDIFrame::GetAppTopWindow());

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		cdstring style_name = dlog.mName;

		// Add style to list and menu
		SACLStyle style = make_pair(style_name, rights);
		((SACLStyleList&) mList->GetValue()).push_back(style);
		mList->SetDirty();
	}
}

// Delete existing styles
void CACLStylePopup::DoDeleteStyle(void)
{
	// Create the dialog
	CDeleteACLStyleDialog dlog(CSDIFrame::GetAppTopWindow());

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		// Get selection from list
		ulvector& styles = dlog.mStyleSelect;

		// Remove in reverse order to stay in sync
		for(ulvector::reverse_iterator iter = styles.rbegin(); iter != styles.rend(); iter++)
		{
			((SACLStyleList&) mList->GetValue()).erase(((SACLStyleList&) mList->GetValue()).begin() + *iter);
			mList->SetDirty();
		}
	}
}
