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

#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginRegisterDialog.h"

#include <UModalDialogs.h>

#pragma mark ____________________________consts

enum
{
	//str_licensee = 128,
	//str_serial_number = 129,
	//str_organisation = 130,
	//colm_base = 128,
	app_demo_id = 128,
	crypto_demo_id = 129
};
const OSType	cCRCType = 'colm';
const OSType	cDemoType = 'teda';

#if PP_Target_Carbon
const char* cCFLicenseeKey = "Registration:Licensee";
const char* cCFOrganisationKey = "Registration:Organisation";
const char* cCFSerialNumberKey = "Registration:SerialNumber";
const char* cCFAppRegistrationKey = "Registration:Registration";
const char* cCFCryptoRegistrationKey = "Registration:CryptoRegistration";

const char* cCFTimeoutKey = "Registration:Timeout";
const char* cCFCryptoTimeoutKey = "Registration:CryptoTimeout";
#endif


#pragma mark ____________________________Registration Info

static short sAppResFile = 0;

void CRegistration::LoadAppInfo(short resFile)
{
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (sAppResFile == 0)
		sAppResFile = old_res;
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;

	// Get licensee name
	mLicensee = GetIllegalCopyStr();
	StringHandle licensee = ::GetString(str_licensee);
	if (licensee)
	{
		{
			StHandleLocker lock((Handle) licensee);
			cdstring pcpy((unsigned char*) *licensee);
			mLicensee = pcpy;
		}
		::ReleaseResource((Handle) licensee);
	}

	// Get serial number
	mSerialNumber = GetIllegalCopyStr();
	StringHandle serial_number = ::GetString(str_serial_number);
	if (serial_number)
	{
		{
			StHandleLocker lock((Handle) serial_number);
			cdstring pcpy((unsigned char*) *serial_number);
			mSerialNumber = pcpy;
		}
		::ReleaseResource((Handle) serial_number);
	}

	// Get organisation if required
	mOrganisation = GetIllegalCopyStr();
	StringHandle organisation = ::GetString(str_organisation);
	if (organisation)
	{
		{
			StHandleLocker lock((Handle) organisation);
			cdstring pcpy((unsigned char*) *organisation);
			mOrganisation = pcpy;
		}
		::ReleaseResource((Handle) organisation);
	}

	// Get app registration code
	StringHandle regkey = ::GetString(str_appregkey);
	if (regkey)
	{
		{
			StHandleLocker lock((Handle) regkey);
			cdstring pcpy((unsigned char*) *regkey);
			mAppRegistrationKey = pcpy;
		}
		::ReleaseResource((Handle) regkey);
	}

	// Get crypto registration code
	regkey = ::GetString(str_cryptoregkey);
	if (regkey)
	{
		{
			StHandleLocker lock((Handle) regkey);
			cdstring pcpy((unsigned char*) *regkey);
			mCryptoRegistrationKey = pcpy;
		}
		::ReleaseResource((Handle) regkey);
	}

	// Look for date resource
	Handle timeoutH = ::Get1Resource(cDemoType, crypto_demo_id);

	if (timeoutH)
	{
		mCryptoTimeout = *(time_t*) *timeoutH;
		if (mCryptoTimeout == 0)
			mCryptoTimeout = 1;
	}
	else
		mCryptoTimeout = 0;

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);

	// On Carbon we can't write internal registration as the application file is
	// read-only. Thus we need to use CoreFoundation preferences for registering
	// Mulberry from within itself. However, the AdminTool can write internal
	// registration so we look for that first and then try CoreFoundation.

#if PP_Target_Carbon
	// Use CoreFoundation preferences if internal prefs not found
	if (mAppRegistrationKey.empty())
	{
		// Licensee/Organisation/Serial Number
		::GetCFPreference(cCFLicenseeKey, mLicensee); 
		::GetCFPreference(cCFOrganisationKey, mOrganisation); 
		::GetCFPreference(cCFSerialNumberKey, mSerialNumber); 
		::GetCFPreference(cCFAppRegistrationKey, mAppRegistrationKey); 
		::GetCFPreference(cCFCryptoRegistrationKey, mCryptoRegistrationKey); 

		// Crypto Timeout & timeout key
		cdstring timeoutstr;
		if (::GetCFPreference(cCFCryptoTimeoutKey, timeoutstr))
		{
			// Convert to numeric values
			mCryptoTimeout = ::strtoul(timeoutstr, NULL, 10);
			if (mCryptoTimeout == 0)
				mCryptoTimeout = 1;
		}
		else
			mCryptoTimeout = 0;
	}
#endif
}


