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

#include "CSIEVEClient.h"

#include "CCharSpecials.h"
#include "CErrorHandler.h"
#include "CINETCommon.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#else
#include "StValueChanger.h"
#endif

#include <memory>
#include <stdio.h>
#include <stdlib.h>

#include "CSIEVECommon.cp"

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CSIEVEClient::CSIEVEClient(CFilterProtocol* owner)
	: CFilterClient(owner)
{
	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitSIEVEClient();

}

// Copy constructor
CSIEVEClient::CSIEVEClient(const CSIEVEClient& copy, CFilterProtocol* owner)
	: CFilterClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitSIEVEClient();

}

CSIEVEClient::~CSIEVEClient()
{
}

void CSIEVEClient::InitSIEVEClient()
{
	// Always has async literals
	mAsyncLiteral = true;

	// Cannot use 'LOGIN'
	mLoginAllowed = false;

	// Allows initial client data in SASL
	mAuthInitialClientData = true;

	// Init instance variables
	mLogType = CLog::eLogFilters;

	// Set protocol specific items
	mUseTag = false;

	mCommandState = eNone;
	mExtensions = CFilterProtocol::eNone;
	mStringListResult = NULL;
	mStringResult = NULL;
	mSTARTTLSAllowed = false;
}

// Create duplicate, empty connection
CINETClient* CSIEVEClient::CloneConnection()
{
	// Copy construct this
	return new CSIEVEClient(*this);

}

// Get default port
tcp_port CSIEVEClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return cSIEVEServerPort_SSL;
	else
		return cSIEVEServerPort;
}

#pragma mark ____________________________Protocol

// Process for output force literal rather than quote
int CSIEVEClient::ProcessString(cdstring& str)
{
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
		case 0: // Atom		// SIEVE always quotes
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
}

// Process greeting response
bool CSIEVEClient::_ProcessGreeting()
{
	bool result =true;

	try
	{
		StCommandState state(eCAPABILITY, this);

		// Process greeting responses
		INETProcess();
		
		// Now parse out capability items
		_ProcessCapability();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Error
		result = false;
	}

	// Look for specific tag responses
	return result;
}


void CSIEVEClient::_Capability(bool after_tls)
{
	// Must set to capability state
	StCommandState state(eCAPABILITY, this);

	CINETClient::_Capability(after_tls);
}

// Initialise capability flags to empty set
void CSIEVEClient::_InitCapability()
{
	// Init capabilities to empty set
	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;
	mExtensions = CFilterProtocol::eNone;
	
	mCapabilities.clear();

	mCapability = cdstring::null_str;
}

// Check version of server
void CSIEVEClient::_ProcessCapability()
{
	// Look for SASL items
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	cdstrmap::const_iterator found = mCapabilities.find(cSIEVE_SASL);
	if (found != mCapabilities.end())
	{
		// Parse out extensions
		cdstring items = (*found).second;
		
		// Tokenise for each extension
		char* p = items.c_str_mod();
		while(p && *p)
		{
			char* q = ::strgettokenstr(&p, WHITE_SPACE);
			if (q && *q)
			{
				if (::strcmpnocase(q, cPLAIN) == 0)
					mAuthPlainAllowed = true;
				else if (::strcmpnocase(q, cANONYMOUS) == 0)
					mAuthAnonAllowed = true;
			}
		}
	}
	
	// Look for STARTTLS
	found = mCapabilities.find(cSTARTTLS);
	if (found != mCapabilities.end())
		mSTARTTLSAllowed = true;

	// Clear SIEVE extensions
	mExtensions = CFilterProtocol::eNone;

	// Look for SIEVE extensions
	found = mCapabilities.find(cSIEVE);
	if (found != mCapabilities.end())
	{
		// Parse out extensions
		cdstring items = (*found).second;
		
		// Tokenise for each extension
		char* p = items.c_str_mod();
		while(p && *p)
		{
			char* q = ::strgettokenstr(&p, WHITE_SPACE);
			if (q && *q)
			{
				if (::strcmpnocase(q, cREJECT) == 0)
					mExtensions = static_cast<CFilterProtocol::EExtension>(mExtensions | CFilterProtocol::eReject);
				else if (::strcmpnocase(q, cFILEINTO) == 0)
					mExtensions = static_cast<CFilterProtocol::EExtension>(mExtensions | CFilterProtocol::eFileInto);
				else if (::strcmpnocase(q, cVACATION) == 0)
					mExtensions = static_cast<CFilterProtocol::EExtension>(mExtensions | CFilterProtocol::eVacation);
				else if (::strcmpnocase(q, cRELATIONAL) == 0)
					mExtensions = static_cast<CFilterProtocol::EExtension>(mExtensions | CFilterProtocol::eRelational);
				else if (::strcmpnocase(q, cIMAP4FLAGS) == 0)
					mExtensions = static_cast<CFilterProtocol::EExtension>(mExtensions | CFilterProtocol::eIMAP4Flags);
			}
		}
	}
	
	// For now do nothing special with other extensions

}

