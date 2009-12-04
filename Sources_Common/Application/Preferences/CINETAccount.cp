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


// CINETAccount.cp

// Header file for class/structs that define mail accounts

#include "CINETAccount.h"

#include "CPreferenceVersions.h"

#include "char_stream.h"
#include "CStringUtils.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

#pragma mark ____________________________CINETAccount

const char* cINETDescriptors[] =
	{"Unknown",
	 "IMAP",
	 "POP3",
	 "Local",
	 "SMTP",
	 "NNTP",
	 "WebDAV Prefs",
	 "IMSP",
	 "ACAP",
	 "LDAP",
	 "Whois++",
	 "Finger",
	 "Local Adbk",
	 "Manage SIEVE",
	 "OS Adbk",
	 "Local Calendar",
	 "HTTP Calendar",
	 "WebDAV Calendar",
	 "CalDAV Calendar",
	 "CardDAV Calendar",
	 NULL};

const char* cTLSDescriptors[] =
	{"NoTLS",
	 "SSLv3",
	 "SSLv3Real",
	 "TLSv1",
	 "TLSv1SSLHello"};

CINETAccount::CINETAccount()
{
	mServerType = eUnknown;
	mTLSType = eNoTLS;
	mUseTLSClientCert = false;
	mLogonAtStart = false;
	mProtocol = NULL;
}

void CINETAccount::_copy(const CINETAccount& copy)
{
	mName = copy.mName;
	mServerType = copy.mServerType;
	mServerIP = copy.mServerIP;
	mTLSType = copy.mTLSType;
	mUseTLSClientCert = copy.mUseTLSClientCert;
	mTLSClientCert = copy.mTLSClientCert;
	mAuthenticator = copy.mAuthenticator;
	mLogonAtStart = copy.mLogonAtStart;
	mCWD = copy.mCWD;
	mFuture = copy.mFuture;
	mProtocol = copy.mProtocol;
}

// Compare with same type (do not compare protocols - only compare contents)
int CINETAccount::operator==(const CINETAccount& comp) const
{
	return (mName == comp.mName) &&
			(mServerType == comp.mServerType) &&
			(mServerIP == comp.mServerIP) &&
			(mTLSType == comp.mTLSType) &&
			(mUseTLSClientCert == comp.mUseTLSClientCert) &&
			(mTLSClientCert == comp.mTLSClientCert) &&
			(mAuthenticator == comp.mAuthenticator) &&
			(mLogonAtStart == comp.mLogonAtStart) &&
			(mCWD == comp.mCWD);
}

// New account being created
void CINETAccount::NewAccount()
{
	// Does nothing
}

const char* CINETAccount::GetServerTypeString() const
{
	return cINETDescriptors[mServerType];
}

void CINETAccount::SetServerType(EINETServerType type)
{
	mServerType = type;
}

bool CINETAccount::SupportsTLSType(ETLSType type) const
{
	switch (mServerType)
	{
	case eIMAP:
	case ePOP3:
	case eSMTP:
	case eIMSP:
	case eACAP:
	case eLDAP:
		return true;
	case eLocal:
	case eNNTP:
	case eWHOISPP:
	case eFinger:
	case eLocalAdbk:
	case eOSAdbk:
	case eLocalCalendar:
	default:
		return false;
		
	// ManageSIEVE only does STARTTLS - no high port SSL
	case eManageSIEVE:
		return ((type != eSSL) && (type != eSSLv3));
		
	// HTTP only does SSL - no STARTTLS
	case eWebDAVPrefs:
	case eHTTPCalendar:
	case eWebDAVCalendar:
	case eCalDAVCalendar:
	case eCardDAVAdbk:
		return (type == eNoTLS) || (type == eSSL) || (type == eSSLv3);
	}
}

cdstring CINETAccount::GetInfo() const
{
	// Create string list of items
	cdstring info;
	cdstring temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	info += temp;
	info += cSpace;

	temp = cINETDescriptors[mServerType];
	temp.quote();
	info += temp;
	info += cSpace;

	temp = mServerIP;
	temp.quote();
	info += temp;
	info += cSpace;

	info += (mLogonAtStart ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += "((";
	info += mAuthenticator.GetInfo();
	info += "))";
	info += cSpace;

	info += '(';
	info += GetCWD().GetInfo();
	info += ')';
	info += cSpace;

	// New for TLS support
	temp = cTLSDescriptors[mTLSType];
	temp.quote();
	info += temp;
	info += cSpace;
	
	// New for TLS client cert support
	info += (mUseTLSClientCert ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	// New for TLS client cert support
	temp = mTLSClientCert;
	temp.quote();
	temp.ConvertFromOS();
	info += temp;
	info += cSpace;

	// Future expansion
	info += mFuture.GetInfo();

	// Got it all
	return info;
}

bool CINETAccount::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	// Determine account type first
	txt.get(mName, true);

	char* p = txt.get();
	mServerType = static_cast<EINETServerType>(::strindexfind(p, cINETDescriptors, eUnknown));

	txt.get(mServerIP);

	txt.get(mLogonAtStart);

	// Force reset of type to enforce typing rules
	// Must do this before authenticator is set
	SetServerType(mServerType);

	if (txt.start_sexpression())
	{
		if (txt.start_sexpression())
		{
			result = mAuthenticator.SetInfo(txt, vers_prefs);
			txt.end_sexpression();
		}
		txt.end_sexpression();
	}

	// >= v2.0a2
	if (VersionTest(vers_prefs, VERS_2_0_0_A_2) >= 0)
	{
		txt.start_sexpression();
			GetCWD().SetInfo(txt, vers_prefs);
		txt.end_sexpression();
	}

	// Expansion items:
	
	// TLS state
	p = txt.get();
	mTLSType = eNoTLS;
	if (p)
		for(int i = eNoTLS; i <= eTLSTypeMax; i++)
		{
			if (::strcmp(p, cTLSDescriptors[i]) == 0)
				mTLSType = (ETLSType) i;
		}

	// TLS client cert
	p = txt.get();
	if (p)
	{
		mUseTLSClientCert = (::strcmpnocase(p, cValueBoolTrue) == 0);
		
		txt.get(mTLSClientCert, true);
	}

	// Unknown expansion items
	mFuture.SetInfo(txt, vers_prefs);

	return result;
}
