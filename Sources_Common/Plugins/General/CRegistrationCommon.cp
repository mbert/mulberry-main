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
#include "CLog.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryApp.h"
#endif
#include "CPluginRegisterDialog.h"

#include <cstdlib>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char cIllegalCopy[] = "illegal copy";
#define crc_key_v1 0x12CDAB00
#define crc_key_v2 0x20CDAB00
#define crc_key_v3 "30CDABOO"
#if __dest_os == __linux_os
#define crc_key_v4 crc_key_v3
#else
#define crc_key_v4 "40CDABOO"
#endif
const OSType cCryptoPluginKey = ('Mlby' | 'PGP5');

#pragma mark ____________________________statics

CRegistration CRegistration::sRegistration;

#pragma mark ____________________________CRegistration

CRegistration::CRegistration()
{
	mLicensee = cIllegalCopy;
	mSerialNumber = cIllegalCopy;
	mAppTimeout = 0;
	mCryptoTimeout = 0;
}

// Copy licensee
void CRegistration::SetLicensee(const cdstring& licensee)
{
	if (licensee.length())
		mLicensee = licensee;
	else
		mLicensee = cIllegalCopy;
}

// Copy Organisation
void CRegistration::SetOrganisation(const cdstring& organisation)
{
	if (organisation.length())
		mOrganisation = organisation;
	else
		mOrganisation = cdstring::null_str;
}

// Copy serial number
void CRegistration::SetSerialNumber(const cdstring& serialNumber)
{
	if (serialNumber.length())
		mSerialNumber = serialNumber;
	else
		mSerialNumber = cIllegalCopy;
}

#pragma mark ____________________________App Info

bool CRegistration::ValidInfoCurrent(const cdstring& serial, const cdstring& regkey) const
{
	// Must have valid serial number to start off with
	if (!ValidSerial(serial))
		return false;

	// Check length validity
	if (regkey.length() != 16)
		return false;

	// Get hashed key
	cdstring calc_key = DigestCode(serial, GetKeyV4());
	
	// Check hash against key
	if (regkey != calc_key)
		return false;
	
	return true;
}

bool CRegistration::ValidInfoOld(const cdstring& serial, const cdstring& regkey) const
{
	return ValidInfoV3(serial, regkey) || ValidInfoV1_2(serial, regkey);
}

bool CRegistration::ValidInfoV3(const cdstring& serial, const cdstring& regkey) const
{
	// Must have valid serial number to start off with
	if (!ValidSerial(serial))
		return false;

	// Check length validity
	if (regkey.length() != 16)
		return false;

	// Get hashed key
	cdstring calc_key = DigestCode(serial, GetKeyV3());
	
	// Check hash against key
	if (regkey != calc_key)
		return false;
	
	return true;
}

bool CRegistration::ValidInfoV1_2(const cdstring& serial, const cdstring& regkey) const
{
	// Must have valid serial number to start off with
	if (!ValidSerial(serial))
		return false;

	// Map key to crc
	if (regkey.length() != 8)
		return false;
	unsigned long test = ~::strtoul(regkey.c_str(), NULL, 16);

	// Do crc calc
	unsigned long crc_v1 = 0;
	unsigned long crc_v2 = 0;

	// Do not allow failure
	try
	{
		crc_v1 = CRCCalculate(serial, GetCRCKeyV1());
		crc_v2 = CRCCalculate(serial, GetCRCKeyV2());
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Check key and crc
	if ((crc_v1 != test) && (crc_v2 != test))
		return false;
	
	return true;
}

// Validate registration
bool CRegistration::ValidSerial(const cdstring& serial) const
{
	// Reject specific range of serial numbers
	if (!LegalSerial(serial))
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalCopy");
		return false;
	}
		
	// Check data if serial number == U
	if (*serial.c_str() == 'U')
	{
		// Valid format 'U-nnnnnn'
		const char* p = serial.c_str();
		if ((serial.length() != 8) ||
			(*++p != '-') ||
			!isdigit(*++p) || !isdigit(*++p) || !isdigit(*++p) ||
			!isdigit(*++p) || !isdigit(*++p) || !isdigit(*++p))
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalCopy");
			return false;
		}
	}
		
	// Check data if serial number == D for Demo
	else if (*serial.c_str() == 'D')
	{
		// Get maximum values from licensee (format "Dxxx-mmm-yyyy")
		long max_year = 0;
		long max_month = 0;

		// Get month
		unsigned long month = 0;
		::strncpy((char*) &month, ((const char*) serial) + ::strlen(serial) - 8, 3);

		switch(htonl(month))
		{
		case 'Jan\0':
			max_month = 1;
			break;
		case 'Feb\0':
			max_month = 2;
			break;
		case 'Mar\0':
			max_month = 3;
			break;
		case 'Apr\0':
			max_month = 4;
			break;
		case 'May\0':
			max_month = 5;
			break;
		case 'Jun\0':
			max_month = 6;
			break;
		case 'Jul\0':
			max_month = 7;
			break;
		case 'Aug\0':
			max_month = 8;
			break;
		case 'Sep\0':
			max_month = 9;
			break;
		case 'Oct\0':
			max_month = 10;
			break;
		case 'Nov\0':
			max_month = 11;
			break;
		case 'Dec\0':
			max_month = 12;
			break;
		default:
			max_month = 0;
		}

		// Get year
		max_year = ::atol(((const char*) serial) + ::strlen(serial) - 4);

		time_t systime = ::time(NULL);
		struct tm* currtime = ::localtime(&systime);

		if ((currtime->tm_year + 1900 > max_year) ||
			((currtime->tm_year + 1900 == max_year) && (currtime->tm_mon + 1 > max_month)))
		{
			// Force quit if over time limit
			CErrorHandler::PutStopAlertRsrc("Alerts::General::NoDemoTime");
			return false;
		}
	}
	
	return true;
}