// Handle failed capability response
void CSIEVEClient::_NoCapability()
{
	// No capabilities

}

// Handle failed capability response
void CSIEVEClient::_PreProcess()
{
	// No special preprocess

}

// Handle failed capability response
void CSIEVEClient::_PostProcess()
{
	// No special postprocess

}

// Handle failed capability response
void CSIEVEClient::_ParseResponse(char** txt, CINETClientResponse* response)
{
	SIEVEParseResponse(txt, response);
}

#pragma mark ____________________________Commands

// Check for server extension
bool CSIEVEClient::_HasExtension(CFilterProtocol::EExtension ext) const
{
	// Must support each of the ones asked for
	return (mExtensions & ext) == ext;
}

// Check script size write to server
void CSIEVEClient::_HaveSpace(const cdstring& name, unsigned long size)
{
	StCommandState state(eHASSPACE, this);

	// Send HAVESPACE message to server
	INETStartSend("Status::SIEVE::HaveSpace", "Error::IMSP::OSErrHaveSpace", "Error::IMSP::NoBadHaveSpace");
	INETSendString(cHAVESPACE);
	INETSendString(cSpace);
	INETSendString(cdstring(size));
	INETFinishSend();
}

// Store script on server
void CSIEVEClient::_PutScript(const cdstring& name, const cdstring& script)
{
	StCommandState state(ePUTSCRIPT, this);

	// Must use CRLF for script
	cdstring temp(script);
	temp.ConvertEndl(eEndl_CRLF);

	// Send PUTSCRIPT message to server
	INETStartSend("Status::SIEVE::PutScript", "Error::IMSP::OSErrPutScript", "Error::IMSP::NoBadPutScript");
	INETSendString(cPUTSCRIPT);
	INETSendString(cSpace);
	INETSendString(name, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(temp, eQueueLiteral);	// Always send as a literal
	INETFinishSend();
}

// List available scripts
void CSIEVEClient::_ListScripts(cdstrvect& scripts, cdstring& active)
{
	StCommandState state(eLISTSCRIPTS, this);

	// Set up vars to copy results into
	StValueChanger<cdstrvect*> _change1(mStringListResult, &scripts);
	StValueChanger<cdstring*> _change2(mStringResult, &active);

	// Send LISTSCRIPTS message to server
	INETStartSend("Status::SIEVE::ListScripts", "Error::IMSP::OSErrListScripts", "Error::IMSP::NoBadListScripts");
	INETSendString(cLISTSCRIPTS);
	INETFinishSend();
}

// Make server script the active one
void CSIEVEClient::_SetActive(const cdstring& script)
{
	StCommandState state(eSETACTIVE, this);

	// Send HAVESPACE message to server
	INETStartSend("Status::SIEVE::SetActive", "Error::IMSP::OSErrSetActive", "Error::IMSP::NoBadSetActive");
	INETSendString(cSETACTIVE);
	INETSendString(cSpace);
	INETSendString(script, eQueueProcess);
	INETFinishSend();
}

// Get named script
void CSIEVEClient::_GetScript(const cdstring& name, cdstring& script)
{
	StCommandState state(eGETSCRIPT, this);

	// Set up vars to copy results into
	StValueChanger<cdstring*> _change1(mStringResult, &script);

	// Send GETSCRIPT message to server
	INETStartSend("Status::SIEVE::GetScript", "Error::IMSP::OSErrGetScript", "Error::IMSP::NoBadGetScript");
	INETSendString(cGETSCRIPT);
	INETSendString(cSpace);
	INETSendString(name, eQueueProcess);
	INETFinishSend();
}

// Delete names script
void CSIEVEClient::_DeleteScript(const cdstring& script)
{
	StCommandState state(eDELETESCRIPT, this);

	// Send DELETESCRIPT message to server
	INETStartSend("Status::SIEVE::DeleteScript", "Error::IMSP::OSErrDeleteScript", "Error::IMSP::NoBadDeleteScript");
	INETSendString(cDELETESCRIPT);
	INETSendString(cSpace);
	INETSendString(script, eQueueProcess);
	INETFinishSend();
}

#pragma mark ____________________________Handle Errors

// Force reconnect
void CSIEVEClient::INETRecoverReconnect()
{
}

// Force disconnect
void CSIEVEClient::INETRecoverDisconnect()
{
}

#pragma mark ____________________________Parsing

// Parse text sent by server (advance pointer to next bit to be parsed)
void CSIEVEClient::INETParseResponse(char** txt, CINETClientResponse* response)
{
	// Clear response message first
	response->tag_msg = cdstring::null_str;

	// No tags - so always process as tagged response
	INETParseTagged(txt, response);
}

// Parse text sent by server
void CSIEVEClient::INETParseTagged(char** txt, CINETClientResponse* response)
{
	// Look for specific tag responses
	if (::stradvtokcmp(txt,cOK) == 0)
	{
		response->code = cTagOK;
		
		SIEVEParseResponseCode(txt, response);
	}
	else if (::stradvtokcmp(txt,cNO) == 0)
	{
		response->code = cTagNO;
		
		SIEVEParseResponseCode(txt, response);
	}
	else if (::stradvtokcmp(txt,cBYE) == 0)
	{
		response->code = cStarBYE;
		
		SIEVEParseResponseCode(txt, response);
	}

	// Do protocols processing
	else
		_ParseResponse(txt, response);
}

// Parse text sent by server (advance pointer to next bit to be parsed)
void CSIEVEClient::SIEVEParseResponse(char** txt, CINETClientResponse* response)
{
	// Depends on command state
	switch(mCommandState)
	{
	case eCAPABILITY:
		SIEVEParseCAPABILITYResponse(txt, response);
		break;
	case eLISTSCRIPTS:
		SIEVEParseLISTSCRIPTSResponse(txt, response);
		break;
	case eGETSCRIPT:
		SIEVEParseGETSCRIPTResponse(txt, response);
		break;
	default:;
	}
}

// Parse text sent by server (advance pointer to next bit to be parsed)
void CSIEVEClient::SIEVEParseResponseCode(char** txt, CINETClientResponse* response)
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
		if (!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_AUTHTOOWEAK) ||
			!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_ENCRYPTNEEDED) ||
			!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_QUOTA) ||
			!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_TRANSITIONNEEDED) ||
			!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_TRYLATER))
		{
			// No need to do anything!
		}

		// Special case: look for SASL
		else if (!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_SASL))
		{
			// Ignore string
			delete INETParseString(&p);

			// Pretend that this is really an OK
			response->code = cTagOK;
		}

		// Special case: look for REFERRAL
		else if (!::stradvtokcmp(&p, cSIEVE_RESPONSECODE_REFERRAL))
		{
			// Ignore string
			delete INETParseString(&p);

			// Pretend that this is really an OK
			response->code = cTagOK;
		}

		else
		{
			// Unknown token
			
			// Ignore iana-token
			delete INETParseString(&p);
			
			// Look for extension data
			while(*p == ' ')
				SIEVEParseExtensionItem(&p);
		}
		
		p++;
	}
	
	// Get response descriptor
	response->tag_msg.steal(INETParseString(&p));

}

