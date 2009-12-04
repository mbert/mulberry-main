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


// Source for CMultibitPopup class

#include "CMultibitPopup.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMultibitPopup::CMultibitPopup(const JCharacter*	title,
					JXContainer*		enclosure,
					const HSizingOption	hSizing,
					const VSizingOption	vSizing,
					const JCoordinate	x,
					const JCoordinate	y,
					const JCoordinate	w,
					const JCoordinate	h)
		: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
{
	// Always listen to self to do the bit toggle
	ListenTo(this);
}

// Default destructor
CMultibitPopup::~CMultibitPopup()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CMultibitPopup::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == this)
	{
		if (message.Is(JXMenu::kNeedsUpdate))
		{
			SetupCurrentMenuItem();
			return;
		}
		else if (message.Is(JXMenu::kItemSelected))
		{
			JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
			ToggleBit(index);
			return;
		}
	}
	
	// Do inherited
	HPopupMenu::Receive(sender, message);
}

// Make sure bit array is the same size as the menu
void CMultibitPopup::InitBits()
{
	// Verify that bits vector is same size as menu
	JSize num_items = GetItemCount();
	if (mBits.size() != num_items)
	{
		// Check for add/remove and adjust list
		if (mBits.size() < num_items)
			mBits.insert(mBits.end(), num_items - mBits.size(), false);
		else
			mBits.erase(mBits.end() - (mBits.size() - num_items), mBits.end());
	}
}

void CMultibitPopup::SetBit(JIndex index, bool set)
{
	// Do range check
	if (index <= mBits.size())
		mBits[index - 1] = set;
}

bool CMultibitPopup::GetBit(JIndex index) const
{
	// Do range check
	if (index <= mBits.size())
		return mBits[index - 1];
	else
		return false;
}

void CMultibitPopup::ToggleBit(JIndex index)
{
	SetBit(index, !GetBit(index));
}

// Set appropriate checks
void CMultibitPopup::SetupCurrentMenuItem()
{
	// Make sure bit array is consistent with menu
	InitBits();

	// Turn on check for bits that are set
	JSize num_items = GetItemCount();
	for(JIndex i = 1; i <= num_items; i++)
	{
		if (GetBit(i))
			CheckItem(i);
	}
}

void CMultibitPopup::AdjustPopupChoiceTitle(const JIndex index)
{
  //we don't want to do the title thing here, so override this
  //to do nothing
  return;
}
