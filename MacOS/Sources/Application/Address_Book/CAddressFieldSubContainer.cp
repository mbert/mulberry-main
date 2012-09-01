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


// Source for CAddressFieldSubContainer class

#include "CAddressFieldSubContainer.h"

#include "CAdbkAddress.h"
#include "CAddressFieldContainer.h"
#include "CAddressFieldText.h"
#include "CAddressFieldMultiLine.h"

#include <LTextGroupBox.h>

// Default constructor
CAddressFieldSubContainer::CAddressFieldSubContainer()
{
	mType = eTel;
    mDirty = false;
}

// Constructor from stream
CAddressFieldSubContainer::CAddressFieldSubContainer(LStream *inStream)
		: LView(inStream)
{
	mType = eTel;
    mDirty = false;
}

// Default destructor
CAddressFieldSubContainer::~CAddressFieldSubContainer()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CAddressFieldSubContainer::FinishCreateSelf()
{
	// Do inherited
	LView::FinishCreateSelf();

	// Locate the parent
	LView* parent = GetSuperView();
	while((parent != NULL) && (dynamic_cast<LWindow*>(parent) == NULL))
		parent = parent->GetSuperView();
	mContainer = dynamic_cast<LWindow*>(parent);

	// Get all controls
	mSeparator = (LPane*) FindPaneByID(paneid_AddressFieldSubContainerSeparator);
	
    if (mType == eTel)
        static_cast<LTextGroupBox*>(mSeparator)->SetDescriptor(LStr255("Telephone"));
    else if (mType == eEmail)
        static_cast<LTextGroupBox*>(mSeparator)->SetDescriptor(LStr255("Email"));
    else if (mType == eIM)
        static_cast<LTextGroupBox*>(mSeparator)->SetDescriptor(LStr255("IM"));
    else if (mType == eCalAddress)
        static_cast<LTextGroupBox*>(mSeparator)->SetDescriptor(LStr255("Calendar"));
    else if (mType == eAddress)
        static_cast<LTextGroupBox*>(mSeparator)->SetDescriptor(LStr255("Address"));

    CAddressFieldBase* field = AddField();
    field->SetBottom();
}

// Activate search item
void CAddressFieldSubContainer::DoActivate()
{
	// Iterate over all criteria and first first that wants to activate
	long num = mFields.GetCount();
	for(long i = 1; i <= num; i++)
	{
		//if (static_cast<CAddressFieldBase*>(mFields[i])->DoActivate())
		//	break;
	}
}

