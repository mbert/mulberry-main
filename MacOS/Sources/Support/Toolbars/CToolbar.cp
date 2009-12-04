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
#include "CContextMenu.h"
#include "CFilterManager.h"
#include "CMailboxToolbarPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionClick.h"
#include "CPreferences.h"
#include "CSelectPopup.h"
#include "CServerViewPopup.h"
#include "CSMTPAccountPopup.h"
#include "CStaticText.h"
#include "CToolbarButton.h"
#include "CToolbarManager.h"
#include "CToolbarView.h"
#include "CXstringResources.h"

#include <LBevelButton.h>
#include <LIconControl.h>
#include <LPictureControl.h>
#include <LPopupButton.h>
#include <LSeparatorLine.h>
#include <UGAColorRamp.h>

#include <algorithm>

// Static members

CToolbar::CToolbarList CToolbar::sToolbars;

CToolbar::CToolbarTrackAttachment* CToolbar::sTracker = NULL;

const int cBtnStart = 8;
const int cLargeIconBtnSize = 32;
const int cSmallIconBtnSize = 20;
const int cCaptionXtraBtnSize = 12;
const int cClickAndPopupXtraBtnSize = 12;
const int cPopupXtraBtnSize = 8;
const int cTextBtnHeight = 16;

const int cDropCursorWidth = 4;

const int cSpaceWidth = 16;
const int cExpandSpaceMinWidth = 4;
const int cSeparatorWidth = 8;

const ResIDT Txtr_CaptionText = 261;

enum
{
	ePopup_RemoveButton = 1,
	ePopup_MoveButton,
	ePopup_Separator1,
	ePopup_AddButton,
	ePopup_Separator2,
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
	
	sToolbars.push_back(this);
}

