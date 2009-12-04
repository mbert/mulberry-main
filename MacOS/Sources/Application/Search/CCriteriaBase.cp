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

#include <LBevelButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCriteriaBase::CCriteriaBase()
{
}

// Constructor from stream
CCriteriaBase::CCriteriaBase(LStream *inStream)
		: LView(inStream)
{
}

// Default destructor
CCriteriaBase::~CCriteriaBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCriteriaBase::FinishCreateSelf(void)
{
	// Do inherited
	LView::FinishCreateSelf();

	// Get controls
	mMoveUp = (LBevelButton*) FindPaneByID(paneid_CriteriaBaseMoveUp);
	mMoveDown = (LBevelButton*) FindPaneByID(paneid_CriteriaBaseMoveDown);
}

// Handle buttons
void CCriteriaBase::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_CriteriaBaseMoveUp:
			OnMove(true);
			break;

		case msg_CriteriaBaseMoveDown:
			OnMove(false);
			break;

		default:
			break;
	}
}

void CCriteriaBase::SetTop(bool top)
{
	if (top)
		mMoveUp->Disable();
	else
		mMoveUp->Enable();
}

void CCriteriaBase::SetBottom(bool bottom)
{
	if (bottom)
		mMoveDown->Disable();
	else
		mMoveDown->Enable();
}

void CCriteriaBase::OnMove(bool up)
{
	TArray<LPane*>& siblings = GetSuperView()->GetSubPanes();
	ArrayIndexT index = siblings.FetchIndexOf(this);

	if (up && (index != 1))
		index--;
	else if (!up && (index != siblings.GetCount()))
		index++;
	CCriteriaBase* switch_with = static_cast<CCriteriaBase*>(siblings[index]);

	if (switch_with != this)
		SwitchWith(switch_with);
}
