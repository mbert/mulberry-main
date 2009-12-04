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


#include "JXTabs.h"

#include "CDrawUtils.h"
#include "JXTabButton.h"

#include <JXFontManager.h>
#include <jXGlobals.h>

#include <algorithm>

/******************************************************************************
 Constructor

 ******************************************************************************/

const JSize cTabButtonHeight = 25;
const JSize cTabButtonWidthXtra = 16;
const JSize cTabButtonIconWidth = 18;
const JSize cCardsTop = cTabButtonHeight - 2;

JXTabs::JXTabs
	(
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXRadioGroup(enclosure, hSizing, vSizing, x,y, w,h), mBtns(JPtrArrayT::kForgetAll)
{
	SetBorderWidth(0);

	// Always create the card holder
	mCards = new JXTabCardFile(this, JXWidget::kHElastic, JXWidget::kVElastic, 0, cCardsTop, w, h - cCardsTop);

	// Always listen to ourself
	ListenTo(this);
}

/******************************************************************************
 Destructor

 ******************************************************************************/

JXTabs::~JXTabs()
{
}

void JXTabs::AppendCard(JXWidgetSet* card, const char* title, JIndex icon)
{
	// Add the card
	if (card)
		mCards->AppendCard(card);

	// Create a button and add to top
	JIndex cardno = mBtns.GetElementCount() + 1;

	// Determine width of button from title and icon
	JSize start_width = cTabButtonWidthXtra +
					GetFontManager()->GetStringWidth(JGetDefaultFontName(), kJXDefaultFontSize, JFontStyle(), title);

	// Move other buttons and reset their IDs
	JCoordinate btn_start = 0;
	for(JIndex idx = 1; idx <= mBtns.GetElementCount(); idx++)
		btn_start += GetTabButton(idx)->GetFrameWidth();

	JXTabButton* btn = new JXTabButton(cardno, title, this, JXWidget::kFixedLeft, JXWidget::kFixedTop,
										btn_start, 0, start_width, cTabButtonHeight);
	btn->SetIcon(icon);

	mBtns.Append(btn);
	ListenTo(btn);

	Refresh();
}

void JXTabs::ShowCard(JIndex index)
{
	if (mCards->GetCardCount())
		mCards->ShowCard(index);
}

void JXTabs::ActivateCard(JIndex index)
{
	// Activate the button
	GetTabButton(index)->Activate();
}

void JXTabs::DeactivateCard(JIndex index)
{
	// Deactivate the button
	GetTabButton(index)->Deactivate();
}

void JXTabs::SetIconCard(JIndex index, JIndex icon)
{
	// Only if different
	if (GetTabButton(index)->GetIcon() != icon)
	{
		// Change the icon
		JXTabButton* btn = GetTabButton(index);
		JCoordinate old_width = btn->GetFrameWidth();
		btn->SetIcon(icon);
		JCoordinate new_width = btn->GetFrameWidth();
		
		// Move other buttons and reset their IDs
		for(JIndex idx = index + 1; idx <= mBtns.GetElementCount(); idx++)
			GetTabButton(idx)->Move(new_width - old_width, 0);		
	}
}

void JXTabs::RemoveCard(JIndex index)
{
	// Delete and remove the card widget
	if (mCards->GetCardCount())
		mCards->DeleteCard(index);

	// Remove the button and shift the others
	JXContainer* btn = GetTabButton(index);

	// Get its width
	JCoordinate width = btn->GetFrameWidth();

	// Move other buttons and reset their IDs
	for(JIndex idx = index + 1; idx <= mBtns.GetElementCount(); idx++)
	{
		GetTabButton(idx)->Move(-width, 0);
		GetTabButton(idx)->SetID(idx - 1);
	}
	
	// Remove the button from the radio group object as well
	itsButtons->RemoveElement(index);
	if (itsSelection == btn)
		itsSelection = NULL;

	// Now remove the button
	mBtns.DeleteElement(index);
	
	Refresh();
}

void JXTabs::RenameCard(JIndex index, const char* title, JIndex icon)
{
	// Get current button width
	JSize oldtext_width = GetTabButton(index)->GetFrameWidth() - ((GetTabButton(index)->GetIcon() > 0) ? cTabButtonIconWidth : 0);
	JSize oldwidth = GetTabButton(index)->GetFrameWidth();

	// Determine width of button from new title
	JSize newtext_width = cTabButtonWidthXtra +
					GetFontManager()->GetStringWidth(JGetDefaultFontName(), kJXDefaultFontSize, JFontStyle(), title);
	JSize newwidth = newtext_width + (icon ? cTabButtonIconWidth : 0);

	// Rename the button
	GetTabButton(index)->SetLabel(title);
	GetTabButton(index)->SetIcon(icon);
	
	// Change its width
	if (newwidth != oldwidth)
	{
		JSize move_by = newwidth - oldwidth;

		// Move others right before increasing width
		if (move_by > 0)
		{
			for(JIndex i = index + 1; i <= GetTabCount(); i++)
				GetTabButton(i)->Move(move_by, 0);
		}

		// Now change button width (only by amount of text change - icon effect already takn care of)
		GetTabButton(index)->AdjustSize(newtext_width - oldtext_width, 0);

		// Move others left after decreasing width
		if (move_by < 0)
		{
			for(JIndex i = index + 1; i <= GetTabCount(); i++)
				GetTabButton(i)->Move(move_by, 0);
		}
	}
}

void JXTabs::MoveCard(JIndex oldindex, JIndex newindex)
{
	// Check current selected item
	JIndex selected = GetSelectedItem();

	// Get current card at oldindex
	JXWidgetSet* card = mCards->GetCardCount() ? mCards->RemoveCard(oldindex) : NULL;

	// Get current label and icon
	JString old_str = *GetTabButton(oldindex)->GetLabel();
	JIndex old_icon = GetTabButton(oldindex)->GetIcon();
	
	// Now shift others into old index
	long direction = (newindex > oldindex ? 1 : -1);
	for(unsigned long i = oldindex; i != newindex; i += direction)
	{
		JString str = *GetTabButton(i + direction)->GetLabel();
		JIndex icon = GetTabButton(i + direction)->GetIcon();
		RenameCard(i, str, icon);
	}

	// Write old one into new position
	RenameCard(newindex, old_str, old_icon);
	
	// Reinsert old card at new position
	if (card)
		mCards->InsertCard(newindex, card);

	// Now change the value if it moved
	if ((selected >= std::min(oldindex, newindex)) && (selected <= std::max(oldindex, newindex)))
	{
		if (selected == oldindex)
		{
			SelectItem(newindex);
			ShowCard(newindex);
		}
		else
		{
			SelectItem(selected - direction);
			ShowCard(selected - direction);
		}
	}
}

void JXTabs::HandleMouseDown(const JPoint& pt, const JXMouseButton button, const JSize clickCount,
									const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Look for right-click
	if (button == kJXRightButton)
		// Broadcast the right-click
		OnRightClick(0, pt, buttonStates, modifiers);
	else
		// Do default
		JXRadioGroup::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void JXTabs::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged) && (sender == this))
	{
		// Change the card
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		ShowCard(index);

		// Do this to force refresh of buttons so that
		// buttons next to the selected one get redrawn properly
		Refresh();
	}
	else if (message.Is(JXTabButton::kRightClick))
	{
		// Find card index
		JIndex index = 1;
		for(; index <= mBtns.GetElementCount(); index++)
		{
			if (GetTabButton(index) == sender)
			{
				// Get the right-click message
				const JXTabButton::RightClick* rclick = dynamic_cast<const JXTabButton::RightClick*>(&message);
				
				// Adjust point by ofset of button
				JPoint adjust_pt(rclick->GetPt());
				adjust_pt.x += GetTabButton(index)->GetBoundsGlobal().left - GetBoundsGlobal().left;
				
				// Now do the action
				OnRightClick(index, adjust_pt, rclick->GetButtonStates(), rclick->GetModifiers());
				break;
			}
		}
	}
	else if (message.Is(JXTabButton::kShiftClick))
	{
		// Find card index
		JIndex index = 1;
		for(; index <= mBtns.GetElementCount(); index++)
		{
			if (GetTabButton(index) == sender)
			{
				OnShiftClick(index);
				break;
			}
		}
	}
}

void JXTabs::OnRightClick(JIndex index, const JPoint& pt, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Nothing to do in this class
}

void JXTabs::OnShiftClick(JIndex index)
{
	// Nothing to do in this class
}
