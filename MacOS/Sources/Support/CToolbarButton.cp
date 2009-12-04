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


// Source for CToolbarButton class

#include "CToolbarButton.h"

#include "CMulberryCommon.h"
#include "COptionClick.h"
#include "CToolbarButtonImp.h"
#include "CToolbarItem.h"
#include "CTooltip.h"

#include <memory>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CToolbarButton::CToolbarButton(LStream *inStream)
	: LBevelButton(inStream, imp_class_ID)
{
	mSmallIcon = false;
	mShowIcon = true;
	mShowCaption = true;
	mIconToggle = false;
	mHasTooltip = false;
}

CToolbarButton::CToolbarButton(
		const SPaneInfo	&inPaneInfo,
		MessageT		inValueMessage,
		SInt16			inBevelProc,
		SInt16			inBehavior,
		SInt16			inContentType,
		SInt16			inContentResID,
		ResIDT			inTextTraits,
		ConstStringPtr	inTitle,
		SInt16			inInitialValue,
		SInt16			inTitlePlacement,
		SInt16			inTitleAlignment,
		SInt16			inTitleOffset,
		SInt16			inGraphicAlignment,
		Point			inGraphicOffset)
	: LBevelButton(inPaneInfo, inValueMessage, inBevelProc, inBehavior, inContentType, inContentResID, inTextTraits, inTitle,
					inInitialValue, inTitlePlacement, inTitleAlignment, inTitleOffset, inGraphicAlignment, inGraphicOffset, imp_class_ID)
{
	mSmallIcon = false;
	mShowIcon = true;
	mShowCaption = true;
	mIconToggle = false;
	mHasTooltip = false;
	mDragMode = false;
	mValueMenu = false;
}

CToolbarButton::CToolbarButton(
		const SPaneInfo	&inPaneInfo,
		MessageT		inValueMessage,
		SInt16			inBevelProc,
		ResIDT			inMenuID,
		SInt16			inMenuPlacement,
		SInt16			inContentType,
		SInt16			inContentResID,
		ResIDT			inTextTraits,
		ConstStringPtr	inTitle,
		SInt16			inInitialValue,
		SInt16			inTitlePlacement,
		SInt16			inTitleAlignment,
		SInt16			inTitleOffset,
		SInt16			inGraphicAlignment,
		Point			inGraphicOffset,
		Boolean			inCenterPopupGlyph)
	: LBevelButton(inPaneInfo, inValueMessage, inBevelProc, inMenuID, inMenuPlacement, inContentType, inContentResID, inTextTraits,
					inTitle, inInitialValue, inTitlePlacement, inTitleAlignment, inTitleOffset, inGraphicAlignment, inGraphicOffset, inCenterPopupGlyph, imp_class_ID)
{
	mSmallIcon = false;
	mShowIcon = true;
	mShowCaption = true;
	mIconToggle = false;
	mHasTooltip = false;
	mDragMode = false;
	mValueMenu = false;
}

// Default destructor
CToolbarButton::~CToolbarButton()
{
	// Always hide tooltip which has a pointer to this cached
	if (mHasTooltip)
		CTooltip::HideTooltip();
}

void CToolbarButton::SetDetails(const CToolbarItem*	item)
{
	// Set icon toggle state
	SetIconToggle(item->IsToggleIcon());
	
	// Add option click attachment if alt command is present
	if (item->GetAltCommand() != 0 || item->GetMenuAltCommand() != 0)
		AddAttachment(new COptionClick);
}

// Set value AFTER all other changes to prevent delete error
void CToolbarButton::SetValue(SInt32 inValue)
{
	// Only change the value if we have too
	if (mValue != inValue)
	{
		// Get the value setup last
		LBevelButton::SetValue(inValue);
		
		// Force redraw
		Draw(NULL);
	}
}

// Set menu value for case of click and popup
void CToolbarButton::SetMenuValue(SInt32 inValue, bool silent)
{
	SetCurrentMenuItem(inValue);						
	
	// Broadcast change if required
	if (!silent)
	{
		SInt32	longChoice = mMenuChoice;
		MessageT msg = mValueMessage;
		if (static_cast<CToolbarButtonImp*>(mControlImp)->GetClickAndPopup())
			msg++;
		BroadcastMessage(msg, &longChoice);
	}
}

void CToolbarButton::SetDescriptor(ConstStringPtr inDescriptor)
{
	// Force redraw
	LBevelButton::SetDescriptor(inDescriptor);
		
	// Force redraw
	Draw(NULL);
}

