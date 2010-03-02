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


// CLDAPClient

// Class that does LDAP lookups

#include "CLDAPClient.h"

#include "CINETClient.h"
#include "CINETProtocol.h"
#include "CMailControl.h"
#include "CMulberryApp.h"
#include "CPasswordManager.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CTCPSocket.h"
#include "CXStringResources.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#elif __dest_os == __linux_os
#include "CWaitCursor.h"
#define USE_OPENLDAP
#define LDAP_DEPRECATED	1
#endif

#if __dest_os == __mac_os || __dest_os == __win32_os
#define __winsock
#endif

#include <algorithm>

#ifndef USE_OPENLDAP
#define MACOS
#define NEEDPROTOS
#define NEEDGETOPT
#define LDAPFUNCDECL
//#define NO_USERINTERFACE
#define FILTERFILE	"ldapfilter.conf"
#define TEMPLATEFILE	"ldaptemplates.conf"
#endif

#ifdef __VCPP__
#define LDAP_UNICODE 0
#include <winldap.h>
#define LDAPS_PORT	LDAP_SSL_PORT
#else
#include <lber.h>
#include <ldap.h>
#endif

const char* cLDAPcn = "cn";
const char* cLDAPpostalAddress = "postalAddress";
const char* cLDAPmail = "mail";
const char* cLDAPtelephoneNumber = "telephoneNumber";
const char* cLDAPfaxNumber = "facsimileTelephoneNumber";

CBusyContext* CLDAPClient::sBusy = NULL;
CLDAPClient::CLDAPClientList CLDAPClient::sClients;

#ifdef __winsock
#if __dest_os == __mac_os || __dest_os == __mac_os_x
pascal long MyBlockingHook();
pascal long MyBlockingHook()
#elif __dest_os == __win32_os
int pascal MyBlockingHook();
int pascal MyBlockingHook()
#else
#error __dest_os
#endif
{
	return CLDAPClient::BlockingHook();
}
#endif

int CLDAPClient::BlockingHook()
{
#ifdef __winsock
	if (CMailControl::ProcessBusy(sBusy))
		::WSACancelBlockingCall();
#endif

	return 0;
}

// Methods

CLDAPClient::CLDAPClient()
{
	sClients.push_back(this);
}

CLDAPClient::~CLDAPClient()
{
	sClients.erase(remove(sClients.begin(), sClients.end(), this), sClients.end());
}

