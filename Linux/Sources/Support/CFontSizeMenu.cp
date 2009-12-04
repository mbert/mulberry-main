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


// CFontSizeMenu.cp : implementation file
//

#include "CFontSizeMenu.h"

#include "CFontNameMenu.h"

#include "cdstring.h"

#include <JXChooseFontSizeDialog.h>
#include <JFontManager.h>
#include <JXWindow.h>
#include <jXGlobals.h>

#include <cassert>

static const JCharacter* kVarSizeStr = "Other...";

// JBroadcaster message types

const JCharacter* CFontSizeMenu::kSizeNeedsUpdate = "SizeNeedsUpdate::JXFontSizeMenu";
const JCharacter* CFontSizeMenu::kSizeChanged     = "SizeChanged::JXFontSizeMenu";

CFontSizeMenu::CFontSizeMenu
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
}

CFontSizeMenu::CFontSizeMenu(JXImage* image, const JBoolean menuOwnsImage,
		   JXContainer* enclosure,
		   const HSizingOption hSizing, const VSizingOption vSizing,
		   const JCoordinate x, const JCoordinate y,
		   const JCoordinate w, const JCoordinate h)
	: CMenu(image, menuOwnsImage, enclosure, hSizing, vSizing, x,y, w,h)
{
}

CFontSizeMenu::CFontSizeMenu
	(
	JXMenu* owner,
	const JIndex itemIndex,
	JXContainer* enclosure
	)
	:
	CMenu(owner, itemIndex, enclosure)
{
}

void CFontSizeMenu::OnCreate(CFontNameMenu* fontMenu)
{
	CFontSizeMenuX(fontMenu);
	const JString fontName = itsFontNameMenu->GetFontName();
	BuildMenu(fontName);
	SetFontSize(kJXDefaultFontSize);
}


/******************************************************************************
 SetFontSize

 ******************************************************************************/

void
CFontSizeMenu::SetFontSize
	(
	const JSize size
	)
{
	assert( size > 0 );

	const JString itemText(size, 0, JString::kForceNoExponent);

	JIndex newIndex = 0;
	const JIndex count = GetItemCount();
	for (JIndex i=1; i<=count; i++)
		{
		if (GetItemText(i) == itemText)
			{
			newIndex = i;
			break;
			}
		}

	if (newIndex > 0)
		{
		itsCurrIndex = newIndex;
		itsFontSize  = size;
		AdjustVarSizeItem(0);
		}
	else if (itsVarSizeIndex > 0)
		{
		itsCurrIndex = itsVarSizeIndex;
		itsFontSize  = size;
		AdjustVarSizeItem(itsFontSize);
		}

	SetPopupChoice(itsCurrIndex);
	if (itsBroadcastSizeChangeFlag)
		{
		Broadcast(SizeChanged(itsFontSize));
		}
}

/******************************************************************************
 BuildMenu (private)

	Caller should call SetFontSize() afterwards.

 ******************************************************************************/

void
CFontSizeMenu::BuildMenu
	(
	const JCharacter* fontName
	)
{
	RemoveAllItems();

	JSize minSize, maxSize;
	JArray<JSize> sizeList;
	(GetFontManager())->GetFontSizes(fontName, &minSize, &maxSize, &sizeList);

	const JSize count = sizeList.GetElementCount();
	if (count > 0)
		{
		for (JIndex i=1; i<=count; i++)
			{
			const JString itemText(sizeList.GetElement(i), 0, JString::kForceNoExponent);
			AppendItem(itemText, kTrue, kTrue);
			}
		itsVarSizeIndex = 0;
		}
	else
		{
		for (JSize size=minSize; size<=maxSize; size+=2)
			{
			const JString itemText(size, 0, JString::kForceNoExponent);
			AppendItem(itemText, kTrue, kTrue);
			}
		ShowSeparatorAfter(GetItemCount());
		AppendItem(kVarSizeStr, kTrue, kTrue);
		itsVarSizeIndex = GetItemCount();
		}
	SetUpdateAction(kDisableNone);

	itsCurrIndex = 1;
	itsFontSize  = minSize;

	ListenTo(this);
}

