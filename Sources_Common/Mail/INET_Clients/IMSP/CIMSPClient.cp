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


// Code for IMAP client class

#include "CIMSPClient.h"

#include "CAdbkACL.h"
#include "CAddressBook.h"
#include "CCharSpecials.h"
#include "CGeneralException.h"
#include "CIMAPCommon.h"
#include "CINETClientResponses.h"
#include "CINETCommon.h"
#include "CMailControl.h"
#include "CRFC822.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#include "CVCardMapper.h"

#include "CVCardAddressBook.h"
#include "CVCardVCard.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#else
#include "StValueChanger.h"
#endif

#include <stdio.h>
#include <stdlib.h>

template<> class StValueChanger<bool>
{
public:
	StValueChanger(bool &ioVariable, bool inNewValue)
		: mVariable(ioVariable),
		  mOrigValue(ioVariable)
		{ ioVariable = inNewValue; }
	~StValueChanger()
		{ mVariable = mOrigValue; }

	bool &mVariable;
	bool mOrigValue;
};

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CIMSPClient::CIMSPClient(COptionsProtocol* options_owner, CAdbkProtocol* adbk_owner)
	: COptionsClient(options_owner),
	  CAdbkClient(adbk_owner)
{
	// Must set INET to real owner (can only be one)
	if ((options_owner && adbk_owner) ||
		(!options_owner && !adbk_owner))
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	if (options_owner)
	{
		mOwner = options_owner;
		mOptions = true;
	}
	else
	{
		mOwner = adbk_owner;
		mOptions = false;
	}

	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitIMSPClient();

}

// Copy constructor
CIMSPClient::CIMSPClient(const CIMSPClient& copy, COptionsProtocol* options_owner, CAdbkProtocol* adbk_owner)
	: COptionsClient(copy, options_owner),
	  CAdbkClient(copy, adbk_owner)
{
	// Must set INET to real owner (can only be one)
	if ((options_owner && adbk_owner) ||
		(!options_owner && !adbk_owner))
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	if (options_owner)
	{
		mOwner = options_owner;
		mOptions = true;
	}
	else
	{
		mOwner = adbk_owner;
		mOptions = false;
	}

	// Stream will be copied by parent constructor

	// Init instance variables
	InitIMSPClient();

}

CIMSPClient::~CIMSPClient()
{
	mMembersGroups = NULL;
}

void CIMSPClient::InitIMSPClient()
{
	// Init instance variables
	mVersion = eIMSP;

	mSearchMode = false;

	mMembersGroups = NULL;

	mLogType = CLog::eLogIMSP;

}

// Create duplicate, empty connection
CINETClient* CIMSPClient::CloneConnection()
{
	// Copy construct this
	return new CIMSPClient(*this);

} // CIMSPClient::CloneConnection

// Get default port
tcp_port CIMSPClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return cIMSPServerPort_SSL;
	else
		return cIMSPServerPort;
}

#pragma mark ____________________________Protocol

// Process for output force literal rather than quote
int CIMSPClient::ProcessString(cdstring& str)
{
	bool quote = false;
	bool literal = false;

	// Look for special cases
	const char* p = str.c_str();
	while(*p && !literal)
	{
		switch(cINETChar[(unsigned char) *p++])
		{
		case 0: // Atom
			break;
		case 1: // Quote
			quote = true;
			break;
		case 2: // Escaped - force to literal
		case 3: // Literal
			literal = true;
			break;
		}
	}

	// Process literal above all else (hack for IMSP bug: strings of length > 512 always literal)
	if (literal || (str.length() > 512))

		// Just return literal size - will be sent as is
		return str.length();

	// Quote forcibly (use INET style escape chars)
	if (quote || str.empty())
		str.quote(true, true);

	return 0;
}

// Initialise capability flags to empty set
void CIMSPClient::_InitCapability()
{
	// Init capabilities to empty set
	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CIMSPClient::_ProcessCapability()
{
	// Look for other capabilities
	mAuthLoginAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHLOGIN, true);
	mAuthPlainAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHPLAIN, true);
	mAuthAnonAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHANON, true);
	mSTARTTLSAllowed = mLastResponse.CheckUntagged(cSTARTTLS, true);
	
	// IMSP always has ACLs
	GetAdbkOwner()->SetHasACL(true);
}

// Handle failed capability response
void CIMSPClient::_NoCapability()
{
	// No capabilities

}

// Handle failed capability response
void CIMSPClient::_PreProcess()
{
	// No special preprocess

}

// Handle failed capability response
void CIMSPClient::_PostProcess()
{
	// No special postprocess

}

// Handle failed capability response
void CIMSPClient::_ParseResponse(char** txt,
							CINETClientResponse* response)
{
	IMSPParseResponse(txt, response);
} // CIMSPClient::_ParseResponse

#pragma mark ____________________________Options

// Find all options below this key
void CIMSPClient::_FindAllAttributes(const cdstring& entry)
{
	cdstring option = GetKeyPrefix(entry) + cWILDCARD;

	// Send GET message to server
	INETStartSend("Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");
	INETSendString(cGET);
	INETSendString(cSpace);
	INETSendString(option);
	INETFinishSend();
}

