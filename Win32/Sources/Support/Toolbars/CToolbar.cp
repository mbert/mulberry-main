/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CToolbar class

#include "CToolbar.h"

#include "CBaseView.h"
#include "CCommander.h"
#include "CDrawUtils.h"
#include "CFilterManager.h"
#include "CIconMenu.h"
#include "CIconWnd.h"
#include "CMailboxToolbarPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPopupButton.h"
#include "CPreferences.h"
#include "CSelectPopup.h"
#include "CServerViewPopup.h"
#include "CSMTPAccountPopup.h"
#include "CTextButton.h"
#include "CToolbarButton.h"
#include "CToolbarPopupButton.h"
#include "CToolbarView.h"
#include "CXstringResources.h"

#include "StValueChanger.h"

#include <algorithm>

// Static members

CToolbar::CToolbarList CToolbar::sToolbars;

BEGIN_MESSAGE_MAP(CToolbar, CContainerWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

const int cToolbarWidth = 128;
const int cToolbarHeight = 45;
const int cBtnStart = 8;
const int cLargeIconBtnSize = 32;
const int cSmallIconBtnSize = 20;
const int cCaptionXtraBtnSize = 14;
const int cClickAndPopupXtraBtnSize = 12;
const int cPopupXtraBtnSize = 8;
const int cTextBtnHeight = 18;

const int cDropCursorWidth = 4;

const int cSpaceWidth = 16;
const int cExpandSpaceMinWidth = 4;
const int cSeparatorWidth = 8;
const int cPopupWidth = 200;
const int cPopupTitleWidth = 64;

enum
{
	ePopup_RemoveButton = 0,
	ePopup_MoveButton,
	//ePopup_Separator1,
	ePopup_AddButton,
	//ePopup_Separator2,
	ePopup_ResetButtons
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CToolbar::CToolbar()
{
	mIs3Pane = false;
	mShowIt = true;
	mSmallIcons = CPreferences::sPrefs->mToolbarSmallIcons.GetValue();
	mShowIcons = CPreferences::sPrefs->mToolbarShowIcons.GetValue();
	mShowCaptions = CPreferences::sPrefs->mToolbarShowCaptions.GetValue();
	mLeftJustOffset = cBtnStart;
	mLastCommander = NULL;
	mDragIndex = 0xFFFFFFFF;
	
	sToolbars.push_back(this);
}

// Default destructor
CToolbar::~CToolbar()
{
	sToolbars.erase(remove(sToolbars.begin(), sToolbars.end(), this), sToolbars.end());
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CToolbar::PrefsChanged()
{
	// Rebuild each toolbar
	for(CToolbarList::iterator iter = sToolbars.begin(); iter != sToolbars.end(); iter++)
	{
		(*iter)->ResetButtons(false);
	}
}

bool CToolbar::RulesChanged()
{
	bool result = false;

	// Rebuild each toolbar
	for(CToolbarList::iterator iter = sToolbars.begin(); iter != sToolbars.end(); iter++)
	{
		result |= (*iter)->UpdateRulesButtons();
	}
	
	return result;
}

void CToolbar::InitToolbar(bool is_3pane, CToolbarView* parent)
{
	mIs3Pane = is_3pane;
	
	DWORD dwStyle = AFX_WS_DEFAULT_VIEW;
	dwStyle &= ~WS_BORDER;
	
	// Create this
	CRect rect(0, 0, cToolbarWidth, GetBtnSize().cy + 1);
	Create(NULL, NULL, dwStyle, rect, parent, IDC_STATIC);
}

int CToolbar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CContainerWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}


void CToolbar::OnPaint()
{
	// Fill the middle
	CPaintDC dc(this);
	CRect client;
	GetClientRect(client);
	dc.FillSolidRect(client, afxData.clrBtnFace);
}

void CToolbar::BuildToolbar()
{
	// Get the toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetToolbarItems(GetType());
	
	// Add each one
	UINT paneid = IDC_TOOLBAR_BTN1;	// Double this for every button as click-and-popup buttons use up two message ids
	for(CToolbarItem::CToolbarPtrItems::const_iterator iter = items.begin(); iter != items.end(); iter++, paneid += 2)
	{
		// Double-check we have something valid
		if ((*iter).GetItem() == NULL)
		{
			// Force entire reset
			ResetButtons(true);
			return;
		};

		// Separator is special cased as its not derived from toolbar button class
		if ((*iter).GetItem()->GetType() == CToolbarItem::eSeparator)
		{
			CRect rect1(0, 0, cSeparatorWidth, 44);
			CGrayBackground* sep_container = new CGrayBackground;
			sep_container->Create(_T(""), WS_CHILD | WS_VISIBLE, rect1, this, paneid);

			CRect rect2(cSeparatorWidth/2 - 2, 2, cSeparatorWidth/2, 40);
			CStatic* sep = new CStatic;
			sep->Create(_T(""), WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, rect2, sep_container, IDC_STATIC);
			sep_container->AddAlignment(new CWndAlignment(sep, CWndAlignment::eAlign_LeftHeight));

			AddItem(sep_container, *iter);
		}
		else if (((*iter).GetItem()->GetType() == CToolbarItem::eSpace) ||
					((*iter).GetItem()->GetType() == CToolbarItem::eExpandSpace))
		{
			CRect rect(0, 0, cSpaceWidth, 44);
			CStatic* sep = new CStatic;
			sep->Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, paneid);
			AddItem(sep, *iter);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::ePopupMenu)
		{
			CRect rect1(0, 10, cPopupWidth, 20);
			CGrayBackground* popup_container = new CGrayBackground;
			popup_container->Create(_T(""), WS_CHILD | WS_VISIBLE, rect1, this, paneid);

			CRect rect2(0, 0, cPopupTitleWidth, 2);
			CStatic* title = new CStatic;
			CString s = cdustring(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));
			title->Create(s, WS_CHILD | WS_VISIBLE, rect2, popup_container, IDC_STATIC);
			title->SetFont(CMulberryApp::sAppFont);

			CRect rect3(cPopupTitleWidth, 0, cPopupWidth - cPopupTitleWidth, 20);
			CPopupButton* popup = new CPopupButton;
			popup->Create(_T(""), rect3, popup_container, paneid, IDC_STATIC, IDI_POPUPBTN);
			popup->SetMenu((*iter).GetItem()->GetPopupMenu());
			popup->SetFont(CMulberryApp::sAppFont);

			AddItem(popup, *iter);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eSMTPAccountPopup)
		{
			const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
			CRect rect(0, 0, 212, 22 + small_offset);
			CString s = cdustring(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));
			CSMTPAccountPopup* popup = new CSMTPAccountPopup(paneid, s);
			popup->Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, paneid);
			AddItem(popup, *iter);
		}

		// Icon is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eStaticIcon)
		{
			CRect rect(6, 0, 32, 32);
			CIconWnd* icon = new CIconWnd;
			icon->Create(NULL, WS_CHILD | WS_VISIBLE | SS_ICON, rect, this, paneid);
			icon->SetIconID((*iter).GetItem()->GetIconID());
			AddItem(icon, *iter);
		}
		else
		{
			CRect rect(0, 0, 44, 44);

			CString title = cdustring(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));

			// Apply rules titles depend on the rule
			if ((*iter).GetItem()->GetTitleID() == CToolbarManager::eToolbar_ApplyRules)
			{
				// The name is actual the uid encoded as a string
				unsigned long uid = ::strtoul((*iter).GetExtraInfo(), NULL, 10);
				
				if (uid > 0)
				{
					const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetManualFilter(uid);
					if (filter)
						title = filter->GetName();
				}
				else
					title = rsrc::GetString("CToolbarManager::AllRules");
			}

			CToolbarButton* tbtn = NULL;
			CToolbarPopupButton* ptbtn = NULL;
			
			switch((*iter).GetItem()->GetType())
			{
			case CToolbarItem::ePushButton:
			case CToolbarItem::eToggleButton:
				tbtn = new CToolbarButton;
				break;
			case CToolbarItem::ePopupButton:
			case CToolbarItem::ePopupPushButton:
			case CToolbarItem::ePopupToggleButton:
				tbtn = ptbtn = new CToolbarPopupButton;
				break;
			case CToolbarItem::eCabinetButton:
				tbtn = new CServerViewPopup;
				break;
			case CToolbarItem::eCopyToButton:
				tbtn = new CMailboxToolbarPopup;
				static_cast<CMailboxToolbarPopup*>(tbtn)->SetClickAndPopup(true);
				static_cast<CMailboxToolbarPopup*>(tbtn)->SetCopyTo(true);
				break;
			case CToolbarItem::eSelectButton:
				tbtn = ptbtn = new CSelectPopup;
				break;
			}

			tbtn->Create(title, rect, this, paneid, IDC_STATIC, (*iter).GetItem()->GetIconID(), 0, (*iter).GetItem()->IsToggleIcon() ? (*iter).GetItem()->GetIconID() + 1 : 0, 0);
			if ((ptbtn != 0) && ((*iter).GetItem()->GetPopupMenu() != 0))
				ptbtn->SetMenu((*iter).GetItem()->GetPopupMenu());

			// Now add to the display
			AddButton(tbtn, *iter);
		}
	}
	
	// We have to reset the toolbar to get the expand item width calculated properly
	ResetLayout();
}

