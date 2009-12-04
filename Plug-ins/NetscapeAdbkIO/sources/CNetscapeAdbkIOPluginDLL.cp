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

// CNetscapeAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Netscape address book IO DLL based plug-in for use in Mulberry.
//
// History:
// 13-Jan-1998: Created initial header and implementation.
//

#include "CNetscapeAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "cdstring.h"
#include "CURL.h"
#include "CStringUtils.h"
#include "HTML_Tags.h"

#include <string.h>
#include <stdlib.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Netscape v3 Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Netscape v3 address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "NetscapeV3";

const char* cNetscapeAnon = "Anonymous";
const char* cNetscapeTab = "\t";
const size_t cNetscapeTabLen = 1;
const char* cNetscapeCommaSpace = ", ";
const size_t cNetscapeCommaSpaceLen = 2;
const char* cNetscapeQuoteSpace = "\" ";
const size_t cNetscapeQuoteSpaceLen = 2;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
const char* cNetscapeLineEnd = "\r";
const size_t cNetscapeLineEndLen = 1;
#elif __dest_os == __win32_os
const char* cNetscapeLineEnd = "\r\n";
const size_t cNetscapeLineEndLen = 2;
#elif __dest_os == __linux_os
const char* cNetscapeLineEnd = "\n";
const size_t cNetscapeLineEndLen = 1;
#endif

const char html_indent[] = "    ";
const char html_address_start[] = "<DT><A HREF=\"mailto:";
const char html_address_aliasid[] = "ALIASID=\"";
const char html_address_aliasof[] = "ALIASOF=\"";
const char html_address_nickname[] = "NICKNAME=\"";

const char html_group_start[] = "<DT><H3";
const char html_group_folded[] = "FOLDED";
const char html_group_entry_start[] = "<DL><p>";
const char html_group_entry_stop[] = "</DL><p>";

const char html_tag_end[] = ">";
const char html_quote_tag_end[] = "\">";

#pragma mark ____________________________CNetscapeAdbkIOPluginDLL

// Constructor
CNetscapeAdbkIOPluginDLL::CNetscapeAdbkIOPluginDLL()
{
}

// Destructor
CNetscapeAdbkIOPluginDLL::~CNetscapeAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CNetscapeAdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "Netscape v3 Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CNetscapeAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CNetscapeAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CNetscapeAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CNetscapeAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CNetscapeAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CNetscapeAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CNetscapeAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CNetscapeAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CNetscapeAdbkIOPluginDLL::ImportAddresses(char* data)
{
	char* p = data;
	char* adl;
	char* name;
	char  whole_name[256];
	char* eaddr;
	char* notes;

	while (*p)
	{
		// nil out pointers at start
		adl = nil;
		name = nil;
		whole_name[0] = '\0';
		eaddr = nil;
		notes = nil;

		// Remove leading white space
		while(*p == ' ') p++;
		
		// Look for either group or address
		char* addr = ::strstr(p, html_address_start);
		char* grp = ::strstr(p, html_group_start);

		// Determine what to parse
		if (addr && ((addr < grp) || !grp))
		{
			// Create empty address
			CAdbkIOPluginAddress iaddr;
	
			// Import the address and advance ptr past it
			if (ImportNetscapeAddress(addr, iaddr) == 0)
				return 0;
			p = addr;

			// Give address to application
			(*mImportCallback)(iaddr.GetAddressData(), 0);

		}
		else if (grp && ((grp < addr) || !addr))
		{
			// Create empty address
			CAdbkIOPluginGroup igrp;
	
			// Import the group and advance ptr past it
			if (ImportNetscapeGroup(grp, igrp) == 0)
				return 0;
			p = grp;

			// Give address to application
			(*mImportCallback)(igrp.GetGroupData(), 1);
		}
		else
			break;
		
	}
	
	return 1;
}