// Set all options in owmer map
void CIMSPClient::_SetAllAttributes(const cdstring& entry)
{
	cdstring prefix = GetKeyPrefix(entry);

	for(cdstrmap::const_iterator iter = GetOptionsOwner()->GetMap()->begin(); iter != GetOptionsOwner()->GetMap()->end(); iter++)
	{
		// Form key
		cdstring key = prefix + (*iter).first;

		// Send SET message to server
		INETStartSend("Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");
		INETSendString(cSET);
		INETSendString(cSpace);
		INETSendString(key);
		INETSendString(cSpace);
		INETSendString((*iter).second, eQueueProcess);
		INETFinishSend();
	}
}

// Delete the entire entry
void CIMSPClient::_DeleteEntry(const cdstring& entry)
{
	cdstring prefix = GetKeyPrefix(entry);

	for(cdstrmap::const_iterator iter = GetOptionsOwner()->GetMap()->begin(); iter != GetOptionsOwner()->GetMap()->end(); iter++)
	{
		// Form key
		cdstring key = prefix + (*iter).first;

		// Send UNSET message to server
		INETStartSend("Status::IMSP::Unsetting", "Error::IMSP::OSErrUnset", "Error::IMSP::NoBadUnset");
		INETSendString(cUNSET);
		INETSendString(cSpace);
		INETSendString(key);
		INETFinishSend();
	}
}

// Get option
void CIMSPClient::_GetAttribute(const cdstring& entry, const cdstring& attribute)
{
	try
	{
		// Form key
		cdstring fullkey = GetKeyPrefix(entry);
		if (!attribute.empty())
			fullkey += attribute;
		else
			fullkey[fullkey.length() - 1] = 0;
		cdstring temp = fullkey;

		// Set for single opeeration
		mSingle = true;
		mSingleKey = &temp;

		// Send GET message to server
		INETStartSend("Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");
		INETSendString(cGET);
		INETSendString(cSpace);
		INETSendString(fullkey);
		INETFinishSend();

		// Reset single opeeration
		mSingle = false;
		mSingleKey = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Reset single opeeration
		mSingle = false;
		mSingleKey = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set option
void CIMSPClient::_SetAttribute(const cdstring& entry, const cdstring& attribute, const cdstring& value)
{
	// Form key
	cdstring fullkey = GetKeyPrefix(entry);
	if (!attribute.empty())
		fullkey += attribute;
	else
		fullkey[fullkey.length() - 1] = 0;

	// Send SET message to server
	INETStartSend("Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");
	INETSendString(cSET);
	INETSendString(cSpace);
	INETSendString(fullkey);
	INETSendString(cSpace);
	INETSendString(value, eQueueProcess);
	INETFinishSend();
}

// Unset option with this key
void CIMSPClient::_DeleteAttribute(const cdstring& entry, const cdstring& attribute)
{
	// Form key
	cdstring fullkey = GetKeyPrefix(entry) + attribute;

	// Send SET message to server
	INETStartSend("Status::IMSP::Unsetting", "Error::IMSP::OSErrUnset", "Error::IMSP::NoBadUnset");
	INETSendString(cUNSET);
	INETSendString(cSpace);
	INETSendString(fullkey);
	INETFinishSend();
}

#pragma mark ____________________________Address Books

// Operations on address books

void CIMSPClient::_ListAddressBooks(CAddressBook* root)
{
	// Must add wildcard and quote value
	cdstring qvalue = (root->IsProtocol() ? cdstring::null_str : root->GetName());
	qvalue += cWILDCARD;

	// Send ADDRESSBOOK message to server
	INETStartSend("Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	INETSendString(cADDRESSBOOK);
	INETSendString(cSpace);
	INETSendString(qvalue, eQueueProcess);
	INETFinishSend();
}

// Find all adbks below this path
void CIMSPClient::_FindAllAdbks(const cdstring& path)
{
	// Must add wildcard and quote value
	cdstring qvalue = path;
	qvalue += cWILDCARD;

	// Send ADDRESSBOOK message to server
	INETStartSend("Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	INETSendString(cADDRESSBOOK);
	INETSendString(cSpace);
	INETSendString(qvalue, eQueueProcess);
	INETFinishSend();
}

// Create adbk
void CIMSPClient::_CreateAdbk(const CAddressBook* adbk)
{
	// Send CREATEADDRESSBOOK message to server
	INETStartSend("Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook", adbk->GetName());
	INETSendString(cCREATEADDRESSBOOK);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETFinishSend();
}

bool CIMSPClient::_AdbkChanged(const CAddressBook* adbk)
{
	// No way to tell if there have been changes on the server, so return true to force entire sync
	return true;
}

void CIMSPClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// No way to tell if there have been changes on the server, so return true to force entire sync
}

// Delete adbk
void CIMSPClient::_DeleteAdbk(const CAddressBook* adbk)
{
	// Send DELETEADDRESSBOOK message to server
	INETStartSend("Status::IMSP::DeleteAddressBook", "Error::IMSP::OSErrDeleteAddressBook", "Error::IMSP::NoBadDeleteAddressBook", adbk->GetName());
	INETSendString(cDELETEADDRESSBOOK);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETFinishSend();
}

// Rename adbk
void CIMSPClient::_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk)
{
	// Send RENAMEADDRESSBOOK message to server
	INETStartSend("Status::IMSP::RenameAddressBook", "Error::IMSP::OSErrRenameAddressBook", "Error::IMSP::NoBadRenameAddressBook", old_adbk->GetName());
	INETSendString(cRENAMEADDRESSBOOK);
	INETSendString(cSpace);
	INETSendString(old_adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);
	INETSendString(new_adbk, eQueueProcess);
	INETFinishSend();
}

void CIMSPClient::_SizeAdbk(CAddressBook* adbk)
{
	// Does nothing
}

// Operations with addresses

void CIMSPClient::_TestFastSync(const CAddressBook* adbk)
{
	// Does nothing in this implementation
}

void CIMSPClient::_FastSync(const CAddressBook* adbk, cdstrmap& changed, cdstrset& removed, cdstring& synctoken)
{
	// Does nothing in this implementation
}

// Find all addresses in adbk
void CIMSPClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	_FindAllAddresses(adbk);
}

// Write all addresses in adbk
void CIMSPClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	
}