void CLDAPClient::Lookup(const cdstring& item, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field, CAddressList& addr_list)
{
#if __dest_os != __win32_os
	LDAP* ld = NULL;
	LDAPMessage* res = NULL;
	LDAPMessage* e = NULL;
	char* a = NULL;
	char* dn = NULL;
	void* ptr = NULL;
	struct berval** vals = NULL;

	// Must insert blocking hook as LDAP lib blocks
#ifdef __winsock
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	WSABlockingHookUPP myBlockingHook = NewWSABlockingHookProc(MyBlockingHook);
	WSABlockingHookUPP oldBlockingHook = ::WSASetBlockingHook(myBlockingHook);
#elif __dest_os == __win32_os
	FARPROC oldBlockingHook = ::WSASetBlockingHook(MyBlockingHook);
#else
#error __dest_os
#endif
#endif
	sBusy = &mBusy;

	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &cdstring::null_str);

	// Start busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Only do this in the main thread
	//if (CTaskQueue::sTaskQueue.InMainThread())
	//	::BeginResSpinning(crsr_StdSpin);
	// 22/09/1999 - no longer needed after mod to SpinCursor library
	//if (CMulberryApp::sSuspended)
	//	::PauseSpinning(true);
	CMailControl::PreventYield(true);		// Hack for non-re-entrant WinSock during blocking call
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

	// Look for port # appended to server
	cdstring rname = GetAccount()->GetServerIP();
	tcp_port rport = ((GetAccount()->GetTLSType() == CINETAccount::eSSL) || (GetAccount()->GetTLSType() == CINETAccount::eSSLv3)) ? LDAPS_PORT : LDAP_PORT;
	if (::strchr(rname.c_str(), ':'))
	{
		rname = cdstring(GetAccount()->GetServerIP(), 0, ::strcspn(GetAccount()->GetServerIP(), ":"));
		const char* num = ::strchr(GetAccount()->GetServerIP(), ':');
		num++;
		rport = ::atoi(num);
	}

	// Parameterise root
	cdstring root = GetAccount()->GetLDAPRoot();

	if (::strchr(root.c_str(), '%'))
	{
		// Tokenise user item into root tokens - ignore first item which is the field name
		cdstring item_tok = item;
		const char* t = ::strtok(item_tok.c_str_mod(), ";");
		cdstrvect items;
		if (*item.c_str() == ';')
			items.push_back(t);
		while((t = ::strtok(NULL, ";")) != NULL)
			items.push_back(t);

		cdstring temp;
		cdstring result;

		char* p = root.c_str_mod();

		while(*p)
		{
			switch(*p)
			{
			case '%':
				// Check for parameter
				p++;
				if (::isdigit(*p))
				{
					unsigned long num = ::strtoul(p, &p, 10);
					if (num <= items.size())
						temp += items.at(num - 1);
					else
					{
						// Punt this root item
						while(*p && (*p != ',')) p++;
						if (*p == ',') p++;
						temp = cdstring::null_str;
					}
				}
				else
					// Treat as oridinary '%'
					temp += '%';
				break;
			case ',':
				// Got end of item - copy entire item as is
				temp += *p++;
				result += temp;
				temp = cdstring::null_str;
				break;
			default:
				// Just accumulate it
				temp += *p++;
				break;
			}
		}

		// Give it back to root and add remainder
		root = result + temp;
	}

	// Terminate user item at first parameter
	cdstring mitem = item;
	if (::strchr(mitem.c_str(), ';'))
		*::strchr(mitem.c_str_mod(), ';') = 0;

	// Get entry match
	cdstring matchit = "(";
	matchit += GetAccount()->GetLDAPMatch(field);
	matchit += "=";

	switch(match)
	{
	case CAdbkAddress::eMatchExactly:
		matchit += mitem;
		break;
	case CAdbkAddress::eMatchAtStart:
		matchit += mitem;
		matchit += '*';
		break;
	case CAdbkAddress::eMatchAtEnd:
		matchit += '*';
		matchit += mitem;
		break;
	case CAdbkAddress::eMatchAnywhere:
		matchit += '*';
		matchit += mitem;
		matchit += '*';
		break;
	}
	matchit += ")";

	try
	{
		// Set status
		SetStatus("Status::LDAP::Connecting");

		try
		{
			// Init connection
			if ((ld = ::ldap_init(rname, rport)) == 0L)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Setup protocol version
			int version = GetAccount()->GetLDAPVersion();
			if (::ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_OPT_SUCCESS)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Setup SSL options
			if (GetAccount()->GetTLSType() != CINETAccount::eNoTLS)
			{
				// Allow illegals certs through for now
				int cert_opt = LDAP_OPT_X_TLS_ALLOW;
				if (::ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &cert_opt) != LDAP_OPT_SUCCESS)
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}

#if 0
				SSL* ssl = NULL;
				if (::ldap_get_option(ld, LDAP_OPT_X_TLS_SSL_CTX, &ssl) != LDAP_OPT_SUCCESS)
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
				else
					// Set callback parameter
					SSL_set_ex_data(ssl, CTLSSocket::sDataIndex, (char*) this);
#endif

				// Set SSL protocol type
				if ((GetAccount()->GetTLSType() == CINETAccount::eTLS) ||
					(GetAccount()->GetTLSType() == CINETAccount::eTLSBroken))
				{
					// Do STARTTLS operation now
					if (::ldap_start_tls_s( ld, 0L, 0L ) != LDAP_SUCCESS)
					{
						CLOG_LOGTHROW(CGeneralException, -1L);
						throw CGeneralException(-1L);
					}
				}
				else
				{
					// Force connection to start in SSL
					int option = LDAP_OPT_X_TLS_HARD;
					if (::ldap_set_option(ld, LDAP_OPT_X_TLS, &option) != LDAP_OPT_SUCCESS)
					{
						CLOG_LOGTHROW(CGeneralException, -1L);
						throw CGeneralException(-1L);
					}
				}
			}
		}

		catch(...)
		{
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask("Error::LDAP::LDAPOpenFail");
			task->Go();

			CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
			throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
		}

		// Check authenticator
		switch(GetAccount()->GetAuthenticatorType())
		{
		case CAuthenticator::eNone:
		{
			// Treat as anonymous
			int errcode = ::ldap_simple_bind_s(ld, 0L, 0L);
			if ((errcode == LDAP_PROTOCOL_ERROR) && (GetAccount()->GetLDAPVersion() > LDAP_VERSION2))
			{
				// Decrease version number and try again
				int version = GetAccount()->GetLDAPVersion();
				GetAccount()->SetLDAPVersion(--version);
				if (::ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_OPT_SUCCESS)
				{
					CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
					throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
				}
				
				// Try to bind again
				errcode = ::ldap_simple_bind_s(ld, 0L, 0L);
			}
			if (!HandleResult(errcode))
			{
				CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
				throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
			}
			break;
		}
		case CAuthenticator::ePlainText:
		{
			bool first = true;
			bool done = false;

			// Loop while trying to authentciate
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

			// Loop while waiting for successful login with optional user prompt
			while(CMailControl::PromptUser(auth, GetAccount(), (GetAccount()->GetTLSType() != CINETAccount::eNoTLS),
											false, false, false, true, false, false, first))
			{
				first = false;

				// Send UID and password data and exit loop if success
				int errcode = ::ldap_simple_bind_s(ld, const_cast<cdstring&>(auth->GetUID()).c_str_mod(),
																		const_cast<cdstring&>(auth->GetPswd()).c_str_mod());
				
				// Try LDAPv2 downgrade if protocol error
				if ((errcode == LDAP_PROTOCOL_ERROR) && (GetAccount()->GetLDAPVersion() > LDAP_VERSION2))
				{
					// Decrease version number and try again
					int version = GetAccount()->GetLDAPVersion();
					GetAccount()->SetLDAPVersion(--version);
					if (::ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_OPT_SUCCESS)
					{
						CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
						throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
					}
					
					// Try to bind again
					errcode = ::ldap_simple_bind_s(ld, const_cast<cdstring&>(auth->GetUID()).c_str_mod(),
																		const_cast<cdstring&>(auth->GetPswd()).c_str_mod());
				}

				if (HandleResult(errcode))
				{
					done = true;
					break;
				}

				// Check type of returned error
				switch(errcode)
				{
				case LDAP_INAPPROPRIATE_AUTH:
				case LDAP_INVALID_CREDENTIALS:
				case LDAP_INSUFFICIENT_ACCESS:
				case LDAP_AUTH_UNKNOWN:
					break;
				default:
					CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
					throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
				}
			}
			
			if (!done)
			{
				// If cancelled just throw up
				CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
				throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
			}
			break;
		}

		// These are not supported by LDAPv2
		case CAuthenticator::ePlugin:
		default:
			// Treat as anonymous
			if (!HandleResult(::ldap_simple_bind_s(ld, 0L, 0L)))
			{
				CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
				throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
			}
			break;
		}

		// Recache user id & password after successful logon
		if (GetAccount()->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

			// Only bother if it contains something
			if (!auth->GetPswd().empty())
			{
				CINETProtocol::SetCachedPswd(auth->GetUID(), auth->GetPswd());
				CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
			}
		}

		// Set status
		SetStatus("Status::LDAP::Lookup");

		// Restrict attributes returned?
		std::auto_ptr<const char*> attrs;
		cdstrvect attrlist;
		if (!GetAccount()->GetLDAPNotes())
		{
			// Add each non-empty attribute
			for(int index = CAdbkAddress::eName; index < CAdbkAddress::eNumberFields; index++)
			{
				const cdstring& temp = GetAccount()->GetLDAPMatch(static_cast<CAdbkAddress::EAddressField>(index));
				if (!temp.empty())
					attrlist.push_back(temp);
			}
			
			if (attrlist.size())
			{
				attrs.reset(new const char*[attrlist.size() + 1]);
				const char** p = attrs.get();
				for(cdstrvect::const_iterator iter = attrlist.begin(); iter != attrlist.end(); iter++)
					*p++ = (*iter).c_str();
				*p = NULL;
			}
		}

		// search for entries
		if (!HandleResult(::ldap_search_s(ld, root.c_str_mod(), LDAP_SCOPE_SUBTREE, matchit, const_cast<char**>(attrs.get()), 0, &res)))
		{
			CLOG_LOGTHROW(CINETClient::CINETException, CINETClient::CINETException::err_NoResponse);
			throw CINETClient::CINETException(CINETClient::CINETException::err_NoResponse);
		}

		// step through each entry returned
		for(e = ::ldap_first_entry(ld, res); e != 0L; e = ::ldap_next_entry(ld, e))
		{
			// print name
			dn = ::ldap_get_dn(ld, e);
			::free(dn);
			dn = NULL;

			// Create new address
			CAdbkAddress* new_addr = new CAdbkAddress;
			cdstring notes;

			// print each attribute
			for(a = ::ldap_first_attribute(ld, e, (berelement**) &ptr); a != 0L; a = ::ldap_next_attribute(ld, e, (berelement*) ptr))
			{
				// Get values
				vals = ::ldap_get_values_len(ld, e, a);
				int val_ctr = ::ldap_count_values_len(vals);

				// Look at each value
				for(int i = 0; i < val_ctr; i++)
				{
					if (((*vals[i]).bv_len > 0) && ((*vals[i]).bv_len < 256))
					{
						cdstring val((*vals[i]).bv_val, (*vals[i]).bv_len);
						unsigned char* p = (unsigned char*) val.c_str();
						bool safe_string = true;
						while(*p)
						{
							if ((*p == '\n') && (*(p-1) != '\r'))
								*p = '\r';
							if (*p++ == 0)
							{
								safe_string = false;
								break;
							}
						}

						if (safe_string)
						{
							// Get value
							cdstring use_val((*vals[i]).bv_val, (*vals[i]).bv_len);

							if (GetAccount()->GetLDAPNotes())
							{
								if (!notes.empty())
									notes += os_endl;
								notes += a;
								notes += ": ";
								notes += use_val;
							}

							// Some attributes can only have one value (i.e. i == 0)

							// Match attributes
							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eName)))
								new_addr->SetName(use_val);

							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eNickName)))
								new_addr->SetADL(use_val);

							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eEmail)))
							{
								// Must preserve existing name
								cdstring temp = new_addr->GetName();
								new_addr->ParseAddress(use_val);
								new_addr->SetName(temp);
							}
							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eCompany)))
								new_addr->SetCompany(use_val);

							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eAddress)))
							{
								char* p = use_val;
								char* q = use_val;
								while(*p)
								{
									if ((*p == '$') && (*(p-1) == ' ') && (*(p+1) == ' '))
									{
										*(q-1) = lendl1;
#if __line_end == __crlf
										*q++ = lendl2;
#endif
										p += 2;
									}
									else
										*q++ = *p++;
								}
								*q = 0;
								new_addr->SetAddress(use_val, CAdbkAddress::eDefaultAddressType);
							}
							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::ePhoneWork)))
								new_addr->SetPhone(use_val, CAdbkAddress::eWorkPhoneType);

							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::ePhoneHome)))
								new_addr->SetPhone(use_val, CAdbkAddress::eHomePhoneType);

							if (!i && !::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eFax)))
								new_addr->SetPhone(use_val, CAdbkAddress::eFaxType);

							// Allow multiple values
							if (!::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eURL)))
							{
								if (!new_addr->GetURL().empty())
								{
									cdstring temp = new_addr->GetURL();
									temp += os_endl;
									temp += use_val;
									use_val = temp;
								}
								new_addr->SetURL(use_val);
							}

							// Allow multiple values
							if (!GetAccount()->GetLDAPNotes() &&
								(::strcmpnocase(a, GetAccount()->GetLDAPMatch(CAdbkAddress::eNotes)) == 0))
							{
								if (!new_addr->GetNotes().empty())
								{
									cdstring temp = new_addr->GetNotes();
									temp += os_endl;
									temp += use_val;
									use_val = temp;
								}
								new_addr->SetNotes(use_val);
							}

						}
					}
				}
				if (vals)
					::ldap_value_free_len(vals);
				vals = NULL;
			}

			if (GetAccount()->GetLDAPNotes())
				new_addr->SetNotes(notes);
			addr_list.push_back(new_addr);
		}

		// free search results
		::ldap_msgfree(res);
		res = NULL;

		// close connection
		::ldap_unbind(ld);

		// Stop busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Only do this in the main thread
		//if (CTaskQueue::sTaskQueue.InMainThread())
		//	::StopSpinning();
		CMailControl::PreventYield(false);		// Hack for non-re-entrant WinSock during blocking call
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

		// Set status
		SetStatus("Status::IDLE");