long CNetscapeAdbkIOPluginDLL::ImportNetscapeAddress(char*& data, CAdbkIOPluginAddress& iaddr)
{
	char* adl = nil;
	char* name = nil;
	char* eaddr = nil;
	char* notes = nil;

	// Advance past tag and point to email address
	data += ::strlen(html_address_start);
	eaddr = data;
	
	// Step up to quote and terminate
	while(*data && (*data != '\"')) data++;
	if (!*data)
		return 0;
	if (*data == '\"')
		*data++ = '\0';
	
	// Decode URL
	char* p = eaddr;
	char* q = eaddr;
	while(*p)
	{
		// Is current char escape
		if (*p == cURLEscape)
		{
			// Advance past escape
			p++;

			// Copy nibbles of quoted char
			char c = (cFromHex[(unsigned char) *p++] << 4);
			c |= cFromHex[(unsigned char) *p++];
			*q++ = c;
		}
		else
			*q++ = *p++;
	}
	*q = 0;

	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for alias id - ignore
	if (::stradvstrcmp(&data, html_address_aliasid) == 0)
	{
		// Step up to quote and terminate
		while(*data && (*data != '\"')) data++;
		if (!*data)
			return 0;
		if (*data == '\"')
			*data++ = '\0';
	}
	
	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for alias of - ignore
	if (::stradvstrcmp(&data, html_address_aliasof) == 0)
	{
		// Step up to quote and terminate
		while(*data && (*data != '\"')) data++;
		if (!*data)
			return 0;
		if (*data == '\"')
			*data++ = '\0';
	}
	
	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for nick-name
	if (::stradvstrcmp(&data, html_address_nickname) == 0)
	{
		// Got nick-name
		adl = data;

		// Step up to quote and terminate
		while(*data && (*data != '\"')) data++;
		if (!*data)
			return 0;
		if (*data == '\"')
			*data++ = '\0';
	}
	
	// Step over tag end
	data++;
	
	// Got name
	name = data;
	
	// Step to CR or end and back to tag
	while(*data && (*data != '\r')) data++;
	if (!*data)
		return 0;
	char* end = data - ::strlen(html__Link);
	*end = '\0';
	if (*data == '\r')
	{
		*data++ = '\0';
		if (*data == '\n')
			data++;
	}
	
	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for notes
	if (ImportNetscapeNotes(data, notes) == 0)
		return 0;

	// Fill in fields
	iaddr.mAddress.mNickName = adl;
	iaddr.mAddress.mName = name;
	iaddr.mAddress.mEmail = eaddr;
	iaddr.mAddress.mNotes = notes;
	
	// return address
	return 1;
}

long CNetscapeAdbkIOPluginDLL::ImportNetscapeGroup(char*& data, CAdbkIOPluginGroup& igrp)
{
	char* adl = nil;
	char* name = nil;
	char* notes = nil;

	// Advance past tag and point to email address
	data += ::strlen(html_group_start);

	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for folded - ignore
	::stradvstrcmp(&data, html_group_folded);
	
	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for alias id - ignore
	if (::stradvstrcmp(&data, html_address_aliasid) == 0)
	{
		// Step up to quote and terminate
		while(*data && (*data != '\"')) data++;
		if (!*data)
			return 0;
		if (*data == '\"')
			*data++ = '\0';
	}

	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for nick-name
	if (::stradvstrcmp(&data, html_address_nickname) == 0)
	{
		// Got nick-name
		adl = data;

		// Step up to quote and terminate
		while(*data && (*data != '\"')) data++;
		if (!*data)
			return 0;
		if (*data == '\"')
			*data++ = '\0';
	}
	
	// Step over tag end
	data++;
	
	// Got name
	name = data;
	
	// Step to CR or end and back to tag
	while(*data && (*data != '\r')) data++;
	if (!*data)
		return 0;
	char* end = data - ::strlen(html__Heading3);
	*end = '\0';
	if (*data == '\r')
	{
		*data++ = '\0';
		if (*data == '\n')
			data++;
	}
	
	// If no name use nick-name
	if (!::strlen(name))
		name = adl;

	// Set fields
	igrp.mGroup.mNickName = adl;
	igrp.mGroup.mName = name;

	// Step over space
	while(*data && (*data == ' ')) data++;
	if (!*data)
		return 0;
	
	// Look for notes - ignore
	if (ImportNetscapeNotes(data, notes) == 0)
		return 0;

	// Look for entry
	if (::stradvstrcmp(&data, html_group_entry_start) == 0)
	{
		// Step over space & CR (CRLF)
		while(*data && ((*data == ' ') || (*data == '\r') || (*data == '\n'))) data++;
		if (!*data)
			return 0;
	
		while(::stradvstrcmp(&data, html_group_entry_stop) != 0)
		{
			// Look for address
			if (::strncmp(data, html_address_start, ::strlen(html_address_start)) == 0)
			{
				// Create empty address
				CAdbkIOPluginAddress iaddr;
		
				// Import the address and advance ptr past it
				if (ImportNetscapeAddress(data, iaddr) == 0)
					return 0;

				// Add to list
				igrp.AddAddress(iaddr.mAddress.mEmail);
			}
			
			// Look for group
			else if (::stradvstrcmp(&data, html_group_start) == 0)
			{
				// Step over space
				while(*data && (*data == ' ')) data++;
				if (!*data)
					return 0;
				
				// Look for alias of - ignore
				if (::stradvstrcmp(&data, html_address_aliasof) == 0)
				{
					// Step up to quote and terminate
					while(*data && (*data != '\"')) data++;
					if (!*data)
						return 0;
					if (*data == '\"')
						*data++ = '\0';
				}
				
				// Bump over end tag
				data++;
				
				// Got name
				char* subgrp_name = data;

				// Step to CR or end and back to tag
				while(*data && (*data != '\r')) data++;
				if (!*data)
					return 0;
				char* end = data - ::strlen(html__Heading3);
				*end = '\0';
				if (*data == '\r')
				{
					*data++ = '\0';
					if (*data == '\n')
						data++;
				}

				// Add to list
				igrp.AddAddress(subgrp_name);

				// Step over space
				while(*data && (*data == ' ')) data++;
				if (!*data)
					return 0;
				
				// Look for notes - ignore
				if (ImportNetscapeNotes(data, notes) == 0)
					return 0;
			}
		}
	}

	// Return group
	return 1;
}

