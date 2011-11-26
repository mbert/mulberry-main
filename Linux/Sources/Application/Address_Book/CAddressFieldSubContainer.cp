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
#include "CStaticText.h"

#include "templs.h"

#include <JXEngravedRect.h>

#include <cassert>

// Default constructor
CAddressFieldSubContainer::CAddressFieldSubContainer(JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h) :
		JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
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
void CAddressFieldSubContainer::OnCreate()
{
	// begin JXLayout1

		JXEngravedRect* obj1 =
	        new JXEngravedRect(this,
	                    JXWidget::kHElastic, JXWidget::kVElastic, 0,16, 496,9);
	    assert( obj1 != NULL );

	    mTitle =
	        new CStaticText("", this,
	                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,1, 146,15);
	    assert( mTitle != NULL );

	// end JXLayout1
	
    if (mType == eTel)
    	mTitle->SetText("Telephone");
    else if (mType == eEmail)
    	mTitle->SetText("Email");
    else if (mType == eIM)
    	mTitle->SetText("IM");
    else if (mType == eCalAddress)
    	mTitle->SetText("Calendar");
    else if (mType == eAddress)
    	mTitle->SetText("Address");

    CAddressFieldBase* field = AddField();
    field->SetBottom();
}

void CAddressFieldSubContainer::SetAddress(const CAdbkAddress* addr)
{
    // Remove all existing fields
	while(mFields.size() > 1)
	{
        JXWidget* child = mFields.front();
        delete child;
        mFields.erase(mFields.begin());
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
    
	for(std::vector<JXWidget*>::size_type i = 0; i < mFields.size() - 1; i++)
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
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                phones.insert(CAdbkAddress::phonemap::value_type(static_cast<CAdbkAddress::EPhoneType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eEmail)
        {
            CAdbkAddress::emailmap& emails = addr->GetEmails();
            emails.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                emails.insert(CAdbkAddress::emailmap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eAddress)
        {
            CAdbkAddress::addrmap& addrs = addr->GetAddresses();
            addrs.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                addrs.insert(CAdbkAddress::addrmap::value_type(static_cast<CAdbkAddress::EAddressType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eIM)
        {
            CAdbkAddress::immap& ims = addr->GetIMs();
            ims.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
                ims.insert(CAdbkAddress::immap::value_type(static_cast<CAdbkAddress::EEmailType>(newtypes[i]), newdatas[i]));
        }        
        else if (mType == eCalAddress)
        {
            CAdbkAddress::caladdrmap& caladdrs = addr->GetCalendars();
            caladdrs.clear();
            for(ulvector::size_type i = 0; i < newtypes.size(); i++)
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
const int cCriteriaHeight = 25;
const int cCriteriaMultiHeight = 65;
const int cCriteriaWidth = 496;

CAddressFieldBase* CAddressFieldSubContainer::AddField(bool multi)
{
    CAddressFieldBase* field = multi ?
    		(CAddressFieldBase*) new CAddressFieldMultiLine(this, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cCriteriaWidth, cCriteriaMultiHeight) :
    		(CAddressFieldBase*) new CAddressFieldText(this, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, cCriteriaWidth, cCriteriaHeight);
    field->OnCreate();
    field->SetMultipleInstance();
    if (mType == eTel)
        field->SetMenu(true);
    
    AddView(field);
    
    return field;
}

void CAddressFieldSubContainer::AddView(JXWidget* view)
{
    // Get last view in criteria bottom
    JPoint new_pos(cCriteriaHOffset, cCriteriaVInitOffsetNested);
    if (mFields.size() > 0)
        mFields.insert(mFields.end() - 1, view);
    else
        mFields.push_back(view);
    
    // Now adjust sizes
    JRect size = view->GetFrame();
    
    // Resize groups so that width first inside criteria
    JRect gsize = GetFrame();
    view->AdjustSize(gsize.width() - cCriteriaHWidthAdjust - cCriteriaHOffset - size.width(), 0);
    
    // Position new sub-panel
    Layout();
    view->Show();
}

void CAddressFieldSubContainer::AppendField()
{
    CAddressFieldBase* field = AddField(mType == eAddress);
    mDirty = true;
}

void CAddressFieldSubContainer::RemoveField(JXWidget* view)
{
	for(std::vector<JXWidget*>::iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
        JXWidget* child = *iter;
        if (child == view)
        {
            delete child;
            mFields.erase(iter);
            Layout();
            mDirty = true;
            break;
        }
	}
}

void CAddressFieldSubContainer::Layout()
{
    JPoint new_pos(cCriteriaHOffset, cCriteriaVInitOffsetNested);
    JCoordinate total_height = cCriteriaVInitOffsetNested;
	for(std::vector<JXWidget*>::const_iterator iter = mFields.begin(); iter != mFields.end(); iter++)
	{
        JXWidget* child = *iter;
        JRect child_size = child->GetFrame();
        child->Place(new_pos.x, new_pos.y);
        new_pos.y += child_size.height();
        total_height += child_size.height();
	}
    
    JRect gsize = GetFrame();
    SetSize(gsize.width(), total_height + cCriteriaVInitOffset);

    if (dynamic_cast<CAddressFieldContainer*>(GetEnclosure()) != NULL)
        dynamic_cast<CAddressFieldContainer*>(GetEnclosure())->Layout();
}