// Resize sub-views
void CToolbar::OnSize(UINT nType, int cx, int cy)
{
	CContainerWnd::OnSize(nType, cx, cy);
	ResetLayout();
}

unsigned long CToolbar::GetMinimumWidth() const
{
	// Size of left area + size of right area plus some space between them
	return mLeftJustOffset;
}

unsigned long CToolbar::GetActualWidth() const
{
	// Take into account collapsed state
	return GetMinimumWidth();
}

CSize CToolbar::GetBtnSize(const CToolbarButton* tb) const
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 12 : 0;

	CSize result;

	// If captions visible, horizontal size is always full size
	if (mShowCaptions)
		result.cx = cLargeIconBtnSize + cCaptionXtraBtnSize + large_offset;
	else
		result.cx = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;
	
	// No icons => use text button height
	if (!mShowIcons)
		result.cy = cTextBtnHeight;
	else
	{
		// Vertical size depends on icon
		result.cy = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;

		// If caption visible, vertical size always contains caption extra space
		if (mShowCaptions)
			result.cy += cCaptionXtraBtnSize + small_offset;

	}

	// Special toolbar button sizes
	if (tb)
	{
		// Click-and-popup always has extra space
		if (tb->GetClickAndPopup())
			result.cx += cClickAndPopupXtraBtnSize;
		// If popup without captions or icons add extra width for popup glyph
		else if ((!mShowCaptions || !mShowIcons) && tb->HasPopup())
			result.cx += cPopupXtraBtnSize;
	}

	return result;
}

