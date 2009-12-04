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

// CPasswordManagerKeyring.cp

#include "CPasswordManagerKeyring.h"

#include "CConnectionManager.h"
#include "CINETAccount.h"
#include "CLocalCommon.h"
#include "base64.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <openssl/rc4.h>
#include <openssl/md5.h>

const char* cKeyRing = ".keyring";


CPasswordManagerKeyring::CPasswordManagerKeyring()
{
	mKeyringPath = CConnectionManager::sConnectionManager.GetUserCWD();
	::addtopath(mKeyringPath, cKeyRing);
	mActive = false;
}

CPasswordManagerKeyring::~CPasswordManagerKeyring()
{
}

void CPasswordManagerKeyring::MakePasswordManagerKeyring()
{
	if (sPasswordManager == NULL)
		sPasswordManager = new CPasswordManagerKeyring();
}

bool CPasswordManagerKeyring::KeyringExists() const
{
	return ::fileexists(mKeyringPath);
}

cdstring CPasswordManagerKeyring::GetAccountURL(const CINETAccount* acct) const
{
	cdstring server = acct->GetServerIP();

	cdstring user = acct->GetName();
	if (acct->GetAuthenticator().RequiresUserPswd())
		user = acct->GetAuthenticatorUserPswd()->GetUID();

	cdstring scheme;
	switch(acct->GetServerType())
	{
	case CINETAccount::eIMAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "imaps:";
		else
			scheme = "imap:";
		break;
	case CINETAccount::ePOP3:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "pop3s:";
		else
			scheme = "pop3:";
		break;
	case CINETAccount::eSMTP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "smtps:";
		else
			scheme = "smtp:";
		break;
	case CINETAccount::eIMSP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "imsps:";
		else
			scheme = "imsp:";
		break;
	case CINETAccount::eACAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "acaps:";
		else
			scheme = "acap:";
		break;
	case CINETAccount::eLDAP:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "ldaps:";
		else
			scheme = "ldap:";
		break;
	case CINETAccount::eManageSIEVE:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "managesieves:";
		else
			scheme = "managesieve:";
		break;
	case CINETAccount::eWebDAVPrefs:
	case CINETAccount::eHTTPCalendar:
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
	case CINETAccount::eCardDAVAdbk:
		if ((acct->GetTLSType() == CINETAccount::eSSL) ||
			(acct->GetTLSType() == CINETAccount::eSSLv3))
			scheme = "https://";
		else
			scheme = "http://";
		break;
	default:
		scheme = "unknown:";
	}

	return scheme + user + "@" + server;
}

cdstrmap CPasswordManagerKeyring::ReadEncryptedMap() const
{
	cdstrmap results;

	std::ifstream fin(mKeyringPath.c_str());
	cdstring data;
	fin >> data;
	fin.close();
	if (data.length() == 0)
		return results;
	size_t encrypted_len = 0;
	std::auto_ptr<unsigned char> encrypted(::base64_decode(data.c_str(), encrypted_len));

	unsigned char digest[MD5_DIGEST_LENGTH];
	RC4_KEY key;
	std::auto_ptr<unsigned char> decrypted(new unsigned char[encrypted_len + 1]);

	MD5(reinterpret_cast<const unsigned char*>(mPassphrase.c_str()), mPassphrase.length(), digest);

	RC4_set_key(&key, MD5_DIGEST_LENGTH, digest);
	RC4(&key, encrypted_len, encrypted.get(), decrypted.get());
	decrypted.get()[encrypted_len] = 0;

	std::istringstream sin(reinterpret_cast<const char*>(decrypted.get()));
	cdstrpair kv;
	bool key_or_value = true;
	while(sin)
	{
		getline(sin, key_or_value ? kv.first : kv.second);
		key_or_value = not key_or_value;
		if (key_or_value)
			if (not kv.first.empty() and not kv.second.empty())
				results[kv.first] = kv.second;
	}

	return results;
}

void CPasswordManagerKeyring::WriteEncryptedMap(const cdstrmap& pswds) const
{
	std::ostringstream ostr;
	for(cdstrmap::const_iterator iter = pswds.begin(); iter != pswds.end(); iter++)
	{
		ostr << (*iter).first << std::endl << (*iter).second << std::endl;
	}

	std::string data = ostr.str();

	unsigned char digest[MD5_DIGEST_LENGTH];
	RC4_KEY key;
	std::auto_ptr<unsigned char> encrypted(new unsigned char[data.length() + 1]);

	MD5(reinterpret_cast<const unsigned char*>(mPassphrase.c_str()), mPassphrase.length(), digest);

	RC4_set_key(&key, MD5_DIGEST_LENGTH, digest);
	RC4(&key, data.length(), reinterpret_cast<const unsigned char*>(data.c_str()), encrypted.get());
	std::auto_ptr<char> encrypted_txt(::base64_encode(encrypted.get(), data.length()));

	std::ofstream fout(mKeyringPath.c_str());
	fout << encrypted_txt.get();
	fout.close();
}

bool CPasswordManagerKeyring::GetPassword(const CINETAccount* acct, cdstring& pswd)
{
	if (not mActive)
		return false;

	cdstring accturl = GetAccountURL(acct);

	cdstrmap pswdmap = ReadEncryptedMap();
	if (pswdmap.count(accturl))
	{
		pswd = pswdmap[accturl];
		return true;
	}
	else
		return false;
}

void CPasswordManagerKeyring::AddPassword(const CINETAccount* acct, const cdstring& pswd)
{
	if (not mActive)
		return;

	cdstring accturl = GetAccountURL(acct);

	cdstrmap pswdmap = ReadEncryptedMap();
	pswdmap[accturl] = pswd;
	WriteEncryptedMap(pswdmap);
}
