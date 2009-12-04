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

/******************************************************************************
 JXTabs.h

 ******************************************************************************/

#ifndef _H_JXTabs
#define _H_JXTabs

#include <JXRadioGroup.h>

#include "JXTabCardFile.h"
#include "JXTabButton.h"

class JXTabs : public JXRadioGroup
{
public:

	JXTabs(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	virtual ~JXTabs();

	JXCardFile* GetCardEnclosure()
		{ return mCards; }

	JSize GetTabCount() const
		{ return mBtns.GetElementCount(); }

	JXWidgetSet* GetCurrentCard()
		{ return mCards->GetCurrentCard(); }

	virtual void AppendCard(JXWidgetSet* card, const char* title, JIndex icon = 0);

	virtual void ShowCard(JIndex index);
	virtual void ActivateCard(JIndex index);
	virtual void DeactivateCard(JIndex index);
	virtual void SetIconCard(JIndex index, JIndex icon);
	virtual void RemoveCard(JIndex index);
	virtual void RenameCard(JIndex index, const char* title, JIndex icon = 0);
	virtual void MoveCard(JIndex oldindex, JIndex newindex);

protected:
	JXTabCardFile*			mCards;
	JPtrArray<JXContainer>	mBtns;

	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	JXTabButton* GetTabButton(JIndex index)
		{ return static_cast<JXTabButton*>(mBtns.NthElement(index)); }

	virtual void OnRightClick(JIndex index, const JPoint& pt,
								const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers);
	virtual void OnShiftClick(JIndex index);

private:
  // not allowed

  JXTabs(const JXTabs& source);
  const JXTabs& operator=(const JXTabs& source);
};

#endif