#ifdef __winsock
		// Stop blocking hook
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		::WSASetBlockingHook(oldBlockingHook);
#if TARGET_RT_MAC_CFM  && !TARGET_API_MAC_CARBON
		DisposeRoutineDescriptor(myBlockingHook);
#endif
#elif __dest_os == __win32_os
		::WSASetBlockingHook(oldBlockingHook);
#endif
#endif
		sBusy = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		if (dn)
			::free(dn);

		if (vals)
			::ldap_value_free_len(vals);

		if (res)
			::ldap_msgfree(res);

		// close connection
		if (ld)
			::ldap_unbind(ld);

		// Stop busy cursor
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		//::StopSpinning();
		CMailControl::PreventYield(false);		// Hack for non-re-entrant WinSock during blocking call
#elif __dest_os == __win32_os
		CMulberryApp::sApp->EndWaitCursor();
#elif __dest_os == __linux_os
		//Busy cursor should turn itself off next time through event loop
#else
#error __dest_os
#endif

		// Set status
		SetStatus("Status::IDLE");

#ifdef __winsock
		// Stop blocking hook
#if __dest_os == __mac_os || __dest_os == __mac_os_x
//#if PP_Target_Classic
		::WSASetBlockingHook(oldBlockingHook);
#if TARGET_RT_MAC_CFM  && !TARGET_API_MAC_CARBON
		DisposeRoutineDescriptor(myBlockingHook);