// Do copy protection checking
bool CRegistration::LegalSerial(const cdstring& serial) const
{
	// Reject specific range of serial numbers
	char serial_char = *serial.c_str();

	return !(serial.empty() ||
				(serial_char < 'D') ||
				(serial_char > 'W') ||
				(serial_char == 'I') ||
				(serial_char == 'O'));
}

#pragma mark ____________________________Tests

bool CRegistration::CheckAppRegistration(bool current) const
{
	// Current is V3 check
	cdstring calc_key = GetFullCode(GetKeyV4());
	return calc_key == mAppRegistrationKey;
}

bool CRegistration::CheckCryptoRegistration() const
{
	bool result = false;

	try
	{
		// Map key to crc
		unsigned long test = ::strtoul(mCryptoRegistrationKey.c_str(), NULL, 10);

		// Do crc calc
		unsigned long crc = GetFullCRC(GetCryptoKeyV3());

		result = (test == crc);
	}
	catch(...)
	{
		result = false;
	}

	return result;
}

bool CRegistration::AppDemoExpired(void) const
{
	// Get time difference
	double diff = ::difftime(::time(NULL), mAppTimeout);
	
	// Check that it can run
	return (diff >= 31L*24L*60L*60L);
}

bool CRegistration::CryptoDemoExpired(void) const
{
	// Get time difference
	double diff = ::difftime(::time(NULL), mCryptoTimeout);
	
	// Check that it can run
	return (diff >= 31L*24L*60L*60L);
}

bool CRegistration::CryptoDemoValid(void) const
{
	// Zero means not set
	return mCryptoTimeout != 0;
}

#pragma mark -

unsigned long CRegistration::GetFullCRC(unsigned long key) const
{
	cdstring encode = mLicensee + mOrganisation + mSerialNumber;
	return CRCCalculate(encode, key);
}

// Calculate CRC code from text
unsigned long CRegistration::CRCCalculate(const cdstring& str, unsigned long key) const
{
	short i;
	size_t len = str.length();
	unsigned long value = len;
	const unsigned char* text = (const unsigned char*) str.c_str();
	while(len--)
	{
		value ^= *text++;
		for(i = 0; i < 8; i++) {
			if (value & 0x0001)
				value = (value >> 1) ^ key;
			else
				value >>= 1;
		}
	}

	return value;
}

cdstring CRegistration::GetFullCode(const cdstring& key) const
{
	cdstring encode = mLicensee + mOrganisation + mSerialNumber;
	return DigestCode(encode, key);
}

cdstring CRegistration::DigestCode(const cdstring& str, const cdstring& key) const
{
	// Append key
	cdstring txt = str;
	txt += key;
	
	// Get MD5 hash
	cdstring digest;
	txt.md5(digest);
	
	// Chop to 16 characters
	digest.c_str_mod()[16] = 0;
	
	return digest;
}

const char* CRegistration::GetIllegalCopyStr() const
{
	return cIllegalCopy;
}
	
unsigned long CRegistration::GetCRCKeyV1() const
{
	return crc_key_v1;
}

unsigned long CRegistration::GetCRCKeyV2() const
{
	return crc_key_v2;
}

const char* CRegistration::GetKeyV3() const
{
	return crc_key_v3;
}

unsigned long CRegistration::GetCryptoKeyV3() const
{
	return cCryptoPluginKey;
}

const char* CRegistration::GetKeyV4() const
{
	return crc_key_v4;
}