// Capabilities
void CSIEVEClient::SIEVEParseCAPABILITYResponse(char** txt, CINETClientResponse* response)
{
	// Possibly two strings
	std::auto_ptr<char> str1(INETParseString(txt));
	std::auto_ptr<char> str2(INETParseString(txt));
	
	// Add to capability map
	mCapabilities.insert(cdstrmap::value_type(cdstring(str1.get()), cdstring(str2.get())));
}

// List scripts
void CSIEVEClient::SIEVEParseLISTSCRIPTSResponse(char** txt, CINETClientResponse* response)
{
	// Possibly two strings
	std::auto_ptr<char> str1(INETParseString(txt));
	std::auto_ptr<char> str2(INETParseString(txt));
	
	// Add first string to list results
	if (mStringListResult)
		mStringListResult->push_back(cdstring(str1.get()));

	// Add first string to active script result
	if (str1.get() && str2.get() && (::strcmpnocase(str2.get(), cACTIVE) == 0) && mStringResult)
		mStringResult->steal(str1.release());
}

// Get script
void CSIEVEClient::SIEVEParseGETSCRIPTResponse(char** txt, CINETClientResponse* response)
{
	// Possibly two strings
	std::auto_ptr<char> str1(INETParseString(txt));
	
	// Add script result
	if (str1.get() && mStringResult)
		mStringResult->steal(str1.release());
}

// Parse SIEVE extension items
void CSIEVEClient::SIEVEParseExtensionItem(char** txt)
{
	char* p = *txt;

	// Punt on any space
	while(*p == ' ') p++;
	
	// Look for a list of them
	if (*p == '(')
	{
		p++;
		while(*p && (*p != ')'))
			SIEVEParseExtensionItem(&p);
		
		if (*p) p++;
	}
	else
		// Ignore atom/string/number
		delete INETParseString(&p);
	
	// Update return pointer
	*txt = p;
}
