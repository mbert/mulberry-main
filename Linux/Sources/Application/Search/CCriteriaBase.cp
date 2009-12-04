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


// Source for CCriteriaBase class

#include "CCriteriaBase.h"

#include "JXMultiImageButton.h"

#include <algorithm>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCriteriaBase::CCriteriaBase(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h)
	: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mMoveUp = NULL;
	mMoveDown = NULL;
}

// Default destructor
CCriteriaBase::~CCriteriaBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CCriteriaBase::SetBtns(JXMultiImageButton* up, JXMultiImageButton* down)
{
	// Move buttons
	mMoveUp = up;
	mMoveDown = down;

	ListenTo(mMoveUp);
	ListenTo(mMoveDown);
}

void CCriteriaBase::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mMoveUp)
		{
			OnSetMove(true);
			return;
		}
		else if (sender == mMoveDown)
		{
			OnSetMove(false);
			return;
		}
	}

	JXWidgetSet::Receive(sender, message);
}

void CCriteriaBase::SetTop(bool top)
{
	if (top)
		mMoveUp->Deactivate();
	else
		mMoveUp->Activate();
}

void CCriteriaBase::SetBottom(bool bottom)
{
	if (bottom)
		mMoveDown->Deactivate();
	else
		mMoveDown->Activate();
}

void CCriteriaBase::OnSetMove(bool up)
{
	CCriteriaBaseList& list = GetList();
	CCriteriaBaseList::iterator found = std::find(list.begin(), list.end(), this);
	unsigned long index = 0;
	if (found != list.end())
		index = found - list.begin();

	if (up && (index != 0))
		index--;
	else if (!up && (index != list.size() - 1))
		index++;
	CCriteriaBase* switch_with = list.at(index);

	if (switch_with != this)
		SwitchWith(switch_with);
}

