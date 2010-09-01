/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for INET client class

// This is an abstract base class that can be used with IMAP, IMSP & ACAP protocols

#include "CINETClient.h"
#include "CINETCommon.h"
#include "CINETProtocol.h"

#include "CAdbkProtocol.h"
#include "CAddress.h"
#include "CAuthPlugin.h"
#include "CCalendarProtocol.h"
#include "CCertificateManager.h"
#include "CCharSpecials.h"
#include "CFilterProtocol.h"
#include "CIdentity.h"
#include "CMailControl.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "COptionsProtocol.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CSecurityPlugin.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CTCPException.h"
#include "CXStringResources.h"

#include "base64.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x

#include <LThread.h>

#elif __dest_os == __win32_os

#include <WIN_LStream.h>
#include "StValueChanger.h"

#elif __dest_os == __linux_os
#include "CWaitCursor.h"
#include "UNX_LStream.h"
#include "StValueChanger.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include <strstream>
#include <typeinfo>

extern const char* cSpace;

#pragma mark ____________________________CINETClientResponse

CINETClient::CINETClientResponse::CINETClientResponse()
{
	code = cNoResponse;
}

// Copy constructor
CINETClient::CINETClientResponse::CINETClientResponse(const CINETClientResponse& copy)
	: code(copy.code),
	  tag_msg(copy.tag_msg),
	  untag_msgs(copy.untag_msgs)
{
}

// Assignment with same type
CINETClient::CINETClientResponse& CINETClient::CINETClientResponse::operator=(const CINETClientResponse& copy)
{
	code = copy.code;
	tag_msg = copy.tag_msg;
	untag_msgs = copy.untag_msgs;

	return *this;
}

// Find some text in a tagged response
bool CINETClient::CINETClientResponse::FindTagged(const char* tagged) const
{
	 return (::strstrnocase(tag_msg, tagged) != NULL);
}

// Added untagged response to list
void CINETClient::CINETClientResponse::AddUntagged(const char* untagged)
{
	untag_msgs.push_back(untagged);
}

// Find some text in an untagged response
bool CINETClient::CINETClientResponse::CheckUntagged(const char* key, bool token) const
{
	for(cdstrvect::const_iterator iter = untag_msgs.begin(); iter != untag_msgs.end(); iter++)
	{
		cdstring temp = *iter;
		::strupper(temp);
		if (token)
		{
			const char* tok = ::strtok(temp, " ");
			while(tok)
			{
				if (!::strcmp(tok, key))
					return true;
				
				tok = ::strtok(NULL, " ");
			}
		}
		else if (::strstr(temp.c_str(), key) != NULL)
			return true;
	}

	return false;
}

// Get untagged response matching key
const cdstring& CINETClient::CINETClientResponse::GetUntagged(const char* key) const
{
	for(cdstrvect::const_iterator iter = untag_msgs.begin(); iter != untag_msgs.end(); iter++)
	{
		if (::strstrnocase(*iter, key) != NULL)
			return *iter;
	}

	return cdstring::null_str;
}

// Get untagged response matching key and remove from list
cdstring CINETClient::CINETClientResponse::PopUntagged(const char* key)
{
	for(cdstrvect::iterator iter = untag_msgs.begin(); iter != untag_msgs.end(); iter++)
	{
		if (::strstrnocase(*iter, key) != NULL)
		{
			cdstring temp(*iter);
			untag_msgs.erase(iter);
			return temp;
		}
	}

	return cdstring::null_str;
}

// Get last untagged response
cdstring CINETClient::CINETClientResponse::PopUntagged()
{
	cdstring result;

	if (untag_msgs.size())
	{
		result = untag_msgs.back();
		untag_msgs.pop_back();
	}
	
	return result;
}

// Clear messages
void CINETClient::CINETClientResponse::Clear()
{
	code = cResponseError;
	tag_msg = cdstring::null_str;
	untag_msgs.clear();
}

#pragma mark ____________________________CINETClient

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CINETClient::CINETClient()
{
	mOwner = NULL;
	mPrivateAccount = NULL;
	mStream = NULL;

	// Init instance variables
	InitINETClient();

}

CINETClient::CINETClient(CINETProtocol* owner)
{
	mOwner = owner;
	mPrivateAccount = NULL;
	mStream = NULL;

	// Init instance variables
	InitINETClient();
}

// Copy constructor
CINETClient::CINETClient(const CINETClient& copy, CINETProtocol* owner)
{
	mOwner = owner;
	mPrivateAccount = NULL;
	mStream = copy.mStream ? new CTCPStream(*copy.mStream) : NULL;

	// Init instance variables
	InitINETClient();

	// Clone server details
	mServerAddr = copy.mServerAddr;
}

CINETClient::~CINETClient()
{
	// Always delete buffers just in case
	delete mLineData;
	mLineData = NULL;
	delete mLongLine;
	mLongLine = NULL;

	mOwner = NULL;
	delete mPrivateAccount;
	mPrivateAccount = NULL;
	delete mStream;
	mStream = NULL;
}

// Create duplicate, empty connection
CINETClient* CINETClient::CloneConnection()
{
	// Copy construct this
	return NULL;

}

void CINETClient::InitINETClient()
{
	// Init instance variables
	mServerAddr = cdstring::null_str;

	mLineData = NULL;
	mLongLine = NULL;
	mTag[0] = 0;
	mTagCount = 0;
	mUseTag = true;
	mDoesTaggedResponses = false;

	mRcvStream = NULL;
	mRcvOStream = NULL;

	mAsyncLiteral = false;
	mLoginAllowed = true;
	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mAuthInitialClientData = false;
	mAuthBase64 = true;
	mSTARTTLSAllowed = false;

	mSilentCommand = false;
	mSilentError = false;
	mAllowLog = true;
	mLogType = CLog::eLogNone;

	mItemCtr = 0;
	mItemTotal = 0;

	mStatusQueue = 0;
	mStatusID = NULL;
	mErrorID = NULL;
	mNoBadID = NULL;

}

void CINETClient::SetPrivateAccount(CINETAccount* acct)
{
	delete mPrivateAccount;
	mPrivateAccount = acct;
}

CINETAccount* CINETClient::GetAccount() const
{
	return (mPrivateAccount != NULL) ? mPrivateAccount : mOwner->GetAccount();
}

const cdstring& CINETClient::GetCertText() const
{
	if (mStream != NULL)
	{
		return mStream->GetCertText();
	}
	else
		return cdstring::null_str;
}

