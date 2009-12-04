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

#include "CACAPClient.h"

#include "CAdbkACL.h"
#include "CAddressBook.h"
#include "CCharSpecials.h"
#include "CGeneralException.h"
#include "CINETClientResponses.h"
#include "CINETCommon.h"
#include "CMailControl.h"
#include "COptionsMap.h"
#include "CRFC822.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include <UMemoryMgr.h>
#else
#include "StValueChanger.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <strstream>

#include "CACAPCommon.cp"

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CACAPClient::CACAPClient(COptionsProtocol* options_owner, CAdbkProtocol* adbk_owner)
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
	InitACAPClient();

}

// Copy constructor
CACAPClient::CACAPClient(const CACAPClient& copy, COptionsProtocol* options_owner, CAdbkProtocol* adbk_owner)
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
	InitACAPClient();

}

CACAPClient::~CACAPClient()
{
}

void CACAPClient::InitACAPClient()
{
	// Init instance variables
	mVersion = eACAP;
	mImplementation = eGeneric;
	mParserState = eParseIgnore;

	// Always has async literals
	mAsyncLiteral = true;

	// Cannot use 'LOGIN'
	mLoginAllowed = false;

	// Allows initial client data in SASL and uses binary SASL data
	mAuthInitialClientData = true;
	mAuthBase64 = false;

	// Has tagged responses
	mDoesTaggedResponses = true;

	mLogType = CLog::eLogACAP;

	mPreKeySize = 0;

} // CACAPClient::CACAPClient

// Create duplicate, empty connection
CINETClient* CACAPClient::CloneConnection()
{
	// Copy construct this
	return new CACAPClient(*this);

} // CACAPClient::CloneConnection

// Get default port
tcp_port CACAPClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return cACAPServerPort_SSL;
	else
		return cACAPServerPort;
}

#pragma mark ____________________________Protocol

// Process for output force literal rather than quote
int CACAPClient::ProcessString(cdstring& str)
{
	bool quote = false;
	bool literal = false;

	// Look for special cases
	const char* p = str.c_str();
	unsigned long len = 0;
	while(*p && !literal)
	{
		switch(cINETChar[(unsigned char) *p++])
		{
		case 0: // Atom		// ACAP always quotes
		case 1: // Quote
			quote = true;
			break;
		case 2: // Escaped
			quote = true;
			len++;
			break;
		case 3: // Literal
			literal = true;
			break;
		}
		len++;

		// Strings longer than 1024 must be literals
		if (len > 1024)
			literal = true;
	}

	// Process literal above all else
	if (literal)

		// Just return literal size - will be sent as is
		return str.length();

	// Quote forcibly (use INET style escape chars)
	if (quote || str.empty())
		str.quote(true, true);

	return 0;
}

// Process greeting response
bool CACAPClient::_ProcessGreeting()
{
	// Receive first info from server
	mStream->qgetline(mLineData, cINETBufferLen);

	// Store in greeting
	mGreeting = mLineData;

	// Write to log file
	mLog.LogEntry(mLineData);

	return INETCompareResponse((EINETResponseCode) cStarACAP);
}

// Check version of server
void CACAPClient::_Capability(bool after_tls)
{
	// Clear out existng capabilities before issuing command again
	_InitCapability();

	// Always get another greeting after STARTTLS
	if (after_tls)
	{
		if (!_ProcessGreeting())
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
			throw CINETException(CINETException::err_BadResponse);
		}
	}

	// Store capability string - the same as the greeting
	mCapability = mGreeting;

	_ProcessCapability();

} // CACAPClient::_Capability

// Initialise capability flags to empty set
void CACAPClient::_InitCapability()
{
	// Init capabilities to empty set
	mImplementation = eGeneric;
	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CACAPClient::_ProcessCapability()
{
	// Look for SASL mechanisms
	cdstring temp(mGreeting);
	
	char* p = temp.c_str_mod();
	
	// Must have a '*'
	if ((*p++ != '*') &&
		(*p++ != ' '))
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Must have ACAP token
	if (::stradvtokcmp(&p, cRESP_ACAP)!=0)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Look for start of init-capability
	cdstring token;
	while(*p++ == '(')
	{
		token.steal(::strdupquotestr(&p));
		::strupper(token.c_str_mod());

		// Look for IMPLEMENTATION
		if (token == cACAP_RESPONSE_IMPLEMENTATION)
		{
			// Get name of implementation
			cdstring impl;
			impl.steal(INETParseString(&p));
			
			// Look for Communigate
			if (::strstrnocase(impl, "CommuniGate"))
				mImplementation = eCommunigate;
		}
		
		// Look for SASL
		else if (token == cACAP_RESPONSE_SASL)
		{
			cdstring mech;
			while(*p && (*p != ')'))
			{
				mech.steal(INETParseString(&p));
				::strupper(mech.c_str_mod());
				if (mech == cLOGIN)
					mAuthLoginAllowed = true;
				else if (mech == cPLAIN)
					mAuthPlainAllowed = true;
				else if (mech == cANONYMOUS)
					mAuthAnonAllowed = true;
			}
		}

		// Look for STARTTLS
		else if (token == cACAP_RESPONSE_STARTTLS)
		{
			// Empty data
			mSTARTTLSAllowed = true;
		}
		
		// Uninteresting capability
		else
		{
			// Parse strings until ')'
			while(*p && (*p != ')'))
				delete INETParseString(&p);
		}

		// Look for end of init-capability
		if (*p++ != ')')
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}
		
		// Punt white space
		while(*p == ' ') p++;
	}

	// ACAP always has ACLs
	GetAdbkOwner()->SetHasACL(true);

} // CACAPClient::_ProcessCapability

