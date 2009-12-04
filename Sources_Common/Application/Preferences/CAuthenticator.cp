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


// CAuthenticator.cp

// Header file for class/structs that define address book accounts

#include "CAuthenticator.h"

#include "CAdminLock.h"
#include "CAuthPlugin.h"
#include "CPluginManager.h"
#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// Classes

#pragma mark ____________________________CAuthenticator

const char* cAuthenticatorDescriptors[] =
	{"None",
	 "Plain Text",
	 "SSL Client Certificate"};

const char* cSASLDescriptors[] =
	{"ANONYMOUS",
	 "PLAIN",
	 "EXTERNAL"};

CAuthenticator::CAuthenticator(bool letter)
{
	mAuthenticator = NULL;
	if (letter)
	{
		mType = eNone;
		mDescriptor = cAuthenticatorDescriptors[eNone];
	}
	else
		ResetAuthenticatorType(ePlainText);
}

CAuthenticator::CAuthenticator(const CAuthenticator& copy)
{
	mType = copy.mType;
	mDescriptor = copy.mDescriptor;
	if (copy.mAuthenticator)
	{
		if (dynamic_cast<CAuthenticatorNone*>(copy.mAuthenticator))
			mAuthenticator = new CAuthenticatorNone(*(CAuthenticatorNone*) copy.mAuthenticator);
		else if (dynamic_cast<CAuthenticatorUserPswd*>(copy.mAuthenticator))
			mAuthenticator = new CAuthenticatorUserPswd(*(CAuthenticatorUserPswd*) copy.mAuthenticator);
		else if (dynamic_cast<CAuthenticatorKerberos*>(copy.mAuthenticator))
			mAuthenticator = new CAuthenticatorKerberos(*(CAuthenticatorKerberos*) copy.mAuthenticator);
	}
	else
		mAuthenticator = NULL;
}

CAuthenticator::~CAuthenticator()
{
	delete mAuthenticator;
	mAuthenticator = NULL;
}

// Assignment with same type
CAuthenticator& CAuthenticator::operator=(const CAuthenticator& copy)
{
	if (this != &copy)
	{
		mType = copy.mType;
		mDescriptor = copy.mDescriptor;

		// delete existing
		delete mAuthenticator;
		mAuthenticator = NULL;

		if (copy.mAuthenticator)
		{
			if (dynamic_cast<CAuthenticatorNone*>(copy.mAuthenticator))
				mAuthenticator = new CAuthenticatorNone(*(CAuthenticatorNone*) copy.mAuthenticator);
			else if (dynamic_cast<CAuthenticatorUserPswd*>(copy.mAuthenticator))
				mAuthenticator = new CAuthenticatorUserPswd(*(CAuthenticatorUserPswd*) copy.mAuthenticator);
			else if (dynamic_cast<CAuthenticatorKerberos*>(copy.mAuthenticator))
				mAuthenticator = new CAuthenticatorKerberos(*(CAuthenticatorKerberos*) copy.mAuthenticator);
		}
	}

	return *this;
}

// Compare with same type
int CAuthenticator::operator==(const CAuthenticator& comp) const
{
	int result = 0;
	if ((mType == comp.mType) && (mDescriptor == comp.mDescriptor))
	{
		if (dynamic_cast<CAuthenticatorNone*>(comp.mAuthenticator))
			result = (*(CAuthenticatorNone*) mAuthenticator == *(CAuthenticatorNone*) comp.mAuthenticator);
		else if (dynamic_cast<CAuthenticatorUserPswd*>(comp.mAuthenticator))
			result = (*(CAuthenticatorUserPswd*) mAuthenticator == *(CAuthenticatorUserPswd*) comp.mAuthenticator);
		else if (dynamic_cast<CAuthenticatorKerberos*>(comp.mAuthenticator))
			result = (*(CAuthenticatorKerberos*) mAuthenticator == *(CAuthenticatorKerberos*) comp.mAuthenticator);
	}

	return result;
}

void CAuthenticator::SetAuthenticatorType(EAuthenticators type)
{
	// Delete existing
	delete mAuthenticator;
	mAuthenticator = NULL;

	// Add new
	mType = type;
	CAuthPlugin::EAuthPluginUIType ui_type = CAuthPlugin::eAuthAnonymous;
	switch(mType)
	{
	case eNone:
		ui_type = CAuthPlugin::eAuthAnonymous;
		break;
	case ePlainText:
	case eSSL:
		ui_type = CAuthPlugin::eAuthUserPswd;
		break;
	case ePlugin:
#ifdef __MULBERRY
		CAuthPlugin* plugin = GetPlugin();
		if (plugin)
			ui_type = plugin->GetAuthUIType();
#else
		if ((mDescriptor == "CRAM-MD5") ||
			(mDescriptor == "DIGEST-MD5"))
			ui_type = CAuthPlugin::eAuthUserPswd;
		else if (mDescriptor == "Kerberos V4")
			ui_type = CAuthPlugin::eAuthKerberos;
		else if (mDescriptor == "GSSAPI")
			ui_type = CAuthPlugin::eAuthKerberos;
		else
			ui_type = CAuthPlugin::eAuthAnonymous;
#endif
		break;
	}
	switch(ui_type)
	{
	case CAuthPlugin::eAuthUserPswd:
		mAuthenticator = new CAuthenticatorUserPswd;
		break;
	case CAuthPlugin::eAuthKerberos:
		mAuthenticator = new CAuthenticatorKerberos;
		break;
	case CAuthPlugin::eAuthAnonymous:
	default:
		mAuthenticator = new CAuthenticatorNone;
		break;
	}
	mAuthenticator->mDescriptor = mDescriptor;
}