// Resize after change to buttons size/captions
void CToolbar::Reset()
{
	// First resize the toolbar itself
	CRect rect;
	GetWindowRect(rect);
	GetParent()->ScreenToClient(rect);
	rect.bottom = rect.top + GetBtnSize().cy + 1;
	MoveWindow(rect);
}

// Resize after change to buttons size/captions
void CToolbar::ResetLayout()
{
	CRect rect;
	GetWindowRect(rect);

	// Resize each button and accumulate total size (excluding additional expand item)
	unsigned long total_width = 0;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			CRect btnrect;
			(*iter).mWnd->GetWindowRect(btnrect);

			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eSeparator:
			case CToolbarItem::eSpace:
				// Resize height - keep width the same
				btnrect.bottom = btnrect.top + GetBtnSize().cy;
				(*iter).mWnd->MoveWindow(btnrect);
				break;
			case CToolbarItem::eExpandSpace:
				// Resize height and width to standards - width will be adjusted to expanded size later
				btnrect.right = btnrect.left + cExpandSpaceMinWidth;
				btnrect.bottom = btnrect.top + GetBtnSize().cy;
				(*iter).mWnd->MoveWindow(btnrect);
				break;
			case CToolbarItem::ePushButton:
			case CToolbarItem::eToggleButton:
			case CToolbarItem::ePopupButton:
			case CToolbarItem::ePopupPushButton:
			case CToolbarItem::ePopupToggleButton:
			case CToolbarItem::eCabinetButton:
			case CToolbarItem::eCopyToButton:
			case CToolbarItem::eSelectButton:
			// These are all toolbar buttons
			{
				// See if it is a toolbar button
				CToolbarButton* btn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
				if (btn)
				{
					// Set its size/caption visibility
					btn->SetSmallIcon(mSmallIcons);
					btn->SetShowIcon(mShowIcons);
					btn->SetShowCaption(mShowCaptions);
					
					btnrect.right = btnrect.left + GetBtnSize(btn).cx;
					btnrect.bottom = btnrect.top + GetBtnSize().cy;
					btn->MoveWindow(btnrect);
				}
				break;
			}
			default:;
			}

			// Accumalate size
			total_width += btnrect.Width();
		}
	}

	// Now reposition each button from the start accounting for expansion
	mLeftJustOffset = cBtnStart;
	long expand_by = rect.Width() - 2 * cBtnStart - total_width;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			// Get current item rect
			CRect btnrect;
			(*iter).mWnd->GetWindowRect(btnrect);

			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eExpandSpace:
				// Resize width by expand amount if positive
				if (expand_by > 0)
				{
					btnrect.right = btnrect.left + expand_by;
					(*iter).mWnd->MoveWindow(btnrect);
				}
				break;
			default:;
			}

			// Now reposition it
			btnrect.OffsetRect(-btnrect.left, -btnrect.top);
			PositionButton(btnrect);
			(*iter).mWnd->MoveWindow(btnrect);
		}
	}
	
	// Tell parent to adjust itself if we are visible
	if (IsVisible())
		static_cast<CToolbarView*>(GetParent())->AdjustSize();
}

void CToolbar::PositionButton(CRect& btnrect)
{
	// Center vertically
	CRect client;
	GetClientRect(client);
	int top_offset = (client.Height() - btnrect.Height()) / 2;

	btnrect.OffsetRect(mLeftJustOffset, top_offset);
	mLeftJustOffset += btnrect.Width();
}

