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


// Source for CSearchBase class

#include "CSearchBase.h"

#include "CSearchCriteriaContainer.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Default constructor
CSearchBase::CSearchBase(bool rules)
{
	mRules = rules;
	mGroupItems = NULL;
	mFilterType = CFilterItem::eLocal;
}

// Default destructor
CSearchBase::~CSearchBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Respond to list changes
void CSearchBase::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CSearchCriteriaContainer::eBroadcast_SearchCriteriaContainerResized:
		// Change the size of this one
		Resized(*reinterpret_cast<unsigned long*>(param));
		break;
	default:;
	}
}

// Activate search item
void CSearchBase::DoActivate()
{
	// Active first criteria that wants to acticate
	if (mGroupItems)
		mGroupItems->DoActivate();
}

void CSearchBase::OnMore()
{
	AddCriteria();
}

void CSearchBase::OnFewer()
{
	RemoveCriteria();
}

void CSearchBase::OnClear()
{
	// Remove all
	while(mGroupItems->GetCount() > 0)
		RemoveCriteria();

	// Reset the first one
	AddCriteria();
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHOffset = 4;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 14;

void CSearchBase::MakeGroup()
{
	if (mGroupItems != NULL)
		return;

	// Reanimate a new search criteria panel
	LCommander* defCommander;
	GetContainerWnd()->GetSubCommanders().FetchItemAt(1, defCommander);
	GetContainerWnd()->SetDefaultCommander(defCommander);
	GetContainerWnd()->SetDefaultView(GetContainerWnd());

	mGroupItems = (CSearchCriteriaContainer*) UReanimator::ReadObjects('PPob', paneid_SearchCriteriaContainer);
	mGroupItems->SetTopLevel();
	mGroupItems->SetRules(mRules);
	mGroupItems->FinishCreate();
	mGroupItems->InitGroup(mFilterType, NULL);
	mGroupItems->Add_Listener(this);

	// Put inside panel
	mGroupItems->PutInside(GetParentView());

	// Get size to increase
	SDimension16 size;
	mGroupItems->GetFrameSize(size);
	
	// Get size to increase
	SDimension16 psize;
	GetParentView()->GetFrameSize(psize);
	
	// Resize groups so that width first inside criteria
	mGroupItems->ResizeFrameBy(psize.width - cCriteriaHWidthAdjust - size.width, 0, false);

	// Shrink the size of this one
	Resized(size.height);

	// Position new sub-panel
	mGroupItems->PlaceInSuperFrameAt(cCriteriaHOffset, cCriteriaVInitOffset, false);
	mGroupItems->Show();
}

void CSearchBase::InitCriteria(const CSearchItem* spec)
{
	// Always make the group - it will only be done if not done already
	MakeGroup();

	// Pass to the group
	RemoveAllCriteria();
	mGroupItems->InitCriteria(spec);

	// Do button state
	if (mGroupItems->GetCount() > 1)
		GetFewerBtn()->Show();
}

void CSearchBase::AddCriteria(const CSearchItem* spec, bool use_or)
{
	// Pass to the group
	if (mGroupItems)
		mGroupItems->AddCriteria(spec, use_or);

	// Do button state
	if (mGroupItems->GetCount() > 1)
		GetFewerBtn()->Show();
}

void CSearchBase::RemoveCriteria()
{
	// Pass to the group
	if (mGroupItems)
		mGroupItems->RemoveCriteria();

	// Do button state
	if (mGroupItems->GetCount() < 2)
		GetFewerBtn()->Hide();
}

void CSearchBase::RemoveAllCriteria()
{
	// Remove all
	if (mGroupItems)
	{
		while(mGroupItems->GetCount() > 0)
			RemoveCriteria();
	}

	// Do button state
	GetFewerBtn()->Hide();
}

#pragma mark ____________________________Build Search

CSearchItem* CSearchBase::ConstructSearch() const
{
	// Pass to the group
	return mGroupItems->ConstructSearch();
}
