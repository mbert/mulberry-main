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

// COutlookExAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Eudora address book IO DLL based plug-in for use in Mulberry.
//
// History:
// 14-Jan-1998: Created initial header and implementation.
// 04-Feb-1999: Modified for Eudora 4 format.
//

#include "COutlookExAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "CStringUtils.h"

#include "cdstring.h"
#include "CCharSpecials.h"

#include <vector.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Outlook Express Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Outlook/Outlook Express address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "Outlook";

	enum
	{
		eOEFirstName = 0,
		eOELastName,
		eOENickname,
		eOECompany,
		eOEWorkAddress1,
		eOEWorkAddress2,
		eOEWorkAddress3,
		eOEWorkCity,
		eOEWorkState,
		eOEWorkZip,
		eOEWorkCountry,
		eOEWorkURL,
		eOEHomeAddress1,
		eOEHomeAddress2,
		eOEHomeAddress3,
		eOEHomeCity,
		eOEHomeState,
		eOEHomeZip,
		eOEHomeCountry,
		eOEHomeURL,
		eOEHomePhone1,
		eOEHomeFax,
		eOEWorkPhone1,
		eOEWorkFax,
		eOEEmailAddress1,
		eOENotes,
		eOEIgnore
	};

const char* cFields[][4] =  {{"First Name", NULL},
							{"Last Name", NULL},
							{"Nickname", NULL},
							{"Company", NULL},
							{"Work Street Address(Line 1)", "Business Street", NULL},
							{"Work Street Address(Line 2)", "Business Street 2", NULL},
							{"Work Street Address(Line 3)", "Business Street 3", NULL},
							{"Work City", "Business City", NULL},
							{"Work State", "Business State", NULL},
							{"Work Zip", "Business Postal Code", NULL},
							{"Work Country", "Business Country", NULL},
							{"Work URL", "Business URL", "Web Page", NULL},
							{"Home Street Address(Line 1)", "Home Street", NULL},
							{"Home Street Address(Line 2)", "Home Street 2", NULL},
							{"Home Street Address(Line 3)", "Home Street 3", NULL},
							{"Home City", NULL},
							{"Home State", NULL},
							{"Home Zip", "Home Postal Code", NULL},
							{"Home Country", NULL},
							{"Home URL", NULL},
							{"Home Phone 1", "Home Phone", NULL},
							{"Home Fax", NULL},
							{"Work Phone 1", "Business Phone", NULL},
							{"Work Fax", "Business Fax", NULL},
							{"Email Address 1", "E-mail Address", NULL},
							{"Notes", NULL},
							{NULL}};

#pragma mark ____________________________COutlookExAdbkIOPluginDLL

// Constructor
COutlookExAdbkIOPluginDLL::COutlookExAdbkIOPluginDLL()
{
}

// Destructor
COutlookExAdbkIOPluginDLL::~COutlookExAdbkIOPluginDLL()
{
}

