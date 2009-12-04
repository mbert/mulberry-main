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
	CVCardMapper.h

	Author:
	Description:	XML DTDs & consts for calendar store objects
*/

#ifndef CVCardMapper_H
#define CVCardMapper_H

#include "CVCardAddressBookRef.h"

class CAdbkAddress;
class CAddressBook;
class CAddressList;

namespace vCard 
{
	class CVCardVCard;
	class CVCardAddressBook;
};

namespace vcardstore
{

	void MapFromVCards(CAddressBook* adbk);
	void MapFromVCard(CAddressBook* adbk, const vCard::CVCardVCard& vcard);
	void MapFromVCard(CAddressList* addrs, const vCard::CVCardVCard& vcard);
	void MapToVCards(CAddressBook* adbk);
	vCard::CVCardVCard* GenerateVCard(const vCard::CVCardAddressBookRef& adbk, const CAdbkAddress* addr, bool is_new = false);
	void ChangeVCard(vCard::CVCardAddressBook* adbk, const CAdbkAddress* addr);
	void UpdateVCard(vCard::CVCardVCard* vcard, const CAdbkAddress* addr);

}	// namespace vcardstore

#endif	// CVCardMapper_H
