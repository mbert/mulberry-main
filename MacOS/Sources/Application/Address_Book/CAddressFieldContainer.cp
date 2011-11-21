/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Source for CAddressFieldContainer class

#include "CAddressFieldContainer.h"

#include "CAdbkAddress.h"
#include "CAddressFieldText.h"
#include "CAddressFieldMultiLine.h"
#include "CAddressFieldSubContainer.h"

// Default constructor
CAddressFieldContainer::CAddressFieldContainer()
{
}

// Constructor from stream
CAddressFieldContainer::CAddressFieldContainer(LStream *inStream)
		: LView(inStream)
{
}

// Default destructor
CAddressFieldContainer::~CAddressFieldContainer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CAddressFieldContainer::FinishCreateSelf()
{
	// Do inherited
	LView::FinishCreateSelf();

	// Locate the parent
	LView* parent = GetSuperView();
	while((parent != NULL) && (dynamic_cast<LWindow*>(parent) == NULL))
		parent = parent->GetSuperView();
	mContainer = dynamic_cast<LWindow*>(parent);

    CAddressFieldBase* name = AddField();
    CAddressFieldBase* nickname = AddField();
    CAddressFieldBase* organization = AddField();
    AddContainer(CAddressFieldSubContainer::eTel);
    AddContainer(CAddressFieldSubContainer::eEmail);
    AddContainer(CAddressFieldSubContainer::eIM);
    AddContainer(CAddressFieldSubContainer::eCalAddress);
    AddContainer(CAddressFieldSubContainer::eAddress);
    CAddressFieldBase* notes = AddField(true);
}

// Activate search item
void CAddressFieldContainer::DoActivate()
{
	// Iterate over all criteria and first first that wants to activate
	long num = mFields.GetCount();
	for(long i = 1; i <= num; i++)
	{
		//if (static_cast<CAddressFieldBase*>(mFields[i])->DoActivate())
		//	break;
	}
}

void CAddressFieldContainer::SetAddress(const CAdbkAddress* addr)
{
    // Only called if top level
    static_cast<CAddressFieldBase*>(mFields[1])->SetDetails("Name:", 0, addr ? addr->GetName() : "");
    static_cast<CAddressFieldBase*>(mFields[2])->SetDetails("Nick-Name:", 0, addr ? addr->GetADL() : "");
    static_cast<CAddressFieldBase*>(mFields[3])->SetDetails("Organization:", 0, addr ? addr->GetCompany() : "");
    static_cast<CAddressFieldSubContainer*>(mFields[4])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[5])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[6])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[7])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[8])->SetAddress(addr);
    static_cast<CAddressFieldBase*>(mFields[9])->SetDetails("Note:", 0, addr ? addr->GetNotes() : "");
}

bool CAddressFieldContainer::GetAddress(CAdbkAddress* addr)
{
    bool changed = false;
    int newtype = 0;
    cdstring newdata;
    if (static_cast<CAddressFieldBase*>(mFields[1])->GetDetails(newtype, newdata))
    {
        addr->SetName(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[2])->GetDetails(newtype, newdata))
    {
        addr->SetADL(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[3])->GetDetails(newtype, newdata))
    {
        addr->SetCompany(newdata);
        changed = true;
    }
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[4])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[5])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[6])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[7])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[8])->GetAddress(addr);
    if (static_cast<CAddressFieldBase*>(mFields[9])->GetDetails(newtype, newdata))
    {
        addr->SetNotes(newdata);
        changed = true;
    }
    
    return changed;
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 4;

CAddressFieldBase* CAddressFieldContainer::AddField(bool multi)
{
    // Reanimate a new search criteria panel
    LCommander* defCommander;
    mContainer->GetSubCommanders().FetchItemAt(1, defCommander);
    mContainer->SetDefaultCommander(defCommander);
    mContainer->SetDefaultView(mContainer);
    
    CAddressFieldBase* field = (CAddressFieldBase*) UReanimator::ReadObjects('PPob', multi ? paneid_AddressFieldMultiLine : paneid_AddressFieldText);
    field->FinishCreate();
    field->SetSingleInstance();
    
    AddView(field);
    
    return field;
}

CAddressFieldSubContainer* CAddressFieldContainer::AddContainer(int ctype)
{
    // Reanimate a new search criteria panel
    LCommander* defCommander;
    mContainer->GetSubCommanders().FetchItemAt(1, defCommander);
    mContainer->SetDefaultCommander(defCommander);
    mContainer->SetDefaultView(mContainer);
    
    CAddressFieldSubContainer* container = (CAddressFieldSubContainer*) UReanimator::ReadObjects('PPob', paneid_AddressFieldSubContainer);
    container->SetContainerType(static_cast<CAddressFieldSubContainer::EContainerType>(ctype));
    container->FinishCreate();
    
    AddView(container);
    
    return container;
}

void CAddressFieldContainer::AddView(LView* view)
{
    // Get last view in criteria bottom
    SPoint32 new_pos = {0, cCriteriaVInitOffset};
    
    // Put inside panel
    mFields.AddItem(view);
    view->PutInside(this);
    
    // Now adjust sizes
    SDimension16 size;
    view->GetFrameSize(size);
    
    // Resize groups so that width first inside criteria
    SDimension16 gsize;
    GetFrameSize(gsize);
    view->ResizeFrameBy(gsize.width - cCriteriaHWidthAdjust - size.width, 0, false);
    
    // Position new sub-panel
    Layout();
    view->Show();
}

void CAddressFieldContainer::Layout()
{
    SPoint32 new_pos = {0, cCriteriaVInitOffset};
    SInt16 total_height = new_pos.v;
	for(long i = 1; i <= mFields.GetCount(); i++)
	{
        LView* child = static_cast<LView*>(mFields[i]);
        SDimension16 child_size;
        child->GetFrameSize(child_size);
        child->PlaceInSuperFrameAt(new_pos.h, new_pos.v, false);
        new_pos.v += child_size.height;
        total_height += child_size.height;
	}
    
    SDimension16 gsize;
    GetFrameSize(gsize);
    ResizeFrameBy(0, total_height - gsize.height, true);
}