// Initialise plug-in
void COutlookExAdbkIOPluginDLL::Initialise(void)
{
	// Do default
	CAdbkIOPluginDLL::Initialise();

	// Assign UI name
	::strncpy(mAdbkIOInfo.mAdbkIOType, cPluginIOName, 255);
	mAdbkIOInfo.mAdbkIOType[255] = 0;

	// Type of opertaion allowed
	mAdbkIOInfo.mDoesImport = true;
	mAdbkIOInfo.mDoesExport = true;

	// Assigned file types
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mAdbkIOInfo.mTypes[0] = 0L;
	mAdbkIOInfo.mTypes[1] = 0L;
	mAdbkIOInfo.mTypes[2] = 0L;
	mAdbkIOInfo.mTypes[3] = 0L;

	mAdbkIOInfo.mCreator = '????';
#elif __dest_os == __win32_os
	::strncpy(mAdbkIOInfo.mTypes, "Outlook/Express Exported Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool COutlookExAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool COutlookExAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool COutlookExAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* COutlookExAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long COutlookExAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType COutlookExAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* COutlookExAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* COutlookExAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long COutlookExAdbkIOPluginDLL::ImportAddresses(char* data)
{
	// NB - no groups OE does not seem to export groups so we don't need to import them

	// First line contains titles
	char* p = data;
	while(*p && (*p != '\r') && (*p != '\n')) p++;
	cdstring titles(data, p - data);

	// Tokenise titles to get field positions
	vector<unsigned long> indices;

	// Get the first field
	char* q = titles.c_str_mod();
	char* r = q;
	while(*r && (*r != '\t')) r++;
	if (*r)
		*r++ = 0;
	while(r != q)
	{
		// Compare field against all the field titles we know about
		int i = 0;
		bool found_field = false;
		for(; !found_field && (i < eOEIgnore); i++)
		{
			// Remove any quotes around field title
			cdstring txt(q);
			txt.unquote();
			
			// Compare against Outlook/OE titles
			const char** fields = cFields[i];
			while(*fields)
			{
				if (!::strcmpnocase(txt.c_str(), *fields))
				{
					// Add matching index to array
					indices.push_back(i);
					found_field = true;
					break;
				}
				
				fields++;
			}
		}

		// Field title not found => ignore it
		if (i == eOEIgnore)
			indices.push_back(eOEIgnore);

		// Punt to next field
		q = r;
		while(*r && (*r != '\t')) r++;
		if (*r)
			*r++ = 0;
	}

	// Skip line-end
	data = p;
	while((*data == '\r') || (*data == '\n')) data++;;

	// Array of data items for the one's we know about
	char* items[eOEIgnore];
	
	// Loop until done
	while(data && *data)
	{
		// Initialise pointers
		for(int i = 0; i < eOEIgnore; i++)
			items[i] = NULL;

		// Read in each field and assign to array
		bool got_one = false;
		for(int i = 0; i < indices.size(); i++)
		{
			// Get quoted string and advance
			bool lineend = false;
			char* p = getquotestr(&data, lineend);
			
			// Add to array in appropriate place
			if (p && (indices[i] != eOEIgnore))
			{
				items[indices[i]] = p;
				got_one = true;
			}
			
			// Exit if no more items
			if (lineend)
				break;
		}

		// Terminate loop if nothing found
		if (!got_one)
			break;

		// Now create the address
		SAdbkIOPluginAddress addr;
		addr.mNumFields = 10;

		addr.mNickName = items[eOENickname];

		cdstring name;
		if (items[eOEFirstName])
			name += items[eOEFirstName];
		if (items[eOELastName])
		{
			if (name.length())
				name += " ";
			name += items[eOELastName];
		}
		addr.mName = name;

		addr.mEmail = items[eOEEmailAddress1];

		addr.mCompany = items[eOECompany];

		cdstring address;
		if (items[eOEWorkAddress1] || items[eOEWorkAddress2] || items[eOEWorkAddress3] ||
			items[eOEWorkCity] || items[eOEWorkState] || items[eOEWorkCountry])
		{
			if (items[eOEWorkAddress1])
				address += items[eOEWorkAddress1];
			if (items[eOEWorkAddress2])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEWorkAddress2];
			}
			if (items[eOEWorkAddress3])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEWorkAddress3];
			}
			if (items[eOEWorkCity])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEWorkCity];
			}
			if (items[eOEWorkState])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEWorkState];

				if (items[eOEWorkZip])
				{
					address += " ";
					address += items[eOEWorkZip];
				}
			}
			if (items[eOEWorkCountry])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEWorkCountry];
			}
		}
		else if (items[eOEHomeAddress1] || items[eOEHomeAddress2] || items[eOEHomeAddress3] ||
			items[eOEHomeCity] || items[eOEHomeState] || items[eOEHomeCountry])
		{
			if (items[eOEHomeAddress1])
				address += items[eOEHomeAddress1];
			if (items[eOEHomeAddress2])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEHomeAddress2];
			}
			if (items[eOEHomeAddress3])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEHomeAddress3];
			}
			if (items[eOEHomeCity])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEHomeCity];
			}
			if (items[eOEHomeState])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEHomeState];

				if (items[eOEHomeZip])
				{
					address += " ";
					address += items[eOEHomeZip];
				}
			}
			if (items[eOEHomeCountry])
			{
				if (address.length())
					address += os_endl;
				address += items[eOEHomeCountry];
			}
		}
		addr.mAddress = address;

		addr.mPhoneWork = items[eOEWorkPhone1];

		addr.mPhoneHome = items[eOEHomePhone1];

		if (items[eOEWorkFax])
			addr.mFax = items[eOEWorkFax];
		else if (items[eOEHomeFax])
			addr.mFax = items[eOEHomeFax];
		else
			addr.mFax = NULL;

		if (items[eOEWorkURL])
			addr.mURL = items[eOEWorkURL];
		else if (items[eOEHomeURL])
			addr.mURL = items[eOEHomeURL];
		else
			addr.mURL = NULL;

		addr.mNotes = items[eOENotes];

		// Callback to import it
		(*mImportCallback)(&addr, 0);
	}

	return 1;
}

// Start export
long COutlookExAdbkIOPluginDLL::StartExport(const char* fname)
{
	// Do inherited first
	long result = CAdbkIOPluginDLL::StartExport(fname);

	if (result)
	{
		// Write title details
		for(int i = eOEFirstName; i < eOEIgnore; i++)
		{
			if (i > eOEFirstName)
				::fwrite("\t", 1, 1, mExportFile);
			::fwrite(cFields[i][0], 1, ::strlen(cFields[i][0]), mExportFile);
		}
		::fwrite(os_endl, 1, os_endl_len, mExportFile);
	}

	return result;
}