// Add string for output processing
void CINETClient::SendString(const char* str, int flags)
{
	// Only if we have a stream
	if (!mStream)
		return;

	// Indicates literal sending required
	size_t literal_size = 0;

	const char* s = str;
	cdstring temp;

	// Look at flags
	if (flags & (eQueueProcess | eQueueLiteral | eQueueManualLiteral | eQueueBuffer))
	{
		// Check forced literal
		if (flags & eQueueManualLiteral)
			literal_size = GetManualLiteralLength();
		else if (flags & eQueueLiteral)
			literal_size = ::strlen(s);
		else if (flags & eQueueBuffer)
			literal_size = *reinterpret_cast<const size_t*>(s);
		else
		{
			temp = s;
			literal_size = ProcessString(temp);
			if (!literal_size)
				s = temp.c_str();
		}
	}

	// If literal_size => do literal
	if (literal_size)
	{
		char size[32];
		::snprintf(size, 32, mAsyncLiteral ? "{%d+}" : "{%d}", literal_size);
		mStream->write(size, ::strlen(size));
		mStream->write(cCRLF, 2);
		if (mAllowLog)
			mLog.LogPartialEntry(size);

		// Synchronising literal
		if (!mAsyncLiteral)
		{
			// Flush out to server
			*mStream << std::flush;

			// Now process responses waiting for continuation
			INETProcess();

			// Only valid state is to have a continuation so fall through and send the literal
		}

		// Non-synchronising literal
		if (flags & eQueueManualLiteral)
			// Send manual literal
			SendManualLiteral();
		else if (flags & eQueueBuffer)
		{
			mStream->write(s + sizeof(size_t), literal_size);
			if (mAllowLog)
			{
				cdstring temp2(s + sizeof(size_t), literal_size);
				temp2.FilterInEscapeChars(cCEscapeChar);
				mLog.LogPartialEntry(temp2);
			}
		}
		else
		{
			mStream->write(s, ::strlen(s));
			if (mAllowLog)
				mLog.LogPartialEntry(s);
		}
	}
	else if (s && *s)
	{
		mStream->write(s, ::strlen(s));
		if (mAllowLog)
			mLog.LogPartialEntry(s);
	}
	else
	{
		mStream->write("\"\"", 2);
		if (mAllowLog)
			mLog.LogPartialEntry("\"\"");
	}
}

// Process for output (maybe quote specials or literalise), always quote
int CINETClient::ProcessString(cdstring& str)
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
		case 2: // Escaped	 - quoting will force escaping
			quote = true;
			break;
		case 3: // Literal
			literal = true;
			break;
		}
	}

	// Process literal above all else
	if (literal)

		// Just return literal size - will be sent as is
		return str.length();

	// Quote forcibiliy  (use INET style escape chars)
	if (quote || str.empty())
		str.quote(true, true);

	return 0;
}

// Get length of manually processed literal
int CINETClient::GetManualLiteralLength()
{
	return 0;	// Default action
}

// Send manual literal
void CINETClient::SendManualLiteral()
{
	// Do nothing
}

// Initialise item counter
void CINETClient::InitItemCtr(unsigned long total)
{
	// Reset item counter for feedback
	mItemTotal = total;
	mItemCtr = 0;
	mStatusUpdateTime = ::time(NULL);
}

// Bump the item counter
void CINETClient::BumpItemCtr(const char* rsrcid)
{
	mItemCtr++;
	time_t now = ::time(NULL);

	if (::difftime(now, mStatusUpdateTime) > 0)
	{
		mStatusUpdateTime = now;
		DisplayItemCtr(rsrcid);
	}
}

// Check and update time since last status
void CINETClient::DisplayItemCtr(const char* rsrcid) const
{
	if (mItemTotal)
		CStatusWindow::SetIMAPStatus2(rsrcid, mItemCtr, mItemTotal);
	else
		CStatusWindow::SetIMAPStatus1(rsrcid, mItemCtr);
}

#pragma mark ____________________________Start/Stop

// Start TCP
void CINETClient::Open()
{
	// Only if we have a stream
	if (!mStream)
		return;

	try
	{
		// Start action
		StINETClientAction status(this, "Status::INET::Connecting", "Error::INET::OSErrOpen", "Error::INET::NoBadOpen");

		// Init TCP
		mStream->TCPOpen();

		// Set server TCP info if not already
		LookupServer();

		// Create buffers
		mLineData = new char[cINETBufferLen];

		// Create log entry
		mLog.StartLog(mLogType, mServerAddr);
	}
	catch (std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		mStream->TCPClose();

		Close();
		mServerAddr = cdstring::null_str;

		// Handle error
		INETDisplayError(ex, "Error::INET::OSErrOpen", "Error::INET::NoBadOpen");

		// Set status
		CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");

		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		mStream->TCPClose();

		Close();
		mServerAddr = cdstring::null_str;

		// Handle error
		INETDisplayError(CINETException::err_INETUnknown, "Error::INET::OSErrOpen", "Error::INET::NoBadOpen");

		// Set status
		CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");

		CLOG_LOGRETHROW;
		throw;
	}

}

// Reset acount info
void CINETClient::Reset()
{
	// Does nothing
}

// Release TCP
void CINETClient::Close()
{
	// Delete buffers
	delete mLineData;
	mLineData = NULL;

	// Set status
	CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");

	// Last log entry
	mLog.StopLog();

	// Stop busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	//::StopSpinning();
#elif __dest_os == __win32_os
	CMulberryApp::sApp->EndWaitCursor();
#elif __dest_os == __linux_os
	//Busy cursor will turn itself off
#else
#error __dest_os
#endif

}

// Program initiated abort
void CINETClient::Abort()
{
	// Just set flag in stream-socket and allow it to abort itself
	if (mStream)
		mStream->SetAbort();
}

// Forced close
void CINETClient::Forceoff()
{
}

// Lookup IMAP server
void CINETClient::LookupServer()
{
	// Only if we have a stream
	if (!mStream)
		return;

	// reset server address if owner exists (it may not e.g. in password plugins)
	cdstring desc;
	if (mOwner)
	{
		mServerAddr = GetAccount()->GetServerIP();

		// Socket descriptor is server address, authenticator descriptor and TLS type
		desc = mServerAddr;
		desc += GetAccount()->GetAuthenticator().GetDescriptor();
		desc += cdstring((long) GetAccount()->GetTLSType());
	}
	else
		// If no owner just use existing server address - it must have already been set
		desc = mServerAddr;

	// Set server TCP info if not already
	if (mStream->GetDescriptor() != desc)
	{
		mStream->SetDescriptor(desc);

		// Find out whether reverse lookup is required
		bool need_cname = false;
		if (mOwner)
		{
			switch(GetAccount()->GetAuthenticatorType())
			{
			case CAuthenticator::eNone:			// Actually ANONYMOUS!
			case CAuthenticator::ePlainText:
			case CAuthenticator::eSSL:			// Actually EXTERNAL
			default:;
				break;

			// These ones do AUTHENTICATE processing via plugin
			case CAuthenticator::ePlugin:
				{
					// See if plugin wants cname
					CAuthPlugin* plugin	= GetAccount()->GetAuthenticator().GetPlugin();
					need_cname = plugin ? plugin->NeedCNAME() : false;
				}
				break;
			}
		}

		try
		{
			// Specify remote ip addr (will do reverse lookup if required by auth plugin)
			mStream->TCPSpecifyRemoteName(mServerAddr, GetDefaultPort(), need_cname);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Force invalid descriptor to make sure its looked up again
			mStream->SetDescriptor(cdstring::null_str);

			CLOG_LOGRETHROW;
			throw;
		}
	}

}

#pragma mark ____________________________Login & Logout

