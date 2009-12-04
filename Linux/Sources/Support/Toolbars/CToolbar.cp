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


// Source for CToolbar class

#include "CToolbar.h"

#include "CBaseView.h"
#include "CCommander.h"
#include "CDivider.h"
#include "CFilterManager.h"
#include "CFocusBorder.h"
#include "CIconLoader.h"
#include "CMailboxToolbarPopup.h"
#include "CPreferences.h"
#include "CSelectPopup.h"
#include "CServerViewPopup.h"
#include "CSMTPAccountPopup.h"
#include "CToolbarButton.h"
#include "CToolbarManager.h"
#include "CToolbarPopupButton.h"
#include "CToolbarView.h"
#include "CXStringResources.h"

#include "TPopupMenu.h"

#include <JXDisplay.h>
#include <JXFlatRect.h>
#include <JXImageButton.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextMenu.h>
#include "JXTextPushButton.h"
#include "JXWindow.h"

#include <algorithm>

// Static members

CToolbar::CToolbarList CToolbar::sToolbars;

const int cToolbarWidth = 128;
const int cToolbarHeight = 45;
const int cBtnStart = 8;
const int cLargeIconBtnSize = 32;
const int cSmallIconBtnSize = 20;
const int cCaptionXtraBtnSize = 12;
const int cClickAndPopupXtraBtnSize = 12;
const int cPopupXtraBtnSize = 8;
const int cTextBtnHeight = 18;

const int cDropCursorWidth = 4;

const int cSpaceWidth = 16;
const int cExpandSpaceMinWidth = 4;
const int cSeparatorWidth = 8;

enum
{
	ePopup_RemoveButton = 1,
	ePopup_MoveButton,
	//ePopup_Separator1,
	ePopup_AddButton,
	//ePopup_Separator2,
	ePopup_ResetButtons
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CToolbar::CToolbar(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mIs3Pane = false;
	mShowIt = true;
	mSmallIcons = CPreferences::sPrefs->mToolbarSmallIcons.GetValue();
	mShowIcons = CPreferences::sPrefs->mToolbarShowIcons.GetValue();
	mShowCaptions = CPreferences::sPrefs->mToolbarShowCaptions.GetValue();
	mLeftJustOffset = cBtnStart;
	mLastCommander = NULL;

	mContextPopup = NULL;
	mContextAddPopup = NULL;
	mContextRulesPopup = NULL;
	
	mContextItem = NULL;
	mHighlighter = NULL;
	mContextAddItem = 0;
	mDragIndex = 0xFFFFFFFF;
	
	sToolbars.push_back(this);
}

// Default destructor
CToolbar::~CToolbar()
{
	delete mContextPopup;
	delete mHighlighter;

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

// Do various bits
void CToolbar::OnCreate()
{
	// Resize the toolbar itself - this will trigger layout change if the size changes
	JRect psize = GetFrame();
	if (GetBtnSize().y != psize.height())
		AdjustSize(0, GetBtnSize().y - psize.height());
	
	// Always create the highlighter before anything else so that it gets drawn after all other siblings
	// i.e. on top of buttons
	mHighlighter = new CFocusBorder(this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
	static_cast<CFocusBorder*>(mHighlighter)->SetFocus(true);
	static_cast<CFocusBorder*>(mHighlighter)->SetTransparent(true);
	mHighlighter->Hide();
}

void CToolbar::BuildToolbar()
{
	// Get the toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetToolbarItems(GetType());
	
	// Add each one
	int paneid = 'BTN1';	// Double this for every button as click-and-popup buttons use up two message ids
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
        	JXWidget* sep_container = new JXFlatRect(this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 2, cSeparatorWidth, 44);
    		CDivider* sep = new CDivider(sep_container, JXWidget::kFixedLeft, JXWidget::kFixedTop, cSeparatorWidth/2 - 2, 2, 2, 40);

			AddItem(sep_container, *iter);
		}
		else if (((*iter).GetItem()->GetType() == CToolbarItem::eSpace) ||
					((*iter).GetItem()->GetType() == CToolbarItem::eExpandSpace))
		{
        	JXWidget* sep = new JXFlatRect(this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, cSpaceWidth, 44);

			AddItem(sep, *iter);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::ePopupMenu)
		{
			cdstring title = CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID());
			HPopupMenu* popup = new HPopupMenu(title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 10, 200, 20);
			
			cdstring popup_items;
			popup_items.FromResource((*iter).GetItem()->GetPopupMenu());
			popup->SetMenuItems(popup_items);

			AddItem(popup, *iter);
			ListenTo(popup);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eSMTPAccountPopup)
		{
			cdstring title = CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID());
			CSMTPAccountPopup* popup = new CSMTPAccountPopup(this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 10, 232, 20);
			popup->OnCreate();
			AddItem(popup, *iter);
			ListenTo(popup->GetPopup());
		}
		// Icon is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eStaticIcon)
		{
			JXImageWidget* icon = new JXImageWidget(this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,6, 32,32);
			AddItem(icon, *iter);
		}
		else
		{
			JRect rect(0, 0, 44, 44);

			cdstring title = CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID());

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
				tbtn = new CToolbarButton(title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 44, 44);
				break;
			case CToolbarItem::ePopupButton:
			case CToolbarItem::ePopupPushButton:
			case CToolbarItem::ePopupToggleButton:
				tbtn = ptbtn = new CToolbarPopupButton(title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 44, 44);
				break;
			case CToolbarItem::eCabinetButton:
				tbtn = new CServerViewPopup(title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 44, 44);
				static_cast<CServerViewPopup*>(tbtn)->OnCreate();
				break;
			case CToolbarItem::eCopyToButton:
				tbtn = new CMailboxToolbarPopup(true, title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 44, 44);
				static_cast<CMailboxToolbarPopup*>(tbtn)->SetClickAndPopup(true);
				break;
			case CToolbarItem::eSelectButton:
				tbtn = ptbtn = new CSelectPopup(title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 44, 44);
				break;
			default:;
			}

			tbtn->SetImage((*iter).GetItem()->GetIconID(), (*iter).GetItem()->IsToggleIcon() ? (*iter).GetItem()->GetIconID() + 1 : 0);