// Handle failed capability response
void CACAPClient::_NoCapability()
{
	// No capabilities

} // CACAPClient::_NoCapability

// Handle failed capability response
void CACAPClient::_PreProcess()
{
	// No special preprocess

} // CACAPClient::_PreProcess

// Handle failed capability response
void CACAPClient::_PostProcess()
{
	// No special postprocess

} // CACAPClient::_PostProcess

// Handle failed capability response
void CACAPClient::_ParseResponse(char** txt,
							CINETClientResponse* response)
{
	ACAPParseResponse(txt, response);
} // CACAPClient::_ParseResponse

#pragma mark ____________________________Options

// Find all options below this key
void CACAPClient::_FindAllAttributes(const cdstring& entry)
{
	StParserState parser(this, eParseFindAllAttributes);

	cdstring dataset = GetOptionDataset();
	cdstring option_attribute = cOPTIONSDATASET_ATTR;
	option_attribute += cWILDCARD;

	// Send SEARCH message to server
	// SEARCH "/option/~/vendor.<vendor>/<app>/" return ("option.*") EQUAL "entry" "i;octet" "<entry>"
	INETStartSend("Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_RETURN);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(option_attribute, eQueueProcess);
	INETSendString(")");
	INETSendString(cSpace);
	INETSendString(cASEARCH_ENTRY);
	INETSendString(cSpace);
	INETSendString(entry, eQueueProcess);
	INETFinishSend();
}

// Set all options in owmer map
void CACAPClient::_SetAllAttributes(const cdstring& entry)
{
	StParserState parser(this, eParseSetAllAttributes);

	cdstring dataset = GetOptionDatasetEntry(entry);

	// Send STORE message to server
	// STORE ("/option/~/vendor.<vendor>/<app>/<entry>" ["option.<key>" "<value>"]+)
	INETStartSend("Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);

	for(cdstrmap::const_iterator iter = GetOptionsOwner()->GetMap()->begin(); iter != GetOptionsOwner()->GetMap()->end(); iter++)
	{
		cdstring option_attribute = cOPTIONSDATASET_ATTR;
		option_attribute += (*iter).first;

		INETSendString(cSpace);
		INETSendString(option_attribute, eQueueProcess);
		INETSendString(cSpace);
		INETSendString((*iter).second, eQueueProcess);
	}

	INETSendString(")");
	INETFinishSend();
}

// Unset all options in owmer map
void CACAPClient::_DeleteEntry(const cdstring& entry)
{
	StParserState parser(this, eParseDeleteEntry);

	cdstring dataset = GetOptionDatasetEntry(entry);

	// Send STORE message to server
	// STORE ("/option/~/vendor.<vendor>/<app>/<entry>" "entry" NIL)
	INETStartSend("Status::IMSP::Unsetting", "Error::IMSP::OSErrUnset", "Error::IMSP::NoBadUnset");
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cATTR_ENTRY, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cNIL);
	INETSendString(")");
	INETFinishSend();
}

// Get option
void CACAPClient::_GetAttribute(const cdstring& entry, const cdstring& attribute)
{
	StParserState parser(this, eParseGetAttribute);

	cdstring dataset = GetOptionDataset();

	cdstring option_attribute = cOPTIONSDATASET_ATTR;
	if (!attribute.empty())
		option_attribute += attribute;
	else
		option_attribute += cATTRMETA_VALUE;

	// Send SEARCH message to server
	// SEARCH "/option/~/vendor.<vendor>/<app>/" return ("option.<key>") EQUAL "entry" "i;octet" "<entry>"
	INETStartSend("Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_RETURN);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(option_attribute, eQueueProcess);
	INETSendString(")");
	INETSendString(cSpace);
	INETSendString(cASEARCH_ENTRY);
	INETSendString(cSpace);
	INETSendString(entry, eQueueProcess);
	INETFinishSend();
}

// Set option
void CACAPClient::_SetAttribute(const cdstring& entry, const cdstring& attribute, const cdstring& value)
{
	StParserState parser(this, eParseSetAttribute);

	cdstring dataset = GetOptionDatasetEntry(entry);
	cdstring option_attribute = cOPTIONSDATASET_ATTR;
	if (!attribute.empty())
		option_attribute += attribute;
	else
		option_attribute += cATTRMETA_VALUE;

	// Send STORE message to server
	// STORE ("/option/~/vendor.<vendor>/<app>/<entry>" "option.<key>" "<value>")
	INETStartSend("Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(option_attribute, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(value, eQueueProcess);
	INETSendString(")");
	INETFinishSend();
}

// Unset option with this key
void CACAPClient::_DeleteAttribute(const cdstring& entry, const cdstring& attribute)
{
	StParserState parser(this, eParseDeleteAttribute);

	cdstring dataset = GetOptionDatasetEntry(entry);
	cdstring option_attribute = cOPTIONSDATASET_ATTR;
	if (!attribute.empty())
		option_attribute += attribute;
	else
		option_attribute += cATTRMETA_VALUE;

	// Send STORE message to server
	// STORE ("/option/~/vendor.<vendor>/<app>/<entry>" "option.<key>" NIL)
	INETStartSend("Status::IMSP::Unsetting", "Error::IMSP::OSErrUnset", "Error::IMSP::NoBadUnset");
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(option_attribute, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cNIL);
	INETSendString(")");
	INETFinishSend();
}

#pragma mark ____________________________Address Books

// Operations on address books

// Find all adbks below this path
void CACAPClient::_ListAddressBooks(CAddressBook* root)
{
	StParserState parser(this, eParseFindAllAdbks);

	// Send SEARCH message to server
	// SEARCH "/addressbook/user/" DEPTH 0 RETURN ("addressbook.CommonName") NOT EQUAL "subdataset" NIL
	INETStartSend("Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	
	// Special for CommuniGate
	if (mImplementation == eCommunigate)
	{
		// Use "~/" instead of "user/"
		cdstring default_name = cDATASET_ADBK;
		default_name += "~/";
		INETSendString(default_name, eQueueProcess);

		// Most forcibly add default address book
		cdstring adbk_name = "user/";
		adbk_name += GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();
		CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), false, true, adbk_name);
		GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);

	}
	else
		INETSendString(cDATASET_ADBKUSER, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_ADBK);
	INETFinishSend();
}

// Find all adbks below this path
void CACAPClient::_FindAllAdbks(const cdstring& path)
{
	StParserState parser(this, eParseFindAllAdbks);

	// Send SEARCH message to server
	// SEARCH "/addressbook/user/" DEPTH 0 RETURN ("addressbook.CommonName") NOT EQUAL "subdataset" NIL
	INETStartSend("Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	
	// Special for CommuniGate
	if (mImplementation == eCommunigate)
	{
		// Use "~/" instead of "user/"
		cdstring default_name = cDATASET_ADBK;
		default_name += "~/";
		INETSendString(default_name, eQueueProcess);

		// Most forcibly add default address book
		cdstring adbk_name = "user/";
		adbk_name += GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();
		CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), false, true, adbk_name);
		GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);

	}
	else
		INETSendString(cDATASET_ADBKUSER, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_ADBK);
	INETFinishSend();
}