// Logon to IMAP server
void CINETClient::Logon()
{
	// Only if we have a stream
	if (!mStream)
		return;

	//OSErr err = noErr;
	bool connection_up = false;
	bool pre_auth = false;
	bool auth_done = false;
	bool redo_dns = true;

	try
	{
		// Start action
		StINETClientAction status(this, "Status::INET::LoggingOn", "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

		// Force tag to zero
		mTagCount = 0;

		// Init TCP
		mStream->TCPOpen();

		// Set server TCP info if not already
		LookupServer();

		// Look for SSL and turn on here
		if (mOwner && ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
					   (GetAccount()->GetTLSType() == CINETAccount::eSSLv3)))
		{
			mStream->TLSSetTLSOn(true, GetAccount()->GetTLSType());
			
			// Check for client cert
			if (GetAccount()->GetUseTLSClientCert())
			{
				// Try to load client certificate
				if (!DoTLSClientCertificate())
				{
					CLOG_LOGTHROW(unsigned long, 1UL);
					throw 1UL;
				}
			}
		}
		else
			mStream->TLSSetTLSOn(false);

		// Make connection with server
		mStream->TCPStartConnection();

		// Flag connection
		connection_up = true;

		if (!_ProcessGreeting())
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
			throw CINETException(CINETException::err_BadResponse);
		}
		redo_dns = false;

		pre_auth = INETCompareResponse(cStarPREAUTH);

		// Check capability
		_Capability();

		// Look for TLS and do STARTTLS
		if (mOwner &&
			((GetAccount()->GetTLSType() == CINETAccount::eTLS) ||
			 (GetAccount()->GetTLSType() == CINETAccount::eTLSBroken)))
		{
			// Check for client cert
			if (GetAccount()->GetUseTLSClientCert())
			{
				// Try to load client certificate
				if (!DoTLSClientCertificate())
				{
					CLOG_LOGTHROW(unsigned long, 1UL);
					throw 1UL;
				}
			}
			
			DoStartTLS();
		}

		// Only logon if not aleady
		if (!pre_auth)
			auth_done = DoAuthentication();
		else
			auth_done = true;
	}
	catch(unsigned long /*num*/)
	{
		CLOG_LOGCATCH(unsigned long num);

		// Fall through and treat like a failed auth
	}
	catch (std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Restore logging
		mAllowLog = true;

		// Handle error
		INETDisplayError(ex, "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

		// Disconnect if connected
		if (connection_up)
			mStream->TCPCloseConnection();

		// For connection failure lets always force the client to re-do its DNS lookup
		if (redo_dns)
			mStream->SetDescriptor(cdstring::null_str);

		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Restore logging
		mAllowLog = true;

		// Handle error
		INETDisplayError(CINETException::err_INETUnknown, "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

		// Disconnect if connected
		if (connection_up)
			mStream->TCPCloseConnection();

		// For connection failure lets always force the client to re-do its DNS lookup
		if (redo_dns)
			mStream->SetDescriptor(cdstring::null_str);

		CLOG_LOGRETHROW;
		throw;
	}

	// Check for cancel of authentication and throw out
	if (!auth_done)
	{
		// Disconnect if connected
		if (connection_up)
			mStream->TCPCloseConnection();

		CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
		throw CINETException(CINETException::err_BadResponse);
	}
}

// Get default port
tcp_port CINETClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	return 0;
}

// Get name of command for SASL
const char* CINETClient::GetAuthCommand() const
{
	return cAUTHENTICATE;
}

// Get name of command for TLS
const char* CINETClient::GetStartTLSCommand() const
{
	// Most protocols use STARTTLS, but POP3 doesn't
	return cSTARTTLS;
}

// Setup TLS certificate
bool CINETClient::DoTLSClientCertificate()
{
	bool result = false;

	// Only if we have a stream and account
	if (!mStream || !mOwner)
		return false;

	// Get client certificate name from account
	cdstring certfingerprint = GetAccount()->GetTLSClientCert();
	
	// Get the subject of the cert with this fingerprint
	cdstring certname;
	if (!CCertificateManager::sCertificateManager->GetSubject(certfingerprint, certname, CCertificateManager::eByFingerprint))
		return false;
	
	// Pause busy cursor before we do the prompt
	bool paused = INETPauseAction(true);
	
	// Get a passphrase for this certificate
	cdstrvect users;
	users.push_back(certname);
	const char** user_list = cdstring::ToArray(users);

	cdstring passphrase;
	passphrase.reserve(512);
	unsigned long chosen;
	while(true)
	{
		if (CSecurityPlugin::GetPassphrase(user_list, passphrase.c_str_mod(), chosen))
		{
			// Try to load private key (this will verify that the password etc is valid)
			if (mStream->TLSSetClientCert(certfingerprint, passphrase))
			{
				result = true;
				break;
			}

			// Display cert error alert
			CTCPException ex(CTCPException::err_TCPSSLClientCertLoad);
			INETDisplayError(ex, "Error::INET::OSErrTLSClientCert", "Error::INET::NoBadTLSClientCert");
			
			// Clear password cache
			CSecurityPlugin::ClearLastPassphrase();
		}
		else
			break;
	}

	// Clean-up	
	cdstring::FreeArray(user_list);

	// Restart busy cursor when dialog completes
	if (paused)
		paused = INETPauseAction(false);

	return result;
}

// Start TLS
void CINETClient::DoStartTLS()
{
	// Only if we have a stream
	if (!mStream)
		return;

	// Issue STARTTLS command
	INETStartSend("Status::INET::StartTLS", "Error::INET::OSErrStartTLS", "Error::INET::NoBadStartTLS");
	INETSendString(GetStartTLSCommand());
	INETFinishSend();

	// Now force TLS negotiation
	mStream->TLSSetTLSOn(true, GetAccount()->GetTLSType());
	mStream->TLSStartConnection();

	// Must redo capability after STARTTLS
	_Capability(true);

}

// Do authentication to server
bool CINETClient::DoAuthentication()
{
	bool first = true;
	bool done = false;

	// Loop while trying to authentciate
	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();

	// Determine type of display
	bool rmail = (typeid(*mOwner) == typeid(CMboxProtocol));
	bool rsmtp = false;
	bool rprefs = (typeid(*mOwner) == typeid(COptionsProtocol));
	bool radbk = (typeid(*mOwner) == typeid(CAdbkProtocol));
	bool rfilter = (typeid(*mOwner) == typeid(CFilterProtocol));
	bool rhttp = (typeid(*mOwner) == typeid(calstore::CCalendarProtocol));

	// Pause busy cursor before we do the prompt
	bool paused = INETPauseAction(true);
	
	// Loop while waiting for successful login with optional user prompt
	while(CMailControl::PromptUser(acct_auth, GetAccount(), GetAccount()->IsSecure(),
									rmail, rsmtp, rprefs, radbk, rfilter, rhttp, first))
	{
		// Restart busy cursor while login proceeds
		if (paused)
			paused = INETPauseAction(false);

		first = false;

		// Do authentication, but trap protocol failures
		try
		{
			switch(GetAccount()->GetAuthenticatorType())
			{
			case CAuthenticator::eNone:			// Actually ANONYMOUS!
				DoAnonymousAuthentication();
				break;

			case CAuthenticator::ePlainText:
				DoPlainAuthentication();
				break;

			case CAuthenticator::eSSL:
				DoExternalAuthentication();
				break;

			// These ones do AUTHENTICATE processing via plugin
			case CAuthenticator::ePlugin:
				DoPluginAuthentication();
				break;
			}
			done = true;
		}
		catch(CINETException& ex)
		{
			CLOG_LOGCATCH(CINETException&);

			// Display error to user
			if (!ex.handled())
				INETDisplayError(ex, "Error::INET::OSErrLogon", "Error::INET::NoBadLogon");

			// Force it to recycle
			done = false;
		}

		// Auth done - exit the loop
		if (done)
			break;

		// Pause busy cursor again before we do the prompt
		paused = INETPauseAction(true);
	}

	// Restart busy cursor when login completes
	if (paused)
		paused = INETPauseAction(false);
	
	return done;
}