// Bump over notes and terminate
long CNetscapeAdbkIOPluginDLL::ImportNetscapeNotes(char*& data, char*& notes)
{

	// Zero out to start
	notes = nil;

	// Look for notes
	if (::stradvstrcmp(&data, html_GlossaryEntry) == 0)
	{
		notes = data;

		// Step until CRLF and terminate
		while(*data && (*data != '\r') && (*data != '\n'))
			data++;
		if (!*data)
			return 0;
		*data++ = '\0';
		if (*data == '\n')
			data++;

		// Step over space
		while(*data && (*data == ' ')) data++;
		if (!*data)
			return 0;
	}
	
	return 1;
}

// Start export
long CNetscapeAdbkIOPluginDLL::StartExport(const char* fname)
{
	// Do inherited
	if (CAdbkIOPluginDLL::StartExport(fname))
	{
		// Write file header

		// Write title
		::fwrite(html_Title, 1, ::strlen(html_Title), mExportFile);
		const char* _fname = ::strrchr(fname, os_dir_delim);
		if (_fname)
			_fname++;
		else
			_fname = fname;
		::fwrite(_fname, 1, ::strlen(_fname), mExportFile);
		::fwrite(html__Title, 1, ::strlen(html__Title), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);

		// Heading - same as title
		::fwrite(html_Heading1, 1, ::strlen(html_Heading1), mExportFile);
		::fwrite(_fname, 1, ::strlen(_fname), mExportFile);
		::fwrite(html__Heading1, 1, ::strlen(html__Heading1), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
		
		// Start glossary of addresses
		::fwrite(html_Glossary, 1, ::strlen(html_Glossary), mExportFile);
		::fwrite(html_Paragraph, 1, ::strlen(html_Paragraph), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
	}

	return mExportFile != nil;
}

// Export an address
long CNetscapeAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	return ExportNetscapeAddress(addr, 1);
}

// Export an address
long CNetscapeAdbkIOPluginDLL::ExportNetscapeAddress(SAdbkIOPluginAddress* addr, short indent)
{
	cdstring write;
	
	// Write address start
	for(short i = 1; i <= indent; i++)
		::fwrite(html_indent, 1, ::strlen(html_indent), mExportFile);
	::fwrite(html_address_start, 1, ::strlen(html_address_start), mExportFile);

	// Write mail address
	write = addr->mEmail;
	write.EncodeURL();
	::fwrite(write.c_str(), 1, write.length(), mExportFile);

	// Write nick-name (adl)
	if (addr->mNickName && *addr->mNickName)
	{
		::fwrite(cNetscapeQuoteSpace, 1, cNetscapeQuoteSpaceLen, mExportFile);
		::fwrite(html_address_nickname, 1, ::strlen(html_address_nickname), mExportFile);
		
		// Convert to Netscape nickname (lowercase and numbers only)
		write = addr->mNickName;
		{
			const char* p = write.c_str();
			char* q = (char*) write.c_str();
			
			while(*p)
			{
				// Check for valid character
				if (((*p >= '0') && (*p <= '9')) ||
					((*p >= 'a') && (*p <= 'z')))
					
					// Copy valid character
					*q++ = *p++;

				else if ((*p >= 'A') && (*p <= 'Z'))
				
					// Switch to lower
					*q++ = ::tolower(*p++);
				
				else
				
					// Skip invalid
					*p++;
			}
			*q = '\0';
		}
		::fwrite(write.c_str(), 1, write.length(), mExportFile);
	}
	::fwrite(html_quote_tag_end, 1, ::strlen(html_quote_tag_end), mExportFile);

	// Write name
	if (addr->mName && *addr->mName)
		::fwrite(addr->mName, 1, ::strlen(addr->mName), mExportFile);
	
	// Write address end of line
	::fwrite(html__Link, 1, ::strlen(html__Link), mExportFile);
	::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
	
	// Write notes
	if (addr->mNotes && *addr->mNotes)
	{
		for(short i = 1; i <= indent + 1; i++)
			::fwrite(html_indent, 1, ::strlen(html_indent), mExportFile);
		::fwrite(html_GlossaryEntry, 1, ::strlen(html_GlossaryEntry), mExportFile);
		::fwrite(addr->mNotes, 1, ::strlen(addr->mNotes), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
	}
	
	return 1;
}

// Export a group
long CNetscapeAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	const char* txt;
	char write[256];
	
	// Write group start
	::fwrite(html_indent, 1, ::strlen(html_indent), mExportFile);
	::fwrite(html_group_start, 1, ::strlen(html_group_start), mExportFile);

	// Write nick-name
	if (((txt = grp->mNickName) != nil) && ::strlen(txt))
	{
		::fwrite(" ", 1, 1, mExportFile);
		::fwrite(html_address_nickname, 1, ::strlen(html_address_nickname), mExportFile);

		// Convert to Netscape nickname (lowercase and no space)
		::strcpy(write, txt);
		{
			char* p = write;
			char* q = write;
			
			while(*p)
			{
				// Check for valid character
				if (((*p >= '0') && (*p <= '9')) ||
					((*p >= 'a') && (*p <= 'z')))
					
					// Copy valid character
					*q++ = *p++;
				
				else if ((*p >= 'A') && (*p <= 'Z'))
				
					// Switch to lower
					*q = ::tolower(*p++);
				
				else
				
					// Skip invalid
					*p++;
			}
			*q = '\0';
		}
		::fwrite(write, 1, ::strlen(write), mExportFile);
		::fwrite(html_quote_tag_end, 1, ::strlen(html_quote_tag_end), mExportFile);
	}
	else
		::fwrite(html_tag_end, 1, ::strlen(html_tag_end), mExportFile);

	// Write name
	if (((txt = grp->mName) != nil) && ::strlen(txt))
		::fwrite(txt, 1, ::strlen(txt), mExportFile);
	
	// Write group end of line
	::fwrite(html__Heading3, 1, ::strlen(html__Heading3), mExportFile);
	::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
	
	// No group notes
	
	// Write any individuals
	if (grp->mAddresses)
	{
		// Entry header start
		::fwrite(html_indent, 1, ::strlen(html_indent), mExportFile);
		::fwrite(html_group_entry_start, 1, ::strlen(html_group_entry_start), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);

		const char** p = grp->mAddresses;
		while(*p)
		{
			CAdbkIOPluginAddress addr;
			addr.mAddress.mEmail = *p++;
			ExportNetscapeAddress(addr.GetAddressData(), 2);
		}

		// Entry header stop
		::fwrite(html_indent, 1, ::strlen(html_indent), mExportFile);
		::fwrite(html_group_entry_stop, 1, ::strlen(html_group_entry_stop), mExportFile);
		::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);
	}

	return 1;
}

// Stop export
long CNetscapeAdbkIOPluginDLL::StopExport(void)
{
	// Write file trailer

	// Stop glossary of addresses
	::fwrite(html__Glossary, 1, ::strlen(html__Glossary), mExportFile);
	::fwrite(html_Paragraph, 1, ::strlen(html_Paragraph), mExportFile);
	::fwrite(cNetscapeLineEnd, 1, cNetscapeLineEndLen, mExportFile);

	// Return inherited
	return CAdbkIOPluginDLL::StopExport();
}