// Create adbk
void CACAPClient::_CreateAdbk(const CAddressBook* adbk)
{
	StParserState parser(this, eParseCreateAdbk);

	cdstring dataset = GetAdbkDatasetEntry(adbk->GetName());
	cdstring attribute = cADBKDATASET_ATTR;
	attribute += cADBK_NAME;

	// Send STORE message to server
	// STORE ("/addressbook/<name>" "addressbook.CommonName" "<name>" "subdataset" ".")
	INETStartSend("Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook", adbk->GetName());
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(attribute, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(adbk->GetName(), eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cSUBDATASET_ADD);
	INETSendString(")");
	INETFinishSend();
}

bool CACAPClient::_AdbkChanged(const CAddressBook* adbk)
{
	// No way to tell if there have been changes on the server, so return true to force entire sync
	return true;
}

void CACAPClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// No way to tell if there have been changes on the server, so return true to force entire sync
}

// Delete adbk
void CACAPClient::_DeleteAdbk(const CAddressBook* adbk)
{
	StParserState parser(this, eParseDeleteAdbk);

	cdstring dataset = GetAdbkDatasetEntry(adbk->GetName());

	// Send STORE message to server
	// STORE ("/addressbook/<name>" "entry" NIL)
	INETStartSend("Status::IMSP::DeleteAddressBook", "Error::IMSP::OSErrDeleteAddressBook", "Error::IMSP::NoBadDeleteAddressBook", adbk->GetName());
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	INETSendString("(");
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cATTR_ENTRY, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cNIL);
	INETSendString(")");
	INETFinishSend();
}

// Rename adbk
void CACAPClient::_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk)
{
	StParserState parser(this, eParseRenameAdbk);
}

void CACAPClient::_SizeAdbk(CAddressBook* adbk)
{
	// Does nothing
}

// Operations with addresses

// Find all addresses in adbk
void CACAPClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	_FindAllAddresses(adbk);
}

// Write all addresses in adbk
void CACAPClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	
}

// Find all addresses in adbk
void CACAPClient::_FindAllAddresses(CAddressBook* adbk)
{
	// Reset item counter for feedback
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;

	StParserState parser(this, eParseFindAllAddresses);

	cdstring dataset = GetAdbkDataset(adbk->GetName());

	// Send SEARCH message to server
	// SEARCH "/addressbook/<name>/" RETURN ("addressbook.*") EQUAL "subdataset" NIL
	INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_ADDRS);
	INETFinishSend();
}

// Fetch named address
void CACAPClient::_FetchAddress(CAddressBook* adbk,
								const cdstrvect& names)
{
	StParserState parser(this, eParseFetchAddress);

}

