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
		fontNames.SetCompareFunction(JCompareStringsCaseInsensitive);
		fontNames.SetSortOrder(JOrderedSetT::kSortAscending);

		// Look for cache
		JPtrArray<JString>*& fontcache = (mMonoSpaced ? itsXMonoFontNames : itsXFontNames);
		if (fontcache)
			fontNames.CopyObjects(*fontcache, fontNames.GetCleanUpAction(), kJFalse);
		else
		{
			// Cache screen items
			CacheScreenMenu(mMonoSpaced || !quick);
			fontNames.CopyObjects(*fontcache, fontNames.GetCleanUpAction(), kJFalse);
		}
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

void CFontNameMenu::GetXFontNames(JPtrArray<JString>* fontNames)
{
	fontNames->DeleteAll();

	int nameCount;
	char** nameList = ::XListFonts(*GetDisplay(), "*", INT_MAX, &nameCount);
	if (nameList == NULL)
	{
		return;
	}

	for (int i=0; i<nameCount; i++)
	{
		if (::strcmp(nameList[i], "nil") != 0)
		{
			JString name = nameList[i];
			fontNames->Append(name);
		}
	}

	::XFreeFontNames(nameList);
}

void CFontNameMenu::CacheScreenMenu(bool do_mono)
{
	// Set up arrays for holding all fonts and mono fonts
	JPtrArray<JString> allfontNames(JPtrArrayT::kDeleteAll);
	allfontNames.SetCompareFunction(JCompareStringsCaseInsensitive);
	allfontNames.SetSortOrder(JOrderedSetT::kSortAscending);

	JPtrArray<JString> monofontNames(JPtrArrayT::kDeleteAll);
	monofontNames.SetCompareFunction(JCompareStringsCaseInsensitive);
	monofontNames.SetSortOrder(JOrderedSetT::kSortAscending);

	// Get all font names - never mind regex matching
	JPtrArray<JString> xfontNames(JPtrArrayT::kDeleteAll);
	xfontNames.SetCompareFunction(JCompareStringsCaseInsensitive);
	xfontNames.SetSortOrder(JOrderedSetT::kSortAscending);
	GetXFontNames(&xfontNames);
	
	// Now parse each name removing duplicates
	const JSize xcount = xfontNames.GetElementCount();
	JString last_name;
	for (JIndex i = 1; i <= xcount; i++)
	{
		// Get copy and tokenize
		JString name;
		JString charset;
		cdstring xfontName(*xfontNames.NthElement(i));

		// Must start with '-'
		if (xfontName[0UL] != '-')
			continue;

		// Foundary - skip
		char* p = ::strtok(xfontName.c_str_mod(), "-");

		// Type face
		p = ::strtok(NULL, "-");
		if (p)
			name = p;

		if (name.IsEmpty() || name == "nil")
			continue;

		// ConvertToPSFontName
		const JSize length = name.GetLength();
		for (JIndex j=1; j<=length; j++)
		{
			if (j == 1 || name.GetCharacter(j-1) == ' ')
				name.SetCharacter(j, toupper(name.GetCharacter(j)));
		}

		// Do quick duplicate test - assumes font family variants are all returned
		// consequetively
		if (name != last_name)
		{
			JBoolean isDuplicate;
			const JIndex index = allfontNames.GetInsertionSortIndex(&name, &isDuplicate);
			if (!isDuplicate)
			{
				// Add to checked list
				JString* n = new JString(name);
				assert( n != NULL );
				allfontNames.InsertAtIndex(index, n);

				// Now check for mono-space if required
				if (do_mono)
				{
					bool do_insert = false;
					XFontStruct* xfont = ::XLoadQueryFont(*GetDisplay(), *xfontNames.NthElement(i));
					if (xfont != NULL)
					{
						do_insert = (xfont->min_bounds.width == xfont->max_bounds.width);
						::XFreeFont(*GetDisplay(), xfont);
					}

					// Add to mono list if required
					if (do_insert)
					{
						const JIndex mindex = monofontNames.GetInsertionSortIndex(&name, &isDuplicate);
						n = new JString(name);
						assert( n != NULL );
						monofontNames.InsertAtIndex(mindex, n);
					}
				}
			}

			// Cache name for fast duplicate compare
			last_name = name;
		}
	}

	xfontNames.DeleteAll();

	// Cache names for next time
	if (itsXFontNames == NULL)
	{
		itsXFontNames = new JPtrArray<JString>(JPtrArrayT::kDeleteAll, allfontNames.GetElementCount());
		assert( itsXFontNames != NULL );
		itsXFontNames->CopyObjects(allfontNames, itsXFontNames->GetCleanUpAction(), kJFalse);
	}

	if (itsXMonoFontNames == NULL)
	{
		itsXMonoFontNames = new JPtrArray<JString>(JPtrArrayT::kDeleteAll, monofontNames.GetElementCount());
		assert( itsXMonoFontNames != NULL );
		itsXMonoFontNames->CopyObjects(monofontNames, itsXMonoFontNames->GetCleanUpAction(), kJFalse);
	}

	// Clean up
	allfontNames.DeleteAll();
	monofontNames.DeleteAll();
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