// Find all addresses in adbk
void CIMSPClient::_FindAllAddresses(CAddressBook* adbk)
{
	// Reset item counter for feedback
	mTempList.clear();
	InitItemCtr();

	try
	{
		// Send SEARCHADDRESS message to server
		INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
		INETSendString(cSEARCHADDRESS);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETFinishSend();

		// Reset item counter for feedback
		InitItemCtr(mTempList.size());

		// Start busy then make silent
		INETStartAction("Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress", adbk->GetName());
		mSilentCommand = true;

		// Get all addresses
		if (mTempList.size())
			BufferedFetchAddress(adbk);

		mTempList.clear();

		// Resolve members groups
		if (mMembersGroups)
		{
			CAddressList* addr_list = adbk->GetAddressList();
			CGroupList* grp_list = adbk->GetGroupList();

			// Loop over all members groups
			for(CGroupList::iterator iter1 = mMembersGroups->begin(); iter1 != mMembersGroups->end(); iter1++)
			{
				// Loop over all entries in the group
				for(cdstrvect::iterator iter2 = (*iter1)->GetAddressList().begin(); iter2 != (*iter1)->GetAddressList().end(); iter2++)
				{
					// Find member in single list
					for(CAddressList::const_iterator iter3 = addr_list->begin(); iter3 != addr_list->end(); iter3++)
					{
						if ((*iter3)->GetName() == *iter2)
						{
							// Replace member alias with full address
							*iter2 = (*iter3)->GetFullAddress();
							break;
						}
					}
				}

				// Push modified group into address book
				grp_list->push_back(*iter1);
			}

			// Groups now owned by address book
			mMembersGroups->clear_without_delete();
		}

		// Delete any members groups
		delete mMembersGroups;
		mMembersGroups = NULL;

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always clear cache
		mTempList.clear();

		// Delete any memebers groups
		delete mMembersGroups;
		mMembersGroups = NULL;

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();

		CLOG_LOGRETHROW;
		throw;
	}
}

// Fetch named address
void CIMSPClient::_FetchAddress(CAddressBook* adbk,
								const cdstrvect& names)
{
	try
	{
		// Cache actionable address book
		mActionAdbk = adbk;

		// Send FETCHADDRESS message to server
		INETStartSend("Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress", adbk->GetName());
		INETSendString(cFETCHADDRESS);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		
		for(cdstrvect::const_iterator iter = names.begin(); iter != names.end(); iter++)
		{
			// Must quote values
			cdstring qname = *iter;
			qname.ConvertFromOS();

			INETSendString(cSpace);
			INETSendString(qname, eQueueProcess);
		}
		INETFinishSend();

		mActionAdbk = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		mActionAdbk = NULL;
		CLOG_LOGRETHROW;
		throw;
	}
}

// Store address
void CIMSPClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Iterate over each one in the list
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		_StoreAddress(adbk, static_cast<const CAdbkAddress*>(*iter));
}

// Store address
void CIMSPClient::_StoreAddress(CAddressBook* adbk, const CAdbkAddress* addr)
{
	INETStartSend("Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress", adbk->GetName());
	INETSendString(cSTOREADDRESS);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);

	// Form address spec for IMSP
	{
		// Name (NB must exist so try nick-name then email address if it does not)
		cdstring temp = addr->GetName();
		if (temp.empty())
		{
			// Try nick-name
			temp = addr->GetADL();

			// Next try email
			if (temp.empty())
				temp = addr->GetMailAddress();
		}
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);

		// Email
		temp = addr->GetMailAddress();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_EMAIL);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Nick-name
		temp = addr->GetADL();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_ALIAS);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Calendar
		temp = addr->GetCalendar();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_CALENDAR);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}
		
		// Company
		temp = addr->GetCompany();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_COMPANY);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Address (filtered)
		temp = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_ADDRESS);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Phone work
		temp = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_PHONE_WORK);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Phone home
		temp = addr->GetPhone(CAdbkAddress::eHomePhoneType);
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_PHONE_HOME);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Fax
		temp = addr->GetPhone(CAdbkAddress::eFaxType);
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_FAX);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// URLs
		temp = addr->GetURL();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_URLS);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Notes
		temp = addr->GetNotes();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_NOTES);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}
	}

	// Send STOREADDRESS message to server
	INETFinishSend();

	// Set addresses's entry to its name during store
	const_cast<CAdbkAddress*>(addr)->SetEntry(addr->GetName());
}