#ifdef _J_USE_XFT
    		tbtn->SetFontSize(7);
#else
    		tbtn->SetFontSize(10);
#endif
			ListenTo(tbtn);

			if ((ptbtn != 0) && ((*iter).GetItem()->GetPopupMenu() != 0))
			{
				if ((*iter).GetItem()->GetPopupMenu() != 0)
					ptbtn->GetPopupMenu()->SetMenuItems(CToolbarManager::sToolbarManager.GetPopupMenuItems((*iter).GetItem()->GetPopupMenu()));
				ListenTo(ptbtn->GetPopupMenu());
			}

			// Now add to the display
			AddButton(tbtn, *iter);
		}
	}
	
	// We have to reset the toolbar to get the expand item width calculated properly
	ResetLayout();

	// If currently visible, redo state setting to force checkboxes into proper state
	if (IsVisible())
	{
		Activate();
		Show();
	}
	//Enable();
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

JPoint CToolbar::GetBtnSize(const CToolbarButton* tb) const
{
	JPoint result;

	// If captions visible, horizontal size is always full size
	if (mShowCaptions)
		result.x = cLargeIconBtnSize + cCaptionXtraBtnSize;
	else
		result.x = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;
	
	// No icons => use text button height
	if (!mShowIcons)
		result.y = cTextBtnHeight;
	else
	{
		// Vertical size depends on icon
		result.y = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;

		// If caption visible, vertical size always contains caption extra space
		if (mShowCaptions)
			result.y += cCaptionXtraBtnSize;

	}
	
	// Special toolbar button sizes
	if (tb)
	{
		// Click-and-popup always has extra space
		if (tb->GetClickAndPopup())
			result.x += cClickAndPopupXtraBtnSize;
		// If popup without captions or icons add extra width for popup glyph
		else if ((!mShowCaptions || !mShowIcons) && tb->HasPopup())
			result.x += cPopupXtraBtnSize;
	}

	return result;
}

// Resize columns
void CToolbar::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do default then reset toolbar layout
	JXWidgetSet::ApertureResized(dw, dh);
	if ((dw != 0) || (dh != 0))
		ResetLayout();
}

// Resize after change to buttons size/captions
void CToolbar::Reset()
{
	// Resize the toolbar itself - this will trigger layout change if the size changes
	JRect psize = GetFrame();
	if (GetBtnSize().y != psize.height())
		AdjustSize(0, GetBtnSize().y - psize.height());
	else
		ResetLayout();
}

