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
#include "CBlankScrollable.h"

// Default constructor
CAddressFieldContainer::CAddressFieldContainer(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h) :
		JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressFieldContainer::~CAddressFieldContainer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CAddressFieldContainer::OnCreate()
{
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

void CAddressFieldContainer::SetAddress(const CAdbkAddress* addr)
{
    // Only called if top level
    static_cast<CAddressFieldBase*>(mFields[0])->SetDetails("Name:", 0, addr ? addr->GetName() : "");
    static_cast<CAddressFieldBase*>(mFields[1])->SetDetails("Nick-Name:", 0, addr ? addr->GetADL() : "");
    static_cast<CAddressFieldBase*>(mFields[2])->SetDetails("Organization:", 0, addr ? addr->GetCompany() : "");
    static_cast<CAddressFieldSubContainer*>(mFields[3])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[4])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[5])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[6])->SetAddress(addr);
    static_cast<CAddressFieldSubContainer*>(mFields[7])->SetAddress(addr);
    static_cast<CAddressFieldBase*>(mFields[8])->SetDetails("Note:", 0, addr ? addr->GetNotes() : "");
}

bool CAddressFieldContainer::GetAddress(CAdbkAddress* addr)
{
    bool changed = false;
    int newtype = 0;
    cdstring newdata;
    if (static_cast<CAddressFieldBase*>(mFields[0])->GetDetails(newtype, newdata))
    {
        addr->SetName(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[1])->GetDetails(newtype, newdata))
    {
        addr->SetADL(newdata);
        changed = true;
    }
    if (static_cast<CAddressFieldBase*>(mFields[2])->GetDetails(newtype, newdata))
    {
        addr->SetCompany(newdata);
        changed = true;
    }
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[3])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[4])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[5])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[6])->GetAddress(addr);
    changed |= static_cast<CAddressFieldSubContainer*>(mFields[7])->GetAddress(addr);
    if (static_cast<CAddressFieldBase*>(mFields[8])->GetDetails(newtype, newdata))
    {
        addr->SetNotes(newdata);
        changed = true;
    }
    
    return changed;
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 4;
const int cCriteriaHeight = 25;
const int cCriteriaMultiHeight = 65;
const int cCriteriaWidth = 496;

CAddressFieldBase* CAddressFieldContainer::AddField(bool multi)
{
    CAddressFieldBase* field = multi ?
    		(CAddressFieldBase*) new CAddressFieldMultiLine(this, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cCriteriaWidth, cCriteriaMultiHeight) :
    		(CAddressFieldBase*) new CAddressFieldText(this, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cCriteriaWidth, cCriteriaHeight);
    field->OnCreate();
    field->SetSingleInstance();
    
    AddView(field);
    
    return field;
}

CAddressFieldSubContainer* CAddressFieldContainer::AddContainer(int ctype)
{
	CAddressFieldSubContainer* container = new CAddressFieldSubContainer(this, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cCriteriaWidth, cCriteriaHeight);
    container->SetContainerType(static_cast<CAddressFieldSubContainer::EContainerType>(ctype));
    container->OnCreate();
    
    AddView(container);
    
    return container;
}

void CAddressFieldContainer::AddView(JXWidget* view)
{
    // Get last view in criteria bottom
    JPoint new_pos(0, cCriteriaVInitOffset);
    
    // Put inside panel
    mFields.push_back(view);
    
    // Now adjust sizes
    JRect size = view->GetFrame();
    
    // Resize groups so that width first inside criteria
    JRect gsize = GetFrame();
    view->AdjustSize(gsize.width() - cCriteriaHWidthAdjust - size.width(), 0);
    
    // Position new sub-panel
    Layout();
    view->Show();
}

void CAddressFieldContainer::Layout()
{
    JPoint new_pos(0, cCriteriaVInitOffset);
    JCoordinate total_height = cCriteriaVInitOffset;
	for(std::vector<JXWidget*>::const_iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
        JXWidget* child = *iter;
        JRect child_size = child->GetFrame();
        child->Place(new_pos.x, new_pos.y);
        new_pos.y += child_size.height();
        total_height += child_size.height();
	}
    
    JRect fsize = GetFrame();
    SetSize(fsize.width(), total_height + cCriteriaVInitOffset);
    
    fsize = GetEnclosure()->GetBoundsGlobal();
    static_cast<CBlankScrollable*>(GetEnclosure())->SetBounds(fsize.width(), total_height + cCriteriaVInitOffset);
}