// Store address
void CACAPClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StParserState parser(this, eParseStoreAddress);

	// Send STORE message to server
	// STORE ("/addressbook/<name>/<entry>" "addressbook.CommonName" <real_name>) (...)
	INETStartSend("Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress", adbk->GetName());
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);

	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);

		// Must have valid entry
		ValidAddressEntry(adbk, addr);
		cdstring dataset = GetAdbkDatasetEntry(adbk->GetName(), addr);

		INETSendString("(");
		INETSendString(dataset, eQueueProcess);
		INETSendString(cSpace);

		cdstring temp = cADBKDATASET_ATTR;
		temp += cADBK_NAME;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetName();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		temp = cADBKDATASET_ATTR;
		temp += cADBK_NICKNAME;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetADL();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		temp = cADBKDATASET_ATTR;
		temp += cADBK_EMAIL;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetMailAddress();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		temp = cADBKDATASET_ATTR;
		temp += cADBK_ADDRESS;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		temp = cADBKDATASET_ATTR;
		temp += cADBK_COMPANY;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetCompany();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		// Handle phone items
		cdstrpairvect items;

		if (!addr->GetPhone(CAdbkAddress::eWorkPhoneType).empty())
			items.push_back(cdstrpairvect::value_type(addr->GetPhone(CAdbkAddress::eWorkPhoneType), cADBK_PHONE_WORK));

		if (!addr->GetPhone(CAdbkAddress::eHomePhoneType).empty())
			items.push_back(cdstrpairvect::value_type(addr->GetPhone(CAdbkAddress::eHomePhoneType), cADBK_PHONE_HOME));

		if (!addr->GetPhone(CAdbkAddress::eFaxType).empty())
			items.push_back(cdstrpairvect::value_type(addr->GetPhone(CAdbkAddress::eFaxType), cADBK_PHONE_FAX));


		temp = cADBKDATASET_ATTR;
		temp += cADBK_PHONE;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		if (items.size())
		{
			temp = items.front().first;
			temp.ConvertFromOS();
			std::ostrstream buffer;
			size_t buflen = 0;
			buffer.write(reinterpret_cast<const char*>(&buflen), sizeof(size_t));
			buffer << temp;
			buffer << '\0';
			buffer << items.front().second;
			buflen = buffer.pcount();
			cdstring p;
			p.steal(buffer.str());
			*(unsigned long*)(p.c_str()) = buflen - sizeof(size_t);
			INETSendString(p, eQueueBuffer);
			INETSendString(cSpace);
			items.erase(items.begin());
		}
		else
		{
			INETSendString(cdstring::null_str, eQueueProcess);
			INETSendString(cSpace);
		}

		if (items.size() < 2)
		{
			temp = cADBKDATASET_ATTR;
			temp += cADBK_PHONEOTHER;
			INETSendString(temp, eQueueProcess);
			INETSendString(cSpace);
			if (items.size())
			{
				temp = items.front().first;
				temp.ConvertFromOS();
				std::ostrstream buffer;
				size_t buflen = 0;
				buffer.write(reinterpret_cast<const char*>(&buflen), sizeof(size_t));
				buffer << temp;
				buffer << '\0';
				buffer << items.front().second;
				buflen = buffer.pcount();
				cdstring p;
				p.steal(buffer.str());
				*(unsigned long*)(p.c_str()) = buflen - sizeof(size_t);
				INETSendString(p, eQueueBuffer);
				INETSendString(cSpace);
			}
			else
			{
				INETSendString(cdstring::null_str, eQueueProcess);
				INETSendString(cSpace);
			}
		}
		else if (items.size() == 2)
		{
			temp = cADBKDATASET_ATTR;
			temp += cADBK_PHONEOTHER;
			INETSendString(temp, eQueueProcess);
			INETSendString(cSpace);
			INETSendString("(");
			INETSendString(cATTRMETA_VALUE, eQueueProcess);
			INETSendString(cSpace);
			INETSendString("(");

			temp = items.front().first;
			temp.ConvertFromOS();
			std::ostrstream buffer1;
			size_t buflen1 = 0;
			buffer1.write(reinterpret_cast<const char*>(&buflen1), sizeof(size_t));
			buffer1 << temp;
			buffer1 << '\0';
			buffer1 << items.front().second;
			buflen1 = buffer1.pcount();
			cdstring p;
			p.steal(buffer1.str());
			*(unsigned long*)(p.c_str()) = buflen1 - sizeof(size_t);
			INETSendString(p, eQueueBuffer);
			INETSendString(cSpace);
			items.erase(items.begin());

			temp = items.front().first;
			temp.ConvertFromOS();
			std::ostrstream buffer2;
			size_t buflen2 = 0;
			buffer2.write(reinterpret_cast<const char*>(&buflen2), sizeof(size_t));
			buffer2 << temp;
			buffer2 << '\0';
			buffer2 << items.front().second;
			buflen2 = buffer2.pcount();
			p.steal(buffer2.str());
			*(unsigned long*)(p.c_str()) = buflen2 - sizeof(size_t);
			INETSendString(p, eQueueBuffer);
			INETSendString("))");
			INETSendString(cSpace);
		}

		temp = cADBKDATASET_ATTR;
		temp += cADBK_URLS;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetURL();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);

		temp = cADBKDATASET_ATTR;
		temp += cADBK_NOTES;
		INETSendString(temp, eQueueProcess);
		INETSendString(cSpace);
		temp = addr->GetNotes();
		temp.ConvertFromOS();
		INETSendString(temp, eQueueProcess);

		INETSendString(")");
	}

	INETFinishSend();
}

// Store group
void CACAPClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StParserState parser(this, eParseStoreGroup);

}

// Change address
void CACAPClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	// Just write over existing items
	_StoreAddress(adbk, addrs);
}

// Change group
void CACAPClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	// Just write over existing items
	_StoreGroup(adbk, grps);
}

// Delete address
void CACAPClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StParserState parser(this, eParseDeleteAddress);

	// Send STORE message to server
	// STORE ("/addressbook/<name>/<entry>" "entry" NIL) (...)
	INETStartSend("Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress", adbk->GetName());
	INETSendString(cCMD_STORE);
	INETSendString(cSpace);
	
	for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);

		// Must have valid entry
		ValidAddressEntry(adbk, addr);
		cdstring dataset = GetAdbkDatasetEntry(adbk->GetName(), addr);

		if (iter != addrs->begin())
			INETSendString(cSpace);
			
		INETSendString("(");
		INETSendString(dataset, eQueueProcess);
		INETSendString(cSpace);
		INETSendString(cATTR_ENTRY, eQueueProcess);
		INETSendString(cSpace);
		INETSendString(cNIL);
		INETSendString(")");
	}

	INETFinishSend();
}

// Delete group
void CACAPClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StParserState parser(this, eParseDeleteGroup);

}

// These must be implemented by specific client