// Store group
void CIMSPClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// Iterate over each one in the list
	for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		_StoreGroup(adbk, *iter);
}

// Store group
void CIMSPClient::_StoreGroup(CAddressBook* adbk, const CGroup* grp)
{
	INETStartSend("Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress", adbk->GetName());
	INETSendString(cSTOREADDRESS);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);

	// Form address spec for IMSP
	{
		// Name (NB must exist)
		cdstring temp = grp->GetName();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);

		// Nick-name
		temp = grp->GetNickName();
		temp.ConvertFromOS();
		if (!temp.empty())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_ALIAS);
			INETSendString(cSpace);
			INETSendString(temp, eQueueProcess);
		}

		// Add group indicator
		INETSendString(cSpace);
		INETSendString(cADDRESS_GROUP);
		INETSendString(cSpace);
		INETSendString(cADDRESS_GROUP);

		// Email
		if (grp->GetAddressList().size())
		{
			INETSendString(cSpace);
			INETSendString(cADDRESS_EMAIL);
			cdstring all;
			bool first = true;
			for(cdstrvect::const_iterator iter = grp->GetAddressList().begin();
				iter != grp->GetAddressList().end(); iter++)
			{
				// Add CRLF if not first
				if (first)
					first = false;
				else
					all += cCRLF;

				// Add email address
				temp = *iter;
				temp.ConvertFromOS();
				all += temp;
			}
			INETSendString(cSpace);
			INETSendString(all, eQueueLiteral);
		}
	}

	// Send STOREADDRESS message to server
	INETFinishSend();

	// Set group's entry to its name during store
	const_cast<CGroup*>(grp)->SetEntry(grp->GetName());
}

// Change address
void CIMSPClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Delete old ones add new ones
	_DeleteAddress(adbk, addrs);
	_StoreAddress(adbk, addrs);
}

// Change group
void CIMSPClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// Delete old ones add new ones
	_DeleteGroup(adbk, grps);
	_StoreGroup(adbk, grps);
}

// Delete address
void CIMSPClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Iterate over each one in the list
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		_DeleteAddress(adbk, static_cast<const CAdbkAddress*>(*iter));
}

// Delete address
void CIMSPClient::_DeleteAddress(CAddressBook* adbk, const CAdbkAddress* addr)
{
	// Must quote values
	cdstring qname = addr->GetEntry();
	qname.ConvertFromOS();

	// Send DELETEADDRESS message to server
	INETStartSend("Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress", adbk->GetName());
	INETSendString(cDELETEADDRESS);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);
	INETSendString(qname, eQueueProcess);
	INETFinishSend();
}

// Delete group
void CIMSPClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// Iterate over each one in the list
	for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		_DeleteGroup(adbk, *iter);
}

// Delete group
void CIMSPClient::_DeleteGroup(CAddressBook* adbk, const CGroup* grp)
{
	// Must quote values
	cdstring qname = grp->GetEntry();
	qname.ConvertFromOS();

	// Send DELETEADDRESS message to server
	INETStartSend("Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress", adbk->GetName());
	INETSendString(cDELETEADDRESS);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);
	INETSendString(qname, eQueueProcess);
	INETFinishSend();
}

// These must be implemented by specific client

// Resolve address nick-name
void CIMSPClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	// Reset item counter for feedback
	mTempList.clear();

	try
	{
		// Must quote value
		cdstring lookup = nick_name;
		lookup.ConvertFromOS();

		// Send SEARCHADDRESS message to server
		INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
		INETSendString(cSEARCHADDRESS);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETSendString(cSpace);
		INETSendString(cADDRESS_ALIAS);
		INETSendString(cSpace);
		INETSendString(lookup, eQueueProcess);
		INETFinishSend();

		// Reset item counter for feedback
		InitItemCtr(mTempList.size());

		// Start busy then make silent
		INETStartAction("Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress", adbk->GetName());
		mSilentCommand = true;

		// Get all addresses
		if (mTempList.size())
			BufferedFetchAddress(adbk);

		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always clear cache
		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();

		CLOG_LOGRETHROW;
		throw;
	}
}

