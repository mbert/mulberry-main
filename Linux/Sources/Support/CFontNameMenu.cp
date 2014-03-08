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


#include "CFontNameMenu.h"

#include "cdstring.h"

#include <jXGlobals.h>
#include <JRegex.h>
#include <JString.h>
#include <JXDisplay.h>
#include <JXFontManager.h>
#include <jAssert.h>

JPtrArray<JString>*	CFontNameMenu::itsXFontNames = NULL;
JPtrArray<JString>*	CFontNameMenu::itsXMonoFontNames = NULL;

// JBroadcaster message types

const JCharacter* CFontNameMenu::kNameNeedsUpdate = "NameNeedsUpdate::JXFontNameMenu";
const JCharacter* CFontNameMenu::kNameChanged     = "NameChanged::JXFontNameMenu";

/******************************************************************************
 Constructor (protected)

 ******************************************************************************/

CFontNameMenu::CFontNameMenu
	(
	const JCharacter*	title,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	CMenu(title, enclosure, hSizing, vSizing, x,y, w,h)
{
	CFontNameMenuX();
}

CFontNameMenu::CFontNameMenu(JXImage* image, const JBoolean menuOwnsImage,
		   JXContainer* enclosure,
		   const HSizingOption hSizing, const VSizingOption vSizing,
		   const JCoordinate x, const JCoordinate y,
		   const JCoordinate w, const JCoordinate h)
	: CMenu(image, menuOwnsImage, enclosure, hSizing, vSizing, x,y, w,h)
{
	CFontNameMenuX();
}

CFontNameMenu::CFontNameMenu
	(
	JXMenu*			owner,
	const JIndex	itemIndex,
	JXContainer*	enclosure
	)
	:
	CMenu(owner, itemIndex, enclosure)
{
	CFontNameMenuX();
}


void CFontNameMenu::CFontNameMenuX()
{
	itsBroadcastNameChangeFlag = kTrue;

	mForScreen = true;
	mForPrinter = false;
	mMonoSpaced = false;
	mDisplayWithFont = false;
}

/******************************************************************************
 Destructor

 ******************************************************************************/

CFontNameMenu::~CFontNameMenu()
{
}

/******************************************************************************
 GetFontName (private)

 ******************************************************************************/

JString
CFontNameMenu::GetFontName
	(
	const JIndex index
	)
	const
{
	JString name = GetItemText(index);

	JString charSet;
	if (GetItemNMShortcut(index, &charSet))
		{
		charSet.TrimWhitespace();
		name = JFontManager::CombineNameAndCharacterSet(name, charSet);
		}

	return name;
}

/******************************************************************************
 SetFontName

 ******************************************************************************/

void
CFontNameMenu::SetFontName
	(
	const JCharacter* name
	)
{
JIndex i;

	const JIndex count = GetItemCount();
	for (i=1; i<=count; i++)
		{
		if (GetFontName(i) == name)
			{
			SetFontName1(i);
			return;
			}
		}

	// catch the case where they don't specify the char set

	for (i=1; i<=count; i++)
		{
		if (GetItemText(i) == name)
			{
			SetFontName1(i);
			return;
			}
		}
}

// private

void
CFontNameMenu::SetFontName1
	(
	const JIndex index
	)
{
	const JIndex origFontIndex = itsFontIndex;

	itsFontIndex = index;
	SetPopupChoice(itsFontIndex);
	if (itsBroadcastNameChangeFlag && itsFontIndex != origFontIndex)
		{
		Broadcast(NameChanged());
		}
}

// Build menu for various types of output device
void CFontNameMenu::BuildMenu()
{
	// Don't build then menu until we actually need it

	// However we must have at least one item as size menu will request it
	AppendItem(JGetDefaultFontName(), kTrue, kTrue);

	itsFontIndex = 1;
	SetFontName(JGetDefaultFontName());
	ListenTo(this);
}

// Build menu for various types of output device
void CFontNameMenu::BuildMenuNow(bool quick)
{
	RemoveAllItems();

	JPtrArray<JString> fontNames(JPtrArrayT::kDeleteAll);

	if (mForPrinter)
		(GetFontManager())->GetFontNames(&fontNames);
	else if (mForScreen)
	{
		(GetFontManager())->GetFontNames(&fontNames);
	}
	else if (mMonoSpaced)
		(GetFontManager())->GetMonospaceFontNames(&fontNames);

	const JSize count = fontNames.GetElementCount();
	JString name, charSet;
	for (JIndex i=1; i<=count; i++)
	{
		const JString* fontName = fontNames.NthElement(i);
		const JBoolean hasCharSet = JFontManager::ExtractCharacterSet(*fontName, &name, &charSet);

		AppendItem(name, kTrue, kTrue);
		if (hasCharSet)
		{
			charSet.PrependCharacter(' ');
			charSet.AppendCharacter(' ');
			SetItemNMShortcut(i, charSet);
		}

		if (mDisplayWithFont)
			SetItemFontName(i, *fontName);
	}
	fontNames.DeleteAll();

	SetUpdateAction(kDisableNone);

	itsFontIndex = 1;
	SetFontName(JGetDefaultFontName());
	ListenTo(this);
}

/******************************************************************************
 Receive (protected)

 ******************************************************************************/

void
CFontNameMenu::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == this && message.Is(JXMenu::kNeedsUpdate))
		{
		itsBroadcastNameChangeFlag = kFalse;
		Broadcast(NameNeedsUpdate());
		CheckItem(itsFontIndex);
		EnableAll();
		itsBroadcastNameChangeFlag = kTrue;
		}
	else if (sender == this && message.Is(JXMenu::kItemSelected))
		{
		const JXMenu::ItemSelected* selection =
			dynamic_cast(const JXMenu::ItemSelected*, &message);
		assert( selection != NULL );
		itsFontIndex = selection->GetIndex();
		Broadcast(NameChanged());
		}

	else
		{
		CMenu::Receive(sender, message);
		}
}

/******************************************************************************
 SetToPopupChoice

 ******************************************************************************/

void
CFontNameMenu::SetToPopupChoice
	(
	const JBoolean isPopup
	)
{
	CMenu::SetToPopupChoice(isPopup, itsFontIndex);
}