/******************************************************************************
 AdjustVarSizeItem (private)

 ******************************************************************************/

void
CFontSizeMenu::AdjustVarSizeItem
	(
	const JSize fontSize
	)
{
	if (itsVarSizeIndex > 0 && fontSize > 0)
		{
		JString str = kVarSizeStr;
		str += " (";
		str += JString(fontSize, 0, JString::kForceNoExponent);
		str += ")";
		SetItemText(itsVarSizeIndex, str);
		}
	else if (itsVarSizeIndex > 0)
		{
		SetItemText(itsVarSizeIndex, kVarSizeStr);
		}
}

void CFontSizeMenu::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == itsFontNameMenu && message.Is(CFontNameMenu::kNameChanged))
		{
		itsBroadcastSizeChangeFlag = kFalse;
		const JSize currSize       = GetFontSize();
		const JString fontName     = itsFontNameMenu->GetFontName();
		BuildMenu(fontName);
		SetFontSize(currSize);
		itsBroadcastSizeChangeFlag = kTrue;
		}

	else if (sender == this && message.Is(JXMenu::kNeedsUpdate))
		{
		itsBroadcastSizeChangeFlag = kFalse;
		Broadcast(SizeNeedsUpdate());
		CheckItem(itsCurrIndex);
		EnableAll();
		itsBroadcastSizeChangeFlag = kTrue;
		}
	else if (sender == this && message.Is(JXMenu::kItemSelected))
		{
		const JXMenu::ItemSelected* selection = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		DoSizeChoice(selection->GetIndex());
		}

	else if (sender == itsChooseSizeDialog &&
			 message.Is(JXDialogDirector::kDeactivated))
		{
		const JXDialogDirector::Deactivated* info =
			dynamic_cast<const JXDialogDirector::Deactivated*>(&message);
		assert( info != NULL );
		if (info->Successful())
			{
			SetFontSize(itsChooseSizeDialog->GetFontSize());
			}
		else
			{
			SetPopupChoice(itsCurrIndex);	// revert displayed string
			}
		itsChooseSizeDialog = NULL;
		}

	else

		CMenu::Receive(sender, message);
}

/******************************************************************************
 SetToPopupChoice

 ******************************************************************************/

void
CFontSizeMenu::SetToPopupChoice
	(
	const JBoolean isPopup
	)
{
	CMenu::SetToPopupChoice(isPopup, itsCurrIndex);
}

// private

void
CFontSizeMenu::CFontSizeMenuX
	(
	CFontNameMenu* fontMenu
	)
{
	itsFontNameMenu = fontMenu;
	if (itsFontNameMenu != NULL)
		{
		ListenTo(itsFontNameMenu);
		}

	itsBroadcastSizeChangeFlag = kTrue;
	itsChooseSizeDialog        = NULL;
}

void CFontSizeMenu::DoSizeChoice(const JIndex sizeIndex)
{
	if (sizeIndex != itsVarSizeIndex)
	{
		itsCurrIndex = sizeIndex;
		const JBoolean ok = (GetItemText(sizeIndex)).ConvertToUInt(&itsFontSize);
		AdjustVarSizeItem(0);
		Broadcast(SizeChanged(itsFontSize));
	}
	else
	{
		JXWindowDirector* supervisor = (GetWindow())->GetDirector();
		itsChooseSizeDialog = new JXChooseFontSizeDialog(supervisor, itsFontSize);
		assert( itsChooseSizeDialog != NULL );
		ListenTo(itsChooseSizeDialog);
		itsChooseSizeDialog->BeginDialog();

		// display the inactive cursor in all the other windows

		JXApplication* app = JXGetApplication();
		app->DisplayInactiveCursor();

		// block with event loop running until we get a response

		JXWindow* window = itsChooseSizeDialog->GetWindow();
		while (itsChooseSizeDialog)
			app->HandleOneEventForWindow(window);
	}
}