// Export an address
long COutlookExAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	// First name
	const char* p = addr->mName ? ::strchr(addr->mName, ' ') : NULL;
	if (p)
	{
		cdstring s(addr->mName, p - addr->mName);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Last Name
	if (p)
		p++;
	else
		p = addr->mName;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Nickname
	p = addr->mNickName;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Company
	p = addr->mCompany;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Address
	p = addr->mAddress;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t\t\t\t\t\t\t", 1, 7, mExportFile);

	// URL
	p = addr->mURL;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t\t\t\t\t\t\t\t\t", 1, 9, mExportFile);

	// Home phone
	p = addr->mPhoneHome;
	if (p)
		::fwrite(p, 1, ::strlen(p), mExportFile);
	::fwrite("\t", 1, 1, mExportFile);

	// Home Fax
	bool faxed = false;
	if (!addr->mPhoneWork && addr->mPhoneHome)
	{
		p = addr->mFax;
		faxed = true;
	}
	else
		p = NULL;
	if (p)
		::fwrite(p, 1, ::strlen(p), mExportFile);
	::fwrite("\t", 1, 1, mExportFile);

	// Work phone
	p = addr->mPhoneWork;
	if (p)
		::fwrite(p, 1, ::strlen(p), mExportFile);
	::fwrite("\t", 1, 1, mExportFile);

	// Work Fax
	p = faxed ? NULL : addr->mFax;
	if (p)
		::fwrite(p, 1, ::strlen(p), mExportFile);
	::fwrite("\t", 1, 1, mExportFile);

	// Email
	p = addr->mEmail;
	if (p)
		::fwrite(p, 1, ::strlen(p), mExportFile);
	::fwrite("\t", 1, 1, mExportFile);

	// Notes
	p = addr->mNotes;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}

	// End of line
	::fwrite(os_endl, 1, os_endl_len, mExportFile);

	return 1;
}

// Export a group
long COutlookExAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	// First name
	::fwrite("\t", 1, 1, mExportFile);

	// Last Name
	const char* p = grp->mName;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Nickname
	p = grp->mNickName;
	if (p)
	{
		cdstring s(p);
		if (s.length())
		{
			s.quote(true, cCEscapeChar);
			::fwrite(s.c_str(), 1, s.length(), mExportFile);
		}
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Company
	::fwrite("\t", 1, 1, mExportFile);

	// Address
	::fwrite("\t\t\t\t\t\t\t", 1, 7, mExportFile);

	// URL
	::fwrite("\t\t\t\t\t\t\t\t\t", 1, 9, mExportFile);

	// Home phone
	::fwrite("\t", 1, 1, mExportFile);

	// Home Fax
	::fwrite("\t", 1, 1, mExportFile);

	// Work phone
	::fwrite("\t", 1, 1, mExportFile);

	// Work Fax
	::fwrite("\t", 1, 1, mExportFile);

	// Email
	const char** addrs = grp->mAddresses;
	if (addrs)
	{
		cdstring s;
		while(*addrs)
		{
			if (s.length())
				s += os_endl;
			s += *addrs;
			addrs++;
		}
		s.FilterInEscapeChars(cCEscapeChar);
		::fwrite(s.c_str(), 1, s.length(), mExportFile);
	}
	::fwrite("\t", 1, 1, mExportFile);

	// Notes

	// End of line
	::fwrite(os_endl, 1, os_endl_len, mExportFile);

	return 1;
}

// Get possibly quoted string
char* COutlookExAdbkIOPluginDLL::getquotestr(char** s1, bool& lineend)
{
	char*	start = *s1;
	char*	end;

	// Remove leading space
	while(*start == ' ') start++;

	// Ignore null strs
	if (!*start) return nil;

	// Handle quoted string
	if (*start=='"')
	{
		end = ++start;

		// Advance string ignoring quoted
		while(*end != '\"')
		{
			switch(*end)
			{
			case 0:
				// Nothing left so error
				return nil;
			case '\\':
				end += 2;
				break;
			default:
				end++;
			}
		}

		// Terminate string at enclosing quote and adjust rtn ptr past it
		*end++ = 0;
		
		// Punt over trailing tab or line end
		if (*end == '\t')
		{
			end++;
			lineend = false;
		}
		else if (*end == '\r')
		{
			end++;
			if (*end == '\n')
				end++;
			lineend = true;
		}
		else if (*end == '\n')
		{
			end++;
			lineend = true;
		}
		*s1 = end;

		// Must unescape always!
		::FilterOutEscapeChars(start);

		// Start past first quote
		return start;
	}

	// Handle unquoted atom
	else
	{
		// Find first delimiter
		end = ::strpbrk(start, "\t\r\n");

		// Terminate string at space (advance past it) or point to end of atom
		if (end)
		{
			char ender = *end;
			*end = 0;
			end++;
			
			switch(ender)
			{
			case '\r':
				lineend = true;
				if (*end == '\n') end++;
				break;
			case '\n':
				lineend = true;
				break;
			default:
				lineend = false;
				break;
			}
		}
		else
		{
			end = start + ::strlen(start);
			lineend = true;
		}
		
		*s1 = end;

		return start;
	}
}
