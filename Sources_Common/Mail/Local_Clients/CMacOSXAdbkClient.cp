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


// Code for MacOSX Adbk client class

#include "CMacOSXAdbkClient.h"

#include "CAddressBook.h"
#include "CINETCommon.h"
#include "CMachOFunctions.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#include "MyCFString.h"

#include <TCFArray.h>

CMachOLoader* sABAddressBookLoader = NULL;
extern void ABAddressBook_LoadSymbols();

#pragma mark -

const char* cDefaultAdbkName = "Mac OS X";

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CMacOSXAdbkClient::CMacOSXAdbkClient(CAdbkProtocol* adbk_owner)
	: CAdbkClient(adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitAdbkClient();

}

// Copy constructor
CMacOSXAdbkClient::CMacOSXAdbkClient(const CMacOSXAdbkClient& copy, CAdbkProtocol* adbk_owner)
	: CAdbkClient(copy, adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitAdbkClient();

	mRecorder = copy.mRecorder;

}

CMacOSXAdbkClient::~CMacOSXAdbkClient()
{
	mRecorder = NULL;
	delete mTempAddr;
	mTempAddr = NULL;
	delete mTempGrp;
	mTempGrp = NULL;
}

void CMacOSXAdbkClient::InitAdbkClient()
{
#if 0	// Framework is directly linked into mach-o executable
	if (sABAddressBookLoader == NULL)
	{
		sABAddressBookLoader = new CMachOLoader("AddressBook.framework");
		if (sABAddressBookLoader->IsLoaded())
			ABAddressBook_LoadSymbols();
	}
#endif

	mRecorder = NULL;
	mRecordID = 0;

	mTempAddr = NULL;
	mTempGrp = NULL;
	mSearchMode = false;

}

// Create duplicate, empty connection
CINETClient* CMacOSXAdbkClient::CloneConnection()
{
	// Copy construct this
	return new CMacOSXAdbkClient(*this, GetAdbkOwner());

}

#pragma mark ____________________________Start/Stop

// Start TCP
void CMacOSXAdbkClient::Open()
{
	// Reset only
	Reset();
}

// Reset Account info
void CMacOSXAdbkClient::Reset()
{
	// Nothing to do
}

// Release TCP
void CMacOSXAdbkClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CMacOSXAdbkClient::Abort()
{
	// Local does nothing

}

#pragma mark ____________________________Login & Logout

// Logon to IMAP server
void CMacOSXAdbkClient::Logon()
{
#if 0	// Framework is directly linked into mach-o executable
	// Do not allow use if addressbook framework was not loaded
	if (!sABAddressBookLoader->IsLoaded())
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
		throw CINETException(CINETException::err_NoResponse);
	}
#endif
}

// Logoff from IMAP server
void CMacOSXAdbkClient::Logoff()
{
	// Local does nothing

}

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CMacOSXAdbkClient::_Tickle(bool force_tickle)
{
	// Local does nothing

}

// Handle failed capability response
void CMacOSXAdbkClient::_PreProcess()
{
	// No special preprocess

}

// Handle failed capability response
void CMacOSXAdbkClient::_PostProcess()
{
	// No special postprocess

}

#pragma mark ____________________________Address Books

// Operations on address books

void CMacOSXAdbkClient::_ListAddressBooks(CAddressBook* root)
{
	StINETClientAction action(this, "Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	InitItemCtr();

	// Provide feedback
	BumpItemCtr("Status::IMSP::AddressBookFind");

	// Use fake name
	cdstring adbk_name = cDefaultAdbkName;

	// Add adress book to list
	CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), true, false, adbk_name);
	GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);
}

// Find all adbks below this path
void CMacOSXAdbkClient::_FindAllAdbks(const cdstring& path)
{
	StINETClientAction action(this, "Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	InitItemCtr();

	// Provide feedback
	BumpItemCtr("Status::IMSP::AddressBookFind");

	// Use fake name
	cdstring adbk_name = cDefaultAdbkName;

	// Add adress book to list
	CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), true, false, adbk_name);
	GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);
}

// Create adbk
void CMacOSXAdbkClient::_CreateAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook");

	// Not supported with OS X
	CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
	throw CINETException(CINETException::err_NoResponse);
}

// Do touch
bool CMacOSXAdbkClient::_TouchAdbk(const CAddressBook* adbk)
{
	// Check it exists and create if not
	if (!_TestAdbk(adbk))
	{
		// Not supported with OS X
		CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
		throw CINETException(CINETException::err_NoResponse);
		
		return false;
	}
	else
		return true;
}

// Do test
bool CMacOSXAdbkClient::_TestAdbk(const CAddressBook* adbk)
{
	// Name must match default name
	return adbk->GetName() == cDefaultAdbkName;
}

bool CMacOSXAdbkClient::_AdbkChanged(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
	return false;
}

void CMacOSXAdbkClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
}

// Delete adbk
void CMacOSXAdbkClient::_DeleteAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddressBook", "Error::IMSP::OSErrDeleteAddressBook", "Error::IMSP::NoBadDeleteAddressBook");

	// Not supported with OS X
	CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
	throw CINETException(CINETException::err_NoResponse);
}