// Clear registration resources
void CRegistration::ClearAppInfo(short resFile) const
{
#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;

	// Clear licensee name
	Handle licensee = ::GetResource('STR ', str_licensee);
	if (licensee)
		::RemoveResource(licensee);

	// Clear serial number
	Handle serial_number = ::GetResource('STR ', str_serial_number);
	if (serial_number)
		::RemoveResource(serial_number);

	// Clear organisation
	Handle organisation = ::GetResource('STR ', str_organisation);
	if (organisation)
		::RemoveResource((Handle) organisation);

	// Clear app registration key
	Handle regkey = ::GetResource('STR ', str_appregkey);
	if (regkey)
		::RemoveResource(regkey);

	// Flush changes
	::UpdateResFile(resFile);

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
#else
	// Use CoreFoundation preferences
	
	// Licensee/Organisation/Serial Number
	::SetCFPreference(cCFLicenseeKey, NULL); 
	::SetCFPreference(cCFOrganisationKey, NULL); 
	::SetCFPreference(cCFSerialNumberKey, NULL); 
	::SetCFPreference(cCFAppRegistrationKey, NULL); 
#endif
}

// Make new registration resources
bool CRegistration::WriteAppInfo(short resFile) const
{
	// Clear existing first
	ClearAppInfo(resFile);

	bool result = false;

#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;
#endif

	try
	{
#if PP_Target_Classic
		Handle rsrc = NULL;

		// Add licensee
		rsrc = ::NewHandle(mLicensee.length() + 1);
		ThrowIfNil_(rsrc);
		cdstring temp = mLicensee;
		c2pstr((char*) temp.c_str());
		::memcpy(*rsrc, temp.c_str(), mLicensee.length() + 1);
		::AddResource(rsrc, 'STR ', str_licensee, "\pLicensee");
		ThrowIfResError_();

		// Add organisation
		rsrc = ::NewHandle(mOrganisation.length() + 1);
		ThrowIfNil_(rsrc);
		temp = mOrganisation;
		c2pstr((char*) temp.c_str());
		::memcpy(*rsrc, temp.c_str(), mOrganisation.length() + 1);
		::AddResource(rsrc, 'STR ', str_organisation, "\pOrganisation");
		ThrowIfResError_();

		// Add serial number
		rsrc = ::NewHandle(mSerialNumber.length() + 1);
		ThrowIfNil_(rsrc);
		temp = mSerialNumber;
		c2pstr((char*) temp.c_str());
		::memcpy(*rsrc, temp.c_str(), mSerialNumber.length() + 1);
		::AddResource(rsrc, 'STR ', str_serial_number, "\pSerial number");
		ThrowIfResError_();

#else
		// Licensee/Organisation/Serial Number
		if (!::SetCFPreference(cCFLicenseeKey, mLicensee))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		if (!::SetCFPreference(cCFOrganisationKey, mOrganisation))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		if (!::SetCFPreference(cCFSerialNumberKey, mSerialNumber))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
#endif

		// Verify existing codes
		ThrowIf_(!ValidInfoCurrent(mSerialNumber, mAppUserCode));
		
		// Get full code
		cdstring calc_key = GetFullCode(GetKeyV4());

#if PP_Target_Classic
		// Add app registration key
		rsrc = ::NewHandle(calc_key.length() + 1);
		ThrowIfNil_(rsrc);
		temp = calc_key;
		c2pstr((char*) temp.c_str());
		::memcpy(*rsrc, temp.c_str(), calc_key.length() + 1);
		::AddResource(rsrc, 'STR ', str_appregkey, "\pRegistration");
		ThrowIfResError_();
#else
		if (!::SetCFPreference(cCFAppRegistrationKey, calc_key))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
#endif

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		ClearAppInfo(resFile);
		result = false;
	}

#if PP_Target_Classic
	// Flush changes
	//::UpdateResFile(resFile);

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
#endif

	return result;
}

time_t CRegistration::LoadAppTimeout(short resFile) const
{
#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;

	// Look for date resource
	Handle timeoutH = ::GetResource('teda', 128);

	time_t timeout = timeoutH ? *(time_t*) *timeoutH : 0;

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
	
	return timeout;
#else
	// Timeout & timeout key
	cdstring timeoutstr;
	cdstring timeoutkeystr;
	::GetCFPreference(cCFTimeoutKey, timeoutstr);
	
	// Convert to numeric values
	time_t timeout = ::strtoul(timeoutstr, NULL, 10);
	
	return timeout;
#endif
}

bool CRegistration::WriteAppTimeout(short resFile) const
{
#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;

	// Look for date resource
	Handle timeoutH = ::GetResource('teda', 128);

	// Set new time in resource
	*(time_t*) *timeoutH = time(NULL);
	::ChangedResource(timeoutH);
	::WriteResource(timeoutH);
	::ReleaseResource(timeoutH);

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
	
	return true;
#else
	// Get current time as string
	time_t timeout = time(NULL);
	cdstring timeoutstr((unsigned long) timeout);

	// Write them out
	::SetCFPreference(cCFTimeoutKey, timeoutstr);
	
	return true;
#endif
}