void CToolbar::AddItem(CWnd* wnd, const CToolbarItem::CToolbarItemInfo& details)
{
	// Position appropriately
	CRect btnrect;
	wnd->GetWindowRect(btnrect);
	btnrect.OffsetRect(-btnrect.left, -btnrect.top);
	PositionButton(btnrect);
	wnd->MoveWindow(btnrect);

	// Add item to list
	mItemList.push_back(SItemSpec(wnd, details));
}

void CToolbar::AddButton(CIconButton* btn, const CToolbarItem::CToolbarItemInfo& details)
{
	CRect btnrect;
	btn->GetWindowRect(btnrect);

	// Set up toolbar button styles
	CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(btn);
	if (tbtn)
	{
		tbtn->SetSmallIcon(mSmallIcons);
		tbtn->SetShowIcon(mShowIcons);
		tbtn->SetShowCaption(mShowCaptions);
				
		btnrect.right = btnrect.left + GetBtnSize(tbtn).cx;
		btnrect.bottom = btnrect.top + GetBtnSize().cy;
	}

	// Position appropriately
	btnrect.OffsetRect(-btnrect.left, -btnrect.top);
	PositionButton(btnrect);
	btn->MoveWindow(btnrect);

	// Add item to list
	mItemList.push_back(SItemSpec(btn, details));
}

BOOL CToolbar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Handle button message ourselves - we map it to the button's command
	if ((nCode == CN_COMMAND) && (nID >= IDC_TOOLBAR_BTN1) && (nID <= IDC_TOOLBAR_BTN1 + 100))
	{
		// Find the button for this message
		for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
		{
			if ((*iter).mWnd)
			{
				if ((*iter).mWnd->GetDlgCtrlID() == nID)
				{
					// Adjust pExtra to extra info if present
					if (!(*iter).mDetails.GetExtraInfo().empty())
						pExtra = const_cast<char*>((*iter).mDetails.GetExtraInfo().c_str());

					// Determine nature of command (ignore Alt key command - the handler will look for the key down
					// Let super commander handle the command if it wants to
					if (GetCommander() && GetCommander()->OnCmdMsg((*iter).mDetails.GetItem()->GetCommand(), nCode, pExtra, pHandlerInfo))
						return true;
				}
				else if ((*iter).mWnd->GetDlgCtrlID() + 1 == nID)
				{
					// Adjust ioParam to extra info if present
					if (!(*iter).mDetails.GetExtraInfo().empty())
						pExtra = const_cast<char*>((*iter).mDetails.GetExtraInfo().c_str());

					// Determine nature of command (ignore Alt key command - the handler will look for the key down
					// Let super commander handle the command if it wants to
					if (GetCommander() && GetCommander()->OnCmdMsg((*iter).mDetails.GetItem()->GetMenuCommand(), nCode, pExtra, pHandlerInfo))
						return true;
				}
			}
		}
	}

	// Let super commander handle the unmapped command if it wants to
	if (GetCommander() && GetCommander()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	BOOL result = CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	return result;
}

void CToolbar::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CBaseView::eBroadcast_ViewActivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ToolbarActivate, this);

		// Always update state after activation - selection change may not be triggered
		UpdateToolbarState();
		break;
	case CBaseView::eBroadcast_ViewDeactivate:
		// Broadcast activation to parent
		Broadcast_Message(eBroadcast_ToolbarDeactivate, this);
		break;
	case CBaseView::eBroadcast_ViewSelectionChanged:
		// Update state
		UpdateToolbarState();
		break;
	}
}		

// Set the commander to send commands to
void CToolbar::SetCommander(CCommander* cmdr)
{
	// Only if valid
	if (!cmdr)
		return;

	// See if already in the list
	CCommanderArray::const_iterator found = std::find(mCmdrs.begin(), mCmdrs.end(), cmdr);
	if (found == mCmdrs.end())
	{
		// Erase all
		mCmdrs.clear();

		mCmdrs.push_back(cmdr);
		// Update state if it changes
		UpdateToolbarState();
	}
}

// Add a commander to send commands to
void CToolbar::AddCommander(CCommander* cmdr)
{
	// Only if valid
	if (!cmdr)
		return;

	// See if already in the list
	CCommanderArray::const_iterator found = std::find(mCmdrs.begin(), mCmdrs.end(), cmdr);
	if (found == mCmdrs.end())
	{
		mCmdrs.push_back(cmdr);
		// Update state if it changes
		UpdateToolbarState();
	}
}

// Remove a commander to send commands to
void CToolbar::RemoveCommander(CCommander* cmdr)
{
	// Only if valid
	if (!cmdr)
		return;

	// See if already in the list
	mCmdrs.erase(std::remove(mCmdrs.begin(), mCmdrs.end(), cmdr), mCmdrs.end());

	// Update state if it changes
	UpdateToolbarState();
}

