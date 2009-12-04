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


// Source for CSelectPopup class

#include "CSelectPopup.h"

#include "CMailboxInfoTable.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSelectPopup::CSelectPopup(LStream *inStream)
		: CToolbarButton(inStream)
{
}

CSelectPopup::CSelectPopup(
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
}

// Default destructor
CSelectPopup::~CSelectPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set size
void CSelectPopup::SetValue(SInt32 inValue)
{
	CToolbarButton::SetValue(inValue);

	// Force off so that any future selection will broadcast change
	mValue = 0;
}

Boolean CSelectPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always sync menu state when clicked
	SetupCurrentMenuItem(GetMacMenuH(), 0);
	
	// Do default
	return CToolbarButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

void CSelectPopup::SetupCurrentMenuItem(MenuHandle inMenuH, SInt16 inCurrentItem)
{
	// ¥ If the current item has changed then make it so, this
	// also involves removing the mark from any old
	if ( inMenuH )
	{
		// Change name of labels
		for(short i = CMailboxInfoTable::eSelectBy_Label1; i <  CMailboxInfoTable::eSelectBy_Label1 + NMessage::eMaxLabels; i++)
		{
			::SetMenuItemTextUTF8(inMenuH, i, CPreferences::sPrefs->mLabels.GetValue()[i -  CMailboxInfoTable::eSelectBy_Label1]->name);
		}
		
		// Remove all marks
		for(short i = 1; i <  ::CountMenuItems(inMenuH); i++)
			::SetItemMark(inMenuH, i, noMark);
		SetMenuValue(0, true);

	}
}