// Constructor from stream
CToolbar::CToolbar(LStream *inStream)
		: LView(inStream)
{
	mIs3Pane = false;
	mShowIt = true;
	mSmallIcons = CPreferences::sPrefs->mToolbarSmallIcons.GetValue();
	mShowIcons = CPreferences::sPrefs->mToolbarShowIcons.GetValue();
	mShowCaptions = CPreferences::sPrefs->mToolbarShowCaptions.GetValue();
	mLeftJustOffset = cBtnStart;
	mLastCommander = NULL;
	
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

// Do various bits
void CToolbar::FinishCreateSelf()
{
	// Do inherited
	LView::FinishCreateSelf();

	// Do reset to set its size based on current preferences
	Reset();

	CContextMenuAttachment::AddUniqueContext(this, 15000, NULL, false);
}

void CToolbar::BuildToolbar()
{
	// Get the toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetToolbarItems(GetType());
	
	// Add each one
	int paneid = 'TBB1';	// Double this for every button as click-and-popup buttons use up two message ids
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
			SPaneInfo pane1;
			pane1.visible = true;
			pane1.enabled = true;
			pane1.userCon = 0L;
			pane1.superView = this;
			pane1.bindings.left = true;
			pane1.bindings.right = false;
			pane1.bindings.top = true;
			pane1.bindings.bottom = false;
			pane1.paneID = paneid;
			pane1.width = cSeparatorWidth;
			pane1.height = 44;
			pane1.left = 0;
			pane1.top = 2;
			SViewInfo view;
			view.imageSize.width = cSeparatorWidth;
			view.imageSize.height = 44;
			view.scrollPos.h = 0;
			view.scrollPos.v = 0;
			view.scrollUnit.h = 0;
			view.scrollUnit.v = 0;
			view.reconcileOverhang = 0;
			LView* sep_container = new LView(pane1, view);
			SPaneInfo pane2;
			pane2.visible = true;
			pane2.enabled = true;
			pane2.userCon = 0L;
			pane2.superView = sep_container;
			pane2.bindings.left = true;
			pane2.bindings.right = false;
			pane2.bindings.top = true;
			pane2.bindings.bottom = true;
			pane2.paneID = paneid;
			pane2.width = 3;
			pane2.height = 40;
			pane2.left = cSeparatorWidth/2 - 2;
			pane2.top = 2;
			LPane* sep = new LSeparatorLine(pane2);
			AddItem(sep_container, *iter);
		}
		else if (((*iter).GetItem()->GetType() == CToolbarItem::eSpace) ||
					((*iter).GetItem()->GetType() == CToolbarItem::eExpandSpace))
		{
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left = true;
			pane.bindings.right = false;
			pane.bindings.top = true;
			pane.bindings.bottom = false;
			pane.paneID = paneid;
			pane.width = cSpaceWidth;
			pane.height = 44;
			pane.left = 0;
			pane.top = 0;
			LPane* sep = new LPane(pane);
			AddItem(sep, *iter);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::ePopupMenu)
		{
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left = true;
			pane.bindings.right = false;
			pane.bindings.top = true;
			pane.bindings.bottom = false;
			pane.paneID = paneid;
			pane.width = 200;	// Provide enough width to keep control title on one line
			pane.height = 20;
			pane.left = 0;
			pane.top = 10;
			LStr255 title(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));
			LPopupButton* popup = new LPopupButton(
								pane,
								paneid,
								popupTitleRightJust,
								(*iter).GetItem()->GetPopupMenu(),
								-1,
								kControlPopupButtonProc + kControlPopupFixedWidthVariant,
								260,
								title,
								0,
								1);
			AddItem(popup, *iter);
			popup->AddListener(this);
		}
		// Popup menu is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eSMTPAccountPopup)
		{
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left = true;
			pane.bindings.right = false;
			pane.bindings.top = true;
			pane.bindings.bottom = false;
			pane.paneID = paneid;
			pane.width = 232;	// Provide enough width to keep control title on one line
			pane.height = 20;
			pane.left = 0;
			pane.top = 10;
			SViewInfo view;
			view.imageSize.width = 232;
			view.imageSize.height = 20;
			view.scrollPos.h = 0;
			view.scrollPos.v = 0;
			view.scrollUnit.h = 0;
			view.scrollUnit.v = 0;
			view.reconcileOverhang = 0;

			LStr255 title(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));
			CSMTPAccountPopup* popup = new CSMTPAccountPopup(
								pane,
								view,
								paneid,
								(*iter).GetItem()->GetPopupMenu(),
								(*iter).GetItem()->GetIconID(),
								title);
			AddItem(popup, *iter);
			popup->GetPopup()->AddListener(this);
		}
		// Icon is special cased as its not derived from toolbar button class
		else if ((*iter).GetItem()->GetType() == CToolbarItem::eStaticIcon)
		{
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left = true;
			pane.bindings.right = false;
			pane.bindings.top = true;
			pane.bindings.bottom = false;
			pane.paneID = paneid;
			pane.width = 32;	// Provide enough width to keep control title on one line
			pane.height = 32;
			pane.left = 0;
			pane.top = 6;
			LIconControl* icon = new LIconControl(
								pane,
								paneid,
								kControlIconSuiteNoTrackProc,
								(*iter).GetItem()->GetIconID());
			icon->SetIconAlignment(kAlignAbsoluteCenter);
			AddItem(icon, *iter);
		}
		else
		{
			SPaneInfo pane;
			pane.visible = true;
			pane.enabled = true;
			pane.userCon = 0L;
			pane.superView = this;
			pane.bindings.left = true;
			pane.bindings.right = false;
			pane.bindings.top = true;
			pane.bindings.bottom = false;
			pane.paneID = paneid;
			pane.width = 44;	// Provide enough width to keep control title on one line
			pane.height = 44;
			pane.left = 0;
			pane.top = 0;

			LStr255 title(CToolbarManager::sToolbarManager.GetTitle((*iter).GetItem()->GetTitleID()));

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

			Point offset = { 0, 0 };

			UInt16 behaviour;
			switch((*iter).GetItem()->GetType())
			{
			case CToolbarItem::ePushButton:
			case CToolbarItem::ePopupButton:
			case CToolbarItem::ePopupPushButton:
			case CToolbarItem::ePopupToggleButton:
				behaviour = kControlBehaviorPushbutton;
				break;
			case CToolbarItem::eToggleButton:
				behaviour = kControlBehaviorToggles;
				break;
			case CToolbarItem::eCabinetButton:
				behaviour = kControlBehaviorMultiValueMenu;
				break;
			case CToolbarItem::eCopyToButton:
				behaviour = kControlBehaviorSingleValueMenu;
				break;
			default:;
			}

			CToolbarButton* tbtn = NULL;
			
			switch((*iter).GetItem()->GetType())
			{
			case CToolbarItem::ePushButton:
			case CToolbarItem::eToggleButton:
				tbtn = new CToolbarButton(
								pane,
								paneid,
								kControlBevelButtonSmallBevelProc,
								behaviour,
								kControlContentIconSuiteRes,
								(*iter).GetItem()->GetIconID(),
								260,
								title,
								0,
								kControlBevelButtonPlaceBelowGraphic,
								kControlBevelButtonAlignTextCenter,
								0,
								kControlBevelButtonAlignCenter,
								offset);
				break;
			case CToolbarItem::ePopupButton:
			case CToolbarItem::ePopupPushButton:
			case CToolbarItem::ePopupToggleButton:
				tbtn = new CToolbarButton(
								pane,
								paneid,
								kControlBevelButtonSmallBevelProc,
								(*iter).GetItem()->GetPopupMenu(),
								kControlBevelButtonMenuOnBottom,
								kControlContentIconSuiteRes,
								(*iter).GetItem()->GetIconID(),
								260,
								title,
								0,
								kControlBevelButtonPlaceBelowGraphic,
								kControlBevelButtonAlignTextCenter,
								0,
								kControlBevelButtonAlignCenter,
								offset,
								false);
				break;
			case CToolbarItem::eCabinetButton:
				tbtn = new CServerViewPopup(
								pane,
								paneid,
								kControlBevelButtonSmallBevelProc,
								(*iter).GetItem()->GetPopupMenu(),
								kControlBevelButtonMenuOnBottom,
								kControlContentIconSuiteRes + kControlBehaviorMultiValueMenu,
								(*iter).GetItem()->GetIconID(),
								260,
								title,
								0,
								kControlBevelButtonPlaceBelowGraphic,
								kControlBevelButtonAlignTextCenter,
								0,
								kControlBevelButtonAlignCenter,
								offset,
								false);
				break;
			case CToolbarItem::eCopyToButton:
				tbtn = new CMailboxToolbarPopup(
								pane,
								paneid,
								kControlBevelButtonSmallBevelProc,
								(*iter).GetItem()->GetPopupMenu(),
								kControlBevelButtonMenuOnBottom,
								kControlContentIconSuiteRes,
								(*iter).GetItem()->GetIconID(),
								260,
								title,
								0,
								kControlBevelButtonPlaceBelowGraphic,
								kControlBevelButtonAlignTextCenter,
								0,
								kControlBevelButtonAlignCenter,
								offset,
								false);
				static_cast<CMailboxToolbarPopup*>(tbtn)->SetCopyTo(true);
				break;
			case CToolbarItem::eSelectButton:
				tbtn = new CSelectPopup(
								pane,
								paneid,
								kControlBevelButtonSmallBevelProc,
								(*iter).GetItem()->GetPopupMenu(),
								kControlBevelButtonMenuOnBottom,
								kControlContentIconSuiteRes,
								(*iter).GetItem()->GetIconID(),
								260,
								title,
								0,
								kControlBevelButtonPlaceBelowGraphic,
								kControlBevelButtonAlignTextCenter,
								0,
								kControlBevelButtonAlignCenter,
								offset,
								false);
				break;
			default:;
			}
			
			// Always complete creation
			tbtn->FinishCreate();

			// Special features
			tbtn->SetDetails((*iter).GetItem());

			// Now add to the display
			AddButton(tbtn, *iter);
			tbtn->AddListener(this);
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
	Enable();
}