CWnd* CToolbar::GetCommander() const
{
	CCommander* found = NULL;
	for(CCommanderArray::const_iterator iter = mCmdrs.begin(); iter != mCmdrs.end(); iter++)
	{
		// Always use the one which is the current target
		if ((*iter)->IsTarget())
		{
			found = *iter;
			break;
		}

		// Cache the one on duty and return that if none are targets
		// Also use the last one if that is found in the array
		if ((*iter)->IsOnDuty() || (*iter == mLastCommander))
			found = *iter;
	}
	
	mLastCommander = found;
	return dynamic_cast<CWnd*>(found);
}

void CToolbar::UpdateControl(CWnd* ctrl, UINT cmd, bool enable, bool show)
{
	CCmdUI cmdui;
	cmdui.m_pOther = ctrl;
	cmdui.m_nID = cmd;
	
	// Only if commander available
	if (GetCommander())
		cmdui.DoUpdate(GetCommander(), true);
	
	// Check for visibility change tied to enable state
	if (show)
		ctrl->ShowWindow(ctrl->IsWindowEnabled() ? SW_SHOW : SW_HIDE);
}

void CToolbar::UpdatePopupControl(CWnd* ctrl)
{
	// Must be a toolbar popup button
	CToolbarPopupButton* popup = dynamic_cast<CToolbarPopupButton*>(ctrl);
	if (!popup)
		return;
	
	// Do command UI processing for each message in the menu
	for(UINT i = 0; i < popup->GetPopupMenu()->GetMenuItemCount(); i++)
	{
		UINT cmd = popup->GetPopupMenu()->GetMenuItemID(i);

		if ((cmd != 0) && (cmd != 0xFFFFFFFF))
		{
			CCmdUI cmdui;
			cmdui.m_pOther = ctrl;
			cmdui.m_nID = cmd;
			cmdui.m_nIndex = i;
			cmdui.m_pMenu = popup->GetPopupMenu();
			
			// Only if commander available
			if (GetCommander())
				cmdui.DoUpdate(GetCommander(), true);
		}
	}	
	
}

bool CToolbar::UpdateRulesButtons()
{
	bool result = false;

	// Update each button
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
		if ((tbtn != NULL) && ((*iter).mDetails.GetItem()->GetTitleID() == CToolbarManager::eToolbar_ApplyRules))
		{
			// Reset button title
			cdstring title;

			// The name is actual the uid encoded as a string
			unsigned long uid = ::strtoul((*iter).mDetails.GetExtraInfo(), NULL, 10);
			
			if (uid > 0)
			{
				const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetManualFilter(uid);
				if (filter)
					title = filter->GetName();
			}
			else
				title = rsrc::GetString("CToolbarManager::AllRules");
 			
			cdstring newtitle(title);
			cdstring oldtitle;
			tbtn->GetTitle(oldtitle);
			if (newtitle != oldtitle)
			{
				tbtn->SetTitle(newtitle);
				result = true;
			}
			
		}
	}
	
	return result;
}

