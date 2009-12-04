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


// Source for CServerViewPopup class

#include "CServerViewPopup.h"

#include "CMailAccountManager.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"

const int cTickIcon = 1;
const int cNoTickIcon = 2;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CServerViewPopup::CServerViewPopup(LStream *inStream)
		: CToolbarButton(inStream)
{
	mDirty = true;
}

CServerViewPopup::CServerViewPopup(
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
	: CToolbarButton(inPaneInfo, inValueMessage, inBevelProc, inMenuID, inMenuPlacement, inContentType, inContentResID, inTextTraits,
					inTitle, inInitialValue, inTitlePlacement, inTitleAlignment, inTitleOffset, inGraphicAlignment, inGraphicOffset, inCenterPopupGlyph)
{
	mDirty = true;
}


// Default destructor
CServerViewPopup::~CServerViewPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CServerViewPopup::FinishCreateSelf()
{
	// Do inherited
	CToolbarButton::FinishCreateSelf();

	if (CMailAccountManager::sMailAccountManager != NULL)
	{
		SyncMenu();
		CMailAccountManager::sMailAccountManager->Add_Listener(this);
	}
}

// Respond to list changes
void CServerViewPopup::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMboxProtocol::eBroadcast_NewList:
	case CTreeNodeList::eBroadcast_ResetList:
	case CMboxProtocol::eBroadcast_RemoveList:
		// Just mark as dirty
		mDirty = true;
		break;
	default:;
	}
}

// Set size
void CServerViewPopup::SetValue(SInt32 inValue)
{
	CToolbarButton::SetValue(inValue);

	// Force off so that any future selection will broadcast change
	mValue = 0;
}

Boolean CServerViewPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always sync menu state when clicked
	SetupCurrentMenuItem(GetMacMenuH(), 0);
	
	// Do default
	return CToolbarButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

void CServerViewPopup::SetupCurrentMenuItem(MenuHandle inMenuH, SInt16 inCurrentItem)
{
	// ¥ If the current item has changed then make it so, this
	// also involves removing the mark from any old
	if ( inMenuH )
	{
		// Check dirty state and resync
		if (mDirty)
			SyncMenu();

		short index = eServerView_First;
		::SetItemMark(inMenuH, eServerView_New, noMark);
		for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++, index++)
			::SetItemMark(inMenuH, index, (*iter)->IsVisible() ? (UInt16)checkMark : (UInt16)noMark);
	}
}

void CServerViewPopup::SyncMenu(void)
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(GetMacMenuH());
	for(short i = eServerView_First; i <= num_menu; i++)
		::DeleteMenuItem(GetMacMenuH(), eServerView_First);

	short index = eServerView_First;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++, index++)
		::AppendItemToMenu(GetMacMenuH(), index, (*iter)->GetName());

	// Reset value & max & min
	mValue = 0;
	SetMenuMinMax();

	// Reset dirty state
	mDirty = false;

	// Always make sure it is listening
	CMailAccountManager::sMailAccountManager->Add_Listener(this);
}