// Resolve address nick-name
void CACAPClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	StParserState parser(this, eParseResolveAddress);

	// Reset item counter for feedback
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;

	cdstring dataset = GetAdbkDataset(adbk->GetName());
	cdstring criteria = cADBKDATASET_ATTR;
	criteria += cADBK_NICKNAME;
	cdstring lookup = nick_name;
	lookup.ConvertFromOS();

	// Send SEARCH message to server
	// SEARCH "/addressbook/<name>/" RETURN ("addressbook.*") AND SUBSTRING "addressbook.Alias" <nick-name> EQUAL "subdataset" NIL
	INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_ALIAS);
	INETSendString(cSpace);
	INETSendString(cASEARCH_AND);
	INETSendString(cSpace);
	INETSendString(cASEARCH_SUBSTRING);
	INETSendString(cSpace);
	INETSendString(criteria, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_INOCASE);
	INETSendString(cSpace);
	INETSendString(lookup, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_NOTADBK);
	INETFinishSend();
}

// Resolve group nick-name
void CACAPClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	StParserState parser(this, eParseResolveGroup);

}

// Search for addresses
void CACAPClient::_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	StParserState parser(this, eParseSearchAddress);

	// Reset item counter for feedback
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;
	mSearchResults = &addr_list;

	cdstring dataset = GetAdbkDataset(adbk->GetName());
	cdstring criteria = cADBKDATASET_ATTR;
	switch(fields.front())	// OK so this won't do multiple fields, but no one really uses ACAP.
	{
	case CAdbkAddress::eName:
		criteria += cADBK_NAME;
		break;
	case CAdbkAddress::eNickName:
		criteria += cADBK_NICKNAME;
		break;
	case CAdbkAddress::eEmail:
		criteria += cADBK_EMAIL;
		break;
	case CAdbkAddress::eCompany:
		criteria += cADBK_COMPANY;
		break;
	case CAdbkAddress::eAddress:
		criteria += cADBK_ADDRESS;
		break;
	case CAdbkAddress::ePhoneWork:
		criteria += cADBK_PHONE;
		break;
	case CAdbkAddress::ePhoneHome:
		criteria += cADBK_PHONE;
		break;
	case CAdbkAddress::eFax:
		criteria += cADBK_PHONEOTHER;
		break;
	case CAdbkAddress::eURL:
		criteria += cADBK_URLS;
		break;
	case CAdbkAddress::eNotes:
		criteria += cADBK_NOTES;
		break;
	default:;
	}
	cdstring lookup = name;
	lookup.ConvertFromOS();

	// Send SEARCH message to server
	// SEARCH "/addressbook/<name>/" RETURN ("addressbook.*") AND SUBSTRING "addressbook.<criteria>" <name> EQUAL "subdataset" NIL
	INETStartSend("Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress", adbk->GetName());
	INETSendString(cCMD_SEARCH);
	INETSendString(cSpace);
	INETSendString(dataset, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_ALIAS);
	INETSendString(cSpace);
	INETSendString(cASEARCH_AND);
	INETSendString(cSpace);

	switch(match)
	{
	case CAdbkAddress::eMatchExactly:
		INETSendString(cASEARCH_EQUAL);
		break;
	case CAdbkAddress::eMatchAtStart:
		INETSendString(cASEARCH_PREFIX);
		break;
	case CAdbkAddress::eMatchAtEnd:
		// no SUFFIX in ACAP :-(
		// Fall through to find it anywhere
	case CAdbkAddress::eMatchAnywhere:
	default:
		INETSendString(cASEARCH_SUBSTRING);
		break;
	}

	INETSendString(cSpace);
	INETSendString(criteria, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_INOCASE);
	INETSendString(cSpace);
	INETSendString(lookup, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(cASEARCH_NOTADBK);
	INETFinishSend();
}

// Set acl on server
void CACAPClient::_SetACL(CAddressBook* adbk, CACL* acl)
{
}

// Delete acl on server
void CACAPClient::_DeleteACL(CAddressBook* adbk, CACL* acl)
{
}

// Get all acls for adbk from server
void CACAPClient::_GetACL(CAddressBook* adbk)
{
}

// Get current user's rights to adbk
void CACAPClient::_MyRights(CAddressBook* adbk)
{
}

#pragma mark ____________________________Others

cdstring CACAPClient::GetOptionDatasetEntry(const cdstring& entry) const
{
	return GetOptionDataset() + entry;
}

cdstring CACAPClient::GetOptionDataset() const
{
	cdstring dataset = cDATASET_OPTIONS;
	dataset += mVendor + "/" + mProduct + "/";

	return dataset;
}

cdstring CACAPClient::GetAdbkDatasetEntry(const cdstring& entry, const CAdbkAddress* addr) const
{
	cdstring dataset = cDATASET_ADBK;
	
	// Special for Communigate
	if (mImplementation == eCommunigate)
	{
		dataset += "~";
		const char* p = entry.c_str();

		// Step over "user/" prefix
		p += ::strlen("user/");
		
		// Step over default address book name
		while(*p && *p != '/') p++;
		
		// Add sub address book name
		if (*p == '/')
			dataset += p;
	}
	else
		dataset += entry;

	if (addr)
	{
		dataset += "/";
		dataset += addr->GetEntry();
	}
	return dataset;
}

cdstring CACAPClient::GetAdbkDataset(const cdstring& entry) const
{
	cdstring dataset = cDATASET_ADBK;
	if (entry.length())
	{
		// Special for Communigate
		if (mImplementation == eCommunigate)
		{
			dataset += "~";
			const char* p = entry.c_str();

			// Step over "user/" prefix
			p += ::strlen("user/");
			
			// Step over default address book name
			while(*p && *p != '/') p++;
			
			// Add sub address book name
			if (*p == '/')
				dataset += p;
		}
		else
			dataset += entry;

		dataset += "/";
	}
	return dataset;
}