// Display and track context menu
void CToolbar::OnContextMenu(CWnd* wnd, CPoint point)
{
	// Convert event point to a local point
	CPoint client_point(point);
	ScreenToClient(&client_point);

	// Find the item under the mouse
	CWnd* pane = ChildWindowFromPoint(client_point, CWP_ALL);
	if (pane == this)
		pane = NULL;
	
	// Always make sure fly-over acti vation is disabled when doing context
	if (pane != NULL)
		pane->SendMessage(WM_MOUSELEAVE);

	// Create popup menu
	CMenu popup;
	popup.LoadMenu(IDR_POPUP_CONTEXT_TOOLBAR);
	CMenu* pPopup = popup.GetSubMenu(0);
	
	// Fill add menu with items
	CIconMenu* submenu = new CIconMenu;
	submenu->CreatePopupMenu();
	CString old_str;
	pPopup->GetMenuString(3, old_str, MF_BYPOSITION);
	pPopup->ModifyMenu(3, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) submenu->m_hMenu, old_str);
	
	// Place holder for apply rules popup if present
	CIconMenu* apply_rules = NULL;
	
	// Get the allowed toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetAllowedToolbarItems(GetType());
	
	// Add each one
	UINT pos = IDM_TOOLBAR_ADD_Start;
	unsigned long apply_rules_index = 0;
	for(CToolbarItem::CToolbarPtrItems::const_iterator iter = items.begin(); iter != items.end(); iter++, pos++)
	{
		// Insert menu item
		cdstring temp = CToolbarManager::sToolbarManager.GetDescriptor((*iter).GetItem()->GetTitleID());

		// Look for apply rules sub menu and treat differently
		if ((*iter).GetItem()->GetTitleID() == CToolbarManager::eToolbar_ApplyRules)
		{
			apply_rules_index = pos;

			if (apply_rules == NULL)
			{
				apply_rules = new CIconMenu;
				apply_rules->CreatePopupMenu();
				submenu->AppendMenu(MF_BYPOSITION | MF_POPUP | MF_OWNERDRAW, (UINT) apply_rules->m_hMenu,
										(const TCHAR*)submenu->AddData(new CIconMenu::SIconMenuData(temp, IDI_RULESICON)));
			}
			
			// Add All item
			apply_rules->AppendMenu(MF_BYPOSITION | MF_OWNERDRAW, IDM_TOOLBAR_RULES_All, (const TCHAR*)apply_rules->AddData(new CIconMenu::SIconMenuData(rsrc::GetString("CToolbarManager::AllRules"), IDI_RULESICON)));
			apply_rules->AppendMenu(MF_SEPARATOR, 0, (TCHAR*) NULL);

			// Disable All item if already in the toolbar
			cdstring temp(0UL);
			if (GetButton(CToolbarManager::eToolbar_ApplyRules, temp) != NULL)
				apply_rules->EnableMenuItem(IDM_TOOLBAR_RULES_All, MF_BYCOMMAND | MF_GRAYED);
			
			// Now fill the menu with apply rules items

			// Get all manual filters
			cdstrvect items;
			CPreferences::sPrefs->GetFilterManager()->GetManualFilters(items);
			
			// Add each one to menu
			UINT rules_pos = IDM_TOOLBAR_RULES_Start;
			for(cdstrvect::const_iterator iter2 = items.begin(); iter2 != items.end(); iter2++, rules_pos++)
			{
				
				apply_rules->AppendMenu(MF_BYPOSITION | MF_OWNERDRAW, rules_pos, (const TCHAR*)apply_rules->AddData(new CIconMenu::SIconMenuData(*iter2, IDI_RULESICON)));
				
				// Get filter UID for rule
				const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetFilter(CFilterItem::eLocal, *iter2);
				if (filter != NULL)
				{
					cdstring temp(filter->GetUID());

					// Disable ones already in the toolbar
					if (GetButton(CToolbarManager::eToolbar_ApplyRules, temp) != NULL)
						apply_rules->EnableMenuItem(rules_pos, MF_BYCOMMAND | MF_GRAYED);
				}
			}
		}
		else
		{
			submenu->AppendMenu(MF_STRING | MF_BYPOSITION | MF_OWNERDRAW, pos,
									(const TCHAR*)submenu->AddData(new CIconMenu::SIconMenuData(temp, (*iter).GetItem()->GetIconID())));
			
			// See whether item is already present (allow multiple separators and ordinary space)
			if (((*iter).GetItem()->GetType() != CToolbarItem::eSeparator) &&
				((*iter).GetItem()->GetType() != CToolbarItem::eSpace) &&
				(GetItem(static_cast<CToolbarManager::EToolbarItem>((*iter).GetItem()->GetTitleID())) != NULL))
				submenu->EnableMenuItem(pos, MF_BYCOMMAND | MF_GRAYED);
		}
	}

	// Remove button specific items if not over a button
	CRect portRect;
	bool drawing = false;
	if (pane == NULL)
	{
		pPopup->DeleteMenu(0, MF_BYPOSITION);
		pPopup->DeleteMenu(0, MF_BYPOSITION);
		pPopup->DeleteMenu(0, MF_BYPOSITION);
	}
	else
	{
		// Adjust titles of menu items
		const CToolbarItem* details = mItemList.at(GetItemIndex(pane)).mDetails.GetItem();
		cdstring btnname = CToolbarManager::sToolbarManager.GetTitle(details->GetTitleID());
		{
			CString old_str;
			pPopup->GetMenuString(IDM_TOOLBAR_REMOVE, old_str, MF_BYCOMMAND);
			cdstring temp(old_str);
			temp.Substitute(btnname);
			pPopup->ModifyMenu(IDM_TOOLBAR_REMOVE, MF_BYCOMMAND | MF_STRING, IDM_TOOLBAR_REMOVE, temp.win_str());
		}
		{
			CString old_str;
			pPopup->GetMenuString(IDM_TOOLBAR_MOVE, old_str, MF_BYCOMMAND);
			cdstring temp(old_str);
			temp.Substitute(btnname);
			pPopup->ModifyMenu(IDM_TOOLBAR_MOVE, MF_BYCOMMAND | MF_STRING, IDM_TOOLBAR_MOVE, temp.win_str());
		}

		// Draw insert marker
		
		// Is click to left or right side of current item
		bool left = true;
		pane->GetWindowRect(portRect);
		left = (point.x < (portRect.left + portRect.right) / 2);
		
		if (left)
		{
			portRect.left -= cDropCursorWidth/2;
			portRect.right = portRect.left + cDropCursorWidth;
		}
		else
		{
			portRect.right += cDropCursorWidth/2;
			portRect.left = portRect.right - cDropCursorWidth;
		}
		
		ScreenToClient(portRect);
		CRect myPort;
		GetClientRect(myPort);
		portRect.top = myPort.top;
		portRect.bottom = myPort.bottom;
		
		// Now draw the rect
		CDC* pDC = GetDC();
		StDCState save(pDC);

		CPen pen;
		pen.CreatePen(PS_SOLID | PS_INSIDEFRAME, 3, CDrawUtils::sDkGrayColor);

		// Hilite outline
		pDC->SelectObject(&pen);
		pDC->Rectangle(portRect);

		drawing = true;
	}

	// Do popup menu of suggestions
	UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);

	// Remove any drawn cursor
	if (drawing)
	{
		RedrawWindow(portRect);
	}

	if (popup_result != 0)
	{
		bool handled = false;
		switch(popup_result)
		{
		case IDM_TOOLBAR_REMOVE:
			RemoveButton(pane);
			break;
		case IDM_TOOLBAR_MOVE:
			MoveButton(pane);
			break;
		case IDM_TOOLBAR_RESET:
			ResetButtons(true);
			break;
		default:
			if ((popup_result >= IDM_TOOLBAR_ADD_Start) && (popup_result <= IDM_TOOLBAR_ADD_End))
			{
				// Get the item
				CToolbarManager::sToolbarManager.AddItemAt(GetType(), popup_result - IDM_TOOLBAR_ADD_Start, GetItemIndex(pane));

				// Rebuild the toolbar
				ResetButtons(false);
			}
			else if ((popup_result >= IDM_TOOLBAR_RULES_All) && (popup_result <= IDM_TOOLBAR_RULES_End))
			{
				// Get uid for item
				unsigned long uid = (popup_result == IDM_TOOLBAR_RULES_All) ? 0 : CPreferences::sPrefs->GetFilterManager()->GetManualUID(popup_result - IDM_TOOLBAR_RULES_Start);
				cdstring uid_txt(uid);

				// Get the item
				CToolbarManager::sToolbarManager.AddItemAt(GetType(), apply_rules_index - IDM_TOOLBAR_ADD_Start, GetItemIndex(pane), uid_txt);

				// Rebuild the toolbar
				ResetButtons(false);
			}
			break;
		}

	}
}