void CAuthenticator::ResetAuthenticatorType(EAuthenticators type)
{
	// Add new
	switch(type)
	{
	case eNone:
	case ePlainText:
	case eSSL:
		SetDescriptor(cAuthenticatorDescriptors[type]);
		break;
	default:;
	}
}

cdstring CAuthenticator::GetSASLID() const
{
#ifdef __MULBERRY
	switch(mType)
	{
	case eNone:
	case ePlainText:
	case eSSL:
		return cSASLDescriptors[mType];
	case ePlugin:
		CAuthPlugin* plugin = GetPlugin();
		if (plugin)
			return plugin->GetAuthTypeID();
	}
#endif
	
	return cdstring::null_str;
}

// Set descriptor and type
void CAuthenticator::SetDescriptor(const cdstring& desc)
{
	// Only bother if different
	if (mDescriptor != desc)
	{
		mDescriptor = desc;
		
		// Now set type based on this
		if (mDescriptor == cAuthenticatorDescriptors[eNone])
			mType = eNone;
		else if (mDescriptor == cAuthenticatorDescriptors[ePlainText])
			mType = ePlainText;
		else if (mDescriptor == cAuthenticatorDescriptors[eSSL])
			mType = eSSL;
		else
			mType = ePlugin;
		SetAuthenticatorType(mType);
	}
}

CAuthPlugin* CAuthenticator::GetPlugin() const
{
	switch(mType)
	{
	case eNone:
	case ePlainText:
	case eSSL:
	default:
		return NULL;
#ifdef __MULBERRY
	case ePlugin:
		return CPluginManager::sPluginManager.GetAuthPlugin(mDescriptor);
#endif
	}
}

cdstring CAuthenticator::GetInfo(void) const
{
	// Create string list of items
	cdstring info;
	cdstring temp = GetDescriptor();
	temp.quote();
	info += temp;
	if (mType != eNone)
	{
		info += cSpace;
		info += mAuthenticator->GetInfo();
	}

	// Got it all
	return info;
}

bool CAuthenticator::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	char* item = txt.get();
	if (item && *item)
	{
		cdstring temp = item;
		SetDescriptor(temp);

		// Parse actual auth data
		if (mType != eNone)
			result = mAuthenticator->SetInfo(txt, vers_prefs);
	}
	else
		result = false;

	return result;
}

#pragma mark ____________________________CAuthenticatorNone

cdstring CAuthenticatorNone::GetInfo(void) const
{
	return mFuture.GetInfo();
}

bool CAuthenticatorNone::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	return mFuture.SetInfo(txt, vers_prefs);
}


#pragma mark ____________________________CAuthenticatorUserPswd

cdstring CAuthenticatorUserPswd::GetInfo(void) const
{
	// Create string list of items
	cdstring info;

	cdstring temp = mUID;
	temp.quote();
	info += temp;
	info += cSpace;

#ifdef __MULBERRY
	if (mSavePswd && !CAdminLock::sAdminLock.mLockSavePswd)
	{
		temp = mPswd;
		temp.Encrypt(cdstring::eEncryptSimplemUTF7);
	}
	else
#endif
		temp = cdstring::null_str;
	temp.quote();
	info += temp;
	info += cSpace;

	info += (mSaveUID ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	info += (mSavePswd ? cValueBoolTrue : cValueBoolFalse);

	info += mFuture.GetInfo();

	return info;
}

bool CAuthenticatorUserPswd::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.get(mUID);
	txt.get(mPswd);
	mPswd.Decrypt(cdstring::eEncryptSimplemUTF7);

	txt.get(mSaveUID);
	txt.get(mSavePswd);

	if (!mSavePswd)
		mPswd = cdstring::null_str;

	mFuture.SetInfo(txt, vers_prefs);

	return result;
}

#pragma mark ____________________________CAuthenticatorKerberos

cdstring CAuthenticatorKerberos::GetInfo(void) const
{
	// Create string list of items
	cdstring info;

	info += (mDefaultPrincipal ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	cdstring temp = mServerPrincipal;
	temp.quote();
	info += temp;
	info += cSpace;

	info += (mDefaultUID ? cValueBoolTrue : cValueBoolFalse);
	info += cSpace;

	temp = mUID;
	temp.quote();
	info += temp;

	info += mFuture.GetInfo();

	return info;
}

bool CAuthenticatorKerberos::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.get(mDefaultPrincipal);
	txt.get(mServerPrincipal);
	txt.get(mDefaultUID);
	txt.get(mUID);

	mFuture.SetInfo(txt, vers_prefs);

	return result;
}
