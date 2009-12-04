/*
    Copyright (c) 2007-2008 Cyrus Daboo. All rights reserved.
    
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

#include "CVCardSearch.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CVCardAddressBook.h"
#include "CVCardComponentDB.h"
#include "CVCardMapper.h"
#include "CVCardVCard.h"

void vcardstore::SearchVCards(const vCard::CVCardAddressBook& vadbk,
							  const cdstring& pattern,
							  const cdstrvect& properties,
							  CAddressBook* adbk,
							  CAddressList* addr_list)
{
	// Look at each vCard
	for(vCard::CVCardComponentDB::const_iterator iter1 = vadbk.GetVCards().begin(); iter1 != vadbk.GetVCards().end(); iter1++)
	{
		const vCard::CVCardVCard* vcard = static_cast<const vCard::CVCardVCard*>((*iter1).second);
		SearchVCard(*vcard, pattern, properties, adbk, addr_list);
	}
}

void vcardstore::SearchVCard(const vCard::CVCardVCard& vcard,
							 const cdstring& pattern,
							 const cdstrvect& properties,
							 CAddressBook* adbk,
							 CAddressList* addr_list)
{
	for(cdstrvect::const_iterator iter1 = properties.begin(); iter1 != properties.end(); iter1++)
	{
		if (vcard.CountProperty(*iter1) != 0)
		{
			for(vCard::CVCardPropertyMap::const_iterator iter2 = vcard.GetProperties().lower_bound(*iter1); iter2 != vcard.GetProperties().upper_bound(*iter1); iter2++)
			{
				if ((*iter2).second.GetValue()->Search(pattern))
				{
					if (addr_list != NULL)
						vcardstore::MapFromVCard(addr_list, vcard);
					else if (adbk != NULL)
						vcardstore::MapFromVCard(adbk, vcard);
					return;
				}
			}
		}
	}
}