// Make sure address entry is valid for this address book
void CACAPClient::ValidAddressEntry(CAddressBook* adbk, const CAdbkAddress* addr) const
{
	// If it has an entry we're done
	if (addr->GetEntry().empty())
		adbk->MakeUniqueEntry(const_cast<CAdbkAddress*>(addr));
}

#pragma mark ____________________________Handle Errors

// Force reconnect
void CACAPClient::INETRecoverReconnect()
{
	// Force reset of address books only
	//if (!mOptions)
	//	CMailControl::AdbkServerReconnect(GetAdbkOwner());
}

// Force disconnect
void CACAPClient::INETRecoverDisconnect()
{
	// Force reset of address books only
	//if (!mOptions)
	//	CMailControl::AdbkServerDisconnect(GetAdbkOwner());
}

// Descriptor for object error context
const char*	CACAPClient::INETGetErrorDescriptor() const
{
	return "Address Book: ";
}

#pragma mark ____________________________Parsing

// Parse text sent by server (advance pointer to next bit to be parsed)
void CACAPClient::INETParseResponse(char** txt, CINETClientResponse* response)
{
	// Clear response message first
	response->tag_msg = cdstring::null_str;

	// Look for tags first
	if (::stradvtokcmp(txt, mTag) == 0)
		INETParseTagged(txt, response);

	// Look for star responses next
	else if (::stradvtokcmp(txt,cSTAR) == 0)
	{

		// Look for specific star responses
		if (::stradvtokcmp(txt,cOK) == 0)
		{
			response->code = cStarOK;
			ACAPParseResponseCode(txt, response, false);
		}
		else if (::stradvtokcmp(txt,cNO) == 0)
		{
			response->code = cStarNO;
			ACAPParseResponseCode(txt, response, false);
		}
		else if (::stradvtokcmp(txt,cBAD) == 0)
		{
			response->code = cStarBAD;
			ACAPParseResponseCode(txt, response, false);
		}

		else if (::stradvtokcmp(txt,cBYE)==0)
		{
			response->code = cStarBYE;
			ACAPParseResponseCode(txt, response, false);
		}

		// Do protocols processing
		else
			_ParseResponse(txt, response);
	}

	// Look for continuation response
	else if (::stradvtokcmp(txt,cPLUS)==0)
		response->code = cPlusLabel;

	// Anything else is an error
	else
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}
		//response->code = cResponseError;
}

// Parse text sent by server
void CACAPClient::INETParseTagged(char** txt, CINETClientResponse* response)
{
	// Look for specific tag responses
	if (::stradvtokcmp(txt,cOK) == 0)
	{
		response->code = cTagOK;
		
		ACAPParseResponseCode(txt, response, true);
	}
	else if (::stradvtokcmp(txt,cNO) == 0)
	{
		response->code = cTagNO;
		
		ACAPParseResponseCode(txt, response, true);
	}
	else if (::stradvtokcmp(txt,cBAD) == 0)
	{
		response->code = cTagBAD;
		
		ACAPParseResponseCode(txt, response, true);
	}
	// Do protocols processing
	else if (mDoesTaggedResponses)
		_ParseResponse(txt, response);
	else
		response->code = cResponseError;
}

// Parse text sent by server (advance pointer to next bit to be parsed)
void CACAPClient::ACAPParseResponse(char** txt, CINETClientResponse* response)
{
	// Found an option
	if (::stradvtokcmp(txt, cRESP_ACAP)==0)
	{
		response->code = cStarACAP;
	}
	else if (::stradvtokcmp(txt, cRESP_ALERT)==0)
	{
		response->code = cStarALERT;
		ACAPParseResponseCode(txt, response, false);
		
		// Need to fake INETClient '[ALERT]' untagged response
		cdstring untagged = cALERT;
		untagged += response->PopUntagged();
		response->AddUntagged(untagged);
	}
	else if (::stradvtokcmp(txt, cRESP_ENTRY)==0)
	{
		response->code = cStarENTRY;
		ACAPParseEntry(txt);
	}
	else if (::stradvtokcmp(txt, cRESP_MODTIME)==0)
	{
		response->code = cStarMODTIME;
		// Ignore
	}
	else
		response->code = cResponseError;

} // CINETClient::ACAPParseResponse

// Parse text sent by server (advance pointer to next bit to be parsed)
void CACAPClient::ACAPParseResponseCode(char** txt, CINETClientResponse* response, bool tagged)
{
	// Look for response code
	char* p = *txt;

	while(*p && (*p == ' ')) p++;

	// Response code starts with '('
	if (*p == '(')
	{
		p++;

		// Look at each response code
		
		// Look for single tokens response codes
		if (!::stradvtokcmp(&p, cACAP_RESPONSE_AUTHTOOWEAK) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_ENCRYPT_NEEDED) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_QUOTA) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_TOOOLD) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_TRANSITIONNEEDED) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_TRYFREECONTEXT) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_TRYLATER) ||
			!::stradvtokcmp(&p, cACAP_RESPONSE_WAYTOOMANY))
		{
			// No need to do anything!
		}

		// Special case: look for NOEXIST
		else if (!::stradvtokcmp(&p, cACAP_RESPONSE_NOEXIST))
		{
			// Ignore string
			delete INETParseString(&p);

			// Pretend that this is really an OK
			response->code = cTagOK;
		}
		else if (!::stradvtokcmp(&p, cACAP_RESPONSE_MODIFIED) ||
				 !::stradvtokcmp(&p, cACAP_RESPONSE_SASL) ||
				 !::stradvtokcmp(&p, cACAP_RESPONSE_TOOMANY))
		{
			// Ignore string
			delete INETParseString(&p);
		}
		else if (!::stradvtokcmp(&p, cACAP_RESPONSE_INVALID))
		{
			// Look for pairs of entry-paths/attributes
			while(*p && (*p != ')'))
			{
				// Ignore both strings
				delete INETParseString(&p);
				delete INETParseString(&p);
				while(*p == ' ') p++;
			}
		}
		else if (!::stradvtokcmp(&p, cACAP_RESPONSE_PERMISSION))
		{
			// Look for start of acl-object
			if (*p++ != '(')
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
			
			// Ignore dataset
			delete INETParseString(&p);
			
			// Option items
			if (*p && (*p == ' '))
			{
				// Ignore attribute
				delete INETParseString(&p);

				// Optional items
				if (*p && (*p == ' '))
				{
					// Ignore entry
					delete INETParseString(&p);
				}
			}

			// Look for end of acl-object
			if (*p++ != ')')
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
		}
		else if (!::stradvtokcmp(&p, cACAP_RESPONSE_REFER))
		{
			// Ignore list of URLs
			ACAPParseReferral(&p, NULL);
		}
		else
		{
			// Unknown token
			
			// Ignore iana-token
			delete INETParseString(&p);
			
			// Look for extension data
			while(*p == ' ')
				ACAPParseExtensionItem(&p);
		}
		
		p++;
	}
	
	// Copy the quoted message text
	if (tagged)
		response->tag_msg = p;
	else
		response->AddUntagged(p);

} // CINETClient::ACAPParseResponse