#pragma mark ____________________________Plugin Info

// Clear CRC resources
void CRegistration::ClearPluginInfo(short resFile) const
{
#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;

	// Clear crypto registration key
	Handle regkey = ::GetResource('STR ', str_cryptoregkey);
	if (regkey)
		::RemoveResource(regkey);

	// Get timeout
	Handle timeoutH = ::Get1Resource(cDemoType, crypto_demo_id);
	if (timeoutH)
		::RemoveResource(timeoutH);

	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
#else
	// Use CoreFoundation preferences
	::SetCFPreference(cCFCryptoRegistrationKey, NULL); 
	::SetCFPreference(cCFCryptoTimeoutKey, NULL); 
#endif
}

// Make new CRC resources
bool CRegistration::WritePluginInfo(short resFile, bool demo)
{
	// Clear existing first
	ClearPluginInfo(resFile);

	bool result = false;

#if PP_Target_Classic
	// Save current res file and change to new
	short old_res = ::CurResFile();
	if (resFile)
		::UseResFile(resFile);
	else
		resFile = old_res;
#endif

	try
	{
#if PP_Target_Classic
		Handle rsrc = NULL;
#endif

		if (demo)
		{
#if PP_Target_Classic
			// Add timeout
			rsrc = ::NewHandle(sizeof(time_t));
			ThrowIfNil_(rsrc);
			mCryptoTimeout = ::time(NULL);
			*(time_t*) *rsrc = mCryptoTimeout;
			::AddResource(rsrc, cDemoType, crypto_demo_id, "\p");
			ThrowIfResError_();
#else
			// Get current time as string
			time_t timeout = mCryptoTimeout = ::time(NULL);
			cdstring timeoutstr((unsigned long) timeout);

			// Write them out
			::SetCFPreference(cCFCryptoTimeoutKey, timeoutstr);
#endif
		}
		else
		{
			// Map key to crc
			if (mCryptoUserCode.length() != 8) ThrowIf_(true);
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
			ThrowIf_(crc != test);

			// Recalc CRC for full encode
			crc = GetFullCRC(GetCryptoKeyV3());

			// Get crc as string
			mCryptoRegistrationKey = crc;

			// Add crc resource
#if PP_Target_Classic
			rsrc = ::NewHandle(mCryptoRegistrationKey.length() + 1);
			ThrowIfNil_(rsrc);
			cdstring temp = mCryptoRegistrationKey;
			c2pstr((char*) temp.c_str());
			::memcpy(*rsrc, temp.c_str(), mCryptoRegistrationKey.length() + 1);
			::AddResource(rsrc, 'STR ', str_cryptoregkey, "\pCrypto Registration");
			ThrowIfResError_();
#else
			if (!::SetCFPreference(cCFCryptoRegistrationKey, mCryptoRegistrationKey))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
#endif

			// Clear timeout
			mCryptoTimeout = 0;
		}

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

#if PP_Target_Classic
	// Restore res file
	if (resFile != old_res)
		::UseResFile(old_res);
#endif

	return result;
}

#pragma mark ____________________________Registration Dialogs

// Do registration dialogs
bool CRegistration::DoAppRegister(bool& demo, short resFile)
{
	return true;
}

bool CRegistration::DoPluginRegister(const CPlugin& plugin, bool& demo, bool& delete_it, bool allow_demo, bool allow_delete)
{
	// Do plug-in register dialog
	bool okayed = false;
	demo = false;
	delete_it = false;

	// Create the dialog
	StDialogHandler theHandler(paneid_PluginRegisterDialog, LCommander::GetTopCommander());
	CPluginRegisterDialog* dlog = (CPluginRegisterDialog*) theHandler.GetDialog();
	dlog->SetPlugin(plugin, allow_demo, allow_delete);
	dlog->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Get details from fields
			mCryptoUserCode = dlog->GetRegKey();

			// Replace existing resources
			okayed = WritePluginInfo(sAppResFile, false);
			if (okayed)
				break;
			else
				// Should display 'Illegal reg code' alert here
				CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalPlugin");
		}
		else if (hitMessage == msg_Cancel)
		{
			break;
		}
		else if (hitMessage == msg_RemovePlugin)
		{
			okayed = true;
			delete_it = true;
			break;
		}
		else if (hitMessage == msg_RunPluginDemo)
		{
			okayed = WritePluginInfo(sAppResFile, true);
			if (okayed)
				demo = true;
			break;
		}
	}


	return okayed;
}
