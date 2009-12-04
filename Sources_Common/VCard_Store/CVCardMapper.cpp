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

/*
	CVCardMapper.cpp

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#include "CVCardMapper.h"

#include "CAdbkAddress.h"
#include "CAddressBook.h"
#include "CAddressList.h"
#include "CGroupList.h"

#include "CVCardAddress.h"
#include "CVCardAddressValue.h"
#include "CVCardAddressBook.h"
#include "CVCardComponentDB.h"
#include "CVCardDefinitions.h"
#include "CVCardMultiValue.h"
#include "CVCardN.h"
#include "CVCardTextValue.h"
#include "CVCardURIValue.h"
#include "CVCardVCard.h"

#include <memory>

//using namespace vcardstore;

// Map from VCard representation into internal address/group format
void vcardstore::MapFromVCards(CAddressBook* adbk)
{
	const vCard::CVCardAddressBook* vadbk = adbk->GetVCardAdbk();

	// Clear existing items
	adbk->GetAddressList()->clear();
	adbk->GetGroupList()->clear();
	
	// Look at each vCard
	for(vCard::CVCardComponentDB::const_iterator iter1 = vadbk->GetVCards().begin(); iter1 != vadbk->GetVCards().end(); iter1++)
	{
		const vCard::CVCardVCard* vcard = static_cast<const vCard::CVCardVCard*>((*iter1).second);
		MapFromVCard(adbk->GetAddressList(), *vcard);
	}
}

void vcardstore::MapFromVCard(CAddressBook* adbk, const vCard::CVCardVCard& vcard)
{
	MapFromVCard(adbk->GetAddressList(), vcard);
}

void vcardstore::MapFromVCard(CAddressList* addrs, const vCard::CVCardVCard& vcard)
{
	// Map to an address
	std::auto_ptr<CAdbkAddress> addr(new CAdbkAddress());

	// Get entry (the component's map key)
	addr->SetEntry(vcard.GetMapKey());

	for(vCard::CVCardPropertyMap::const_iterator iter2 = vcard.GetProperties().begin(); iter2 != vcard.GetProperties().end(); iter2++)
	{
		// Get name (FN)
		if ((*iter2).first.compare(vCard::cVCardProperty_FN, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
				addr->SetName(tv->GetValue());
		}
		// Get nick-name (NICKNAME)
		else if ((*iter2).first.compare(vCard::cVCardProperty_NICKNAME, true) == 0)
		{
			const vCard::CVCardMultiValue* mv = (*iter2).second.GetMultiValue();
			if (mv != NULL)
			{
				for(vCard::CVCardValueList::const_iterator iter3 = mv->GetValues().begin(); iter3 != mv->GetValues().end(); iter3++)
				{
					const vCard::CVCardTextValue* tv = dynamic_cast<const vCard::CVCardTextValue*>(*iter3);
					if (tv != NULL)
						addr->SetADL(tv->GetValue());
				}
			}
		}
		
		// Get email (EMAIL)
		else if ((*iter2).first.compare(vCard::cVCardProperty_EMAIL, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
			{
				// Check for attribute types
				CAdbkAddress::EEmailType type = CAdbkAddress::eDefaultEmailType;
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_HOME, true))
					type = CAdbkAddress::eHomeEmailType;
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_WORK, true))
					type = CAdbkAddress::eWorkEmailType;
				addr->SetEmail(tv->GetValue(), type, true);
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_PREF, true))
				{
					addr->SetPreferredEmail(type);
				}
			}
		}
		
		// Get calendar (X-CALENDAR-ADDRESS)
		else if ((*iter2).first.compare(vCard::cVCardProperty_CALENDAR_ADDRESS, true) == 0)
		{
			const vCard::CVCardURIValue* uv = static_cast<const vCard::CVCardURIValue*>((*iter2).second.GetURIValue());
			if (uv != NULL)
				addr->SetCalendar(uv->GetValue());
		}
		
		// Get company (ORG)
		else if ((*iter2).first.compare(vCard::cVCardProperty_ORG, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
				addr->SetCompany(tv->GetValue());
		}
		
		// Get address (ADR)
		else if ((*iter2).first.compare(vCard::cVCardProperty_ADR, true) == 0)
		{
			const vCard::CVCardAddressValue* av = (*iter2).second.GetAddressValue();
			if (av != NULL)
			{
				// Check for attribute types
				CAdbkAddress::EAddressType type = CAdbkAddress::eDefaultAddressType;
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_ADR_HOME, true))
					type = CAdbkAddress::eHomeAddressType;
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_ADR_WORK, true))
					type = CAdbkAddress::eWorkAddressType;
				addr->SetAddress(av->GetValue().GetFullAddress(), type, true);
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_PREF, true))
				{
					addr->SetPreferredAddress(type);
				}
			}
		}
		
		// Get phone (TEL)
		else if ((*iter2).first.compare(vCard::cVCardProperty_TEL, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
			{
				// Check for attribute types
				CAdbkAddress::EPhoneType type = CAdbkAddress::eDefaultPhoneType;
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_FAX, true))
				{
					type = CAdbkAddress::eFaxType;
					if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_HOME, true))
						type = CAdbkAddress::eHomeFaxType;
					else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_WORK, true))
						type = CAdbkAddress::eWorkFaxType;
				}
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_HOME, true))
					type = CAdbkAddress::eHomePhoneType;
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_WORK, true))
					type = CAdbkAddress::eWorkPhoneType;
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_CELL, true))
					type = CAdbkAddress::eMobilePhoneType;
				else if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_TEL_PAGER, true))
					type = CAdbkAddress::ePagerType;
				addr->SetPhone(tv->GetValue(), type, true);
				if ((*iter2).second.HasAttributeValue(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_PREF, true))
				{
					addr->SetPreferredPhone(type);
				}
			}
		}
		
		// Get url (URL)
		else if ((*iter2).first.compare(vCard::cVCardProperty_URL, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
				addr->SetURL(tv->GetValue());
		}
		
		// Get notes (NOTE)
		else if ((*iter2).first.compare(vCard::cVCardProperty_NOTE, true) == 0)
		{
			const vCard::CVCardTextValue* tv = static_cast<const vCard::CVCardTextValue*>((*iter2).second.GetTextValue());
			if (tv != NULL)
				addr->SetNotes(tv->GetValue());
		}
	}

	// Now add address
	addrs->push_back(addr.release());
}

// Map from internal address/group format to VCards
void vcardstore::MapToVCards(CAddressBook* adbk)
{
	
}

// Generate a vCard from an address object
vCard::CVCardVCard* vcardstore::GenerateVCard(const vCard::CVCardAddressBookRef& adbk, const CAdbkAddress* addr, bool is_new)
{
	vCard::CVCardVCard* vcard = new vCard::CVCardVCard(adbk);
	vcard->Initialise();
	if (is_new)
	{
		vcard->InitRevision();
		vcard->SetUID(cdstring::null_str);
		
		// A new address must have its entry set to point to the new map key of the component
		const_cast<CAdbkAddress*>(addr)->SetEntry(vcard->GetMapKey());
	}
	else
		vcard->SetUID(addr->GetEntry());
	
	UpdateVCard(vcard, addr);

	return vcard;
}

void vcardstore::ChangeVCard(vCard::CVCardAddressBook* adbk, const CAdbkAddress* addr)
{
	vCard::CVCardVCard* vcard = adbk->GetCardByKey(addr->GetEntry());
	if (vcard != NULL)
		UpdateVCard(vcard, addr);
}

void vcardstore::UpdateVCard(vCard::CVCardVCard* vcard, const CAdbkAddress* addr)
{
	// Remove all the ones we can change
	vcard->RemoveProperties(vCard::cVCardProperty_FN);
	vcard->RemoveProperties(vCard::cVCardProperty_N);
	vcard->RemoveProperties(vCard::cVCardProperty_NICKNAME);
	vcard->RemoveProperties(vCard::cVCardProperty_EMAIL);
	vcard->RemoveProperties(vCard::cVCardProperty_ADR);
	vcard->RemoveProperties(vCard::cVCardProperty_TEL);
	vcard->RemoveProperties(vCard::cVCardProperty_CALENDAR_ADDRESS);
	vcard->RemoveProperties(vCard::cVCardProperty_ORG);
	vcard->RemoveProperties(vCard::cVCardProperty_URL);
	vcard->RemoveProperties(vCard::cVCardProperty_NOTE);
	
	// Now set them
	vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_FN, addr->GetName()));
	vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_N, vCard::CVCardN(addr->GetName())));
	if (!addr->GetADL().empty())
		vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_NICKNAME, addr->GetADL()));
	if (addr->GetEmails().size() == 0)
	{
		if (!addr->GetMailAddress().empty())
		{
			vCard::CVCardProperty prop(vCard::cVCardProperty_EMAIL, addr->GetMailAddress());
			vCard::CVCardAttribute attr(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_EMAIL_INTERNET);
			prop.AddAttribute(attr);
			vcard->AddProperty(prop);
		}
	}
	else
	{
		for(CAdbkAddress::emailmap::const_iterator iter = addr->GetEmails().begin(); iter != addr->GetEmails().end(); iter++)
		{
			if ((*iter).second.empty())
				continue;

			vCard::CVCardProperty prop(vCard::cVCardProperty_EMAIL, (*iter).second);
			vCard::CVCardAttribute attr(vCard::cVCardAttribute_TYPE, vCard::cVCardAttribute_TYPE_EMAIL_INTERNET);
			
			// Now handle type
			switch((*iter).first)
			{
			case CAdbkAddress::eDefaultEmailType:
			case CAdbkAddress::eOtherEmailType:
				break;
			case CAdbkAddress::eHomeEmailType:
				attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_HOME);
				break;
			case CAdbkAddress::eWorkEmailType:
				attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_WORK);
				break;
			}
			
			// Check for preferred type
			if ((*iter).first == addr->GetPreferredEmail())
				attr.AddValue(vCard::cVCardAttribute_TYPE_PREF);
			
			// Add attribute
			prop.AddAttribute(attr);
			
			// Now add property to card
			vcard->AddProperty(prop);
		}
	}
	for(CAdbkAddress::addrmap::const_iterator iter = addr->GetAddresses().begin(); iter != addr->GetAddresses().end(); iter++)
	{
		if ((*iter).second.empty())
			continue;

		vCard::CVCardProperty prop(vCard::cVCardProperty_ADR, vCard::CVCardAddress((*iter).second));
		vCard::CVCardAttribute attr;
		attr.SetName(vCard::cVCardAttribute_TYPE);
		
		// Now handle type
		switch((*iter).first)
		{
		case CAdbkAddress::eDefaultAddressType:
			break;
		case CAdbkAddress::eHomeAddressType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_ADR_HOME);
			break;
		case CAdbkAddress::eWorkAddressType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_ADR_WORK);
			break;
		}
		
		// Check for preferred type
		if ((*iter).first == addr->GetPreferredAddress())
			attr.AddValue(vCard::cVCardAttribute_TYPE_PREF);
		
		// Add attribute if something present
		if (attr.GetValues().size() != 0)
			prop.AddAttribute(attr);
		
		// Now add property to card
		vcard->AddProperty(prop);
	}
	for(CAdbkAddress::phonemap::const_iterator iter = addr->GetPhones().begin(); iter != addr->GetPhones().end(); iter++)
	{
		if ((*iter).second.empty())
			continue;

		vCard::CVCardProperty prop(vCard::cVCardProperty_TEL, (*iter).second, vCard::CVCardValue::eValueType_Phone);
		vCard::CVCardAttribute attr;
		attr.SetName(vCard::cVCardAttribute_TYPE);
		
		// Now handle type
		switch((*iter).first)
		{
		case CAdbkAddress::eDefaultPhoneType:
		case CAdbkAddress::eOtherPhoneType:
			break;
		case CAdbkAddress::eHomePhoneType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_VOICE);
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_HOME);
			break;
		case CAdbkAddress::eWorkPhoneType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_VOICE);
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_WORK);
			break;
		case CAdbkAddress::eMobilePhoneType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_CELL);
			break;
		case CAdbkAddress::eFaxType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_FAX);
			break;
		case CAdbkAddress::eHomeFaxType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_FAX);
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_HOME);
			break;
		case CAdbkAddress::eWorkFaxType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_FAX);
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_WORK);
			break;
		case CAdbkAddress::ePagerType:
			attr.AddValue(vCard::cVCardAttribute_TYPE_TEL_PAGER);
			break;
		}
		
		// Check for preferred type
		if ((*iter).first == addr->GetPreferredPhone())
			attr.AddValue(vCard::cVCardAttribute_TYPE_PREF);
		
		// Add attribute if something present
		if (attr.GetValues().size() != 0)
			prop.AddAttribute(attr);
		
		// Now add property to card
		vcard->AddProperty(prop);
	}
	if (!addr->GetCalendar().empty())
		vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_CALENDAR_ADDRESS, addr->GetCalendar(), vCard::CVCardValue::eValueType_URI));
	if (!addr->GetCompany().empty())
		vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_ORG, addr->GetCompany()));
	if (!addr->GetURL().empty())
		vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_URL, addr->GetURL(), vCard::CVCardValue::eValueType_URI));
	if (!addr->GetNotes().empty())
		vcard->AddProperty(vCard::CVCardProperty(vCard::cVCardProperty_NOTE, addr->GetNotes()));
}