// Do authentication to server directly
void CINETClient::DoAnonymousAuthentication()
{
	// Do not allow logging password
	StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Start command
	INETStartSend(NULL, NULL, NULL, cdstring::null_str, false);

	// Send password next
	if (mAuthAnonAllowed)
		INETSendString(GetAuthCommand(), eQueueNoFlags, false);
	else
		INETSendString(cLOGIN, eQueueNoFlags, false);
	INETSendString(cSpace, eQueueNoFlags, false);

	// AUTHENTICATE/LOGIN ANONYMOUS token
	INETSendString(cANONYMOUS, eQueueProcess, false);	// Process since ACAP requires quoted strings always
	
	// Some SASL profiles allow extra data with AUTHENTICATE others do not
	if (mAuthInitialClientData)
		INETSendString(cSpace, eQueueNoFlags, false);
	else
	{
		INETSendString(cCRLF, eQueueNoFlags, false);
		
		// Wait for continuation or failure (which throws)
		INETSendString(NULL, eQueueNoFlags, false);
	}

	// Form buffer of plain text SASL response
	// anonymous token - don't send user info for privacy reasons
	cdstring token = "anonymous@anonymous";

	// Some SASL profiles allow 'raw' data, others require base64
	if (!mAuthBase64)
	{
		// Add as string
		INETSendString(token, eQueueProcess, false);
	}
	else
	{
		cdstring b64;
		b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(token.c_str()), token.length()));
		INETSendString(b64, eQueueProcess, false);
	}

	// Issue first call
	INETFinishSend(false);
}

// Do authentication to server directly
void CINETClient::DoPlainAuthentication()
{
	// Do not allow logging password
	StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Start command
	INETStartSend(NULL, NULL, NULL, cdstring::null_str, false);

	// Send password next
	if (mLoginAllowed && !mAuthLoginAllowed && !mAuthPlainAllowed)
		INETSendString(cLOGIN, eQueueNoFlags, false);
	else
		INETSendString(GetAuthCommand(), eQueueNoFlags, false);
	INETSendString(cSpace, eQueueNoFlags, false);

	CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

	if (mLoginAllowed && !mAuthLoginAllowed && !mAuthPlainAllowed)
	{
		// LOGIN uid pswd
		INETSendString(auth->GetUID(), eQueueProcess, false);
		INETSendString(cSpace, eQueueNoFlags, false);
		INETSendString(auth->GetPswd(), eQueueProcess, false);
	}
	
	// Use AUTH=PLAIN ahead of AUTH=LOGIN
	else if (mAuthPlainAllowed)
	{
		// AUTHENTICATE PLAIN {..+}\0uid\0pswd
		INETSendString(cPLAIN, eQueueProcess, false);	// Process since ACAP requires quoted strings always
		
		// Some SASL profiles allow extra data wiyth AUTHENTICATE others do not
		if (mAuthInitialClientData)
			INETSendString(cSpace, eQueueNoFlags, false);
		else
		{
			INETSendString(cCRLF, eQueueNoFlags, false);
			
			// Wait for continuation of failure (which throws)
			INETSendString(NULL, eQueueNoFlags, false);
		}

		// Form buffer of plain text SASL response
		// \0userid\0pswd
		std::ostrstream buffer;
		size_t buflen = 0;
		buffer.write(reinterpret_cast<const char*>(&buflen), sizeof(size_t));
		buffer.put('\0');
		buffer << auth->GetUID();
		buffer.put('\0');
		buffer << auth->GetPswd();
		buflen = buffer.pcount() - sizeof(size_t);

		// Some SASL profiles allow 'raw' data, others require base64
		if (!mAuthBase64)
		{
			cdstring temp;
			temp.steal(buffer.str());
			*reinterpret_cast<unsigned long*>(temp.c_str_mod()) = buflen;

			// Add as buffer
			INETSendString(temp, eQueueBuffer, false);
		}
		else
		{
			cdstring temp;
			temp.steal(buffer.str());
			cdstring b64;
			b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(temp.c_str() + sizeof(size_t)), buflen));
			INETSendString(b64, eQueueProcess, false);
		}
	}
	
	// AUTH=LOGIN
	else if (mAuthLoginAllowed)
	{
		// AUTHENTICATE LOGIN
		INETSendString(cLOGIN, eQueueProcess, false);	// Process since ACAP requires quoted strings always
		INETSendString(cCRLF, eQueueNoFlags, false);
		
		// Wait for continuation of failure (which throws)
		INETSendString(NULL, eQueueNoFlags, false);
		
		// Send base64 encoded user id
		cdstring buffer = auth->GetUID();
		cdstring b64;
		b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length()));
		INETSendString(b64, eQueueProcess, false);
		INETSendString(cCRLF, eQueueNoFlags, false);
		
		// Wait for continuation of failure (which throws)
		INETSendString(NULL, eQueueNoFlags, false);
		
		// Send base64 encoded password
		buffer = auth->GetPswd();
		b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length()));
		INETSendString(b64, eQueueProcess, false);
		
		// Fall through to finish processing in normal fashion
	}

	// Issue first call
	INETFinishSend(false);
}

// Do authentication to server directly
void CINETClient::DoExternalAuthentication()
{
	// Do not allow logging password
	StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Start command
	INETStartSend(NULL, NULL, NULL, cdstring::null_str, false);

	// Send password next
	INETSendString(GetAuthCommand(), eQueueNoFlags, false);
	INETSendString(cSpace, eQueueNoFlags, false);

	// AUTHENTICATE EXTERNAL token
	INETSendString(cEXTERNAL, eQueueProcess, false);	// Process since ACAP requires quoted strings always
	
	// Some SASL profiles allow extra data with AUTHENTICATE others do not
	if (mAuthInitialClientData)
		INETSendString(cSpace, eQueueNoFlags, false);
	else
	{
		INETSendString(cCRLF, eQueueNoFlags, false);
		
		// Wait for continuation or failure (which throws)
		INETSendString(NULL, eQueueNoFlags, false);
	}

	// Form buffer of plain text SASL response
	// authorization id - unused right now
	cdstring token = "";

	// Some SASL profiles allow 'raw' data, others require base64
	if (!mAuthBase64)
	{
		// Add as string
		INETSendString(token, eQueueProcess, false);
	}
	else
	{
		cdstring b64;
		b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(token.c_str()), token.length()));
		INETSendString(b64, eQueueProcess, false);
	}

	// Issue first call
	INETFinishSend(false);
}