void CToolbarButton::MouseEnter(Point inPortPt, const EventRecord&	inMacEvent)
{
	// Turn on button outline
	static_cast<CToolbarButtonImp*>(mControlImp)->SetMouseInside(true && !mDragMode);
	
	// Use tooltip if no caption and active
	if (!mShowCaption && IsActive())
	{
		// Use title as tooltip, but only show tooltip if title exists
		Str255 title;
		GetDescriptor(title);
		if (*title)
		{
			// Start tooltip timer
			CTooltip::ShowTooltip(this, title);
			mHasTooltip = true;
		}
	}
}

void CToolbarButton::MouseLeave()
{
	// Turn off button outline
	static_cast<CToolbarButtonImp*>(mControlImp)->SetMouseInside(false);
	
	// Hide tooltip window
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
	}
}

void CToolbarButton::SetSmallIcon(bool small_icon)
{
	if (mSmallIcon ^ small_icon)
	{
		mSmallIcon = small_icon;

		// Load in new icon sizes
		//LoadIcons(nIconIDBase + (small_icon ? 1 : 0), 0, 0, 0);
		static_cast<CToolbarButtonImp*>(mControlImp)->SetSmallIcon(mSmallIcon);
		Refresh();
	}
}

void CToolbarButton::SetShowIcon(bool icon)
{
	if (mShowIcon ^ icon)
	{
		mShowIcon = icon;
		static_cast<CToolbarButtonImp*>(mControlImp)->SetShowIcon(mShowIcon);
		Refresh();
	}
}

void CToolbarButton::SetShowCaption(bool caption)
{
	if (mShowCaption ^ caption)
	{
		mShowCaption = caption;
		static_cast<CToolbarButtonImp*>(mControlImp)->SetShowCaption(mShowCaption);
		Refresh();
	}
}

bool CToolbarButton::GetClickAndPopup() const
{
	return static_cast<CToolbarButtonImp*>(mControlImp)->GetClickAndPopup();
}

void CToolbarButton::SetClickAndPopup(bool clickpopup)
{
	static_cast<CToolbarButtonImp*>(mControlImp)->SetClickAndPopup(clickpopup);
}

void CToolbarButton::DeactivateSelf()
{
	// Hide tooltip
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
	}
	
	LBevelButton::DeactivateSelf();
}

void CToolbarButton::ClickSelf(const SMouseDownEvent& inMouseDown)
{
	// Hide tooltip
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
	}
	
	LBevelButton::ClickSelf(inMouseDown);
}

Boolean CToolbarButton::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always sync menu state when clicked
	if (HasPopup() && LCommander::GetTarget())
	{
		// Update status of each menu item with a command
		std::auto_ptr<LMenu> menu(new LMenu(GetMenuID()));
		
		SInt16	index = 0;
		SInt32	command;
		while (menu->FindNextCommand(index, command))
		{
			// Get the item index
			SInt16 index = menu->IndexFromCommand(command);
			
			// Get the command status
			Boolean		isEnabled;
			Boolean		usesMark;
			UInt16		mark;
			Str255		itemName;

			isEnabled   = false;
			usesMark    = false;
			itemName[0] = 0;
			
			if (LCommander::GetTarget() != NULL)
				LCommander::GetTarget()->ProcessCommandStatus(command, isEnabled, usesMark, mark, itemName);
			
			// Now update the item in the button's menu
			if (isEnabled) {
#if TARGET_API_MAC_CARBON

				::MacEnableMenuItem(GetMacMenuH(), (MenuItemIndex) index);

#else

				::EnableItem(GetMacMenuH(), inIndex);

#endif
			} else {
#if TARGET_API_MAC_CARBON

				::DisableMenuItem(GetMacMenuH(), (MenuItemIndex) index);

#else

				::DisableItem(GetMacMenuH(), index);

#endif
			}

			if (usesMark) {
				::SetItemMark(GetMacMenuH(), index, (SInt16) mark);
			}

			if (itemName[0] > 0) {
				::SetMenuItemText(GetMacMenuH(), index, itemName);
			}
		}
		
		// Make sure button value does not cause a marcj to appear
		if (!mValueMenu)
			SetMenuValue(0, true);
	}
	
	// Do default
	return LBevelButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

void CToolbarButton::HotSpotResult(SInt16	/* inHotSpot */)
{
	if (static_cast<CToolbarButtonImp*>(mControlImp)->GetClickOnPopup())
	{
										//   with selected item number
		SInt16	choice;								
		mControlImp->GetDataTag(0, kControlBevelButtonMenuValueTag,
								sizeof(SInt16), &choice);
		SetMenuValue(choice);						
	}
	else
		BroadcastValueMessage();		// Value doesn't change but
}