// Resolve group nick-name
void CIMSPClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	// Reset item counter for feedback
	mTempList.clear();

	try
	{
		// Must quote value
		cdstring lookup = nick_name;
		lookup.ConvertFromOS();

		// Send SEARCHADDRESS message to server
		INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
		INETSendString(cSEARCHADDRESS);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETSendString(cSpace);
		INETSendString(cADDRESS_ALIAS);
		INETSendString(cSpace);
		INETSendString(lookup, eQueueProcess);
		INETFinishSend();

		// Reset item counter for feedback
		InitItemCtr(mTempList.size());

		// Start busy then make silent
		INETStartAction("Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress", adbk->GetName());
		mSilentCommand = true;

		// Get all addresses
		if (mTempList.size())
			BufferedFetchAddress(adbk);

		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always clear cache
		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();

		CLOG_LOGRETHROW;
		throw;
	}
}

// Search for addresses
void CIMSPClient::_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	// Reset item counter for feedback
	mTempList.clear();

	try
	{
		// Switch to serach mode and cache list
		mSearchMode = true;
		mSearchResults = &addr_list;

		cdstring lookup(name);
		CAdbkAddress::ExpandMatch(match, lookup);
		lookup.ConvertFromOS();

		// Send SEARCHADDRESS message to server
		INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
		INETSendString(cSEARCHADDRESS);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
		{
			cdstring criteria;
			switch(*iter)
			{
			case CAdbkAddress::eName:
				criteria = cADDRESS_NAME;
				break;
			case CAdbkAddress::eNickName:
				criteria = cADDRESS_ALIAS;
				break;
			case CAdbkAddress::eEmail:
				criteria = cADDRESS_EMAIL;
				break;
			case CAdbkAddress::eCompany:
				criteria = cADDRESS_COMPANY;
				break;
			case CAdbkAddress::eAddress:
				criteria = cADDRESS_ADDRESS;
				break;
			case CAdbkAddress::ePhoneWork:
				criteria = cADDRESS_PHONE_WORK;
				break;
			case CAdbkAddress::ePhoneHome:
				criteria = cADDRESS_PHONE_HOME;
				break;
			case CAdbkAddress::eFax:
				criteria = cADDRESS_FAX;
				break;
			case CAdbkAddress::eURL:
				criteria = cADDRESS_URLS;
				break;
			case CAdbkAddress::eNotes:
				criteria = cADDRESS_NOTES;
				break;
			default:;
			}

			INETSendString(cSpace);
			INETSendString(criteria);
			INETSendString(cSpace);
			INETSendString(lookup, eQueueProcess);
		}
		INETFinishSend();

		// Reset item counter for feedback
		InitItemCtr(mTempList.size());

		// Start busy then make silent
		INETStartAction("Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress", adbk->GetName());
		mSilentCommand = true;

		// Loop over all addresses
		if (mTempList.size())
			BufferedFetchAddress(adbk);

		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();

		// Done with serach mode and list
		mSearchMode = false;
		mSearchResults = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always clear cache
		mTempList.clear();

		// Remove silence and stop busy
		mSilentCommand = false;
		INETStopAction();

		// Done with serach mode and list
		mSearchMode = false;
		mSearchResults = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set acl on server
void CIMSPClient::_SetACL(CAddressBook* adbk, CACL* acl)
{
	try
	{
		// Cache actionable address book
		mActionAdbk = adbk;

		// Send SETACL ADDRESSBOOK message to server
		INETStartSend("Status::IMSP::SetACLAddressBook", "Error::IMSP::OSErrSetACLAddressBook", "Error::IMSP::NoBadSetACLAddressBook", adbk->GetName());
		INETSendString(cSETACLADDRESSBOOK);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETSendString(cSpace);
		INETSendString(acl->GetFullTextRights());
		INETFinishSend();

		// Must reset cached adbk
		mActionAdbk = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset cached adbk
		mActionAdbk = NULL;
		CLOG_LOGRETHROW;
		throw;
	}
}

// Delete acl on server
void CIMSPClient::_DeleteACL(CAddressBook* adbk, CACL* acl)
{
	try
	{
		// Cache actionable address book
		mActionAdbk = adbk;

		// Send DELETEACL ADDRESSBOOK message to server
		INETStartSend("Status::IMSP::DeleteACLAddressBook", "Error::IMSP::OSErrDeleteACLAddressBook", "Error::IMSP::NoBadDeleteACLAddressBook", adbk->GetName());
		INETSendString(cDELETEACLADDRESSBOOK);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETSendString(cSpace);
		INETSendString(acl->GetUID());
		INETFinishSend();

		// Must reset cached adbk
		mActionAdbk = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset cached adbk
		mActionAdbk = NULL;
		CLOG_LOGRETHROW;
		throw;
	}
}

// Get all acls for adbk from server
void CIMSPClient::_GetACL(CAddressBook* adbk)
{
	try
	{
		// Cache actionable address book
		mActionAdbk = adbk;

		// Send GETACL ADDRESSBOOK message to server
		INETStartSend("Status::IMSP::GetACLAddressBook", "Error::IMSP::OSErrGetACLAddressBook", "Error::IMSP::NoBadGetACLAddressBook", adbk->GetName());
		INETSendString(cGETACLADDRESSBOOK);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETFinishSend();

		// Must reset cached adbk
		mActionAdbk = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset cached adbk
		mActionAdbk = NULL;
		CLOG_LOGRETHROW;
		throw;
	}
}

// Get current user's rights to adbk
void CIMSPClient::_MyRights(CAddressBook* adbk)
{
	try
	{
		// Cache actionable adbk
		mActionAdbk = adbk;

		// Send MYRIGHTS ADDRESSBOOK message to server
		INETStartSend("Status::IMSP::MyRightsAddressBook", "Error::IMSP::OSErrMyRightsAddressBook", "Error::IMSP::NoBadMyRightsAddressBook", adbk->GetName());
		INETSendString(cMYRIGHTSADDRESSBOOK);
		INETSendString(cSpace);
		INETSendString(adbk->GetName(), eQueueProcess);
		INETFinishSend();

		// Must reset cached adbk
		mActionAdbk = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must reset cached adbk
		mActionAdbk = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
}

#pragma mark ____________________________Others

cdstring CIMSPClient::GetKeyPrefix(const cdstring& subdataset)
{
	cdstring temp = mVendor + "." + mProduct  + ".Options.";
	if (!subdataset.empty())
		temp += subdataset + ".";
	::strupper(temp);
	::strreplacespace(temp, '_');

	mPreKeySize = temp.length();

	return temp;
}

#pragma mark ____________________________Handle Errors

// Force reconnect
void CIMSPClient::INETRecoverReconnect()
{
	// Force reset of address books only
	if (!mOptions)
		CMailControl::AdbkServerReconnect(GetAdbkOwner());
}

// Force disconnect
void CIMSPClient::INETRecoverDisconnect()
{
	// Force reset of address books only
	if (!mOptions)
		CMailControl::AdbkServerDisconnect(GetAdbkOwner());
}

// Descriptor for object error context
const char*	CIMSPClient::INETGetErrorDescriptor() const
{
	return "Address Book: ";
}

#pragma mark ____________________________Parsing

// Parse text sent by server (advance pointer to next bit to be parsed)
void CIMSPClient::IMSPParseResponse(char** txt, CINETClientResponse* response)
{
	// Found an option
	if (::stradvtokcmp(txt,cOPTION)==0)
	{
		response->code = cStarOPTION;
		IMSPParseOption(txt);
	}

	// Address books
	else if (::stradvtokcmp(txt,cADDRESSBOOK)==0)
	{
		response->code = cStarADDRESSBOOK;
		IMSPParseAddressBook(txt);
	}
	else if (::stradvtokcmp(txt,cSEARCHADDRESS)==0)
	{
		response->code = cStarSEARCHADDRESS;
		IMSPParseSearchAddress(txt);
	}
	else if (::stradvtokcmp(txt,cFETCHADDRESS)==0)
	{
		response->code = cStarFETCHADDRESS;
		IMSPParseFetchAddress(txt);
	}

	// Address book ACLs
	else if (::stradvtokcmp(txt,cACLADDRESSBOOK)==0)
	{
		response->code = cStarACLADDRESSBOOK;
		IMSPParseACLAddressBook(txt);
	}
	else if (::stradvtokcmp(txt,cMYRIGHTSADDRESSBOOK)==0)
	{
		response->code = cStarMYRIGHTSADDRESSBOOK;
		IMSPParseMyRightsAddressBook(txt);
	}

	else
		response->code = cResponseError;

} // CINETClient::IMSPParseResponse

#pragma mark ____________________________Option Responses

// Parse ACL list
void CIMSPClient::IMSPParseOption(char** txt)
{
	// Get key
	char* prekey = INETParseString(txt);
	if (!prekey)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Get value
	char* value = INETParseString(txt);

	// Get read/write if present (ignore for now)
	if (**txt == ' ')
	{
		char* rw = INETParseString(txt);
		delete rw;
	}

	// Check for single/multiple options
	if (mSingle)
	{
		// Must match key
		if (*mSingleKey == prekey)
			GetOptionsOwner()->SetValue(value);
	}
	else
	{
		// Bump past pre-key
		char* key = prekey + mPreKeySize;

		// Insert
		std::pair<cdstrmap::iterator, bool> result = GetOptionsOwner()->GetMap()->insert(cdstrmap::value_type(key, value));

		// Does it exist already
		if (!result.second)
			// Replace existing
			(*result.first).second = value;
	}

	delete prekey;
	delete value;

} // CIMSPClient::IMSPParseOption

#pragma mark ____________________________Address Book Responses

// Parse IMSP ADDRESSBOOK reply
void CIMSPClient::IMSPParseAddressBook(char** txt)
{
	char* p;
	bool noinferiors = false;
	bool noselect = false;

	// Look for bracket
	p = ::strmatchbra(txt);
	if (!p)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	while(p && *p)
	{

		// Look for all posibilities
		if (CheckStrAdv(&p, cMBOXFLAGNOINFERIORS))
			noinferiors = true;

		else if (CheckStrAdv(&p, cMBOXFLAGNOSELECT))
			noselect = true;

		else
		{
			// Unknown flag - ignore
			while(*p == ' ') p++;
			p = ::strpbrk(p, SPACE);
		}
	}

	// Now get directory delim - remove special quote
	char* delim = ::strgetquotestr(txt);

	if (!delim || ((::strlen(delim) > 1) && (::strcmp(delim, cNIL))))
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Now get name as adbk
	char* adbk_name = INETParseString(txt);
	if (!adbk_name)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Add adress book to list
	CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), !noselect, !noinferiors, adbk_name);
	GetAdbkOwner()->GetStoreRoot()->AddChildHierarchy(adbk);
	delete adbk_name;


}