// Do authentication to server via plugin
void CINETClient::DoPluginAuthentication()
{
	// Only if we have a stream
	if (!mStream)
		return;

	// Find plugin
	cdstring plugin_name;
	CAuthPlugin* plugin	= GetAccount()->GetAuthenticator().GetPlugin();

	if (plugin)
	{
		cdstring capability;
		if (!plugin->DoAuthentication(&GetAccount()->GetAuthenticator(),
									GetAccount()->GetServerType(),
									GetAccount()->GetServerTypeString(),
									*mStream, mLog, mLineData, cINETBufferLen,
									capability))
		{
			const char* p = mLineData;

			// Bump past tag & space "a " if there
			if (::strncmp(p, "a ", 2) == 0)
				p += 2;

			// Fake response
			mLastResponse.code = cTagNO;
			mLastResponse.tag_msg = p;
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
		
		// Handle capability
		if (!capability.empty())
		{
			// Store capability string
			_InitCapability();
			capability.erase(0, 13);
			mCapability = capability;
			
			mLastResponse.code = cStarCAPABILITY;
			mLastResponse.AddUntagged(capability);
			
			_ProcessCapability();
		}
	}
	else
	{
		// Fake bad response
		mLastResponse.code = cTagBAD;
		mLastResponse.tag_msg = "Authentication plugin \"";
		mLastResponse.tag_msg += GetAccount()->GetAuthenticator().GetDescriptor();
		mLastResponse.tag_msg += "\" not found";
		CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
		throw CINETException(CINETException::err_BadResponse);
	}
}

// Logoff from IMAP server
void CINETClient::Logoff()
{
	// Only if we have a stream
	if (!mStream)
		return;

	try
	{
		// Issue logout call
		INETStartSend("Status::INET::LoggingOut", "Error::INET::OSErrLogout", "Error::INET::NoBadLogout");
		INETSendString(cLOGOUT);
		INETFinishSend();

		// Break connection with server
		mStream->TCPCloseConnection();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Set status
		CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");
		CLOG_LOGRETHROW;
		throw;
	}

	// Set status
	CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");

}

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CINETClient::_Tickle(bool force_tickle)
{
	// Only if we have a stream
	if (!mStream)
		return;

	// Intervals
	unsigned long tickleInterval = CPreferences::sPrefs ? CPreferences::sPrefs->mTickleInterval.GetValue() : 25 * 60;
	unsigned long tickleIntervalExpireTimeout = CPreferences::sPrefs ? CPreferences::sPrefs->mTickleIntervalExpireTimeout.GetValue() : 5 * 60;

	// Current time plus timeout
	time_t curr_time = ::time(NULL);
	double timeout = ::difftime(curr_time, mStream->GetTimer());

	// Tickle timeout too large check
	if (timeout > tickleInterval + tickleIntervalExpireTimeout)
	{
		// Write to log file
		mLog.LogEntry("Tickle Abort due to excess idle time");

		// Force abort and reconnect
		Abort();

		// Prevent changing status which is already set
		StINETClientSilent silent(this);

		// Just do NOOP to trigger abort and recovery
		INETStartSend(NULL, NULL, NULL);
		INETSendString(cNOOP);
		INETFinishSend();
	}

	// Timeout check
	if (force_tickle || (timeout > tickleInterval))
	{
		// Write to log file
		mLog.LogEntry("Tickle CHECK next:");

		// Prevent changing status which is already set
		StINETClientSilent silent(this);

		// Just do NOOP
		INETStartSend(NULL, NULL, NULL);
		INETSendString(cNOOP);
		INETFinishSend();
	}

}

// Process greeting response
bool CINETClient::_ProcessGreeting()
{
	// Receive first info from server
	mStream->qgetline(mLineData, cINETBufferLen);

	// Store in greeting
	mGreeting = mLineData;

	// Write to log file
	mLog.LogEntry(mLineData);

	bool ok = INETCompareResponse(cStarOK) || INETCompareResponse(cTagOK) || INETCompareResponse(cStarPREAUTH);

	return ok;
}

// Check version of server
void CINETClient::_Capability(bool after_tls)
{
	// Try capability command - do not allow failure
	try
	{
		// Prevent changing status which is already set
		StINETClientSilent silent(this);

		// Clear out existng capabilities before issuing command again
		_InitCapability();

		// Issue first call
		INETStartSend(NULL, NULL, NULL, cdstring::null_str, false);
		INETSendString(cCAPABILITY, eQueueNoFlags, false);
		INETFinishSend(false);

		_ProcessCapability();
	}

	// Trap NO or BAD response
	catch (CINETException& ex)
	{
		CLOG_LOGCATCH(CINETException&);

		if ((ex.error() == CINETException::err_NoResponse) ||
			(ex.error() == CINETException::err_BadResponse))
		{
			_NoCapability();
		}
		else
		{
			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
	}

}

// Authenticate user
void CINETClient::_Authenticate()
{
	// Does nothing for now

	// Eventually this will use a plug-in authentication mechanism

}

#pragma mark ____________________________Status

// Starting an INET call
void CINETClient::INETStartAction(const char* status_id, const char* err_id, const char* nobad_id, const cdstring& err_context)
{
	// Only if not silent and status done
	if (!mSilentCommand && (status_id != NULL))
	{
		// Cache the error context for later use
		mErrContext  = err_context;

		// Set status
		CStatusWindow::SetIMAPStatus(status_id);

		// Start busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Only do this in the main thread
		//if (LThread::InMainThread())
		//	::BeginResSpinning(crsr_StdSpin);
#elif __dest_os == __win32_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CMulberryApp::sApp->BeginWaitCursor();
#elif __dest_os == __linux_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CWaitCursor::StartBusyCursor();
#else
#error __dest_os
#endif

		mStatusQueue++;
		mStatusID = status_id;
		mErrorID = err_id;
		mNoBadID = nobad_id;
	}

	// Set busy status info
	{
		// Status string
		cdstring status;
		if (status_id != NULL)
			status = rsrc::GetString(status_id);
	
		// Append Error context
		INETGetErrorContext(status);
		
		// Set this as the busy indicator
		if (mStream)
			mStream->SetBusyDescriptor(status);
	}
}

// Pause UI busy
bool CINETClient::INETPauseAction(bool pause)
{
	if (pause && mStatusQueue)
	{
		// Turn off busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Only do this in the main thread
		//if (LThread::InMainThread())
		//	::StopSpinning();
#elif __dest_os == __win32_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CMulberryApp::sApp->EndWaitCursor();
#elif __dest_os == __linux_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CWaitCursor::StopBusyCursor();
#else
#error __dest_os
#endif
		return true;
	}
	else if (!pause && mStatusQueue)
	{
		// Turn on busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Only do this in the main thread
		//if (LThread::InMainThread())
		//	::BeginResSpinning(crsr_StdSpin);
#elif __dest_os == __win32_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CMulberryApp::sApp->BeginWaitCursor();
#elif __dest_os == __linux_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CWaitCursor::StartBusyCursor();
#else
#error __dest_os
#endif
		return false;
	}
	else
		return false;
}

// Stopping an IMAP call
void CINETClient::INETStopAction()
{
	// Only if not silent and status done previously
	if (!mSilentCommand && (mStatusID != NULL))
	{
		// Set status
		CStatusWindow::SetIMAPStatus("Status::IDLE");

		// Stop busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Only do this in the main thread
		//if (LThread::InMainThread())
		//	::StopSpinning();
#elif __dest_os == __win32_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CMulberryApp::sApp->EndWaitCursor();
#elif __dest_os == __linux_os
		// Only do in main thread
		if (CTaskQueue::sTaskQueue.InMainThread())
			CWaitCursor::StopBusyCursor();
#else
#error __dest_os
#endif

		// Only zero these when queue is empty
		if (!--mStatusQueue)
		{
			mStatusID = NULL;
			mErrorID = NULL;
			mNoBadID = NULL;
		}
	}

}

#pragma mark ____________________________Receive Data

// Get a line of data - possibly long
char* CINETClient::INETGetLine()
{
	// Only if we have a stream
	if (!mStream)
		return mLineData;

	// Delete any existing long line buffer
	if (mLongLine)
	{
		delete mLongLine;
		mLongLine = NULL;
	}

	// Now get a line of text
	mStream->qgetline(mLineData, cINETBufferLen);

	// Check for overrun
	if (mStream->fail())
	{
		char* so_far = mLineData;
		char* next = NULL;
		unsigned long len = cINETBufferLen;
		do
		{
			// Current length
			unsigned long current_len;
			current_len = len;

			// Increment buffer size
			len += cINETBufferLen;

			// Create new buffer
			next = new char[len];

			// Copy old to new
			::memcpy(next, so_far, current_len);

			// Try to get some more (after clearing existing error)
			// NB Remember that previous line was terminated with '\0' which we dont now need
			mStream->clear();
			mStream->qgetline(next + current_len - 1, cINETBufferLen + 1);

			if (so_far != mLineData)
				delete so_far;
			so_far = next;

		} while (mStream->fail());

		mLongLine = so_far;
		return mLongLine;
	}
	else
		return mLineData;
}

// Process IMAP request
void CINETClient::INETProcess()
{
	// Clear any previous response
	mLastResponse.Clear();

	// Do pre-processing actions
	_PreProcess();

	// Wait for tag to arrive
	do
	{
		// Now get a line of text
		char* txt = INETGetLine();

		// Write to log file
		if (mAllowLog)
			mLog.AddEntry(txt);

		// Parse received string and act on it
		INETParseResponse(&txt, &mLastResponse);

		// Clean up any long lines
		if (mLongLine)
		{
			delete mLongLine;
			mLongLine = NULL;
		}

	// Tag or continuation must end loop
	} while (!TAG_TEST(mLastResponse.code) && !PLUS_TEST(mLastResponse.code));

	// Look for special unsolicited responses
	while(mLastResponse.CheckUntagged(cALERT))
	{
		const cdstring alert = mLastResponse.PopUntagged(cALERT);
		char* alrt_msg = ::strstrnocase(alert, cALERT);
		if (alrt_msg)
		{
			alrt_msg += ::strlen(cALERT) + 1;
			cdstring txt(alrt_msg, ::strlen(alrt_msg));
			CMailControl::PushAlert(txt);
		}
	}

	// Do post processing actions
	// Should do this BEFORE processing NO and BAD since mailbox updates etc must be processed
	_PostProcess();

	// Make sure the result was OK
	if (!INETCheckLastResponse(cTagOK) && !INETCheckLastResponse(cPlusLabel))
	{
		if (INETCheckLastResponse(cTagNO))
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
			throw CINETException(CINETException::err_NoResponse);
		}
		else
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
			throw CINETException(CINETException::err_BadResponse);
		}
	}

}

