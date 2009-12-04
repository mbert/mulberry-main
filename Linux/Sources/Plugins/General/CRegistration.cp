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

// CRegistration.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 20-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements the app & plug-in registration scheme.
//
// History:
// 20-Nov-1997: Created initial header and implementation.
//

#include "CRegistration.h"

#include "CPlugin.h"

#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CMulberryApp.h"
#include "CPluginRegisterDialog.h"
#include "CStringUtils.h"

#include "cdfstream.h"

#include <strstream>

#pragma mark ____________________________consts

const char cRegistrationLicensee[] = "Licensee";
const char cRegistrationOrganisation[] = "Organisation";
const char cRegistrationSerialNumber[] = "Serial_Number";
const char cRegistrationAppRegistration[] = "Registration";
const char cRegistrationCryptoRegistration[] = "Crypto_Registration";
const char cAppRegistrationTime[] = "Time";
const char cCryptoRegistrationTime[] = "Crypto_Time";

#pragma mark ____________________________App Info

void CRegistration::LoadAppInfo()
{
	// Try internal resource first
	if (!ReadAppResource())
		ReadAppRegistration();
}


// Clear registration resources
void CRegistration::ClearAppInfo()
{
	ClearAppRegistration();
}

// Make new registration resources
bool CRegistration::WriteAppInfo()
{
	bool result = false;

	try
	{
		// Verify existing codes
		if (!ValidInfoCurrent(mSerialNumber, mAppUserCode))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get full code
		mAppRegistrationKey = GetFullCode(GetKeyV4());

		WriteAppRegistration();
		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		ClearAppInfo();
		result = false;
	}

	return result;
}

// Make new registration resources
bool CRegistration::WriteAppDemoInfo()
{
	bool result = false;

	try
	{
		// Clear out string items
		mLicensee = cdstring::null_str;
		mOrganisation = cdstring::null_str;
		mSerialNumber = cdstring::null_str;
		mAppRegistrationKey = cdstring::null_str;

		WriteAppRegistration();
		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		ClearAppInfo();
		result = false;
	}

	return result;
}

#pragma mark ____________________________Plugin Info

// Clear CRC resources
void CRegistration::ClearPluginInfo() const
{
	cdstrmap kvmap;
	ReadRegistrationMap(kvmap);

	kvmap[cRegistrationCryptoRegistration] = mCryptoRegistrationKey;
	{
		char demo_time[256];
		::snprintf(demo_time, 256, "%ld", mCryptoTimeout);
		kvmap[cCryptoRegistrationTime] = demo_time;
	}

	WriteRegistrationMap(kvmap);
}