CToolbarButton*	CToolbar::GetButton(CToolbarManager::EToolbarItem item) const
{
	// Find the button of this type
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
		if ((tbtn != NULL) && static_cast<CToolbarManager::EToolbarItem>((*iter).mDetails.GetItem()->GetTitleID()) == item)
			return tbtn;
	}
	
	return NULL;
}

CToolbarButton*	CToolbar::GetButton(CToolbarManager::EToolbarItem item, const cdstring& extra) const
{
	// Find the button of this type with the same extra data
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
		if ((tbtn != NULL) && (static_cast<CToolbarManager::EToolbarItem>((*iter).mDetails.GetItem()->GetTitleID()) == item) &&
			((*iter).mDetails.GetExtraInfo() == extra))
			return tbtn;
	}
	
	return NULL;
}

CWnd* CToolbar::GetItem(CToolbarManager::EToolbarItem item) const
{
	// Find the button for this message
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if (((*iter).mDetails.GetItem() != NULL) && (static_cast<CToolbarManager::EToolbarItem>((*iter).mDetails.GetItem()->GetTitleID()) == item))
			return (*iter).mWnd;
	}
	
	return NULL;
}

unsigned long CToolbar::GetItemIndex(CWnd* item) const
{
	// Find index of pane in item list
	unsigned long index = 0;
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++, index++)
	{
		if ((*iter).mWnd == item)
		{
			break;
		}
	}
	
	return index;
}

void CToolbar::RemoveButton(CWnd* pane)
{
	// Find index of pane in item list
	unsigned long index = 0;
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++, index++)
	{
		if ((*iter).mWnd == pane)
		{
			// Remove from toolbar manager prefs
			CToolbarManager::sToolbarManager.RemoveItemAt(GetType(), index);

			// Delete the button object
			delete (*iter).mWnd;
			mItemList.erase(iter);
			
			// Reset the toolbar
			ResetLayout();
			return;
		}
	}
}

void CToolbar::MoveButton(CWnd* pane)
{
	TrackStart(pane);
}

void CToolbar::TrackStart(CWnd* pane)
{
	mDragIndex = GetItemIndex(pane);

	SetCapture();

	// Tell each control to be active
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if ((*iter).mWnd != NULL)
		{
			(*iter).mWnd->EnableWindow(true);
			CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
			if (tbtn != NULL)
				tbtn->SetDragMode(true);
		}
	}
	
	TrackHighlight();
}