unsigned long CToolbar::GetMinimumWidth() const
{
	// Size of left area
	return mLeftJustOffset;
}

unsigned long CToolbar::GetActualWidth() const
{
	// Take into account collapsed state
	return GetMinimumWidth();
}

Point CToolbar::GetBtnSize(const CToolbarButton* tb) const
{
	Point result;

	// If captions visible, horizontal size is always full size
	if (mShowCaptions)
		result.h = cLargeIconBtnSize + cCaptionXtraBtnSize;
	else
		result.h = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;
	
	// No icons => use text button height
	if (!mShowIcons)
		result.v = cTextBtnHeight;
	else
	{
		// Vertical size depends on icon
		result.v = mSmallIcons ? cSmallIconBtnSize : cLargeIconBtnSize;

		// If caption visible, vertical size always contains caption extra space
		if (mShowCaptions)
			result.v += cCaptionXtraBtnSize;

	}
	
	// Special toolbar button sizes
	if (tb)
	{
		// Click-and-popup always has extra space
		if (tb->GetClickAndPopup())
			result.h += cClickAndPopupXtraBtnSize;
		// If popup without captions or icons add extra width for popup glyph
		else if ((!mShowCaptions || !mShowIcons) && tb->HasPopup())
			result.h += cPopupXtraBtnSize;
	}

	return result;
}

// Resize after change to buttons size/captions
void CToolbar::Reset()
{
	// Resize the toolbar itself - this will trigger layout change if the size changes
	SDimension16 psize;
	GetFrameSize(psize);
	ResizeFrameBy(0, GetBtnSize().v - psize.height, false);
}