#pragma mark ____________________________Send Data

// Update tag
void CINETClient::INETNextTag()
{
	// Increment tag counter
	mTagCount++;

	// Create tag string
	::snprintf(mTag, 16, "%c%05ld", cTAG_CHAR, mTagCount);

}

// Start sending a new command
void CINETClient::INETStartSend(const char* status_id, const char* err_id, const char* nobad_id, const cdstring& err_context, bool handle_throw)
{
	// Does action status
	INETStartAction(status_id, err_id, nobad_id, err_context);

	// Only if processing tags
	if (mUseTag)
	{
		// Get next tag
		INETNextTag();

		// Push onto front of queue
		mLog.StartPartialEntry();
		INETSendString(mTag, eQueueNoFlags, handle_throw);
		INETSendString(cSpace, eQueueNoFlags, handle_throw);
	}
}

// Send a string
void CINETClient::INETSendString(const char* str, int flags, bool handle_throw)
{
	// Only if we have a stream
	if (!mStream)
		return;

	try
	{
		// NULL input that's not a manual literal => flush and process server response
		if (str || (flags & eQueueManualLiteral))
			SendString(str, flags);
		else
		{
			*mStream << std::flush;

			// Process response
			INETProcess();
		}
	}
	catch (std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		const char* err_id = mErrorID;
		const char* nobad_id = mNoBadID;

		// Stop UI action on failure
		INETStopAction();

		// Handle error
		if (handle_throw)
			INETHandleError(ex, err_id, nobad_id);

		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		const char* err_id = mErrorID;
		const char* nobad_id = mNoBadID;

		// Stop UI action on failure
		INETStopAction();

		// Handle error
		if (handle_throw)
		{
			CINETException iex(CINETException::err_INETUnknown);
			INETHandleError(iex, err_id, nobad_id);
		}

		CLOG_LOGRETHROW;
		throw;
	}
}

// Finish sending string
void CINETClient::INETFinishSend(bool handle_throw)
{
	// Send CRLF and flush, then process response
	mLog.StopPartialEntry();
	INETSendString(cCRLF, eQueueNoFlags, handle_throw);
	INETSendString(NULL, eQueueNoFlags, handle_throw);

	// Stop UI action on success
	INETStopAction();
}

#pragma mark ____________________________Handle Response

// Look for a specific response
bool CINETClient::INETCompareResponse(EINETResponseCode compare)
{
	char* p = mLineData;

	// Clear any previous response
	mLastResponse.Clear();

	// Parse received string to determine nature of response
	INETParseResponse(&p, &mLastResponse);

	return (mLastResponse.code == compare);

}

// Check last response
bool CINETClient::INETCheckLastResponse(EINETResponseCode compare)
{
	return (mLastResponse.code==compare);

}