#pragma mark ____________________________Option Responses

// Parse ENTRY
void CACAPClient::ACAPParseEntry(char** txt)
{
	char* p = *txt;

	// Step over white space
	while(*p == ' ') p++;

	// Dispatch to appropriate routine
	switch(mParserState)
	{
	case eParseIgnore:
		break;
	case eParseFindAllAttributes:
		ACAPParseFindAllAttributes(&p);
		break;
	case eParseSetAllAttributes:
	case eParseDeleteEntry:
		break;
	case eParseGetAttribute:
		ACAPParseGetAttribute(&p);
		break;
	case eParseSetAttribute:
	case eParseDeleteAttribute:
		break;
	case eParseFindAllAdbks:
		ACAPParseFindAllAdbks(&p);
		break;
	case eParseCreateAdbk:
	case eParseDeleteAdbk:
	case eParseRenameAdbk:
		break;
	case eParseFindAllAddresses:
		ACAPParseFindAllAddrs(&p, false);
		break;
	case eParseFetchAddress:
	case eParseStoreAddress:
	case eParseStoreGroup:
	case eParseDeleteAddress:
	case eParseDeleteGroup:
		break;
	case eParseResolveAddress:
		ACAPParseFindAllAddrs(&p, false);
		break;
	case eParseResolveGroup:
		break;
	case eParseSearchAddress:
		ACAPParseFindAllAddrs(&p, true);
		break;
	}
}

// Parse return-attr-list
void CACAPClient::ACAPParseFindAllAttributes(char** txt)
{
	char* p = *txt;

	// Must be pointing at entryname
	cdstring entryname;
	entryname.steal(INETParseString(&p));

	// Punt to first '('
	while(*p== ' ') p++;

	// Must have '('
	if (*p == '(')
	{
		p++;

		// Parse each list item
		while(*p == '(')
		{
			ACAPParseMetaList(&p);

			// Punt past any trailing space
			while(*p == ' ') p++;
		}

		// Must be at trailing ')'
		p++;
	}

	// Update return pointer
	*txt = p;
}

// Parse return-metalist
void CACAPClient::ACAPParseMetaList(char** txt)
{
	char* p = *txt;

	// Punt past first '('
	p++;

	// Get attribute and value
	cdstring option_attribute;
	option_attribute.steal(INETParseString(&p));
	cdstring value;
	value.steal(INETParseString(&p));

	// Adjust attribute pointer past 'option.'
	const char* attribute = option_attribute.c_str() + sizeof(cOPTIONSDATASET_ATTR) - 1;

	// Insert
	std::pair<cdstrmap::iterator, bool> result = GetOptionsOwner()->GetMap()->insert(cdstrmap::value_type(attribute, value));

	// Does it exist already
	if (!result.second)
		// Replace existing
		(*result.first).second = value;

	// Punt past last ')'
	p++;

	// Update return pointer
	*txt = p;
}

// Parse single attribute
void CACAPClient::ACAPParseGetAttribute(char** txt)
{
	char* p = *txt;

	// Must be pointing at entryname
	cdstring entryname;
	entryname.steal(INETParseString(&p));

	// Must be pointing at value for requested attribute
	cdstring value;
	value.steal(INETParseString(&p));

	// Must match key
	GetOptionsOwner()->SetValue(value);

	// Update return pointer
	*txt = p;
}

// Parse single addressbook entry
void CACAPClient::ACAPParseFindAllAdbks(char** txt)
{
	char* p = *txt;

	// Format: "/addressbook/.../..." "<addressbook.CommonName>"

	// Must be pointing at value for requested attribute
	cdstring adbk_name;
	adbk_name.steal(INETParseString(&p));

	// Now get name as adbk
	cdstring adbk_cname;
	adbk_cname.steal(INETParseString(&p));

	if (adbk_cname.length())
	{
		// Add adress book to list
		CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), true, false, adbk_cname);
		GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);
	}

	// Update return pointer
	*txt = p;
}