#endif
//#endif
#elif __dest_os == __win32_os
		::WSASetBlockingHook(oldBlockingHook);
#endif
#endif
		sBusy = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
#endif
}

// Handle LDAP error
bool CLDAPClient::HandleResult(int code)
{
#if __dest_os != __win32_os
	switch(code)
	{
	// We handle this
	case LDAP_SUCCESS:
		return true;

	// These are non-fatal errors - display error message but continue to process results
	case LDAP_PARTIAL_RESULTS:
	case LDAP_TIMELIMIT_EXCEEDED:
	case LDAP_SIZELIMIT_EXCEEDED:
		{
			// Do error alert
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask("Error::LDAP::LDAPLookupFail", ldap_err2string(code));
			task->Go();

			return true;	// We handle this
		}


	// Do error alert and fail for everything else
	default:
		{
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask("Error::LDAP::LDAPLookupFail", ldap_err2string(code));
			task->Go();
			return false;
		}
	}
#else
	return true;
#endif
}

void CLDAPClient::SetStatus(const char* rsrcid)
{
	// Set status
	CStatusWindow::SetIMAPStatus(rsrcid);

	// Set busy status info
	{
		// Status string
		cdstring status = rsrc::GetString("Status::LDAP::Connecting");

		// Append Error context
		status += os_endl2;
		status += "Account: ";
		status += GetAccount()->GetName();
		
		// Set this as the busy indicator
		mBusy.SetDescriptor(status);
	}
}