// Parse text sent by server (advance pointer to next bit to be parsed)
void CINETClient::INETParseResponse(char** txt, CINETClientResponse* response)
{
	// Clear response message first
	response->tag_msg = cdstring::null_str;

	// Look for tags first
	if (!mUseTag || (::stradvtokcmp(txt, mTag) == 0))
		INETParseTagged(txt, response);

	// Look for star responses next
	else if (::stradvtokcmp(txt,cSTAR) == 0)
	{

		// Look for specific star responses
		if (::stradvtokcmp(txt,cOK) == 0)
		{
			response->code = cStarOK;
			response->AddUntagged(*txt);
		}
		else if (::stradvtokcmp(txt,cNO) == 0)
		{
			response->code = cStarNO;
			response->AddUntagged(*txt);
		}
		else if (::stradvtokcmp(txt,cBAD) == 0)
		{
			response->code = cStarBAD;
			response->AddUntagged(*txt);
		}

		else if (::stradvtokcmp(txt,cPREAUTH) == 0)
		{
			response->code = cStarPREAUTH;
			response->AddUntagged(*txt);
		}

		else if (::stradvtokcmp(txt,cCAPABILITY)==0)
		{
			response->code = cStarCAPABILITY;
			response->AddUntagged(*txt);

			// Store capability string
			mCapability = *txt;
		}

		else if (::stradvtokcmp(txt,cBYE)==0)
		{
			response->code = cStarBYE;
			response->tag_msg = *txt;
			response->AddUntagged(*txt);
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
}

// Parse tagged response
void CINETClient::INETParseTagged(char** txt, CINETClientResponse* response)
{
	// Look for specific tag responses
	if (::stradvtokcmp(txt,cOK) == 0)
		response->code = cTagOK;
	else if (::stradvtokcmp(txt,cNO) == 0)
		response->code = cTagNO;
	else if (::stradvtokcmp(txt,cBAD) == 0)
		response->code = cTagBAD;
	// Do protocols processing
	else if (mDoesTaggedResponses)
		_ParseResponse(txt, response);
	else
		response->code = cResponseError;

	// Copy remaining tag message
	if (response->code != cResponseError)
		// Copy the error message text
		response->tag_msg = *txt;
}

#pragma mark ____________________________Handle Errors

// Handle an error condition
void CINETClient::INETHandleError(std::exception& ex, const char* err_id, const char* nobad_id)
{
	// Always make sure outstanding alerts are processed before attempting any reconnects etc
	while(mLastResponse.CheckUntagged(cALERT))
	{
		const cdstring alert = mLastResponse.PopUntagged(cALERT);
		char* alrt_msg = ::strstrnocase(alert, cALERT);
		if (alrt_msg)
		{
			alrt_msg += ::strlen(cALERT) + 1;
			cdstring txt(alrt_msg, ::strlen(alrt_msg));
			CMailControl::PushAlert(txt);
		}
	}

	bool recovered = false;
	bool failed_recovery = false;

	// Check for network or general exception
	CNetworkException* nex = dynamic_cast<CNetworkException*>(&ex);
	//CGeneralException* gex = dynamic_cast<CGeneralException*>(&ex);

	// Handle network exception
	if (nex)
	{
		try
		{
			// See if recovery is possible before presenting an alert to the user
			INETTryRecoverError(*nex, err_id, nobad_id);
			
			recovered = true;
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Catch any recovery error
			
			// use this to force the recover disconnect
			failed_recovery = true;
		}
	}

	// Handle general/unknown exception or failed recovery
	if (!recovered)
	{
		// Recovery is not possible, so...

		// Display alert to user
		INETDisplayError(ex, err_id, nobad_id);
		
		// Error has been reported once so don't let it happen
		// again during the current recovery process
		StValueChanger<bool> value(mSilentError, true);

		// At the moment force the server closed by killing the connection if one present
		if (mStream && (mStream->TCPGetState() >= CTCPSocket::TCPConnected))
			mStream->TCPAbort();

		// Force servers to closed state
		if (failed_recovery || !mOwner || mOwner->IsOpen() || mOwner->IsLoggedOn())
		{
			// Force disconnect cleanup
			if (mOwner)
				mOwner->SetState(CINETProtocol::eINETNotOpen);
			INETRecoverDisconnect();

			// Flag exception as diconnected
			if (nex)
				nex->setdisconnect();
		}
	}
	
	// Mark as handled
	if (nex)
		nex->sethandled();
}

// Handle an error condition
void CINETClient::INETDisplayError(std::exception& ex, const char* err_id, const char* nobad_id)
{
	// Only handle if not silent
	if (mSilentError) return;

	// Tell owner in error state
	if (mOwner)
		mOwner->SetErrorProcess(true);

	// See whether oner wants an alert
	if (mOwner && mOwner->GetNoErrorAlert())
		return;

	// Check for network or general exception
	CNetworkException* nex = dynamic_cast<CNetworkException*>(&ex);
	CTCPException* tex = dynamic_cast<CTCPException*>(&ex);
	CINETException* iex = dynamic_cast<CINETException*>(&ex);
	CGeneralException* gex = dynamic_cast<CGeneralException*>(&ex);

	// Route generic network exception to appropriate handler
	if (iex)
		INETDisplayError(*iex, err_id, nobad_id);
	else if (tex)
		INETDisplayError(*tex, err_id, nobad_id);
	else if (nex)
		INETDisplayError(nex->error(), err_id, nobad_id);
	else if (gex)
		INETDisplayError(gex->GetErrorCode(), err_id, nobad_id);

}

// Handle an error condition
void CINETClient::INETDisplayError(CINETException& ex, const char* err_id, const char* nobad_id)
{
	// Get Error context
	cdstring err_context;
	INETGetErrorContext(err_context);

	// Handle protocol errors
	switch(ex.error())
	{
	case CINETException::err_BadParse:
		{
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask("Error::INET::BadParse", err_context);
			task->Go();
		}
		break;
	case CINETException::err_NoResponse:
	case CINETException::err_BadResponse:
	default:
		{
			cdstring err = mLastResponse.tag_msg;
			err += err_context;
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(nobad_id, err);
			task->Go();
		}
		break;
	}
}

// Handle an error condition
void CINETClient::INETDisplayError(CTCPException& ex, const char* err_id, const char* nobad_id)
{
	// Get Error context
	cdstring err_context;
	INETGetErrorContext(err_context);

	// Handle network errors
	const char* mapped_errid = err_id;
	switch(ex.error())
	{
	case CTCPException::err_TCPAbort:
	case CTCPException::err_TCPFailed:
		mapped_errid = "Error::INET::ConnectionAborted";
		break;
	case CTCPException::err_TCPNoSSLPlugin:
		mapped_errid = "Error::INET::NoSSLPlugin";
		break;
	case CTCPException::err_TCPSSLError:
		mapped_errid = "Error::INET::NoSSLError";
		break;
	case CTCPException::err_TCPSSLCertError:
		mapped_errid = "Error::INET::NoSSLCertError";
		break;
	case CTCPException::err_TCPSSLCertNoAccept:
		mapped_errid = "Error::INET::NoSSLCertNoAccept";
		break;
	case CTCPException::err_TCPSSLClientCertLoad:
		mapped_errid = "Error::INET::OSErrTLSClientCert";
		break;
	default:
		{
			COSErrAlertRsrcTxtTask* task = new COSErrAlertRsrcTxtTask(err_id, ex.error(), err_context);
			task->Go();
		}
		return;
	}

	CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(mapped_errid, err_context);
	task->Go();
}

// Handle an error condition
void CINETClient::INETDisplayError(ExceptionCode err, const char* err_id, const char* nobad_id)
{
	// Get Error context
	cdstring err_context;
	INETGetErrorContext(err_context);

	// Make NULL pointers look like memory full
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (err == err_NilPointer)
		err = memFullErr;
#endif

	// Handle SysErr
	COSErrAlertRsrcTxtTask* task = new COSErrAlertRsrcTxtTask(err_id, err, err_context);
	task->Go();
}

// Recover from an error condition
void CINETClient::INETTryRecoverError(CNetworkException& ex, const char* err_id, const char* nobad_id)
{
	// Only recover if that's what the owner expects
	if (!mOwner || mOwner->GetNoRecovery())
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}

	switch(ex.error())
	{
	case CINETException::err_NoResponse:
	case CINETException::err_BadResponse:
		// No need to do anything as server state OK

		// Display alert to user
		INETDisplayError(ex, err_id, nobad_id);

		// Tell owner not in error state as recovery succeeded
		if (mOwner)
			mOwner->SetErrorProcess(false);
		return;

	case CINETException::err_BadParse:
		// Force connection closed
		if (mStream)
			mStream->TCPAbort();
		// Cannot recover from this
		break;

	case CINETException::err_INETUnknown:

		// Force connection closed
		if (mStream)
			mStream->TCPAbort();

		// Fall through to handle aborted state
	case CTCPException::err_TCPAbort:
	default:	// Catch all standard TCP/IP errors
		// Only try recovery if previously logged on
		if (mOwner && mOwner->IsLoggedOn() && mStream)
		{
			StValueChanger<bool> value(mSilentError, true);

			// Init TCP
			mStream->TCPOpen();

			// Try to recover state
			INETRecoverReconnect();

			// Flag exception as diconnected
			ex.setreconnect();
		}

		// Tell owner not in error state as recovery succeeded
		if (mOwner)
			mOwner->SetErrorProcess(false);
		return;

	case CTCPException::err_TCPFailed:
		// Cannot recover from this
		break;
	}

	// If we get here we cannot recover
	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);

}