// Parse IMSP SEARCHADDRESS reply
void CIMSPClient::IMSPParseSearchAddress(char** txt)
{
	// Display status and bump count
	BumpItemCtr("Status::IMSP::SearchAddressCount");

	// Just parse string and add to list
	mTempList.push_back(cdstring::null_str);
	mTempList.back().steal(INETParseString(txt));
	mTempList.back().ConvertToOS();
}

// Parse IMSP FETCHADDRESS reply
void CIMSPClient::IMSPParseFetchAddress(char** txt)
{
	// Do counter bits here as IMSPParseFetchAddress also gets called for from various locations
	BumpItemCtr(mItemTotal ? "Status::IMSP::FetchAddressCount2" : "Status::IMSP::FetchAddressCount1");

	// Get adbk name (not used)
	char* adbk = INETParseString(txt);
	delete adbk;

	// Entries for address
	cdstring temp;
	temp.steal(INETParseString(txt));
	temp.ConvertToOS();
	char* name = temp.grab_c_str();;
	char* alias = NULL;
	char* email = NULL;
	char* members = NULL;
	char* calendar = NULL;
	char* company = NULL;
	char* address = NULL;
	char* city = NULL;
	char* phone_work = NULL;
	char* phone_home = NULL;
	char* fax = NULL;
	char* urls = NULL;
	char* notes = NULL;
	bool group = false;

	// Now get pairs of items
	char* p = ::strgetquotestr(txt);
	while(p && *p)
	{
		// Parse p first
		enum EIMSPAddrCode
		{
			eCodeUnknown,
			eCodeAlias = 1,
			eCodeEmail,
			eCodeMembers,
			eCodeCalendar,
			eCodeCompany,
			eCodeAddress,
			eCodeCity,
			eCodePhone,
			eCodePhoneWork,
			eCodePhoneHome,
			eCodeFax,
			eCodeHome,
			eCodeURLs,
			eCodeNotes,
			eCodeGroup
		};
		EIMSPAddrCode code = eCodeUnknown;
		if (::strcmpnocase(p, cADDRESS_ALIAS) == 0)
			code = eCodeAlias;
		else if (::strcmpnocase(p, cADDRESS_EMAIL) == 0)
			code = eCodeEmail;
		else if (::strcmpnocase(p, cADDRESS_MEMBER) == 0)
			code = eCodeMembers;
		else if (::strcmpnocase(p, cADDRESS_MEMBERS) == 0)
			code = eCodeMembers;
		else if (::strcmpnocase(p, cADDRESS_CALENDAR) == 0)
			code = eCodeCalendar;
		else if (::strcmpnocase(p, cADDRESS_COMPANY) == 0)
			code = eCodeCompany;
		else if (::strcmpnocase(p, cADDRESS_ADDRESS) == 0)
			code = eCodeAddress;
		else if (::strcmpnocase(p, cADDRESS_CITY) == 0)
			code = eCodeCity;
		else if ((::strcmpnocase(p, cADDRESS_PHONE) == 0) && !phone_work)
			code = eCodePhone;
		else if (::strcmpnocase(p, cADDRESS_PHONE_WORK) == 0)
			code = eCodePhoneWork;
		else if (::strcmpnocase(p, cADDRESS_PHONE_HOME) == 0)
			code = eCodePhoneHome;
		else if (::strcmpnocase(p, cADDRESS_FAX) == 0)
			code = eCodeFax;
		else if ((::strcmpnocase(p, cADDRESS_HOME) == 0) && !urls)
			code = eCodeHome;
		else if (::strcmpnocase(p, cADDRESS_URLS) == 0)
			code = eCodeURLs;
		else if (::strcmpnocase(p, cADDRESS_NOTES) == 0)
			code = eCodeNotes;
		else if (::strcmpnocase(p, cADDRESS_GROUP) == 0)
			code = eCodeGroup;
		else
			code = eCodeUnknown;

		temp.steal(INETParseString(txt));
		temp.ConvertToOS();
		char* value = temp.grab_c_str();;

		// Parse item
		if (code == eCodeAlias)
			alias = value;
		else if (code == eCodeEmail)
			email = value;
		else if (code == eCodeMembers)
			members = value;
		else if (code == eCodeCalendar)
			calendar = value;
		else if (code == eCodeCompany)
			company = value;
		else if (code == eCodeAddress)
			address = value;
		else if (code == eCodeCity)
			city = value;
		else if ((code == eCodePhone) && !phone_work)
			phone_work = value;
		else if (code == eCodePhoneWork)
		{
			delete phone_work;		// Hack for Simeon
			phone_work = value;
		}
		else if (code == eCodePhoneHome)
			phone_home = value;
		else if (code == eCodeFax)
			fax = value;
		else if ((code == eCodeHome) && !urls)
			urls = value;
		else if (code == eCodeURLs)
		{
			delete urls;		// Hack for Simeon
			urls = value;
		}
		else if (code == eCodeNotes)
			notes = value;
		else if (code == eCodeGroup)
		{
			delete value;
			group = true;
		}
		else
			delete value;

		// Get next item
		p = ::strgetquotestr(txt);
	}

	// Determine if Simeon group - but override with Mulberry group
	if (members && !group)
	{
		// Got group
		CGroup* grp = new CGroup(name, alias);
		grp->SetEntry(name);

		// Parse list of members
		p = ::strtok(members, "\r\n");
		while(p)
		{
			grp->AddAddress(p);
			p = ::strtok(NULL, "\r\n");
		}

		// Add group to members if not searching
		if (!mSearchMode)
		{
			if (!mMembersGroups)
				mMembersGroups = new CGroupList;
			mMembersGroups->push_back(grp);
		}
	}
	// Determine if group or single
	else if (group || (email && (::strchr(email, '\r') != 0)))
	{
		// Got group
		CGroup* grp = new CGroup(name, alias);
		grp->SetEntry(name);

		// Parse list of emails
		if (email)
		{
			p = ::strtok(email, "\r\n");
			while(p)
			{
				grp->AddAddress(p);
				p = ::strtok(NULL, "\r\n");
			}
		}

		// Add group to address book if not searching
		if (!mSearchMode && mActionAdbk)
			mActionAdbk->GetGroupList()->push_back(grp);
	}
	else
	{
		// Combine address and city (Simeon hack)
		cdstring temp_addr = address;
		if (city)
		{
			temp_addr += os_endl;
			temp_addr += city;
		}

		// Create address and add to address book or search list
		CAdbkAddress* addr = new CAdbkAddress(name, email, name, alias, calendar, company, temp_addr, phone_work, phone_home, fax, urls, notes);
		if (mSearchMode)
			mSearchResults->push_back(addr);
		else if (mActionAdbk)
		{
			mActionAdbk->GetAddressList()->push_back(addr);
			if (mActionAdbk->GetVCardAdbk() != NULL)
				mActionAdbk->GetVCardAdbk()->AddCard(vcardstore::GenerateVCard(mActionAdbk->GetVCardAdbk()->GetRef(), addr));
		}
	}

	// delete all strings
	delete name;
	delete alias;
	delete email;
	delete members;
	delete company;
	delete address;
	delete city;
	delete phone_work;
	delete phone_home;
	delete fax;
	delete urls;
	delete notes;
}