// This function overrides the one in libldap by being linked in first

int tls_verify_cb(int ok, X509_STORE_CTX *ctx)
{
	// Get pointer to plugin
	SSL* s = (SSL*) X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	
	// Lookup corresponding LDAP client
	CLDAPClient* ldap = NULL;

	// Short circuit for only one lookup
	if (CLDAPClient::sClients.size() == 1)
		ldap = CLDAPClient::sClients.front();
	else if (CLDAPClient::sClients.size() != 0)
	{
		// Not implemented for now
		// Ultimately this needs to look at each LDAP client in the list
		// and determine which one matches the SSL object passed in
	}
	
	return ldap ? ldap->Verify(ok, ctx) : 0;
}

int CLDAPClient::Verify(int ok, X509_STORE_CTX *ctx)
{
	// Add error to list of errors for this certificate
	if (ctx->error != X509_V_OK)
		AddCertError(ctx->error);

	// Always accept the certificate here - we will check the list
	// of errors later and kill the connection if its not acceptable
	return 1;
}

// Add error to list of current certificate errors
void CLDAPClient::AddCertError(int err)
{
	// Add to list if unique
	std::vector<int>::const_iterator found = std::find(mCertErrors.begin(), mCertErrors.end(), err);
	if (found == mCertErrors.end())
		mCertErrors.push_back(err);
}