void CToolbar::TrackStop()
{
	// Tell each control no longer dragging
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if ((*iter).mWnd != NULL)
		{
			CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
			if (tbtn != NULL)
				tbtn->SetDragMode(false);
		}
	}

	mDragIndex = 0xFFFFFFFF;

	// Always reset state
	UpdateToolbarState();
	RedrawWindow();

	ReleaseCapture();
}

void CToolbar::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (mDragIndex != 0xFFFFFFFF)
	{
		TrackStop();
	}
	else
		CContainerWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CToolbar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (mDragIndex != 0xFFFFFFFF)
	{
		TrackStop();
	}
	else
		CContainerWnd::OnLButtonDown(nFlags, point);
}

// Clicked somewhere
void CToolbar::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (mDragIndex != 0xFFFFFFFF)
	{
		TrackStop();
	}
	else
	{
		// Do context menu
		ClientToScreen(&point);
		OnContextMenu(NULL, point);	
	}
}

void CToolbar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (mDragIndex != 0xFFFFFFFF)
	{
		TrackMouseMove(point);
	}
	else
		CContainerWnd::OnMouseMove(nFlags, point);
}

void CToolbar::TrackMouseMove(CPoint point)
{
	// Find the item under the mouse
	unsigned long new_index = mDragIndex;
	CWnd* new_pane = ChildWindowFromPoint(point, CWP_ALL);
	if (new_pane != NULL)
	{
		new_index = GetItemIndex(new_pane);

		// Apply hysteresis to movement to ensure panes with different sizes do not cause
		// rapid switches
		if (new_index != mDragIndex)
		{
			CRect old_portRect;
			mItemList.at(mDragIndex).mWnd->GetWindowRect(old_portRect);
			ScreenToClient(old_portRect);
				
			CRect new_portRect;
			mItemList.at(new_index).mWnd->GetWindowRect(new_portRect);
			ScreenToClient(new_portRect);
			
			if (new_index < mDragIndex)
			{
				// Only use new index if the mouse is within the old button size of the left edge of the new button
				if (point.x >= new_portRect.left + (old_portRect.right - old_portRect.left))
					new_index = mDragIndex;
			}
			else
			{
				// Only use new index if the mouse is within the old button size of the right edge of the new button
				if (point.x < new_portRect.right - (old_portRect.right - old_portRect.left))
					new_index = mDragIndex;
			}
		}
	}

	if (new_index != mDragIndex)
	{
		// Determine index of start/end panes
		CToolbarManager::sToolbarManager.MoveItemAt(GetType(), mDragIndex, new_index);
		
		SItemSpec temp = mItemList.at(mDragIndex);
		mItemList.erase(mItemList.begin() + mDragIndex);
		mItemList.insert(mItemList.begin() + new_index, temp);
		ResetLayout();
		RedrawWindow();
		
		mDragIndex = new_index;
	}
		
	// Draw highlight around moved button
	TrackHighlight();
}

void CToolbar::TrackHighlight()
{
	CRect portRect;
	mItemList.at(mDragIndex).mWnd->GetWindowRect(portRect);
	ScreenToClient(portRect);

	// Now draw the rect
	CDC* pDC = GetDC();
	StDCState save(pDC);

	// Hilite outline
	for(int i = 0; i < 4; i++)
	{
		pDC->FrameRect(portRect, &CDrawUtils::sDkGrayBrush);
		portRect.DeflateRect(1, 1);
	}
}

void CToolbar::ResetButtons(bool use_default)
{
	// Remove all buttons
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Delete the button object
		delete (*iter).mWnd;
	}
	mItemList.clear();
	
	// Reset to default state
	if (use_default)
		CToolbarManager::sToolbarManager.ResetToolbar(GetType());
	
	// Clear positions
	mLeftJustOffset = cBtnStart;
	
	// Rebuild the toolbar
	BuildToolbar();
	UpdateToolbarState();
	RedrawWindow();
}

// Force update of state if items visible
void CToolbar::UpdateToolbarState()
{
	// Do actual update only if items are visible
	DoUpdateToolbarState();
}

// Force update of state
void CToolbar::DoUpdateToolbarState()
{
	// Update each button
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
		if (tbtn)
			UpdateControlState(tbtn, (*iter).mDetails.GetItem()->GetCommand());
	}
}

void CToolbar::ShowToolbar(bool show)
{
	// Only do if different
	if (show ^ mShowIt)
	{
		mShowIt = show;
		ShowWindow(show ? SW_SHOW : SW_HIDE);
	}
}

void CToolbar::SmallIcons(bool small_icon)
{
	if (mSmallIcons ^ small_icon)
	{
		mSmallIcons = small_icon;
		Reset();
	}
}

void CToolbar::ShowIcons(bool show)
{
	if (mShowIcons ^ show)
	{
		mShowIcons = show;
		Reset();
	}
}

void CToolbar::ShowCaptions(bool show)
{
	if (mShowCaptions ^ show)
	{
		mShowCaptions = show;
		Reset();
	}
}