void CAddressFieldSubContainer::SetAddress(const CAdbkAddress* addr)
{
    // Remove all existing fields
	for(long i = mFields.GetCount() - 1; i >= 1; i--)
	{
        LView* child = static_cast<LView*>(mFields[i]);
        child->PutInside(NULL);
        delete child;
        mFields.RemoveItemsAt(1, i);
	}
    
    if (addr != NULL)
    {
        if (mType == eTel)
        {
            CAdbkAddress::phonemap phones = addr->GetPhones();
            for(CAdbkAddress::phonemap::const_iterator iter = phones.begin(); iter != phones.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eEmail)
        {
            CAdbkAddress::emailmap emails = addr->GetEmails();
            for(CAdbkAddress::emailmap::const_iterator iter = emails.begin(); iter != emails.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eAddress)
        {
            CAdbkAddress::addrmap addrs = addr->GetAddresses();
            for(CAdbkAddress::addrmap::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField(true));
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eIM)
        {
            CAdbkAddress::immap ims = addr->GetIMs();
            for(CAdbkAddress::immap::const_iterator iter = ims.begin(); iter != ims.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
        else if (mType == eCalAddress)
        {
            CAdbkAddress::caladdrmap caladdrs = addr->GetCalendars();
            for(CAdbkAddress::caladdrmap::const_iterator iter = caladdrs.begin(); iter != caladdrs.end(); iter++)
            {
                CAddressFieldText* field = static_cast<CAddressFieldText*>(AddField());
                field->SetDetails("", (*iter).first, (*iter).second);
            }
        }
    }

    Layout();
    mDirty = false;
}

bool CAddressFieldSubContainer::GetAddress(CAdbkAddress* addr)
{
    bool changed = mDirty;
    ulvector newtypes;
    cdstrvect newdatas;
    
	for(long i = 1; i <= mFields.GetCount() - 1; i++)
	{
        CAddressFieldBase* field = static_cast<CAddressFieldBase*>(mFields[i]);
        int newtype = 0;
        cdstring newdata;
        if (field->GetDetails(newtype, newdata))
        {
            changed = true;
        }
        newtypes.push_back(newtype);
        newdatas.push_back(newdata);
	}
    
    if (changed)
    {
        if (mType == eTel)
        {
            CAdbkAddress::phonemap& phones = addr->GetPhones();
            phones.clear();
            for(int i = 0; i < newtypes.size(); i++)
                phones.insert(CAdbkAddress::phonemap::value_type(static_cast<CAdbkAddress::EPhoneType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eEmail)
        {
            CAdbkAddress::emailmap& emails = addr->GetEmails();
            emails.clear();
            for(int i = 0; i < newtypes.size(); i++)
                emails.insert(CAdbkAddress::emailmap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eAddress)
        {
            CAdbkAddress::addrmap& addrs = addr->GetAddresses();
            addrs.clear();
            for(int i = 0; i < newtypes.size(); i++)
                addrs.insert(CAdbkAddress::addrmap::value_type(static_cast<CAdbkAddress::EAddressType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eIM)
        {
            CAdbkAddress::immap& ims = addr->GetIMs();
            ims.clear();
            for(int i = 0; i < newtypes.size(); i++)
                ims.insert(CAdbkAddress::immap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eCalAddress)
        {
            CAdbkAddress::caladdrmap& caladdrs = addr->GetCalendars();
            caladdrs.clear();
            for(int i = 0; i < newtypes.size(); i++)
                caladdrs.insert(CAdbkAddress::caladdrmap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }        
    }

    return changed;
}

#pragma mark ____________________________Criteria Panels

const int cCriteriaHOffset = 12;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 4;
const int cCriteriaVInitOffsetNested = 20;

CAddressFieldBase* CAddressFieldSubContainer::AddField(bool multi)
{
    // Reanimate a new search criteria panel
    LCommander* defCommander;
    mContainer->GetSubCommanders().FetchItemAt(1, defCommander);
    mContainer->SetDefaultCommander(defCommander);
    mContainer->SetDefaultView(mContainer);
    
    CAddressFieldBase* field = (CAddressFieldBase*) UReanimator::ReadObjects('PPob', multi ? paneid_AddressFieldMultiLine : paneid_AddressFieldText);
    field->FinishCreate();
    field->SetMultipleInstance();
    if (mType == eTel)
        field->SetMenu(menu_TelephoneType);
    
    AddView(field);
    
    return field;
}

void CAddressFieldSubContainer::AddView(LView* view)
{
    // Get last view in criteria bottom
    SPoint32 new_pos = {cCriteriaHOffset, cCriteriaVInitOffsetNested};
    if (mFields.GetCount() > 0)
        mFields.InsertItemsAt(1, mFields.GetCount(), view);
    else
        mFields.AddItem(view);
    
    // Put inside panel
    view->PutInside(this);
    
    // Now adjust sizes
    SDimension16 size;
    view->GetFrameSize(size);
    
    // Resize groups so that width first inside criteria
    SDimension16 gsize;
    GetFrameSize(gsize);
    view->ResizeFrameBy(gsize.width - cCriteriaHWidthAdjust - cCriteriaHOffset - size.width, 0, false);
    
    // Position new sub-panel
    Layout();
    view->Show();
}

void CAddressFieldSubContainer::AppendField()
{
    CAddressFieldBase* field = AddField(mType == eAddress);
    mDirty = true;
}

void CAddressFieldSubContainer::RemoveField(LView* view)
{
	for(long i = 1; i <= mFields.GetCount(); i++)
	{
        LView* child = static_cast<LView*>(mFields[i]);
        if (child == view)
        {
            child->PutInside(NULL);
            delete child;
            mFields.RemoveItemsAt(1, i);
            Layout();
            mDirty = true;
            break;
        }
	}
}

void CAddressFieldSubContainer::Layout()
{
    SPoint32 new_pos = {cCriteriaHOffset, cCriteriaVInitOffsetNested};
    SInt16 total_height = cCriteriaVInitOffsetNested;
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
    ResizeFrameTo(gsize.width, total_height + cCriteriaVInitOffset, false);

    if (dynamic_cast<CAddressFieldContainer*>(GetSuperView()) != NULL)
        dynamic_cast<CAddressFieldContainer*>(GetSuperView())->Layout();
}