// Force reconnect
void CINETClient::INETRecoverReconnect()
{
}

// Force disconnect
void CINETClient::INETRecoverDisconnect()
{
}

// Get error context string
void CINETClient::INETGetErrorContext(cdstring& error) const
{
	// Add account descriptor
	error += os_endl;
	if (mOwner)
	{
		error += os_endl;
		error += "Account: ";
		error += mOwner->GetAccountName();
	}
	
	// Add object descriptor if present
	if (!mErrContext.empty())
	{
		error += os_endl;
		error += INETGetErrorDescriptor();
		error += mErrContext;
	}
}

// Descriptor for object error context
const char*	CINETClient::INETGetErrorDescriptor() const
{
	return cdstring::null_str;
}

#pragma mark ____________________________Parsing

// Parse INET multi-valued string reply
void CINETClient::INETParseMultiString(char** txt, cdstrvect& strs, bool nullify)
{
	char* p = *txt;

	// Remove leading white space
	while(*p == ' ') p++;

	// Look for '('
	if (*p == '(')
	{
		p++;

		// Parse list of strings
		while(*p && (*p != ')'))
		{
			strs.push_back(cdstring::null_str);
			strs.back().steal(INETParseString(&p, nullify));
			while(*p == ' ') p++;
		}
		if (*p) p++;
	}
	else
	{
		strs.push_back(cdstring::null_str);
		strs.back().steal(INETParseString(&p, nullify));
	}

	// Readjust pointer
	*txt = p;
}

// Parse IMAP string/NULL reply
char* CINETClient::INETParseString(char** txt, bool nullify)
{
	// Only if we have a stream
	if (!mStream)
		return NULL;

	char*	p = *txt;
	char*	msg_txt = NULL;

	// Remove leading white space
	while(*p == ' ') p++;

	// If its NIL return null pointer and update text ptr
	if ((*p == 'N') && (*(p+1) == 'I') && (*(p+2) == 'L') && (!*(p+3) || ::strchr(INETGetAtomSpecials(), *(p+3))))
	{
		*txt = p+3;
		return NULL;
	}

	// Is it a literal (ie {###} ..... )
	if (*p=='{')
	{
		try
		{
			// Match the bracket (fail if no match) update text ptr
			p = ::strmatchbra(txt);
			if (!p)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}

			// Get length of text
			long length = ::strtol(p, NULL, 10);

			// Create new pointer for text (remember +1 for \0)
			msg_txt = new char[length+1];

			// Get data of specified size from TCP
			// NB Only read if length != 0 otherwise we get an exception
			if (length)
				mStream->read(msg_txt, length);
			msg_txt[length] = 0;

			// Must replace NULLs if required
			if (nullify)
			{
				char* p2 = msg_txt;
				while(length--)
				{
					if (!*p2) *p2 = 1;
					p2++;
				}
			}

			// Write to log file
			if (mAllowLog)
				mLog.AddEntry(msg_txt);

			// Now get another line of text and carry on processing
			*txt = INETGetLine();

			// Write to log file
			if (mAllowLog)
				mLog.AddEntry(*txt);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up and throw up
			delete msg_txt;
			CLOG_LOGRETHROW;
			throw;
		}
		return msg_txt;

	}
	else
	{
		// Duplicate atom/quoted string
		*txt = p;
		return ::strduptokenstr(txt, INETGetAtomSpecials());;
	}

}

// Parse IMAP string/NULL reply
void CINETClient::INETParseStringStream(char** txt)
{
	// Only if we have a stream
	if (!mStream)
		return;

	char* p= *txt;

	// Remove leading white space
	while(*p == ' ') p++;

	// If its NIL return null pointer and update text ptr
	if ((*p == 'N') && (*(p+1) == 'I') && (*(p+2) == 'L') && (!*(p+3) || ::strchr(INETGetAtomSpecials(), *(p+3))))
	{
		*txt = p+3;
		return;
	}

	// Is it a literal (ie {###} ..... )
	if (*p=='{')
	{
		// Match the bracket (fail if no match) update text ptr
		p = ::strmatchbra(txt);
		if (!p)
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}

		// Get length of text
		long length = ::strtol(p, NULL, 10);

		// Create network status item for % progress counter
		CNetworkAttachProgress progress;

		// Get data of specified size from TCP
		if (mRcvStream)
			mStream->gettostream(*mRcvStream, NULL, &length, &progress);
		else if (mRcvOStream)
		{
			// May need to filter
			if (mRcvOStream->IsNetworkType())
				mStream->gettostream(mRcvOStream->Stream(), NULL, &length, &progress);
			else
			{
				CStreamFilter filter(new crlf_filterbuf(mRcvOStream->GetEndlType()), mRcvOStream->GetStream());
				mStream->gettostream(filter, NULL, &length, &progress);
			}
		}

		// Now get another line of text and carry on processing
		*txt = INETGetLine();

		// Write to log file
		if (mAllowLog)
			mLog.AddEntry(*txt);

	}
	else
	{
		// Get atom/quoted string
		p = ::strgettokenstr(txt, INETGetAtomSpecials());

		// Send data to stream
		if (p)
		{
			if (mRcvStream)
				mRcvStream->WriteBlock(p, ::strlen(p));
			else if (mRcvOStream)
				mRcvOStream->Stream().write(p, ::strlen(p));
		}
	}

}

// Get ATOM special characters for parsing
const char* CINETClient::INETGetAtomSpecials() const
{
	// This is the default from RFC3501 (IMAP)
	return "\t\n\r \"%()*\\{";
}