// Make new CRC resources
bool CRegistration::WritePluginInfo(bool demo)
{
	bool result = false;

	// Clear existing first
	ClearPluginInfo();

	try
	{
		// if demo clear CRC
		if (demo)
		{
			// Generate demo timeout
			mCryptoTimeout = ::time(NULL);
		}
		else
		{
			// Map key to crc
			if (mCryptoUserCode.length() != 8)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			unsigned long test = ~::strtoul(mCryptoUserCode.c_str(), NULL, 16);

			// Do crc calc
			unsigned long crc = 0;

			// Do not allow failure
			try
			{
				crc = CRCCalculate(mSerialNumber, GetCryptoKeyV3());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}

			// Check key and crc
			if (crc != test)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			
			// Recalc CRC for full encode
			crc = GetFullCRC(GetCryptoKeyV3());

			// Get crc as string
			mCryptoRegistrationKey = crc;

			// Clear timeout
			mCryptoTimeout = 0;
		}

		// Add crc & timeout resource
		WritePluginRegistration();

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	return result;
}

#pragma mark ____________________________Registration Dialogs

// Do registration dialogs
bool CRegistration::DoAppRegister(bool& demo)
{
	return true;
}

bool CRegistration::DoPluginRegister(const CPlugin& plugin, bool& demo, bool& delete_it, bool allow_demo, bool allow_delete)
{
	// Do plug-in register dialog
	bool okayed = false;
	demo = false;
	delete_it = false;

	while(true)
	{
		// Let Dialog process events
		int result = CPluginRegisterDialog::PoseDialog(plugin, allow_demo, allow_delete, mCryptoUserCode);

		if (result == CPluginRegisterDialog::eRegister)
		{
			// Replace existing resources
			okayed = WritePluginInfo(false);
			if (okayed)
				break;
			else
				// Should display 'Illegal reg code' alert here
				CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalPlugin");
		}
		else if (result == CPluginRegisterDialog::eCancel)
		{
			break;
		}
		else if (result == CPluginRegisterDialog::eRemove)
		{
			okayed = true;
			delete_it = true;
			break;
		}
		else if (result == CPluginRegisterDialog::eDemo)
		{
			// Write all plugin details
			okayed = WritePluginInfo(true);
			if (okayed)
				demo = true;
			break;
		}
	}
	
	return okayed;
}

#pragma mark -

// Brute memory search
int brutesearch(const char* p, int plen, const char* a, int alen);
int brutesearch(const char* p, int plen, const char* a, int alen)
{
	int i;
	int j;
	int M = plen;
	int N = alen;
	
	for(i = 0, j = 0; (j < M) && (i < N); i++, j++)
		while((a[i] != p[j]) && (i < N))
		{
			i -= j-1; j=0;
		}
	if (j == M)
		return i - M;
	else
		return -1;
}

bool CRegistration::ReadAppResource()
{
#include "RegistrationBlankUnix.txt"

	char* p = (char*) cRegistrationBlank;

	// Look for proper key at start
	const char* RegKey = "REGISTRATIONSPACE";
	int len = ::strlen(RegKey) + 2;
	if ((::memcmp(p, RegKey, len - 2) == 0) &&
		(::memcmp(p + len, "**********", 10) != 0))
	{
		// Bump past key
		p += len;

		// Put data into stream
		std::istrstream in(reinterpret_cast<const char*>(p));
		cdstring str;

		// Get licensee
		getline(in, str);
		mLicensee = str;
		
		// Get organisation
		getline(in, str);
		mOrganisation = str;

		// Get serial number
		getline(in, str);
		mSerialNumber = str;

		// Get registration code
		getline(in, str);
		mAppRegistrationKey = str;

		// Get registration code
		getline(in, str);
		mCryptoRegistrationKey = str;

		return true;
	}
	else	
		return false;
}

void CRegistration::ReadAppRegistration()
{
	// Read ~/.mulberry/.registration key-values
	cdstrmap kvmap;
	ReadRegistrationMap(kvmap);

	// Licensee
	cdstrmap::const_iterator result = kvmap.find(cRegistrationLicensee);
	if (result != kvmap.end())
		mLicensee = result->second;
	else
		mLicensee = GetIllegalCopyStr();

	// Organisation
	result = kvmap.find(cRegistrationOrganisation);
	if (result != kvmap.end())
		mOrganisation = result->second;
	else
		mOrganisation = cdstring::null_str;

	// Serial number
	result = kvmap.find(cRegistrationSerialNumber);
	if (result != kvmap.end())
		mSerialNumber = result->second;
	else
		mSerialNumber = GetIllegalCopyStr();

	// App registration
	result = kvmap.find(cRegistrationAppRegistration);
	if (result != kvmap.end())
		mAppRegistrationKey = result->second;
	else
		mAppRegistrationKey = GetIllegalCopyStr();

	// Crypto registration
	result = kvmap.find(cRegistrationCryptoRegistration);
	if (result != kvmap.end())
		mCryptoRegistrationKey = result->second;
	else
		mCryptoRegistrationKey = GetIllegalCopyStr();

	// Demo timeout
	result = kvmap.find(cAppRegistrationTime);
	if (result != kvmap.end())
	{
		cdstring str = result->second;
		mAppTimeout = ::atoi(str);
	}
	else
		mAppTimeout = 0;

	// Demo timeout
	result = kvmap.find(cCryptoRegistrationTime);
	if (result != kvmap.end())
	{
		cdstring str = result->second;
		mCryptoTimeout = ::atoi(str);
	}
	else
		mCryptoTimeout = 0;

}

void CRegistration::WriteAppRegistration() const
{
	cdstrmap kvmap;
	ReadRegistrationMap(kvmap);
	kvmap[cRegistrationLicensee] = mLicensee;
	kvmap[cRegistrationOrganisation] = mOrganisation;
	kvmap[cRegistrationSerialNumber] = mSerialNumber;
	kvmap[cRegistrationAppRegistration] = mAppRegistrationKey;
	{
		char demo_time[256];
		::snprintf(demo_time, 256, "%ld", mAppTimeout);
		kvmap[cAppRegistrationTime] = demo_time;
	}

	WriteRegistrationMap(kvmap);
}

void CRegistration::ClearAppRegistration() const
{
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += ".registration";
	::remove_utf8(fname);
}

void CRegistration::WritePluginRegistration() const
{
	cdstrmap kvmap;
	ReadRegistrationMap(kvmap);

	kvmap[cRegistrationCryptoRegistration] = mCryptoRegistrationKey;
	{
		char demo_time[256];
		::snprintf(demo_time, 256, "%ld", mCryptoTimeout);
		kvmap[cCryptoRegistrationTime] = demo_time;
	}

	WriteRegistrationMap(kvmap);
}

void CRegistration::ReadRegistrationMap(cdstrmap& kvmap) const
{
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += ".registration";
	cdifstream fin(fname);

	// Read ~/.mulberry/.registration key-values
	while(fin.good())
	{
		cdstring key;
		cdstring value;
		fin >> key;
		fin.ignore();
		getline(fin, value);
		if (key.empty() || value.empty())
			continue;
		kvmap.insert(cdstrmap::value_type(key, value));
	}
}

void CRegistration::WriteRegistrationMap(const cdstrmap& kvmap) const
{
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += ".registration";

	// Write ~/.mulberry/.registration key-values
	cdofstream fout(fname);

	// Write all key-values to file
	for(cdstrmap::const_iterator iter = kvmap.begin(); iter != kvmap.end(); iter++)
		fout << (*iter).first << " " << (*iter).second << std::endl;

	if (fout.fail())
	{
		CLOG_LOGTHROW(CGeneralException, -1L);
		throw CGeneralException(-1L);
	}
}