// Resize after change to buttons size/captions
void CToolbar::ResetLayout()
{
	SDimension16 psize;
	GetFrameSize(psize);

	// Resize each button and accumulate total size (excluding additional expand item)
	SInt16 total_width = 0;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			// Get current item rect
			SDimension16 wndsize;
			(*iter).mWnd->GetFrameSize(wndsize);
			
			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eSeparator:
			case CToolbarItem::eSpace:
				// Resize height - keep width the same
				wndsize.height = GetBtnSize().v;
				(*iter).mWnd->ResizeFrameTo(wndsize.width, GetBtnSize().v, false);
				break;
			case CToolbarItem::eExpandSpace:
				// Resize height and width to standards - width will be adjusted to expanded size later
				wndsize.width = cExpandSpaceMinWidth;
				(*iter).mWnd->ResizeFrameTo(wndsize.width, GetBtnSize().v, false);
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
					
					btn->ResizeFrameTo(GetBtnSize(btn).h, GetBtnSize().v, false);
					wndsize.width = GetBtnSize(btn).h;
				}
				break;
			}
			default:;
			}

			// Accumalate size
			total_width += wndsize.width;
		}
	}

	// Now reposition each button from the start accounting for expansion
	mLeftJustOffset = cBtnStart;
	SInt16 expand_by = psize.width - 2 * cBtnStart - total_width;

	// Loop over each item
	for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		// Check for space
		if ((*iter).mWnd)
		{
			// Get current item rect
			SDimension16 wndsize;
			(*iter).mWnd->GetFrameSize(wndsize);
			
			switch((*iter).mDetails.GetItem()->GetType())
			{
			case CToolbarItem::eExpandSpace:
				// Resize width by expand amount if positive
				if (expand_by > 0)
				{
					(*iter).mWnd->ResizeFrameBy(expand_by, 0, false);
					wndsize.width += expand_by;
				}
				break;
			default:;
			}

			// Now reposition it
			Rect wndrect = {0, 0, wndsize.height, wndsize.width};
			PositionButton(wndrect);
			(*iter).mWnd->PlaceInSuperFrameAt(wndrect.left, wndrect.top, false);
		}
	}

	// Tell parent to adjust itself if we are visible
	if (IsVisible())
		static_cast<CToolbarView*>(GetSuperView())->AdjustSize();
}

void CToolbar::PositionButton(Rect& btnrect)
{
	// Center vertically
	SDimension16 psize;
	GetFrameSize(psize);
	int top_offset = (psize.height - (btnrect.bottom - btnrect.top)) / 2;

	::OffsetRect(&btnrect, mLeftJustOffset, top_offset);
	mLeftJustOffset += (btnrect.right - btnrect.left);
}

void CToolbar::AddItem(LPane* item, const CToolbarItem::CToolbarItemInfo& details)
{
	// Position appropriately
	SDimension16 fsize;
	item->GetFrameSize(fsize);
	Rect btnrect = {0, 0, fsize.height, fsize.width};
	PositionButton(btnrect);
	item->PlaceInSuperFrameAt(btnrect.left, btnrect.top, false);

	// Add item to list
	mItemList.push_back(SItemSpec(item, details));
}