// Rename adbk
void CMacOSXAdbkClient::_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk)
{
	StINETClientAction action(this, "Status::IMSP::RenameAddressBook", "Error::IMSP::OSErrRenameAddressBook", "Error::IMSP::NoBadRenameAddressBook");

	// Not supported with OS X
	CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
	throw CINETException(CINETException::err_NoResponse);
}

void CMacOSXAdbkClient::_SizeAdbk(CAddressBook* adbk)
{
	// Does nothing
}

// Operations with addresses

// Find all addresses in adbk
void CMacOSXAdbkClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	_FindAllAddresses(adbk);
}

// Write all addresses in adbk
void CMacOSXAdbkClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	
}

// Find all addresses in adbk
void CMacOSXAdbkClient::_FindAllAddresses(CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress");
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all addresses
		ScanAddressBook(adbk);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

}

// Fetch named address
void CMacOSXAdbkClient::_FetchAddress(CAddressBook* adbk, const cdstrvect& names)
{
}

// Store address
void CMacOSXAdbkClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		Append(adbk, addrs, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Store group
void CMacOSXAdbkClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		Append(adbk, NULL, grps);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Change address
void CMacOSXAdbkClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		Change(adbk, addrs, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Change group
void CMacOSXAdbkClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		Change(adbk, NULL, grps);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Delete address
void CMacOSXAdbkClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	try
	{
		Delete(adbk, addrs, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Delete group
void CMacOSXAdbkClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	try
	{
		Delete(adbk, NULL, grps);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// These must be implemented by specific client

// Resolve address nick-name
void CMacOSXAdbkClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all matching addresses
		CAdbkAddress::CAddressFields fields;
		fields.push_back(CAdbkAddress::eNickName);
		SearchAddressBook(adbk, nick_name, CAdbkAddress::eMatchExactly, fields, NULL, NULL, true, false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Resolve group nick-name
void CMacOSXAdbkClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all matching addresses
		CAdbkAddress::CAddressFields fields;
		fields.push_back(CAdbkAddress::eName);
		SearchAddressBook(adbk, nick_name, CAdbkAddress::eMatchExactly, fields, NULL, NULL, false, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Search for addresses
void CMacOSXAdbkClient::_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book - addresses actually go into list provided
	mActionAdbk = adbk;

	try
	{
		// Fetch all addresses
		SearchAddressBook(adbk, name, match, fields, &addr_list, NULL, true, false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Set acl on server
void CMacOSXAdbkClient::_SetACL(CAddressBook* adbk, CACL* acl)
{
}

// Delete acl on server
void CMacOSXAdbkClient::_DeleteACL(CAddressBook* adbk, CACL* acl)
{
}

// Get all acls for adbk from server
void CMacOSXAdbkClient::_GetACL(CAddressBook* adbk)
{
}

// Get current user's rights to adbk
void CMacOSXAdbkClient::_MyRights(CAddressBook* adbk)
{
}

#pragma mark ____________________________Local Ops

// Scan address book for all addresses
void CMacOSXAdbkClient::ScanAddressBook(CAddressBook* adbk)
{
	// Get the address book; there is only one.
	ABAddressBookRef ab = ::ABGetSharedAddressBook();

	// Get all people
	CFArrayRef array = ::ABCopyArrayOfAllPeople(ab);

	// How many records matched?
	CFIndex count = ::CFArrayGetCount(array);
	for(CFIndex i = 0; i < count; i++)
	{
		// Parse into an address
		ParseAddress((ABRecordRef) ::CFArrayGetValueAtIndex(array,i));
	}
	::CFRelease(array);

	// Get all groups
	array = ::ABCopyArrayOfAllGroups(ab);

	// How many records matched?
	count = ::CFArrayGetCount(array);
	for(CFIndex i = 0; i < count; i++)
	{
		// Parse into a group
		ParseGroup((ABRecordRef) ::CFArrayGetValueAtIndex(array,i));
	}
	::CFRelease(array);
}

// Scan address book for matching addresses
void CMacOSXAdbkClient::SearchAddressBook(CAddressBook* adbk, const cdstring& name, CAdbkAddress::EAddressMatch match, const CAdbkAddress::CAddressFields& fields,
											CAddressList* addr_list, CGroupList* grp_list, bool addresses, bool groups)
{
	InitItemCtr();
	
	// Use search mode
	StValueChanger<bool> change1(mSearchMode, true);
	StValueChanger<CAddressList*> change2(mSearchResults, addr_list);

	// Get the address book; there is only one.
	ABAddressBookRef ab = ::ABGetSharedAddressBook();
		
	// Init the search spec
	ABSearchComparison comp = kABContainsSubStringCaseInsensitive;
	switch(match)
	{
	case CAdbkAddress::eMatchExactly:
		comp = kABEqualCaseInsensitive;
		break;
	case CAdbkAddress::eMatchAtStart:
		comp = kABPrefixMatchCaseInsensitive;
		break;
	case CAdbkAddress::eMatchAnywhere:
	default:
		comp = kABContainsSubStringCaseInsensitive;
		break;
	}

	// Do address search if required
	if (addresses)
	{
		// Create a search element
		TCFArray<ABSearchElementRef> array(0, &kCFTypeArrayCallBacks);
		for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
		{
			switch(*iter)
			{
			case CAdbkAddress::eName:
			default:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABFirstNameProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					ABSearchElementRef find2 = ::ABPersonCreateSearchElement(kABLastNameProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					array.Append(find2);
					::CFRelease(find1);
					::CFRelease(find2);
				}	
				break;
			case CAdbkAddress::eNickName:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABNicknameProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::eEmail:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABEmailProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::eCompany:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABOrganizationProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::eAddress:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABAddressProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::ePhoneWork:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABPhoneProperty, kABPhoneWorkLabel, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::ePhoneHome:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABPhoneProperty, kABPhoneHomeLabel, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::eFax:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABPhoneProperty, kABPhoneHomeFAXLabel, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					ABSearchElementRef find2 = ::ABPersonCreateSearchElement(kABPhoneProperty, kABPhoneWorkFAXLabel, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					array.Append(find2);
					::CFRelease(find1);
					::CFRelease(find2);
				}	
				break;
			case CAdbkAddress::eURL:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABHomePageProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			case CAdbkAddress::eNotes:
				{
					ABSearchElementRef find1 = ::ABPersonCreateSearchElement(kABNoteProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
					array.Append(find1);
					::CFRelease(find1);
					break;
				}
			}
		}
		ABSearchElementRef find = ::ABSearchElementCreateWithConjunction(kABSearchOr, array);

		// Run a search
		CFArrayRef results = ::ABCopyArrayOfMatchingRecords(ab, find);
		::CFRelease(find);
		
		// How many records matched?
		CFIndex count = ::CFArrayGetCount(results);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into an address
			ParseAddress((ABRecordRef) ::CFArrayGetValueAtIndex(results,i));
		}
		::CFRelease(results);
	}
	
	// Do group search if required
	if (groups)
	{
		// Create a search element
		ABSearchElementRef find = NULL;
		for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
		{
			switch(*iter)
			{
			case CAdbkAddress::eName:
			default:
				find = ::ABGroupCreateSearchElement(kABGroupNameProperty, NULL, NULL, MyCFString(name, kCFStringEncodingUTF8), comp);
				break;
			}
			if (find)
				break;
		}
		
		// Run a search
		CFArrayRef results = ::ABCopyArrayOfMatchingRecords(ab, find);
		::CFRelease(find);
		
		// How many records matched?
		CFIndex count = ::CFArrayGetCount(results);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into an address
			ParseGroup((ABRecordRef) ::CFArrayGetValueAtIndex(results, i));
		}
		::CFRelease(results);
	}
}


void CMacOSXAdbkClient::ParseAddress(ABRecordRef abref)
{
	// Do counter bits here as
	BumpItemCtr(mSearchMode ? "Status::IMSP::SearchAddressCount" : (mItemTotal ? "Status::IMSP::FetchAddressCount2" : "Status::IMSP::FetchAddressCount1"));
	
	// Parse and add to appropriate list
	if (mSearchResults)
		mSearchResults->push_back(ParseAddressItem(abref));
	else if (mActionAdbk)
		mActionAdbk->GetAddressList()->push_back(ParseAddressItem(abref));
}

CAdbkAddress* CMacOSXAdbkClient::ParseAddressItem(ABRecordRef abref)
{
	std::auto_ptr<CAdbkAddress> addr(new CAdbkAddress);
	
	// Get UID
	addr->SetEntry(GetAddressAttribute(abref, kABUIDProperty));
	
	// Get flags
	long flags = GetIntegerAttribute(abref, kABPersonFlags);
	flags &= kABNameOrderingMask;
	bool last_is_first = (flags == kABLastNameFirst);

	// Get first name
	cdstring firstname = GetAddressAttribute(abref, kABFirstNameProperty);
	
	// Get first name
	cdstring lastname = GetAddressAttribute(abref, kABLastNameProperty);
	
	// Create full name from each part
	cdstring fullname = (last_is_first ? lastname : firstname);
	if (fullname.length() && (last_is_first ? firstname.length() : lastname.length()))
		fullname += " ";
	fullname += (last_is_first ? firstname : lastname);
	addr->SetName(fullname);
	
	// Get nickname
	addr->SetADL(GetAddressAttribute(abref, kABNicknameProperty));
	
	// Get email address (multi-value)
	ParseAddressEmail(abref, addr.get());
	
	// Get organisation
	addr->SetCompany(GetAddressAttribute(abref, kABOrganizationProperty));
	
	// Get address bits
	ParseAddressAddress(abref, addr.get());

	// Get phones
	ParseAddressPhone(abref, addr.get());
	
	// Get url
	cdstring url = GetAddressAttribute(abref, kABHomePageProperty);
	url.ConvertEndl();
	addr->SetURL(url);
	
	// Get notes
	cdstring notes = GetAddressAttribute(abref, kABNoteProperty);
	notes.ConvertEndl();
	addr->SetNotes(notes);

	// Return it
	return addr.release();
}

void CMacOSXAdbkClient::ParseAddressEmail(ABRecordRef abref, CAdbkAddress* addr)
{
	// Get the multi-entry for the attribute
	ABMultiValueRef mvalue = (ABMultiValueRef) ::ABRecordCopyValue(abref, kABEmailProperty);

	if (mvalue)
	{
		unsigned int size = ::ABMultiValueCount(mvalue);
		for(int i = 0; i < size; i++)
		{
			CFStringRef value = (CFStringRef) ::ABMultiValueCopyValueAtIndex(mvalue, i);
			if (value)
			{
				// Get value string
				cdstring valuestr = GetStringValue(value);;
				
				// Now map label to type
				CFStringRef label = ::ABMultiValueCopyLabelAtIndex(mvalue, i);
				if (label)
				{
					if (::CFStringCompare(label, kABEmailWorkLabel, 0) == kCFCompareEqualTo)
						addr->SetEmail(valuestr, CAdbkAddress::eWorkEmailType, true);
					else if (::CFStringCompare(label, kABEmailHomeLabel, 0) == kCFCompareEqualTo)
						addr->SetEmail(valuestr, CAdbkAddress::eHomeEmailType, true);
					else if (::CFStringCompare(label, kABOtherLabel, 0) == kCFCompareEqualTo)
						addr->SetEmail(valuestr, CAdbkAddress::eOtherEmailType, true);
					
					::CFRelease(label);
				}
			}
		}
	}
}

void CMacOSXAdbkClient::ParseAddressAddress(ABRecordRef abref, CAdbkAddress* addr)
{
	// Get the multi-entry for the attribute
	ABMultiValueRef mvalue = (ABMultiValueRef) ::ABRecordCopyValue(abref, kABAddressProperty);

	if (mvalue)
	{
		unsigned int size = ::ABMultiValueCount(mvalue);
		for(int i = 0; i < size; i++)
		{
			CFDictionaryRef dict = (CFDictionaryRef) ::ABMultiValueCopyValueAtIndex(mvalue, i);
			if (dict)
			{
				// Get address bits
				cdstring street = GetDictionaryValue(dict, kABAddressStreetKey);
				cdstring city = GetDictionaryValue(dict, kABAddressCityKey);
				cdstring state = GetDictionaryValue(dict, kABAddressStateKey);
				cdstring zip = GetDictionaryValue(dict, kABAddressZIPKey);
				cdstring country = GetDictionaryValue(dict, kABAddressCountryKey);
				::CFRelease(dict);

				cdstring maddr;
				if (!street.empty())
					maddr += street;
				if (!city.empty())
				{
					if (!maddr.empty())
						maddr += os_endl;
					maddr += city;
				}
				if (!state.empty())
				{
					if (!maddr.empty())
						maddr += os_endl;
					maddr += state;
					
					if (!zip.empty())
					{
						maddr += " ";
						maddr += zip;
					}
				}
				if (!country.empty())
				{
					if (!maddr.empty())
						maddr += os_endl;
					maddr += country;
				}
				maddr.ConvertEndl();
				
				
				// Now map label to type
				CFStringRef label = ::ABMultiValueCopyLabelAtIndex(mvalue, i);
				if (label)
				{
					if (::CFStringCompare(label, kABAddressHomeLabel, 0) == kCFCompareEqualTo)
						addr->SetAddress(maddr, CAdbkAddress::eHomeAddressType, true);
					else if (::CFStringCompare(label, kABAddressWorkLabel, 0) == kCFCompareEqualTo)
						addr->SetAddress(maddr, CAdbkAddress::eWorkAddressType, true);
					
					::CFRelease(label);
				}
			}
		}
	}
}

void CMacOSXAdbkClient::ParseAddressPhone(ABRecordRef abref, CAdbkAddress* addr)
{
	// Get the multi-entry for the attribute
	ABMultiValueRef mvalue = (ABMultiValueRef) ::ABRecordCopyValue(abref, kABPhoneProperty);

	if (mvalue)
	{
		unsigned int size = ::ABMultiValueCount(mvalue);
		for(int i = 0; i < size; i++)
		{
			CFStringRef value = (CFStringRef) ::ABMultiValueCopyValueAtIndex(mvalue, i);
			if (value)
			{
				// Get value string
				cdstring valuestr = GetStringValue(value);;
				
				// Now map label to type
				CFStringRef label = ::ABMultiValueCopyLabelAtIndex(mvalue, i);
				if (label)
				{
					if (::CFStringCompare(label, kABPhoneWorkLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eWorkPhoneType, true);
					else if (::CFStringCompare(label, kABPhoneHomeLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eHomePhoneType, true);
					else if (::CFStringCompare(label, kABPhoneMobileLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eMobilePhoneType, true);
					else if (::CFStringCompare(label, kABPhoneMainLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eDefaultPhoneType, true);
					else if (::CFStringCompare(label, kABPhoneHomeFAXLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eHomeFaxType, true);
					else if (::CFStringCompare(label, kABPhoneWorkFAXLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eWorkFaxType, true);
					else if (::CFStringCompare(label, kABPhonePagerLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::ePagerType, true);
					else if (::CFStringCompare(label, kABOtherLabel, 0) == kCFCompareEqualTo)
						addr->SetPhone(valuestr, CAdbkAddress::eOtherPhoneType, true);
					
					::CFRelease(label);
				}
			}
		}
	}
}

void CMacOSXAdbkClient::ParseGroup(ABRecordRef abref)
{
	// Do counter bits here as
	BumpItemCtr(mSearchMode ? "Status::IMSP::SearchAddressCount" : (mItemTotal ? "Status::IMSP::FetchAddressCount2" : "Status::IMSP::FetchAddressCount1"));
	
	// Prase and add to appropriate list
	if (!mSearchResults && mActionAdbk)
		mActionAdbk->GetGroupList()->push_back(ParseGroupItem(abref));
}

CGroup* CMacOSXAdbkClient::ParseGroupItem(ABRecordRef abref)
{
	std::auto_ptr<CGroup> grp(new CGroup);

	// Get UID
	grp->SetEntry(GetAddressAttribute(abref, kABUIDProperty));
	
	// Get name
	grp->GetName() = GetAddressAttribute(abref, kABGroupNameProperty);

	// Get nickname
	grp->GetNickName() = GetAddressAttribute(abref, kABNicknameProperty);

	// Force nick-name to name as we always need nick-names
	if (grp->GetNickName().empty())
		grp->GetNickName() = grp->GetName();

	// Get all people
	{
		CFArrayRef array = ::ABGroupCopyArrayOfAllMembers((ABGroupRef) abref);

		// How many records matched?
		CAddressList results;
		CFIndex count = ::CFArrayGetCount(array);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into an address
			results.push_back(ParseAddressItem((ABRecordRef) ::CFArrayGetValueAtIndex(array,i)));
			
		}
		::CFRelease(array);

		// Add all results to the group
		grp->AddAddresses(results, true);
	}

	// Get all groups
	{
		CFArrayRef array = ::ABGroupCopyArrayOfAllSubgroups((ABGroupRef) abref);

		// How many records matched?
		CFIndex count = ::CFArrayGetCount(array);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into a group
			std::auto_ptr<CGroup> subgrp(ParseGroupItem((ABRecordRef) ::CFArrayGetValueAtIndex(array,i)));
			
			// Add the groups nick-name (if any) to the parent group
			if (!subgrp->GetNickName().empty())
				grp->AddAddress(subgrp->GetNickName());
		}
		::CFRelease(array);
	}

	// Return it
	return grp.release();
}

long CMacOSXAdbkClient::GetIntegerAttribute(ABRecordRef abref, CFStringRef attrib)
{
	long result = 0;
	CFNumberRef num = (CFNumberRef) ::ABRecordCopyValue(abref, attrib);
	if (num)
		::CFNumberGetValue(num, kCFNumberSInt32Type, &result);
	return result;
}
cdstring CMacOSXAdbkClient::GetAddressAttribute(ABRecordRef abref, CFStringRef attrib)
{
	return GetStringValue((CFStringRef) ::ABRecordCopyValue(abref, attrib));
}

cdstring CMacOSXAdbkClient::GetAddressAttribute(ABRecordRef abref, CFStringRef attrib1, CFStringRef attrib2)
{
	cdstring result;

	// Get the multi-entry for the attribute
	ABMultiValueRef mvalue = (ABMultiValueRef) ::ABRecordCopyValue(abref, attrib1);

	if (mvalue)
	{
		// Get the primary value
		CFStringRef primary = NULL;
		if (attrib2 == NULL)
			attrib2 = primary = ::ABMultiValueCopyPrimaryIdentifier(mvalue);

		// Get index for value we want
		int index = ::ABMultiValueIndexForIdentifier(mvalue, attrib2);
		if (primary)
			::CFRelease(primary);

		CFStringRef text = NULL;
		if (index >= 0)
			text = (CFStringRef) ::ABMultiValueCopyValueAtIndex(mvalue, index);
		result = GetStringValue(text);

		::CFRelease(mvalue);
	}

	return result;
}

cdstring CMacOSXAdbkClient::GetStringValue(CFStringRef text)
{
	cdstring result;
	if (text)
	{
		const char* str = ::CFStringGetCStringPtr(text, kCFStringEncodingUTF8);
		if (str)
			result = str;
		else
		{
			unsigned long length = ::CFStringGetLength(text) + 10;
			result.reserve(length);
			Boolean success = ::CFStringGetCString(text, result.c_str_mod(), length, kCFStringEncodingUTF8);
			if (!success)
				result = cdstring::null_str;
		}
		
		// We always release this as a convenience
		::CFRelease(text);
	}

	return result;
}

cdstring CMacOSXAdbkClient::GetDictionaryValue(CFDictionaryRef dict, CFStringRef dictkey)
{
	return dict ? GetStringValue((CFStringRef) ::CFDictionaryGetValue(dict, dictkey)) : cdstring::null_str;
}

void CMacOSXAdbkClient::Append(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps)
{
	 // Get the address book - there is only one.
	ABAddressBookRef ab = ::ABGetSharedAddressBook();

	if (addrs != NULL)
	{
		// Iterate over each one in the list
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);

			// Create a new record
			ABPersonRef person = ::ABPersonCreate();

			// Fill with address data
			Fill(person, addr);

			// Add record to the Address Book
			if (!::ABAddRecord(ab, person))
			{
				::CFRelease(person);
				CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
				throw CINETException(CINETException::err_NoResponse);
			}

			// Need to read in UID to sync it up
			const_cast<CAdbkAddress*>(addr)->SetEntry(GetAddressAttribute(person, kABUIDProperty));
			
			// Done with record
			::CFRelease(person);
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}

	if (grps != NULL)
	{
		// Iterate over each one in the list
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		{
			const CGroup* grp = static_cast<const CGroup*>(*iter);

			// Create a new record
			ABGroupRef group = ::ABGroupCreate();

			// Fill with address data
			Fill(ab, group, adbk, grp);

			// Add record to the Address Book
			if (!::ABAddRecord(ab, group))
			{
				::CFRelease(group);
				CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
				throw CINETException(CINETException::err_NoResponse);
			}

			// Need to read in UID to sync it up
			const_cast<CGroup*>(grp)->SetEntry(GetAddressAttribute(group, kABUIDProperty));
			
			// Done with record
			::CFRelease(group);
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}
}

void CMacOSXAdbkClient::Change(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps)
{
	 // Get the address book - there is only one.
	ABAddressBookRef ab = ::ABGetSharedAddressBook();

	if (addrs != NULL)
	{
		// Iterate over each one in the list
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);

			ABPersonRef person = (ABPersonRef) ::ABCopyRecordForUniqueId(ab, MyCFString(addr->GetEntry(), kCFStringEncodingUTF8));
			if (person)
			{
				// Fill with address data
				Fill(person, addr);
				
				// Done with record
				::CFRelease(person);
			}
			
			// Update entry to match new name
			const_cast<CAdbkAddress*>(addr)->SetEntry(addr->GetName());
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}

	if (grps != NULL)
	{
		// Iterate over each one in the list
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		{
			const CGroup* grp = static_cast<const CGroup*>(*iter);

			ABGroupRef group = (ABGroupRef) ::ABCopyRecordForUniqueId(ab, MyCFString(grp->GetEntry(), kCFStringEncodingUTF8));
			if (group)
			{
				// Fill with address data
				Fill(ab, group, adbk, grp);
				
				// Done with record
				::CFRelease(group);
			}
			
			// Update entry to match new name
			const_cast<CGroup*>(grp)->SetEntry(grp->GetName());
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}
}

void CMacOSXAdbkClient::Delete(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps)
{
	 // Get the address book - there is only one.
	ABAddressBookRef ab = ::ABGetSharedAddressBook();

	if (addrs != NULL)
	{
		// Iterate over each one in the list
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);

			ABRecordRef person = ::ABCopyRecordForUniqueId(ab, MyCFString(addr->GetEntry(), kCFStringEncodingUTF8));
			if (person)
			{
				// Remove record
				if (!::ABRemoveRecord(ab, person))
				{
					::CFRelease(person);
					CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
					throw CINETException(CINETException::err_NoResponse);
				}

				::CFRelease(person);
			}
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}

	if (grps != NULL)
	{
		// Iterate over each one in the list
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		{
			const CGroup* grp = static_cast<const CGroup*>(*iter);

			ABRecordRef group = ::ABCopyRecordForUniqueId(ab, MyCFString(grp->GetEntry(), kCFStringEncodingUTF8));
			if (group)
			{
				// Remove record
				if (!::ABRemoveRecord(ab, group))
				{
					::CFRelease(group);
					CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
					throw CINETException(CINETException::err_NoResponse);
				}

				::CFRelease(group);
			}
		}

		// Save the Address Book
		if (!::ABSave(ab))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
	}
}

// Copy address items into OS X record
void CMacOSXAdbkClient::Fill(ABPersonRef person, const CAdbkAddress* addr)
{
	// Split name into two parts
	cdstring fullname = addr->GetName();
	cdstring firstname;
	cdstring lastname;
	const char* p = ::strchr(fullname.c_str(), ' ');
	if (p)
	{
		firstname.assign(fullname.c_str(), p - fullname.c_str());
		lastname = p + 1;
		lastname.trimspace();
	}
	else
		lastname = fullname;
	
	// Set first name
	if (!firstname.empty())
		::ABRecordSetValue(person, kABFirstNameProperty, MyCFString(firstname, kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABFirstNameProperty);

	// Set last name
	if (!lastname.empty())
		::ABRecordSetValue(person, kABLastNameProperty, MyCFString(lastname, kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABLastNameProperty);

	// Set nick-name
	if (!addr->GetADL().empty())
		::ABRecordSetValue(person, kABNicknameProperty, MyCFString(addr->GetADL(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABNicknameProperty);

	// Set email (multivalue)
	{
		bool added = false;
		ABMutableMultiValueRef mvalue = ::ABMultiValueCreateMutable();
		added |= FillMulti(mvalue, addr, CAdbkAddress::eHomeEmailType, kABEmailHomeLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eWorkEmailType, kABEmailWorkLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eOtherEmailType, kABOtherLabel);
		
		// If no explicit home/work, then use default
		if (!added)
			added |= FillMulti(mvalue, addr, CAdbkAddress::eDefaultEmailType, kABEmailWorkLabel);

		if (added)
			::ABRecordSetValue(person, kABEmailProperty, mvalue);
		else
			::ABRecordRemoveValue(person, kABEmailProperty);
		::CFRelease(mvalue);
	}

	// Set organisation
	if (!addr->GetCompany().empty())
		::ABRecordSetValue(person, kABOrganizationProperty, MyCFString(addr->GetCompany(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABOrganizationProperty);

	// Address is multi-valued
	{
		bool added = false;
		ABMutableMultiValueRef mvalue = ::ABMultiValueCreateMutable();
		added |= FillMulti(mvalue, addr, CAdbkAddress::eHomeAddressType, kABAddressHomeLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eWorkAddressType, kABAddressWorkLabel);
		
		// If no explicit home/work, then use default
		if (!added)
			added |= FillMulti(mvalue, addr, CAdbkAddress::eDefaultAddressType, kABAddressWorkLabel);

		if (added)
			::ABRecordSetValue(person, kABAddressProperty, mvalue);
		else
			::ABRecordRemoveValue(person, kABAddressProperty);
		::CFRelease(mvalue);
	}

	// Phone is multi-valued
	{
		bool added = false;
		bool added_fax = false;
		ABMutableMultiValueRef mvalue = ::ABMultiValueCreateMutable();
		added |= FillMulti(mvalue, addr, CAdbkAddress::eHomePhoneType, kABPhoneHomeLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eWorkPhoneType, kABPhoneWorkLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eMobilePhoneType, kABPhoneMobileLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eDefaultPhoneType, kABPhoneMainLabel);
		added |= (added_fax |= FillMulti(mvalue, addr, CAdbkAddress::eHomeFaxType, kABPhoneHomeFAXLabel));
		added |= (added_fax |= FillMulti(mvalue, addr, CAdbkAddress::eWorkFaxType, kABPhoneWorkFAXLabel));
		if (!added_fax)
			added |= FillMulti(mvalue, addr, CAdbkAddress::eFaxType, kABPhoneWorkFAXLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::ePagerType, kABPhonePagerLabel);
		added |= FillMulti(mvalue, addr, CAdbkAddress::eOtherPhoneType, kABOtherLabel);

		if (added)
			::ABRecordSetValue(person, kABPhoneProperty, mvalue);
		else
			::ABRecordRemoveValue(person, kABPhoneProperty);
		::CFRelease(mvalue);
	}

	// Set URL
	if (!addr->GetURL().empty())
		::ABRecordSetValue(person, kABHomePageProperty, MyCFString(addr->GetURL(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABHomePageProperty);

	// Set notes
	if (!addr->GetNotes().empty())
		::ABRecordSetValue(person, kABNoteProperty, MyCFString(addr->GetNotes(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(person, kABNoteProperty);
}

bool CMacOSXAdbkClient::FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EEmailType type, CFStringRef attrib)
{
	if (!addr->GetEmail(type).empty())
	{
		::ABMultiValueAdd(mvalue, MyCFString(addr->GetEmail(type), kCFStringEncodingUTF8), attrib, NULL);
		return true;
	}
	else
		return false;
}

bool CMacOSXAdbkClient::FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EAddressType type, CFStringRef attrib)
{
	if (!addr->GetAddress(type).empty())
	{
		// Split mailing address into lines
		cdstring address = addr->GetAddress(type);
		
		CFMutableDictionaryRef maddr = ::CFDictionaryCreateMutable(NULL, 5, NULL, &kCFTypeDictionaryValueCallBacks);

		const char* p = ::strtok(address.c_str_mod(), "\r\n");
		if (p)
			::CFDictionaryAddValue(maddr, kABAddressStreetKey, (CFStringRef)MyCFString(p, kCFStringEncodingUTF8));
		else
			::CFDictionaryAddValue(maddr, kABAddressStreetKey, CFSTR(""));
		p = ::strtok(NULL, "\r\n");
		if (p)			
			::CFDictionaryAddValue(maddr, kABAddressCityKey, (CFStringRef)MyCFString(p, kCFStringEncodingUTF8));
		else
			::CFDictionaryAddValue(maddr, kABAddressCityKey, CFSTR(""));
		::CFDictionaryAddValue(maddr, kABAddressStateKey, CFSTR(""));
		::CFDictionaryAddValue(maddr, kABAddressZIPKey, CFSTR(""));
		p = ::strtok(NULL, "\r\n");
		if (p)			
			::CFDictionaryAddValue(maddr, kABAddressCountryKey, (CFStringRef)MyCFString(p, kCFStringEncodingUTF8));
		else
			::CFDictionaryAddValue(maddr, kABAddressCountryKey, CFSTR(""));

		::ABMultiValueAdd(mvalue, maddr, attrib, NULL);
		::CFRelease(maddr);
		
		return true;
	}
	else
		return false;
}

bool CMacOSXAdbkClient::FillMulti(ABMutableMultiValueRef mvalue, const CAdbkAddress* addr, CAdbkAddress::EPhoneType type, CFStringRef attrib)
{
	if (!addr->GetPhone(type).empty())
	{
		::ABMultiValueAdd(mvalue, MyCFString(addr->GetPhone(type), kCFStringEncodingUTF8), attrib, NULL);
		return true;
	}
	else
		return false;
}

// Copy group items into OS X record
void CMacOSXAdbkClient::Fill(ABAddressBookRef ab, ABGroupRef group, CAddressBook* adbk, const CGroup* grp)
{
	// Set name
	if (!grp->GetName().empty())
		::ABRecordSetValue(group, kABGroupNameProperty, MyCFString(grp->GetName(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(group, kABGroupNameProperty);

	// Set nick-name
	if (!grp->GetNickName().empty())
		::ABRecordSetValue(group, kABNicknameProperty, MyCFString(grp->GetNickName(), kCFStringEncodingUTF8));
	else
		::ABRecordRemoveValue(group, kABNicknameProperty);

	// Get all existing people
	cdstrvect addr_entries;
	{
		CFArrayRef array = ::ABGroupCopyArrayOfAllMembers(group);

		// How many records matched?
		CAddressList results;
		CFIndex count = ::CFArrayGetCount(array);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into an address
			std::auto_ptr<CAdbkAddress> addr(ParseAddressItem((ABRecordRef) ::CFArrayGetValueAtIndex(array,i)));
			addr_entries.push_back(addr->GetEntry());
			
		}
		::CFRelease(array);
	}

	// Get all existing groups
	cdstrvect grp_entries;
	{
		CFArrayRef array = ::ABGroupCopyArrayOfAllSubgroups(group);

		// How many records matched?
		CFIndex count = ::CFArrayGetCount(array);
		for(CFIndex i = 0; i < count; i++)
		{
			// Parse into a group
			std::auto_ptr<CGroup> subgrp(ParseGroupItem((ABRecordRef) ::CFArrayGetValueAtIndex(array,i)));
			grp_entries.push_back(subgrp->GetEntry());
		}
		::CFRelease(array);
	}
	
	// Now get items in current group
	
	// Get people and groups
	cdstrvect addrs;
	cdstrvect grps;
	{
		// Look at each item in the group and try to match with a single address or group in the address book
		for(cdstrvect::const_iterator iter = grp->GetAddressList().begin(); iter != grp->GetAddressList().end(); iter++)
		{
			// If it contains an '@' or '<..>' then its a person
			if (::strpbrk(*iter, "@<>") != NULL)
			{
				// Parse into an address
				CAddress addr(*iter);

				// Find the address in the address book that matches this item
				CAdbkAddress* found = adbk->FindAddress(&addr);
				if (found != NULL)
				{
					// Add entry to list
					addrs.push_back(found->GetEntry());
				}
			}
			else
			{
				// Must be a nick-name
				CAdbkAddress* addr_found;
				CGroup* grp_found;
				if (adbk->FindNickName(*iter, addr_found))
					addrs.push_back(addr_found->GetEntry());
				else if (adbk->FindGroupName(*iter, grp_found))
					grps.push_back(grp_found->GetEntry());
			}
		}
	}
	
	// Now determine which ones in the existing items no longer exist in the new one
	for(cdstrvect::const_iterator iter = addr_entries.begin(); iter != addr_entries.end(); iter++)
	{
		// Try to find this one in the groups actual list
		cdstrvect::const_iterator found = std::find(addrs.begin(), addrs.end(), *iter);
		if (found == addrs.end())
		{
			// Remove the entry from the current group
			ABPersonRef person = (ABPersonRef) ::ABCopyRecordForUniqueId(ab, MyCFString(*iter, kCFStringEncodingUTF8));
			if (person)
			{
				::ABGroupRemoveMember(group, person);
				::CFRelease(person);
			}
		}
	}
	for(cdstrvect::const_iterator iter = grp_entries.begin(); iter != grp_entries.end(); iter++)
	{
		// Try to find this one in the groups actual list
		cdstrvect::const_iterator found = std::find(grps.begin(), grps.end(), *iter);
		if (found == addrs.end())
		{
			// Remove the entry from the current group
			ABGroupRef sub_group = (ABGroupRef) ::ABCopyRecordForUniqueId(ab, MyCFString(*iter, kCFStringEncodingUTF8));
			if (sub_group)
			{
				::ABGroupRemoveGroup(group, sub_group);
				::CFRelease(sub_group);
			}
		}
	}
	
	// Now determine which ones are new
	for(cdstrvect::const_iterator iter = addrs.begin(); iter != addrs.end(); iter++)
	{
		// Try to find this one in the groups previous list
		cdstrvect::const_iterator found = std::find(addr_entries.begin(), addr_entries.end(), *iter);
		if (found == addr_entries.end())
		{
			// Add the entry to the current group
			ABPersonRef person = (ABPersonRef) ::ABCopyRecordForUniqueId(ab, MyCFString(*iter, kCFStringEncodingUTF8));
			if (person)
			{
				::ABGroupAddMember(group, person);
				::CFRelease(person);
			}
		}
	}
	for(cdstrvect::const_iterator iter = grps.begin(); iter != grps.end(); iter++)
	{
		// Try to find this one in the groups actual list
		cdstrvect::const_iterator found = std::find(grp_entries.begin(), grp_entries.end(), *iter);
		if (found == grp_entries.end())
		{
			// Add the entry to the current group
			ABGroupRef sub_group = (ABGroupRef) ::ABCopyRecordForUniqueId(ab, MyCFString(*iter, kCFStringEncodingUTF8));
			if (sub_group)
			{
				::ABGroupAddGroup(group, sub_group);
				::CFRelease(sub_group);
			}
		}
	}
}
