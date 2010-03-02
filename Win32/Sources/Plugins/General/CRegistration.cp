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
#include "CSDIFrame.h"
#include "CWinRegistry.h"

#include "CErrorHandler.h"
#include "CPluginRegisterDialog.h"

#include <strstream>

#pragma mark ____________________________consts

const char cSoftwareSection[] = "SOFTWARE";
const char cRegistrationSection[] = "Registration";
const char cRegistrationLicensee[] = "Licensee";
const char cRegistrationOrganisation[] = "Organisation";
const char cRegistrationSerialNumber[] = "Serial Number";
const char cRegistrationAppRegistration[] = "Registration";
const char cRegistrationCryptoRegistration[] = "Crypto Registration";
const char cAppRegistrationTime[] = "Time";
const char cCryptoRegistrationTime[] = "Crypto Time";

const TCHAR cRegResourceName[] = _T("TEXT");

#define IDT_REGISTRATION	129

#pragma mark ____________________________App Info

void CRegistration::LoadAppInfo(HMODULE hModule)
{
	BOOL got_internal = false;

	{
		// Look for multiuser resource
		HRSRC hrsrc = ::FindResource(hModule, MAKEINTRESOURCE(IDT_REGISTRATION), cRegResourceName);
		if (!hrsrc)
			return;

		// Try to load resource
		HGLOBAL hResource = ::LoadResource(hModule, hrsrc);
		if (!hResource)
			return;

		char* p = (char*) ::LockResource(hResource);
		DWORD rsrc_size = ::SizeofResource(hModule, hrsrc);

		// Look for proper key at start
		const char* RegKey = "REGISTRATIONSPACE";
		int len = ::strlen(RegKey) + 2;
		if ((::memcmp(p, RegKey, len - 2) == 0) &&
			(::memcmp(p + len, "**********", 10) != 0))
		{
			// Bump past key
			p += len;
			rsrc_size -= len;

			// Create mem file for resource data and archive
			std::istrstream sin(p, rsrc_size);
			
			// Get licensee
			::getline(sin, mLicensee);

			// Get organisation
			::getline(sin, mOrganisation);

			// Get serial number
			::getline(sin, mSerialNumber);

			// Get registration code
			::getline(sin, mAppRegistrationKey);

			// Get registration code
			::getline(sin, mCryptoRegistrationKey);

			got_internal = true;
		}
		
		//::FreeResource(hResource); <- not required: crashes NT & 95
		
	}
	
	if (!got_internal)
	{
		cdstring machine_key;
		cdstring appl_key;
		GetRegistryPaths(machine_key, appl_key);

		// Get licensee name
		if (!CWinRegistry::ReadRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationLicensee, mLicensee) &&
			!CWinRegistry::ReadRegString(HKEY_CURRENT_USER, appl_key, cRegistrationLicensee, mLicensee))
			mLicensee = GetIllegalCopyStr();

		// Get organisation
		if (!CWinRegistry::ReadRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationOrganisation, mOrganisation) &&
			!CWinRegistry::ReadRegString(HKEY_CURRENT_USER, appl_key, cRegistrationOrganisation, mOrganisation))
			mOrganisation = cdstring::null_str;

		// Get serial number
		if (!CWinRegistry::ReadRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationSerialNumber, mSerialNumber) &&
			!CWinRegistry::ReadRegString(HKEY_CURRENT_USER, appl_key, cRegistrationSerialNumber, mSerialNumber))
			mSerialNumber = GetIllegalCopyStr();

		// Get app registration code
		if (!CWinRegistry::ReadRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationAppRegistration, mAppRegistrationKey) &&
			!CWinRegistry::ReadRegString(HKEY_CURRENT_USER, appl_key, cRegistrationAppRegistration, mAppRegistrationKey))
			mAppRegistrationKey = GetIllegalCopyStr();

		// Get crypto registration code
		if (!CWinRegistry::ReadRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationCryptoRegistration, mCryptoRegistrationKey) &&
			!CWinRegistry::ReadRegString(HKEY_CURRENT_USER, appl_key, cRegistrationCryptoRegistration, mCryptoRegistrationKey))
			mCryptoRegistrationKey = cdstring::null_str;	// Allowed to be empty => no plugin registration entered

		// Get crypto time key
		unsigned long temp = 1;
		if (!CWinRegistry::ReadRegInt(HKEY_LOCAL_MACHINE, machine_key, cCryptoRegistrationTime, temp) &&
			!CWinRegistry::ReadRegInt(HKEY_CURRENT_USER, appl_key, cCryptoRegistrationTime, temp))
			temp = 0;	// Allowed to be empty => no plugin registration entered
		else if (temp == 0)
			temp = 1;	// Zero value in registry must be mapped to 1 as zero means no registration defined
		mCryptoTimeout = (time_t) temp;
	}
}


// Clear registration resources
void CRegistration::ClearAppInfo(HMODULE hModule) const
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	// Clear licensee name
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cRegistrationLicensee);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cRegistrationLicensee);

	// Clear organisation
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cRegistrationOrganisation);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cRegistrationOrganisation);

	// Clear serial number
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cRegistrationSerialNumber);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cRegistrationSerialNumber);

	// Clear app registration
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cRegistrationAppRegistration);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cRegistrationAppRegistration);
}

