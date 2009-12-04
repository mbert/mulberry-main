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


// Source for CFileOptionsMap class

#include "CFileOptionsMap.h"

#include "CLog.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#ifdef PREFS_USE_LSTREAM
#include "CMulberryCommon.h"
#include <strstream>
#if __dest_os == __linux_os
#include "UNX_LStream.h"
#endif
#endif
// __________________________________________________________________________________________________
// C L A S S __ C D E F A U L T O P T I O N S M A P
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Copy from raw list
CFileOptionsMap::CFileOptionsMap()
{
	mArchive = NULL;
}

// Default destructor
CFileOptionsMap::~CFileOptionsMap(void)
{
	mArchive = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Read map
bool CFileOptionsMap::ReadMap(void)
{
	// Assume file already open
	if (!mArchive) return false;

	// Wipe any existing map
	mMap.clear();

	// Get key/values one line at a time
#ifdef PREFS_USE_LSTREAM
	// Read entire stream into memory as c-string
	char* txt = new char[mArchive->GetLength() + 1];
	long count = mArchive->GetLength();
	mArchive->GetBytes(txt, count);
	txt[mArchive->GetLength()] = 0;

	try
	{
		char* p = txt;
		char* line = p;
		while(p && *p)
		{
			// Get line of text
			while(*p && (*p != '\r') && (*p != '\n')) p++;

			if (*p)
			{
				*p++ = 0;
				while((*p == '\r') && (*p == '\n')) p++;
			}

			// Split into key & value
			char* key = ::strtok(line, "=");
			char* value = ::strtok(NULL, "\r\n");

			// Insert into map - ignore duplicates
			if (key)
			{
				if (value)
					mMap.insert(cdstrmap::value_type(key, value));
				else
					mMap.insert(cdstrmap::value_type(key, cdstring::null_str));
			}

			// Set to next line
			line = p;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	delete txt;
#elif __dest_os == __win32_os
	try
	{
		// Read one character at a time looking for line end
		cdstring line;
		cdstring buffer;
		buffer.reserve(1024);
		char* p = buffer.c_str_mod();
		size_t len = 0;
		char c;
		while(mArchive->Read(&c, 1))
		{
			if ((c != '\r') && (c != '\n'))
			{
			
				// Check buffer overflow
				if (len == 1024)
				{
					line.append(buffer, 0, len);
					p = buffer.c_str_mod();
					len = 0;
				}

				// Write char into buffer
				*p++ = c;
				len++;

				continue;
			}
			
			if (c == '\r')
			{
				// Grab remaining buffer
				if (len != 0)
				{
					line.append(buffer, 0, len);
					p = buffer.c_str_mod();
					len = 0;
				}

				// Split into key & value
				char* key = ::strtok(line.c_str_mod(), "=");
				char* value = ::strtok(NULL, "\r\n");

				// Insert into map - ignore duplicates
				if (key)
				{
					if (value)
						mMap.insert(cdstrmap::value_type(key, value));
					else
						mMap.insert(cdstrmap::value_type(key, cdstring::null_str));
				}
				
				// Reset for next line
				line.clear();
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
#endif

	return true;
}

// Write map
bool CFileOptionsMap::WriteMap(bool verify)
{
	// Assume file already open
	if (!mArchive) return false;

	// Write map to file
#ifdef PREFS_USE_LSTREAM
	std::ostrstream out;
#endif

	for(cdstrmap::const_iterator iter = mMap.begin(); iter != mMap.end(); iter++)
	{
#ifdef PREFS_USE_LSTREAM
		out << (*iter).first;
		out << '=';
		out << (*iter).second;
		out << os_endl;
#elif __dest_os == __win32_os
		mArchive->Write((*iter).first.c_str(), (*iter).first.length());
		mArchive->Write("=", 1);
		mArchive->Write((*iter).second.c_str(), (*iter).second.length());
		mArchive->Write(os_endl, os_endl_len);
#endif
	}

#ifdef PREFS_USE_LSTREAM
	mArchive->SetMarker(0, streamFrom_Start);
	mArchive->SetLength(0);
	long len = out.pcount();
	mArchive->PutBytes(out.str(), len);
	out.freeze(false);
#endif

	return true;
}

// Delete map
void CFileOptionsMap::DeleteMap(void)
{
	// Assume file already open
	if (!mArchive) return;

	// Clear map on close
	mMap.clear();
}