void CToolbar::AddButton(LBevelButton* btn, const CToolbarItem::CToolbarItemInfo& details)
{
	// Set up toolbar button styles - and resize button
	CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(btn);
	if (tbtn)
	{
		tbtn->SetSmallIcon(mSmallIcons);
		tbtn->SetShowIcon(mShowIcons);
		tbtn->SetShowCaption(mShowCaptions);
				
		tbtn->ResizeFrameTo(GetBtnSize(tbtn).h, GetBtnSize().v, false);
	}

	// Position appropriately
	SDimension16 fsize;
	btn->GetFrameSize(fsize);
	Rect btnrect = {0, 0, fsize.height, fsize.width};
	PositionButton(btnrect);
	btn->PlaceInSuperFrameAt(btnrect.left, btnrect.top, false);

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
void CToolbar::SetCommander(LCommander* cmdr)
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
void CToolbar::AddCommander(LCommander* cmdr)
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
void CToolbar::RemoveCommander(LCommander* cmdr)
{
	// See if already in the list
	mCmdrs.erase(std::remove(mCmdrs.begin(), mCmdrs.end(), cmdr), mCmdrs.end());

	// Update state if it changes
	UpdateToolbarState();
}

LCommander* CToolbar::GetCommander() const
{
	LCommander* found = NULL;
	for(CCommanderArray::const_iterator iter = mCmdrs.begin(); iter != mCmdrs.end(); iter++)
	{
		// Always use the one which is the current target
		if ((*iter)->IsTarget())
		{
			found = *iter;
			break;
		}

		// Also use the last one if that is found in the array
		if ((*iter)->IsOnDuty() || (*iter == mLastCommander))
			found = *iter;
	}
	
	mLastCommander = found;
	return found;
}

void CToolbar::UpdateControl(LControl* ctrl, CommandT cmd, bool enable, bool show, bool caption)
{
	// Get command status from commander
	Boolean outEnabled = false;
	Boolean outUsesMark = false;
	UInt16 outMark = noMark;
	Str255 outName = "\p";
	
	// Only if commander available
	if (GetCommander())
		GetCommander()->FindCommandStatus(cmd, outEnabled, outUsesMark, outMark, outName);
	
	// Update control based on data
	if (outEnabled)
	{
		// Change enabled state if required
		if (enable)
			ctrl->Enable();

		// Change visible state if required
		if (show)
			ctrl->Show();
	}
	else
	{
		// Change enabled state if required
		if (enable)
			ctrl->Disable();

		// Change visible state if required
		if (show)
			ctrl->Hide();
	}
	
	// Mark => set value 1
	if (outUsesMark)
	{
		// Don't broadcast the change
		StopListening();
		if (outMark == noMark)
			ctrl->SetValue(0);
		else
			ctrl->SetValue(1);
		StartListening();
	}
	
	// Update caption if required
	if (caption && outEnabled)
	{
		CToolbarButton* tbtn = dynamic_cast<CToolbarButton*>(ctrl);

		// Only change if different (avoids refresh flash)
		if (tbtn)
		{
			Str255 oldcaption;
			tbtn->GetDescriptor(oldcaption);
			if (::PLstrcmp(outName, oldcaption))
				tbtn->SetDescriptor(outName);
		}
	}
}

void CToolbar::UpdateControlValue(LControl* ctrl, const CommandT* cmd, bool caption)
{
	// Scan each command status looking for one that is marked
	bool enabled = false;
	bool found_mark = false;
	short value = 1;
	const CommandT* scan = cmd;
	while(*scan)
	{
		// Get command status from commander
		Boolean outEnabled = false;
		Boolean outUsesMark = false;
		UInt16 outMark = noMark;
		Str255 outName = "\p";

		// Only if commander available
		if (GetCommander())
			GetCommander()->FindCommandStatus(*scan, outEnabled, outUsesMark, outMark, outName);
		
		// Enable entire control if any one item is enabled
		if (outEnabled)
			enabled = true;
		
		// Break out if it is marked
		if (outUsesMark && (outMark == checkMark))
		{
			found_mark = true;
			break;
		}

		scan++;
		value++;
	}
	
	// Update control based on data
	if (enabled)
		ctrl->Enable();
	else
		ctrl->Disable();
	
	// Change the control value if a mark was found
	if (found_mark)
	{
		// Don't broadcast the change
		StopListening();
		
		// Check whether we have a popup menu
		LBevelButton* bbtn = dynamic_cast<LBevelButton*>(ctrl);
		if (bbtn && bbtn->GetMacMenuH())
			// Set the popup menu value not the control value
			bbtn->SetCurrentMenuItem(value);
		else
			// Set the control value
			ctrl->SetValue(value);

		StartListening();
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
			
			LStr255 newtitle(title);
			Str255 oldtitle;
			tbtn->GetDescriptor(oldtitle);
			if (::PLstrcmp(newtitle, oldtitle))
			{
				tbtn->SetDescriptor(newtitle);
				result = true;
			}
			
		}
	}
	
	return result;
}