// Parse IMSP ACL ADDRESSBOOK reply
void CIMSPClient::IMSPParseACLAddressBook(char** txt)
{
	// Get adbk name (not used)
	char* adbk = INETParseString(txt);
	delete adbk;

	// Add each uid/rights pair to ACL list in mailbox
	char* uid = NULL;
	while(*txt && ((uid = INETParseString(txt)) != NULL))
	{
		// Get rights
		char* rights = INETParseString(txt);

		// Create ACL
		CAdbkACL acl;
		acl.SetUID(uid);
		acl.SetRights(rights);

		// Give it to mailbox
		if (mActionAdbk)
			mActionAdbk->AddACL(&acl);

		delete uid;
		delete rights;
	}
}

// Parse IMSP MYRIGHTS ADDRESSBOOK reply
void CIMSPClient::IMSPParseMyRightsAddressBook(char** txt)
{
	// Get adbk name (not used)
	char* adbk = INETParseString(txt);
	delete adbk;

	// Get rights
	char* rights = INETParseString(txt);

	// Create temp ACL
	CAdbkACL temp;
	temp.SetRights(rights);
	delete rights;

	// Copy parsed rights into adbk
	if (mActionAdbk)
		mActionAdbk->SetMyRights(temp.GetRights());
}

// Do FETCHADDRESS using finite server buffer input
void CIMSPClient::BufferedFetchAddress(CAddressBook* adbk)
{
	// Need to work around 4096 char buffer limit on commands in IMSP server
	const unsigned long cTagCommandSize = 20;
	const unsigned long cNameExtras = 3;
	const unsigned long cBufferLimit = 3840;
	unsigned long count = mTempList.size();
	unsigned long index = 0;
	cdstrvect accumulate;
	const unsigned long initial_size = cTagCommandSize + adbk->GetName().length();
	unsigned long size = initial_size;
	while(index != count)
	{
		// Check what the new size would be
		unsigned long add_length = mTempList.at(index).length() + cNameExtras;
		size += add_length;
		if (size > cBufferLimit)
		{
			// Size would be greater than buffer so output what we have so far
			_FetchAddress(adbk, accumulate);
			
			// Readjust size and accumulator to start over
			size = initial_size + add_length;
			accumulate.clear();
		}
		
		// Add current item to accumulator
		accumulate.push_back(mTempList.at(index++));
	}
	
	// Output any that remain
	if (accumulate.size())
		_FetchAddress(adbk, accumulate);
}