// Resize after change to buttons size/captions
void CToolbar::ResetLayout()
{
	JRect psize = GetFrame();
	
	// Resize each button and accumulate total size (excluding additional expand item)
	JSize total_width = 0;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			// Get current item rect
			JRect wndsize = (*iter).mWnd->GetFrame();
			
			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eSeparator:
			case CToolbarItem::eSpace:
				// Resize height - keep width the same
				wndsize.bottom = GetBtnSize().y;
				(*iter).mWnd->SetSize(wndsize.width(), GetBtnSize().y);
				break;
			case CToolbarItem::eExpandSpace:
				// Resize height and width to standards - width will be adjusted to expanded size later
				wndsize.right = wndsize.left + cExpandSpaceMinWidth;
				(*iter).mWnd->SetSize(wndsize.width(), GetBtnSize().y);
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
					
					btn->SetSize(GetBtnSize(btn).x, GetBtnSize().y);
					wndsize.right = wndsize.left + GetBtnSize(btn).x;
				}
				break;
			}
			default:;
			}

			// Accumalate size
			total_width += wndsize.width();
		}
	}

	// Now reposition each button from the start accounting for expansion
	mLeftJustOffset = cBtnStart;
	JCoordinate expand_by = psize.width() - 2 * cBtnStart - total_width;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			// Get current item rect
			JRect wndsize = (*iter).mWnd->GetFrame();
			
			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eExpandSpace:
				// Resize width by expand amount if positive
				if (expand_by > 0)
				{
					(*iter).mWnd->AdjustSize(expand_by, 0);
					wndsize.right += expand_by;
				}
				break;
			default:;
			}

			// Now reposition it
			JRect wndrect(0, 0, wndsize.height(), wndsize.width());
			PositionButton(wndrect);
			(*iter).mWnd->Place(wndrect.left, wndrect.top);
		}
	}

	// Tell parent to adjust itself if we are visible
	if (IsVisible())
		const_cast<CToolbarView*>(static_cast<const CToolbarView*>(GetEnclosure()))->AdjustToolbarSize();
}

void CToolbar::PositionButton(JRect& btnrect)
{
	// Center vertically
	JRect psize = GetFrame();
	int top_offset = (psize.height() - btnrect.height()) / 2;
	btnrect.Shift(mLeftJustOffset, top_offset);
	mLeftJustOffset += btnrect.width();
}

void CToolbar::AddItem(JXWidget* item, const CToolbarItem::CToolbarItemInfo& details)
{
	// Position appropriately
	JRect psize = item->GetFrame();

	JRect btnrect(0, 0, psize.height(), psize.width());
	PositionButton(btnrect);
	item->Place(btnrect.left, btnrect.top);

	// Add item to list
	mItemList.push_back(SItemSpec(item, details));
}