// Make new registration resources
bool CRegistration::WriteAppInfo(HMODULE hModule) const
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	bool result = false;

	try
	{
		// Add licensee name
		if (!CWinRegistry::WriteRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationLicensee, mLicensee) &&
			!CWinRegistry::WriteRegString(HKEY_CURRENT_USER, appl_key, cRegistrationLicensee, mLicensee))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Add organisation
		if (!CWinRegistry::WriteRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationOrganisation, mOrganisation) &&
			!CWinRegistry::WriteRegString(HKEY_CURRENT_USER, appl_key, cRegistrationOrganisation, mOrganisation))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Add serial number
		if (!CWinRegistry::WriteRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationSerialNumber, mSerialNumber) &&
			!CWinRegistry::WriteRegString(HKEY_CURRENT_USER, appl_key, cRegistrationSerialNumber, mSerialNumber))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Verify existing codes
		if (!ValidInfoCurrent(mSerialNumber, mAppUserCode))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		
		// Get full code
		cdstring calc_key = GetFullCode(GetKeyV4());

		// Add app registration code
		if (!CWinRegistry::WriteRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationAppRegistration, calc_key) &&
			!CWinRegistry::WriteRegString(HKEY_CURRENT_USER, appl_key, cRegistrationAppRegistration, calc_key))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		ClearAppInfo(hModule);
		result = false;
	}

	return result;
}

time_t CRegistration::LoadAppTimeout(HMODULE hModule) const
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	// Get timeout
	unsigned long temp = 1;
	if (!CWinRegistry::ReadRegInt(HKEY_LOCAL_MACHINE, machine_key, cAppRegistrationTime, temp) &&
		!CWinRegistry::ReadRegInt(HKEY_CURRENT_USER, appl_key, cAppRegistrationTime, temp))
		temp = 0;
	else if (temp == 0)
		temp = 1;

	return (time_t) temp;
}


// Make new registration resources
bool CRegistration::WriteAppTimeout(HMODULE hModule) const
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	bool result = false;

	try
	{
		// Add timeout
		time_t timeout = time(NULL);
		if (!CWinRegistry::WriteRegInt(HKEY_LOCAL_MACHINE, machine_key, cAppRegistrationTime, timeout) &&
			!CWinRegistry::WriteRegInt(HKEY_CURRENT_USER, appl_key, cAppRegistrationTime, timeout))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		result = false;
	}

	return result;
}

#pragma mark ____________________________Plugin Info

// Clear CRC resources
void CRegistration::ClearPluginInfo(HMODULE hModule) const
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	// Clear crypto registration
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cRegistrationCryptoRegistration);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cRegistrationCryptoRegistration);

	// Clear crypto time key
	CWinRegistry::DeleteRegValue(HKEY_LOCAL_MACHINE, machine_key, cCryptoRegistrationTime);
	CWinRegistry::DeleteRegValue(HKEY_CURRENT_USER, appl_key, cCryptoRegistrationTime);
}

// Make new CRC resources
bool CRegistration::WritePluginInfo(HMODULE hModule, bool demo)
{
	cdstring machine_key;
	cdstring appl_key;
	GetRegistryPaths(machine_key, appl_key);

	// Clear existing first
	ClearPluginInfo(hModule);

	bool result = false;

	try
	{
		// if demo clear CRC
		if (demo)
		{
			// Generate demo timeout
			mCryptoTimeout = ::time(NULL);

			// Add timeout
			if (!CWinRegistry::WriteRegInt(HKEY_LOCAL_MACHINE, machine_key, cCryptoRegistrationTime, mCryptoTimeout) &&
				!CWinRegistry::WriteRegInt(HKEY_CURRENT_USER, appl_key, cCryptoRegistrationTime, mCryptoTimeout))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
		else
		{
			// Map key to crc
			if (mCryptoUserCode.length() != 8) throw -1L;
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
				throw -1L;

			// Recalc CRC for full encode
			crc = GetFullCRC(GetCryptoKeyV3());

			// Get crc as string
			mCryptoRegistrationKey = crc;

			// Add app registration code
			if (!CWinRegistry::WriteRegString(HKEY_LOCAL_MACHINE, machine_key, cRegistrationCryptoRegistration, mCryptoRegistrationKey) &&
				!CWinRegistry::WriteRegString(HKEY_CURRENT_USER, appl_key, cRegistrationCryptoRegistration, mCryptoRegistrationKey))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}

			// Clear timeout
			mCryptoTimeout = 0;
		}

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
bool CRegistration::DoAppRegister(bool& demo, HMODULE hModule)
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
	CPluginRegisterDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetPlugin(plugin, allow_demo, allow_delete);

	while(true)
	{
		// Let Dialog process events
		int result = dlog.DoModal();

		if (result == IDOK)
		{
			// Get details from fields
			mCryptoUserCode = dlog.mRegKey;

			// Replace existing resources
			okayed = WritePluginInfo(AfxGetResourceHandle(), false);
			if (okayed)
				break;
			else
				// Should display 'Illegal reg code' alert here
				CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalPlugin");
		}
		else if (result == IDCANCEL)
		{
			break;
		}
		else if (result == IDC_PLUGINREGISTER_REMOVE)
		{
			okayed = true;
			delete_it = true;
			break;
		}
		else if (result == IDC_PLUGINREGISTER_RUNDEMO)
		{
			// Write all plugin details
			okayed = WritePluginInfo(AfxGetResourceHandle(), true);
			if (okayed)
				demo = true;
			break;
		}
	}
	
	return okayed;
}

#pragma mark -

void CRegistration::GetRegistryPaths(cdstring& machine_key, cdstring& appl_key) const
{
	machine_key = "SOFTWARE\\";
	machine_key += ::AfxGetApp()->m_pszRegistryKey;
	machine_key += "\\";
	machine_key += ::AfxGetApp()->m_pszProfileName;
	machine_key += "\\";
	machine_key += cRegistrationSection;
	
	appl_key = "Software\\";
	appl_key += ::AfxGetApp()->m_pszRegistryKey;
	appl_key += "\\";
	appl_key += ::AfxGetApp()->m_pszProfileName;
	appl_key += "\\";
	appl_key += cRegistrationSection;
}
