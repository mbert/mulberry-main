/*
    Copyright (c) 2009 Cyrus Daboo. All rights reserved.
    
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

// CPasswordManager.cp

#include "CPasswordManagerKeychain.h"

#include "CINETAccount.h"

CPasswordManagerKeychain::CPasswordManagerKeychain()
{
}

CPasswordManagerKeychain::~CPasswordManagerKeychain()
{
}

void CPasswordManagerKeychain::MakePasswordManagerKeychain()
{
	if (sPasswordManager == NULL)
		sPasswordManager = new CPasswordManagerKeychain();
}

void CPasswordManagerKeychain::GetSecurityDetails(const CINETAccount* acct, cdstring& server, cdstring& aname, UInt16& port, SecProtocolType& protocol, SecAuthenticationType& authenticationType)
{
	server = acct->GetServerIP();
	port = 0;
	size_t pos = server.find(':');
	if (pos != cdstring::npos)
	{
		cdstring strport(server, pos + 1, cdstring::npos);
		port = ::atoi(strport.c_str());
		server.erase(pos);
	}

	aname = acct->GetName();
	if (acct->GetAuthenticator().RequiresUserPswd())
		aname = acct->GetAuthenticatorUserPswd()->GetUID();

	switch(acct->GetServerType())
	{
	case CINETAccount::eIMAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = kSecProtocolTypeIMAPS;
		else
			protocol = kSecProtocolTypeIMAP;
		break;
	case CINETAccount::ePOP3:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = kSecProtocolTypePOP3S;
		else
			protocol = kSecProtocolTypePOP3;
		break;
	case CINETAccount::eSMTP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = 'smts';
		else
			protocol = kSecProtocolTypeSMTP;
		break;
	case CINETAccount::eIMSP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = 'imss';
		else
			protocol = 'imsp';
		break;
	case CINETAccount::eACAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = 'acas';
		else
			protocol = 'acap';
		break;
	case CINETAccount::eLDAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = kSecProtocolTypeLDAPS;
		else
			protocol = kSecProtocolTypeLDAP;
		break;
	case CINETAccount::eManageSIEVE:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = 'sies';
		else
			protocol = 'siev';
		break;
	case CINETAccount::eWebDAVPrefs:
	case CINETAccount::eHTTPCalendar:
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
	case CINETAccount::eCardDAVAdbk:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			protocol = kSecProtocolTypeHTTPS;
		else
			protocol = kSecProtocolTypeHTTP;
		if (acct->GetAuthenticatorType() == CAuthenticator::ePlainText)
			authenticationType = kSecAuthenticationTypeHTTPBasic;
		else if (acct->GetAuthenticatorType() == CAuthenticator::ePlugin)
		{
			if (acct->GetAuthenticator().GetSASLID() == "DIGEST-MD5")
				authenticationType = kSecAuthenticationTypeHTTPDigest;
		}
		break;
		default:;
	}
}

bool CPasswordManagerKeychain::GetPasswordWithItem(const CINETAccount* acct, cdstring& pswd, SecKeychainItemRef* item)
{
	cdstring server;
	cdstring aname;
	UInt16 port = 0;
	SecProtocolType protocol;
	SecAuthenticationType authenticationType = kSecAuthenticationTypeDefault;
	GetSecurityDetails(acct, server, aname, port, protocol, authenticationType);

	UInt32 dataLen = 0;
	void* data = NULL;
	OSStatus err = ::SecKeychainFindInternetPassword(NULL, server.length(), server.c_str(), 0, NULL, aname.length(), aname.c_str(),
												     0, NULL, port, protocol, authenticationType, &dataLen, &data, item);
	if (err == noErr)
	{
		pswd.assign((const char*)data, dataLen);
		::SecKeychainItemFreeContent(NULL, data);
		return true;
	}
	else
		return false;
}

bool CPasswordManagerKeychain::GetPassword(const CINETAccount* acct, cdstring& pswd)
{
	return GetPasswordWithItem(acct, pswd, NULL);
}

void CPasswordManagerKeychain::AddPassword(const CINETAccount* acct, const cdstring& pswd)
{
	// First verify its not already set
	cdstring current_pswd;
	SecKeychainItemRef item = NULL;
	if (GetPasswordWithItem(acct, current_pswd, &item) && (current_pswd == pswd))
	{
		if (item != NULL)
			::CFRelease(item);
		return;
	}
	
	cdstring server;
	cdstring aname;
	UInt16 port = 0;
	SecProtocolType protocol;
	SecAuthenticationType authenticationType = kSecAuthenticationTypeDefault;
	GetSecurityDetails(acct, server, aname, port, protocol, authenticationType);

	OSStatus err = noErr;
	if (item == NULL)
	{
		err = ::SecKeychainAddInternetPassword(NULL, server.length(), server.c_str(), 0, NULL, aname.length(), aname.c_str(),
												  0, NULL, port, protocol, authenticationType, pswd.length(), pswd.c_str(), NULL);
	}
	else
	{
		err = ::SecKeychainItemModifyAttributesAndData(item, NULL, pswd.length(), pswd.c_str());
		::CFRelease(item);
	}
}