void CToolbar::AddButton(CToolbarButton* btn, const CToolbarItem::CToolbarItemInfo& details)
{
	// Set up toolbar button styles - and resize button
	CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(btn);
	if (tbtn)
	{
		tbtn->SetSmallIcon(mSmallIcons);
		tbtn->SetShowIcon(mShowIcons);
		tbtn->SetShowCaption(mShowCaptions);
				
		tbtn->SetSize(GetBtnSize(tbtn).x, GetBtnSize().y);
	}

	// Position appropriately
	JRect fsize = btn->GetFrame();
	JRect btnrect(0, 0, fsize.height(), fsize.width());
	PositionButton(btnrect);
	btn->Place(btnrect.left, btnrect.top);

	// Add item to list
	mItemList.push_back(SItemSpec(btn, details));
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

CCommander* CToolbar::GetCommander() const
{
	CCommander* found = NULL;
	for(CCommanderArray::const_iterator iter = mCmdrs.begin(); iter != mCmdrs.end(); iter++)
	{
		// Always use the one which is the current target
		if ((*iter)->IsTarget())
		{
			mLastCommander = *iter;
			return *iter;
		}

		// Cache the one on duty and return that if none are targets
		// Also use the last one if that is found in the array
		if ((*iter)->IsOnDuty() || (*iter == mLastCommander))
			found = *iter;
	}
	
	mLastCommander = found;
	return found;
}

void CToolbar::UpdateControl(JXWidget* ctrl, unsigned long cmd, bool enable, bool show, bool caption)
{
	// Get command status from commander
	CCommander::CCmdUI cmdui;
	cmdui.mCmd = cmd;
	cmdui.mOther = ctrl;
	
	// Only if commander available
	if (GetCommander())
		GetCommander()->UpdateCommand(cmd, &cmdui);
	
	// Update control based on data
	if (cmdui.mEnable)
	{
		// Change enabled state if required
		if (enable)
			ctrl->Activate();

		// Change visible state if required
		if (show)
			ctrl->Show();
	}
	else
	{
		// Change enabled state if required
		if (enable)
			ctrl->Deactivate();

		// Change visible state if required
		if (show)
			ctrl->Hide();
	}
	
	// Mark => set value 1
	CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(ctrl);
	if (tbtn)
	{
		// Don't broadcast the change
		SetListening(kFalse);
		if (cmdui.mCheck)
			tbtn->SetState(kTrue);
		else
			tbtn->SetState(kFalse);
		SetListening(kTrue);
		
		// Update caption if required
		if (caption && cmdui.mEnable)
		{
			if (tbtn->GetLabel() != cmdui.mText)
				tbtn->SetLabel(cmdui.mText);
		}
	}
}

void CToolbar::UpdatePopupControl(JXWidget* ctrl, unsigned long cmd)
{
	// Must be a toolbar popup button
	CToolbarPopupButton* popup = dynamic_cast<CToolbarPopupButton*>(ctrl);
	if (!popup)
		return;
	
	// Do command UI processing for each message
	for(unsigned int i = 1; i <= popup->GetPopupMenu()->GetItemCount(); i++)
	{
		CCommander::CCmdUI cmdui;
		cmdui.mOther = ctrl;
		cmdui.mMenu = popup->GetPopupMenu();
		cmdui.mMenuIndex = i;

		// Only if commander available
		if (GetCommander())
		{
			GetCommander()->UpdateCommand(cmd, &cmdui);

			// Now update the menu item
			popup->GetPopupMenu()->SetItemEnable(i, cmdui.mEnable ? kTrue : kFalse);
			if (cmdui.mCheck)
				popup->GetPopupMenu()->CheckItem(i);
			if (cmdui.mText.length())
				popup->GetPopupMenu()->SetItemText(i, cmdui.mText);
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
 			
			
			if (tbtn->GetLabel() != title)
			{
				tbtn->SetLabel(title);
				result = true;
			}
		}
	}
	
	return result;
}

// Respond to clicks in the icon buttons
void CToolbar::Receive(JBroadcaster* sender, const Message& message)
{
	CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(sender);
	unsigned long item_index = GetItemIndex(tbtn);
	unsigned long cmd = (item_index < mItemList.size()) ? mItemList[item_index].mDetails.GetItem()->GetCommand() : 0;
	if (message.Is(JXButton::kPushed))
	{
		// Look for toolbar button
		if (tbtn)
		{
			// Find the button for this message
			if (GetCommander())
			{
				const SItemSpec& spec = mItemList[item_index];

				// Adjust ioParam to extra info if present
				const CCommander::SMenuCommandChoice* ioParam = NULL;
				if (!spec.mDetails.GetExtraInfo().empty())
					ioParam = reinterpret_cast<const CCommander::SMenuCommandChoice*>(spec.mDetails.GetExtraInfo().c_str());

				// Determine nature of command (ignore Alt key command - the handler will look for the key down
				GetCommander()->ObeyCommand(cmd, const_cast<CCommander::SMenuCommandChoice*>(ioParam));
				return;
			}
		}
	}
	else if(message.Is(CToolbarPopupButton::kMenuSelect))
	{
		const CToolbarPopupButton::MenuSelect* ms = dynamic_cast<const CToolbarPopupButton::MenuSelect*>(&message);
		if (GetCommander())
		{
			CCommander::SMenuCommandChoice menuchoice;
			menuchoice.mMenu = ms->GetMenu();
			menuchoice.mIndex = ms->GetIndex();
			GetCommander()->ObeyCommand(cmd, &menuchoice);
		}
	}
	else if(message.Is(CToolbarPopupButton::kMenuUpdate))
	{
		const CToolbarPopupButton::MenuUpdate* mu = dynamic_cast<const CToolbarPopupButton::MenuUpdate*>(&message);
		if (GetCommander())
		{
			unsigned long item_index = GetItemIndex(mu->GetBtn());
			unsigned long cmd = (item_index < mItemList.size()) ? mItemList[item_index].mDetails.GetItem()->GetCommand() : 0;
			UpdatePopupControl(mu->GetBtn(), cmd);
		}
	}
	else if (((sender == mContextPopup) || (sender == mContextAddPopup) || (sender == mContextRulesPopup)) && message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		ContextResult(dynamic_cast<HPopupMenu*>(sender), is->GetIndex());
		return;
	}
	else if ((sender == mContextPopup) && message.Is(JXMenu::kClosed))
	{
		// Remove highlighting if present
		if (mDragIndex == 0xFFFFFFFF)
		{
			mHighlighter->Hide();
		}
		return;
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

JXWidget* CToolbar::GetItem(CToolbarManager::EToolbarItem item) const
{
	// Find the button for this message
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if (((*iter).mDetails.GetItem() != NULL) && (static_cast<CToolbarManager::EToolbarItem>((*iter).mDetails.GetItem()->GetTitleID()) == item))
			return (*iter).mWnd;
	}
	
	return NULL;
}

unsigned long CToolbar::GetItemIndex(JXWidget* item) const
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

void CToolbar::HandleMouseDown(const JPoint& pt, 
							      const JXMouseButton button,
							      const JSize clickCount, 
							      const JXButtonStates& buttonStates,
							      const JXKeyModifiers& modifiers)
{
	// Check for move operation
	if (mDragIndex != 0xFFFFFFFF)
		TrackStop();

	// Handle context menus
	else if (button == kJXRightButton)
	{
		ContextEvent(pt, buttonStates, modifiers);
		return;
	}
	else
		JXWidgetSet::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void CToolbar::ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Find the item under the mouse
	mContextItem = NULL;
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		JRect bounds = (*iter).mWnd->GetFrame();
		if (bounds.Contains(pt))
		{
			mContextItem = (*iter).mWnd;
			break;
		}
	}
	
	// Always make sure fly-over acti vation is disabled when doing context
	if ((mContextItem != NULL) && (dynamic_cast<CToolbarButton*>(mContextItem) != NULL))
		dynamic_cast<CToolbarButton*>(mContextItem)->HandleMouseLeave();

	// Create popup menu
	if (!mContextPopup)
	{
		mContextPopup = new HPopupMenu("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
		mContextPopup->SetToHiddenPopupMenu(kTrue);
		mContextPopup->SetUpdateAction(JXMenu::kDisableNone);
		mContextPopup->Hide();
		ListenTo(mContextPopup);
	}
	else
		mContextPopup->RemoveAllItems();
	mContextAddPopup = NULL;
	mContextRulesPopup = NULL;
	mContextPopup->AppendItem("Remove %s Button");
	mContextPopup->AppendItem("Move %s Button");
	mContextPopup->ShowSeparatorAfter(2, kTrue);
	mContextPopup->AppendItem("Add Button");
	mContextPopup->ShowSeparatorAfter(3, kTrue);
	mContextPopup->AppendItem("Reset to Default Buttons");

	// Add items popup
	mContextAddPopup = new HPopupMenu("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
	mContextAddPopup->SetToHiddenPopupMenu(kTrue);
	mContextAddPopup->SetUpdateAction(JXMenu::kDisableNone);
	mContextAddPopup->Hide();
	ListenTo(mContextAddPopup);

	mContextPopup->AttachSubmenu(3, mContextAddPopup);

	// Get the allowed toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetAllowedToolbarItems(GetType());
	
	// Add each one
	JSize pos = 1;
	mApplyRulesIndex = 0;
	for(CToolbarItem::CToolbarPtrItems::const_iterator iter = items.begin(); iter != items.end(); iter++, pos++)
	{
		// Insert menu item
		cdstring temp = CToolbarManager::sToolbarManager.GetDescriptor((*iter).GetItem()->GetTitleID());
		mContextAddPopup->AppendItem(temp);

		// Look for apply rules sub menu and treat differently
		if ((*iter).GetItem()->GetTitleID() == CToolbarManager::eToolbar_ApplyRules)
		{
			mApplyRulesIndex = pos;

			// Place holder for apply rules popup if present
			mContextRulesPopup = new HPopupMenu("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
			mContextRulesPopup->SetToHiddenPopupMenu(kTrue);
			mContextRulesPopup->SetUpdateAction(JXMenu::kDisableNone);
			mContextRulesPopup->Hide();
			ListenTo(mContextRulesPopup);
			
			mContextAddPopup->AttachSubmenu(pos, mContextRulesPopup);

			// Add the icon
			mContextAddPopup->SetItemImageID(pos, (*iter).GetItem()->GetIconID());
			
			// Now fill the menu with apply rules items

			// Get all manual filters
			cdstrvect items;
			CPreferences::sPrefs->GetFilterManager()->GetManualFilters(items);
			
			// Add each one to menu
			JIndex rules_pos = 1;
			for(cdstrvect::const_iterator iter2 = items.begin(); iter2 != items.end(); iter2++, rules_pos++)
			{
				mContextRulesPopup->AppendItem(*iter2);
				
				// Add the icon
				mContextRulesPopup->SetItemImageID(rules_pos, (*iter).GetItem()->GetIconID());
				
				// Get filter UID for rule
				const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetFilter(CFilterItem::eLocal, *iter2);
				if (filter != NULL)
				{
					cdstring temp(filter->GetUID());

					// Disable ones already in the toolbar
					if (GetButton(CToolbarManager::eToolbar_ApplyRules, temp) != NULL)
						mContextRulesPopup->DisableItem(rules_pos);
				}
			}
		}
		else
		{
			// Add the icon
			mContextAddPopup->SetItemImageID(pos, (*iter).GetItem()->GetIconID());
			
			// See whether item is already present (allow multiple separators and ordinary space)
			if (((*iter).GetItem()->GetType() != CToolbarItem::eSeparator) &&
				((*iter).GetItem()->GetType() != CToolbarItem::eSpace) &&
				(GetItem(static_cast<CToolbarManager::EToolbarItem>((*iter).GetItem()->GetTitleID())) != NULL))
				mContextAddPopup->DisableItem(pos);
		}
	}

	// Remove button specific items if not over a button
	bool drawing = false;
	if (mContextItem == NULL)
	{
		mContextPopup->RemoveItem(1);
		mContextPopup->RemoveItem(1);
	}
	else
	{
		// Adjust titles of menu items
		const CToolbarItem* details = mItemList.at(GetItemIndex(mContextItem)).mDetails.GetItem();
		cdstring btnname = CToolbarManager::sToolbarManager.GetTitle(details->GetTitleID());
		{
			JString old_str = mContextPopup->GetItemText(ePopup_RemoveButton);
			cdstring temp(old_str);
			temp.Substitute(btnname);
			mContextPopup->SetItemText(ePopup_RemoveButton, temp);
		}
		{
			JString old_str = mContextPopup->GetItemText(ePopup_MoveButton);
			cdstring temp(old_str);
			temp.Substitute(btnname);
			mContextPopup->SetItemText(ePopup_MoveButton, temp);
		}

		// Draw insert marker
		
		// Is click to left or right side of current item
		bool left = true;
		JRect bounds = mContextItem->GetFrame();
		left = (pt.x < (bounds.left + bounds.right) / 2);
		
		mContextAddItem = GetItemIndex(mContextItem);
		if (left)
		{
			bounds.left -= cDropCursorWidth/2;
			bounds.right = bounds.left + cDropCursorWidth;
		}
		else
		{
			bounds.right += cDropCursorWidth/2;
			bounds.left = bounds.right - cDropCursorWidth;
			mContextAddItem++;
		}
		
		JRect myBounds = GetFrame();
		bounds.top = myBounds.top;
		bounds.bottom = myBounds.bottom;
		
		// Show the widget that draws the cursor
		mHighlighter->SetEnclosure(NULL);
		mHighlighter->SetEnclosure(this);
		mHighlighter->SetSize(bounds.width(), bounds.height());
		mHighlighter->Place(bounds.left, bounds.top);
		mHighlighter->Show();
	}

	// Do popup menu of suggestions
	mContextPopup->PopUp(this, pt, buttonStates, modifiers);
}

void CToolbar::ContextResult(HPopupMenu* menu, JIndex index)
{
	// Hide any drawn cursor
	mHighlighter->Hide();

	bool handled = false;
	if (menu == mContextPopup)
	{
		switch(index + ((mContextItem == NULL) ? 2 : 0))
		{
		case ePopup_RemoveButton:
			RemoveButton(mContextItem);
			break;
		case ePopup_MoveButton:
			MoveButton(mContextItem);
			break;
		case ePopup_AddButton:
			break;
		case ePopup_ResetButtons:
			ResetButtons(true);
			break;
		default:;
		}
	}
	else if (menu == mContextAddPopup)
	{
		// Get the item
		CToolbarManager::sToolbarManager.AddItemAt(GetType(), index - 1, mContextAddItem);

		// Rebuild the toolbar
		ResetButtons(false);
	}
	else if (menu == mContextRulesPopup)
	{
		// Get uid for item
		unsigned long uid = CPreferences::sPrefs->GetFilterManager()->GetManualUID(index  - 1);
		cdstring uid_txt(uid);

		// Get the item
		CToolbarManager::sToolbarManager.AddItemAt(GetType(), mApplyRulesIndex - 1, mContextAddItem, uid_txt);

		// Rebuild the toolbar
		ResetButtons(false);
	}
}

void CToolbar::RemoveButton(JXWidget* pane)
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

void CToolbar::MoveButton(JXWidget* pane)
{
	TrackStart(pane);
}

void CToolbar::TrackStart(JXWidget* pane)
{
	mDragIndex = GetItemIndex(pane);

	// Tell each control to be active
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if ((*iter).mWnd != NULL)
		{
			(*iter).mWnd->Activate();
			CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
			if (tbtn != NULL)
				tbtn->SetDragMode(true);
		}
	}
	
	// Create the widget that draws the cursor
	JRect rect = pane->GetFrame();
	mHighlighter->SetSize(rect.width(), rect.height());
	mHighlighter->Place(rect.left, rect.top);
	mHighlighter->Show();

	TrackHighlight();
	
	SetIgnoreEnclosed(kTrue);
	GetWindow()->GrabPointer(this);
}

void CToolbar::TrackStop()
{
	GetWindow()->UngrabPointer(this);
	SetIgnoreEnclosed(kFalse);

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

	// Hide the highlighter
	mHighlighter->Hide();

	// Always reset state
	UpdateToolbarState();
	Redraw();
}

void CToolbar::HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers)
{
	if (mDragIndex != 0xFFFFFFFF)
	{
		TrackMouseMove(pt);
	}
	else
		JXWidgetSet::HandleMouseHere(pt, modifiers);
}

void CToolbar::TrackMouseMove(const JPoint& pt)
{
	// Find the item under the mouse
	unsigned long new_index = mDragIndex;
	JXWidget* new_pane = NULL;
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		JRect bounds = (*iter).mWnd->GetFrame();
		if (bounds.Contains(pt))
		{
			new_pane = (*iter).mWnd;
			break;
		}
	}

	if (new_pane != NULL)
	{
		new_index = GetItemIndex(new_pane);

		// Apply hysteresis to movement to ensure panes with different sizes do not cause
		// rapid switches
		if (new_index != mDragIndex)
		{
			JRect old_rect = mItemList.at(mDragIndex).mWnd->GetFrame();
				
			JRect new_rect = mItemList.at(new_index).mWnd->GetFrame();
			if (new_index < mDragIndex)
			{
				// Only use new index if the mouse is within the old button size of the left edge of the new button
				if (pt.x >= new_rect.left + (old_rect.right - old_rect.left))
					new_index = mDragIndex;
			}
			else
			{
				// Only use new index if the mouse is within the old button size of the right edge of the new button
				if (pt.x < new_rect.right - (old_rect.right - old_rect.left))
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
		Redraw();
		
		mDragIndex = new_index;
	}
		
	// Draw highlight around moved button
	TrackHighlight();
}

void CToolbar::TrackHighlight()
{
	JRect rect = mContextItem->GetFrame();
	mHighlighter->Place(rect.left, rect.top);
}

// Force update of state if items visible
void CToolbar::UpdateToolbarState()
{
	// Do actual update only if items are visible
	DoUpdateToolbarState();
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
	//Refresh();
}

// Force update of state
void CToolbar::DoUpdateToolbarState()
{
	// Update each button
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>((*iter).mWnd);
		if (tbtn)
			UpdateControlState(tbtn, (*iter).mDetails.GetItem()->GetCommand(), (*iter).mDetails.GetItem()->IsDynamicTitle());
	}
}

void CToolbar::ShowToolbar(bool show)
{
	// Only do if different
	if (show ^ mShowIt)
	{
		mShowIt = show;
		if (show)
			Show();
		else
			Hide();
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
