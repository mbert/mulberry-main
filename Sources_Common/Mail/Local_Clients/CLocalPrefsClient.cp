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


// Code for Local Prefs client class

#include "CLocalPrefsClient.h"

#include "CGeneralException.h"
#include "CINETCommon.h"
#include "CLocalCommon.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#include <cerrno>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_STREAM(x) \
	{ if ((x).fail()) { int err_no = os_errno; CLOG_LOGTHROW(CGeneralException, err_no); throw CGeneralException(err_no); } }

#pragma mark -


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CLocalPrefsClient::CLocalPrefsClient(COptionsProtocol* options_owner)
	: COptionsClient(options_owner)
{
	mOwner = options_owner;

	// Init instance variables
	InitPrefsClient();

} // CLocalPrefsClient::CLocalPrefsClient

// Copy constructor
CLocalPrefsClient::CLocalPrefsClient(const CLocalPrefsClient& copy, COptionsProtocol* options_owner)
	: COptionsClient(copy, options_owner)
{
	mOwner = options_owner;

	// Init instance variables
	InitPrefsClient();

	mCWD = copy.mCWD;

} // CLocalPrefsClient::CLocalPrefsClient

CLocalPrefsClient::~CLocalPrefsClient()
{
} // CLocalPrefsClient::~CLocalPrefsClient

void CLocalPrefsClient::InitPrefsClient()
{
} // CLocalPrefsClient::CLocalPrefsClient

// Create duplicate, empty connection
CINETClient* CLocalPrefsClient::CloneConnection()
{
	// Copy construct this
	return new CLocalPrefsClient(*this, GetOptionsOwner());

} // CLocalPrefsClient::CloneConnection

#pragma mark ____________________________Start/Stop

// Start TCP
void CLocalPrefsClient::Open()
{
	// Reset only
	Reset();
}

// Reset account
void CLocalPrefsClient::Reset()
{
	// get CWD from owner
	mCWD = GetOptionsOwner()->GetOfflineCWD();

	// Must append dir delim if not present
	if (mCWD.length() && (mCWD[mCWD.length() - 1] != os_dir_delim))
		mCWD += os_dir_delim;
}

// Release TCP
void CLocalPrefsClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CLocalPrefsClient::Abort()
{
	// Local does nothing

}

#pragma mark ____________________________Login & Logout

// Logon to IMAP server
void CLocalPrefsClient::Logon()
{
	// Must fail if empty CWD
	if (mCWD.empty())
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
		throw CINETException(CINETException::err_BadResponse);
	}

	// Open prefs file
	try
	{
		// Open mailbox
		cdstring fname;
		GetFileName(fname);
		mPrefs.clear();
		mPrefs.open(fname, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
		CHECK_STREAM(mPrefs);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

} // CLocalPrefsClient::Logon

// Logoff from IMAP server
void CLocalPrefsClient::Logoff()
{
	try
	{
		// Close prefs file
		mPrefs.close();
		CHECK_STREAM(mPrefs);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

} // CLocalPrefsClient::Logoff

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CLocalPrefsClient::_Tickle(bool force_tickle)
{
	// Local does nothing

} // CLocalPrefsClient::_Tickle

// Handle failed capability response
void CLocalPrefsClient::_PreProcess()
{
	// No special preprocess

} // CLocalPrefsClient::_PreProcess

// Handle failed capability response
void CLocalPrefsClient::_PostProcess()
{
	// No special postprocess

} // CLocalPrefsClient::_PostProcess

#pragma mark ____________________________Options

// Find all options below this key
void CLocalPrefsClient::_FindAllAttributes(const cdstring& entry)
{
	StINETClientAction action(this, "Status::IMSP::Getting", "Error::IMSP::OSErrGet", "Error::IMSP::NoBadGet");

	cdstring option = cWILDCARD;

	// Go to start of file
	mPrefs.seekg(0, std::ios_base::beg);
	
	// Read lines from file
	while(!mPrefs.fail())
	{
		// Get a line - may end with any endl
		cdstring line;
		getline(mPrefs, line, 0);
		
		// Split into key & value
		char* key = ::strtok(line.c_str_mod(), "=");
		char* value = ::strtok(NULL, "\r\n");

		// Insert into map - ignore duplicates
		if (key && ::strpmatch(key, option))
		{
			if (!value)
				value = cdstring::null_str;

			// Insert
			std::pair<cdstrmap::iterator, bool> result = GetOptionsOwner()->GetMap()->insert(cdstrmap::value_type(key, value));

			// Does it exist already
			if (!result.second)
				// Replace existing
				(*result.first).second = value;
		}

	}
	mPrefs.clear();
}

// Set all options in owmer map
void CLocalPrefsClient::_SetAllAttributes(const cdstring& entry)
{
	StINETClientAction action(this, "Status::IMSP::Setting", "Error::IMSP::OSErrSet", "Error::IMSP::NoBadSet");

	// Close, clear any errors and reopen with truncation
	mPrefs.close();
	mPrefs.clear();
	cdstring fname;
	GetFileName(fname);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	StCreatorType file(cMulberryCreator, cPrefFileType);
#endif
	mPrefs.open(fname, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

	// Go to start of file
	mPrefs.seekp(0, std::ios_base::beg);
	
	for(cdstrmap::const_iterator iter = GetOptionsOwner()->GetMap()->begin(); iter != GetOptionsOwner()->GetMap()->end(); iter++)
	{
		mPrefs << (*iter).first;
		mPrefs.put('=');
		mPrefs << (*iter).second;
		mPrefs << os_endl;
	}
}

// Delete the entire entry
void CLocalPrefsClient::_DeleteEntry(const cdstring& entry)
{
	// Cannot do this on flat file!
}

// Get option
void CLocalPrefsClient::_GetAttribute(const cdstring& entry, const cdstring& attribute)
{
	// Cannot do this on flat file!
}

// Set option
void CLocalPrefsClient::_SetAttribute(const cdstring& entry, const cdstring& attribute, const cdstring& value)
{
	// Cannot do this on flat file!
}

// Unset option with this key
void CLocalPrefsClient::_DeleteAttribute(const cdstring& entry, const cdstring& attribute)
{
	// Cannot do this on flat file!
}

#pragma mark ____________________________Local Ops

void CLocalPrefsClient::GetFileName(cdstring& name)
{
	name = mCWD + "Default.mbp";
}