// Parse single address entry without data
void CACAPClient::ACAPParseFindAllAddrs(char** txt, bool search)
{
	char* p = *txt;

	// Format: "<entryid"> (("addressbook.<attr1>" "<val1>") ("addressbook.<attr2>" "<val2>") (...))

	cdstring entryid;
	entryid.steal(INETParseString(&p));

	// Create new address and add to address book
	CAdbkAddress* addr = new CAdbkAddress;
	addr->SetEntry(entryid);
	
	// Entry must have valid name - the empty name may be returned but that is not a real item
	if (!entryid.empty())
	{
		if (search)
			mSearchResults->push_back(addr);
		else
			mActionAdbk->GetAddressList()->push_back(addr);

		BumpItemCtr(mItemTotal ? "Status::IMSP::FetchAddressCount2" : "Status::IMSP::FetchAddressCount1");
	}

	// Must have '('
	while(*p && (*p != '(')) p++;

	if (*p)
	{
		p++;
		while(*p && (*p != ')'))
			ACAPParseAddressAttribute(&p, addr);
	}

	// Delete any unused address
	if (entryid.empty())
		delete addr;

	// Update return pointer
	*txt = p;
}

// Parse ACAP addressbook.xxx attribute
void CACAPClient::ACAPParseAddressAttribute(char** txt, CAdbkAddress* addr)
{
	char* p = *txt;

	while(*p && (*p != '(')) p++;

	if (*p)
	{
		p++;
		cdstring attribute;
		attribute.steal(INETParseString(&p));
		cdstrvect values;
		INETParseMultiString(&p, values, true);
		for(cdstrvect::iterator iter = values.begin(); iter != values.end(); iter++)
			(*iter).ConvertToOS();

		// Now parse the attribute
		if (!::strncmpnocase(attribute.c_str(), cADBKDATASET_ATTR, sizeof(cADBKDATASET_ATTR) -1))
		{
			const char* item = attribute.c_str() + sizeof(cADBKDATASET_ATTR) -1;

			// Test for possible items
			if (::strcmpnocase(item, cADBK_NAME) == 0)
				addr->SetName(values.front());
			else if (::strcmpnocase(item, cADBK_NICKNAME) == 0)
				addr->SetADL(values.front());
			else if (::strcmpnocase(item, cADBK_EMAIL) == 0)
			{
				cdstring temp = addr->GetName();
				addr->ParseAddress(values.front());	// <-- this is terrible need setter for email address
				addr->SetName(temp);
			}
			else if (::strcmpnocase(item, cADBK_ADDRESS) == 0)
			{
				::FilterEndls(values.front());
				addr->SetAddress(values.front(), CAdbkAddress::eDefaultAddressType);
			}
			else if (::strcmpnocase(item, cADBK_COMPANY) == 0)
				addr->SetCompany(values.front());
			else if (::strcmpnocase(item, cADBK_PHONE) == 0)
			{
				// See if nulls in data
				char* tag = ::strchr(values.front().c_str_mod(), 1);
				if (tag)
				{
					*tag++ = 0;
					if (!::strcmpnocase(tag, cADBK_PHONE_HOME))
						addr->SetPhone(values.front(), CAdbkAddress::eHomePhoneType);
					else if (!::strcmpnocase(tag, cADBK_PHONE_WORK))
						addr->SetPhone(values.front(), CAdbkAddress::eWorkPhoneType);
					else if (!::strcmpnocase(tag, cADBK_PHONE_FAX))
						addr->SetPhone(values.front(), CAdbkAddress::eFaxType);
				}
				else
					addr->SetPhone(values.front(), CAdbkAddress::eWorkPhoneType);
			}
			else if (::strcmpnocase(item, cADBK_PHONEOTHER) == 0)
			{
				// Look at each one
				for(cdstrvect::iterator iter = values.begin(); iter != values.end(); iter++)
				{
					// See if nulls in data
					char* tag = ::strchr((*iter).c_str_mod(), 1);
					if (tag)
					{
						*tag++ = 0;
						if (!::strcmpnocase(tag, cADBK_PHONE_HOME))
							addr->SetPhone(*iter, CAdbkAddress::eHomePhoneType);
						else if (!::strcmpnocase(tag, cADBK_PHONE_WORK))
							addr->SetPhone(*iter, CAdbkAddress::eWorkPhoneType);
						else if (!::strcmpnocase(tag, cADBK_PHONE_FAX))
							addr->SetPhone(*iter, CAdbkAddress::eFaxType);
					}
					else
						addr->SetPhone(*iter, CAdbkAddress::eWorkPhoneType);
				}
			}
			else if (::strcmpnocase(item, cADBK_URLS) == 0)
			{
				::FilterEndls(values.front());
				addr->SetURL(values.front());
			}
			else if (::strcmpnocase(item, cADBK_NOTES) == 0)
			{
				::FilterEndls(values.front());
				addr->SetNotes(values.front());
			}

			while(*p && (*p != ')')) p++;
			if (*p) p++;
			while(*p && (*p == ' ')) p++;
		}
	}

	// Update return pointer
	*txt = p;
}

// Parse ACAP url referral list
void CACAPClient::ACAPParseReferral(char** txt, cdstrvect* urls)
{
	char* p = *txt;

	// Punt on any space
	cdstring item;
	while(*p == ' ')
	{
		item.steal(INETParseString(&p));
		if (urls)
			urls->push_back(item);
	}

	// Update return pointer
	*txt = p;
}

// Parse ACAP extension items
void CACAPClient::ACAPParseExtensionItem(char** txt)
{
	char* p = *txt;

	// Punt on any space
	while(*p == ' ') p++;
	
	// Look for a list of them
	if (*p == '(')
	{
		p++;
		while(*p && (*p != ')'))
			ACAPParseExtensionItem(&p);
		
		if (*p) p++;
	}
	else
		// Ignore atom/string/number
		delete INETParseString(&p);
	
	// Update return pointer
	*txt = p;
}


// Get ATOM special characters for parsing
const char* CACAPClient::INETGetAtomSpecials() const
{
	// This is the default from RFC2244 (ACAP)
	return "\t\n\r \"()\\{";
}