// Respond to clicks in the icon buttons
void CToolbar::ListenToMessage(MessageT inMessage, void *ioParam)
{
	// Look for button commands
	if ((inMessage & 0xFFFFFF00) == ('TBB1' & 0xFFFFFF00))
	{
		// Find the button for this message
		for(CItemArray::iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
		{
			if ((*iter).mWnd)
			{
				if ((*iter).mWnd->GetPaneID() == inMessage)
				{
					// Adjust ioParam to extra info if present
					if (!(*iter).mDetails.GetExtraInfo().empty())
						ioParam = const_cast<char*>((*iter).mDetails.GetExtraInfo().c_str());

					// Determine nature of command
					if (((*iter).mDetails.GetItem()->GetAltCommand() != 0) && COptionClick::sOptionKey)
						GetCommander()->ObeyCommand((*iter).mDetails.GetItem()->GetAltCommand(), ioParam);
					else
						GetCommander()->ObeyCommand((*iter).mDetails.GetItem()->GetCommand(), ioParam);
					return;
				}
				else if ((*iter).mWnd->GetPaneID() + 1 == inMessage)
				{
					// Adjust ioParam to extra info if present
					if (!(*iter).mDetails.GetExtraInfo().empty())
						ioParam = const_cast<char*>((*iter).mDetails.GetExtraInfo().c_str());

					// Determine nature of command
					if (((*iter).mDetails.GetItem()->GetMenuAltCommand() != 0) && COptionClick::sOptionKey)
						GetCommander()->ObeyCommand((*iter).mDetails.GetItem()->GetMenuAltCommand(), ioParam);
					else
						GetCommander()->ObeyCommand((*iter).mDetails.GetItem()->GetMenuCommand(), ioParam);
					return;
				}
			}
		}
	}
}

bool CToolbar::HandleContextMenuEvent(const EventRecord& cmmEvent)
{
	// Always ignore this if we are already tracking
	// This happens because the context menu attachment gets executed before the tracker attachment, so it is
	// possible for a context event to get through whilst tracking is happening.
	if ((sTracker != NULL) && (sTracker->GetOwnerHost() != NULL))
		return true;

	// Convert event point to a local point
	Point pt = cmmEvent.where;
	GlobalToPortPoint(pt);

	// Find the item under the mouse
	LPane* pane = FindShallowSubPaneContaining(pt.h, pt.v);
	
	// Always make sure fly-over acti vation is disabled when doing context
	if (pane != NULL)
		pane->MouseLeave();

	// Create popup menu
	std::auto_ptr<LMenu> popup(new LMenu(15000));
	LMenuBar::GetCurrentMenuBar()->InstallMenu(popup.get(), hierMenu);
	
	// Fill add menu with items
	LMenu* submenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(175);
	
	// Place holder for apply rules popup if present
	std::auto_ptr<LMenu> apply_rules;
	
	// Get the allowed toolbar items
	const CToolbarItem::CToolbarPtrItems& items = CToolbarManager::sToolbarManager.GetAllowedToolbarItems(GetType());
	
	// Add each one
	short pos = 1;
	unsigned long apply_rules_index = 0;
	for(CToolbarItem::CToolbarPtrItems::const_iterator iter = items.begin(); iter != items.end(); iter++, pos++)
	{
		// Insert menu item
		cdstring temp = CToolbarManager::sToolbarManager.GetDescriptor((*iter).GetItem()->GetTitleID());
		::AppendItemToMenu(submenu->GetMacMenuH(), pos, temp);

		// Look for apply rules sub menu and treat differently
		if ((*iter).GetItem()->GetTitleID() == CToolbarManager::eToolbar_ApplyRules)
		{
			apply_rules_index = pos;

			if (apply_rules.get() == NULL)
			{
				apply_rules.reset(new LMenu(176));
				LMenuBar::GetCurrentMenuBar()->InstallMenu(apply_rules.get(), hierMenu);
			}
			
			// Add the hierarchical item
			::SetMenuItemHierarchicalID(submenu->GetMacMenuH(), pos, 176);
			::SetMenuItemIconHandle(submenu->GetMacMenuH(), pos, kMenuIconSuiteType, ::Geticns((*iter).GetItem()->GetIconID()));

			// Deal with All item icon
			::SetMenuItemIconHandle(apply_rules->GetMacMenuH(), 1, kMenuIconSuiteType, ::Geticns((*iter).GetItem()->GetIconID()));

			// Disable All item if already in the toolbar
			cdstring temp(0UL);
			if (GetButton(CToolbarManager::eToolbar_ApplyRules, temp) != NULL)
				apply_rules->DisableItem(1);
			
			// Now fill the menu with apply rules items

			// Get all manual filters
			cdstrvect items;
			CPreferences::sPrefs->GetFilterManager()->GetManualFilters(items);
			
			// Add each one to menu
			short rules_pos = 3;
			for(cdstrvect::const_iterator iter2 = items.begin(); iter2 != items.end(); iter2++, rules_pos++)
			{
				::AppendItemToMenu(apply_rules->GetMacMenuH(), rules_pos, *iter2);
				
				// Add the icon
				::SetMenuItemIconHandle(apply_rules->GetMacMenuH(), rules_pos, kMenuIconSuiteType, ::Geticns((*iter).GetItem()->GetIconID()));
				
				// Get filter UID for rule
				const CFilterItem* filter = CPreferences::sPrefs->GetFilterManager()->GetFilter(CFilterItem::eLocal, *iter2);
				if (filter != NULL)
				{
					cdstring temp(filter->GetUID());

					// Disable ones already in the toolbar
					if (GetButton(CToolbarManager::eToolbar_ApplyRules, temp) != NULL)
						apply_rules->DisableItem(rules_pos);
				}
			}
		}
		else
		{
			// Add the icon
			::SetMenuItemIconHandle(submenu->GetMacMenuH(), pos, kMenuIconSuiteType, ::Geticns((*iter).GetItem()->GetIconID()));
			
			// See whether item is already present (allow multiple separators and ordinary space)
			if (((*iter).GetItem()->GetType() != CToolbarItem::eSeparator) &&
				((*iter).GetItem()->GetType() != CToolbarItem::eSpace) &&
				(GetItem(static_cast<CToolbarManager::EToolbarItem>((*iter).GetItem()->GetTitleID())) != NULL))
				submenu->DisableItem(pos);
		}
	}

	// Remove button specific items if not over a button
	unsigned long item_offset = 0;
	Rect portRect;
	bool drawing = false;
	if (pane == NULL)
	{
		popup->RemoveItem(1);
		popup->RemoveItem(1);
		popup->RemoveItem(1);
		item_offset = 3;
	}
	else
	{
		// Adjust titles of menu items
		const CToolbarItem* details = mItemList.at(GetItemIndex(pane)).mDetails.GetItem();
		cdstring btnname = CToolbarManager::sToolbarManager.GetTitle(details->GetTitleID());
		{
			cdstring temp = ::GetMenuItemTextUTF8(popup->GetMacMenuH(), ePopup_RemoveButton);
			temp.Substitute(btnname);
			::SetMenuItemTextUTF8(popup->GetMacMenuH(), ePopup_RemoveButton, temp);
		}
		{
			cdstring temp = ::GetMenuItemTextUTF8(popup->GetMacMenuH(), ePopup_MoveButton);
			temp.Substitute(btnname);
			::SetMenuItemTextUTF8(popup->GetMacMenuH(), ePopup_MoveButton, temp);
		}

		// Draw insert marker
		
		// Is click to left or right side of current item
		bool left = true;
		if (pane->CalcPortFrameRect(portRect))
		{
			left = (pt.h < (portRect.left + portRect.right) / 2);
		}
		
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
		
		Rect myPort;
		CalcPortFrameRect(myPort);
		portRect.top = myPort.top;
		portRect.bottom = myPort.bottom;
		
		// Now draw the rect
		FocusDraw();
		PortToLocalPoint(topLeft(portRect));
		PortToLocalPoint(botRight(portRect));
		RGBColor temp = UGAColorRamp::GetColor(7);
		::RGBForeColor(&temp);
		::PaintRect(&portRect);
		drawing = true;
	}

	// Do popup menu of suggestions
	Point startPt = cmmEvent.where;
	SInt32 popup_result = ::PopUpMenuSelect(popup->GetMacMenuH(), startPt.v, startPt.h, 0);
	short menu_hit = HiWord(popup_result);
	unsigned short item_hit = LoWord(popup_result);

	// Remove any drawn cursor
	if (drawing)
	{
		FocusDraw();
		RefreshRect(portRect);
		UpdatePort();
	}

	if (item_hit != 0)
	{
		bool handled = false;
		if (menu_hit == 15000)
		{
			switch(item_hit + item_offset)
			{
			case ePopup_RemoveButton:
				RemoveButton(pane);
				break;
			case ePopup_MoveButton:
				MoveButton(pane);
				break;
			case ePopup_AddButton:
				break;
			case ePopup_ResetButtons:
				ResetButtons(true);
				break;
			default:;
			}
		}
		else if (menu_hit == 175)
		{
			// Get the item
			CToolbarManager::sToolbarManager.AddItemAt(GetType(), item_hit - 1, GetItemIndex(pane));

			// Rebuild the toolbar
			ResetButtons(false);
		}
		else if (menu_hit == 176)
		{
			// Get uid for item
			unsigned long uid = (item_hit == 1) ? 0 : CPreferences::sPrefs->GetFilterManager()->GetManualUID(item_hit  - 3);
			cdstring uid_txt(uid);

			// Get the item
			CToolbarManager::sToolbarManager.AddItemAt(GetType(), apply_rules_index - 1, GetItemIndex(pane), uid_txt);

			// Rebuild the toolbar
			ResetButtons(false);
		}
	}
	
	if (apply_rules.get() != NULL)
		LMenuBar::GetCurrentMenuBar()->RemoveMenu(apply_rules.get());
	LMenuBar::GetCurrentMenuBar()->RemoveMenu(submenu);
	LMenuBar::GetCurrentMenuBar()->RemoveMenu(popup.get());

	return true;
}

void CToolbar::ResizeFrameBy(SInt16 inWidthDelta, SInt16 inHeightDelta, Boolean inRefresh)
{
	// Do default then reset toolbar layout
	LView::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	if ((inWidthDelta != 0) || (inHeightDelta != 0))
		ResetLayout();
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

LPane* CToolbar::GetItem(CToolbarManager::EToolbarItem item) const
{
	// Find the button for this message
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if (((*iter).mDetails.GetItem() != NULL) && (static_cast<CToolbarManager::EToolbarItem>((*iter).mDetails.GetItem()->GetTitleID()) == item))
			return (*iter).mWnd;
	}
	
	return NULL;
}

unsigned long CToolbar::GetItemIndex(LPane* item) const
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

void CToolbar::RemoveButton(LPane* pane)
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

void CToolbar::MoveButton(LPane* pane)
{

	if (sTracker == NULL)
		sTracker = new CToolbarTrackAttachment(this);
	else
		sTracker->SetToolbar(this);

	// Install an attachment thatgrabs mouse movements for tracking
	CMulberryApp::sApp->AddAttachment(sTracker);
	
	TrackStart(pane);
}

bool CToolbar::TrackMove(EventRecord* event, CToolbarTrackAttachment* tracker)
{
	switch (event->what)
	{
	case mouseDown:
		if (!mFirstMouseDown)
		{
			// Do action
			//TrackAction();

			// This terminates the tracking
			CMulberryApp::sApp->RemoveAttachment(sTracker);
			TrackStop();
		}
		else
			mFirstMouseDown = false;
		return false;

	case keyDown:
	case autoKey:
	case keyUp:
		// This cancels the tracking
		CMulberryApp::sApp->RemoveAttachment(sTracker);
		TrackStop();
		return false;

	case mouseUp:
	case diskEvt:
	case updateEvt:
	case activateEvt:
	case osEvt:
	case kHighLevelEvent:
		// Allow these event to be processed normally
		return true;
	default:
		// Do tracking
		TrackMouseMove();
		return false;
	}
}

void CToolbar::TrackStart(LPane* pane)
{
	mDragIndex = GetItemIndex(pane);
	mFirstMouseDown = true;

	CMulberryApp::sApp->SetTrackMouse(true);

	FocusDraw();

	// Tell each control to be active
	for(CItemArray::const_iterator iter = mItemList.begin(); iter != mItemList.end(); iter++)
	{
		if ((*iter).mWnd != NULL)
		{
			(*iter).mWnd->Enable();
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

	// Always reset state
	UpdateToolbarState();
	Refresh();

	CMulberryApp::sApp->SetTrackMouse(false);
}

void CToolbar::TrackMouseMove()
{
	FocusDraw();

	// Determine new pane
	Point pt;
	::GetMouse(&pt);
	//GlobalToPortPoint(pt);

	// Find the item under the mouse
	unsigned long new_index = mDragIndex;
	LPane* new_pane = FindShallowSubPaneContaining(pt.h, pt.v);
	if (new_pane != NULL)
	{
		new_index = GetItemIndex(new_pane);

		// Apply hysteresis to movement to ensure panes with different sizes do not cause
		// rapid switches
		if (new_index != mDragIndex)
		{
			Rect old_portRect;
			mItemList.at(mDragIndex).mWnd->CalcPortFrameRect(old_portRect);
				
			Rect new_portRect;
			mItemList.at(new_index).mWnd->CalcPortFrameRect(new_portRect);
			
			if (new_index < mDragIndex)
			{
				// Only use new index if the mouse is within the old button size of the left edge of the new button
				if (pt.h >= new_portRect.left + (old_portRect.right - old_portRect.left))
					new_index = mDragIndex;
			}
			else
			{
				// Only use new index if the mouse is within the old button size of the right edge of the new button
				if (pt.h < new_portRect.right - (old_portRect.right - old_portRect.left))
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
		Refresh();
		FlushPortBuffer();						// Force drawing
		
		mDragIndex = new_index;
	}
		
	// Draw highlight around moved button
	TrackHighlight();
}

void CToolbar::TrackHighlight()
{
	Rect portRect;
	mItemList.at(mDragIndex).mWnd->CalcPortFrameRect(portRect);
	PortToLocalPoint(topLeft(portRect));
	PortToLocalPoint(botRight(portRect));
	RGBColor temp = UGAColorRamp::GetColor(7);
	::RGBForeColor(&temp);
	::PenSize(4, 4);
	::FrameRect(&portRect);
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
	Refresh();
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
