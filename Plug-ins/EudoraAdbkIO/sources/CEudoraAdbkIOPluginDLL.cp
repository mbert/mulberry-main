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

// CEudoraAdbkIOPluginDLL.cp
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
//

#include "CEudoraAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "CStringUtils.h"

#include <string.h>
#include <stdlib.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Eudora Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Eudora address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "Eudora";

const char* cEudoraAnon = "Anonymous";
const char* cEudoraAlias = "alias";
const size_t cEudoraAliasLen = 5;
const char* cEudoraNote = "note";
const char cEudoraBreak = '\03';
const char* cEudoraCommaSpace = ", ";
const size_t cEudoraCommaSpaceLen = 2;

#pragma mark ____________________________CEudoraAdbkIOPluginDLL

// Constructor
CEudoraAdbkIOPluginDLL::CEudoraAdbkIOPluginDLL()
{
}

// Destructor
CEudoraAdbkIOPluginDLL::~CEudoraAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CEudoraAdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "Eudora Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CEudoraAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CEudoraAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CEudoraAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CEudoraAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CEudoraAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CEudoraAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CEudoraAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CEudoraAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CEudoraAdbkIOPluginDLL::ImportAddresses(char* data)
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
		
		// Look for alias or notes
		if (::stradvtokcmp(&p, cEudoraAlias) == 0)
		{
			// Look for end of line and terminate
			char* q = ::strchr(p, '\r');
			if (q)
				*q++ = '\0';
			else
				q = p + ::strlen(p);
			
			// Remove leading white space
			while(*p == ' ') p++;
			
			// Set nick-name
			adl = p;
			
			while(*p && (*p != ' ')) p++;
			if (*p)
				*p++ = '\0';
			
			// Create address list from remaining text
			SAdbkIOPluginAddress* list = AddressListParse(p);
			
			// Check for single address
			if (list && (list[0].mNumFields != 0) && (list[1].mNumFields == 0))
			{
				// Set nick-name in single item
				list[0].mNickName = adl;

				// Give address to application
				(*mImportCallback)(&list[0], 0);
			}
			else if (list)
			{
				// Got group
				CAdbkIOPluginGroup grp;
				grp.mGroup.mNickName = adl;
				grp.mGroup.mName = adl;
				
				SAdbkIOPluginAddress* plist = list;
				while(plist->mNumFields)
				{
					// Add all address to group
					grp.AddAddress(plist->mEmail);
					plist++;
				}
				
			// Give group to application
				(*mImportCallback)(grp.GetGroupData(), 1);
			}
			
			if (list)
				::free(list);
			
			// Set position to next line
			p = q;
		}
		else if (::stradvtokcmp(&p, cEudoraNote) == 0)
		{
			// Look for end of line and terminate
			char* q = ::strchr(p, '\r');
			if (q)
				*q++ = '\0';
			else
				q = p + ::strlen(p);
			
			// Remove leading white space
			while(*p == ' ') p++;
			
			while(*p && (*p != ' ')) p++;
			if (*p)
				*p++ = '\0';
			
			// Set position to next line
			p = q;
		}
		else
			// Unrecognised token: go to end of line
			while(*p && (*p != '\r')) p++;
		
		// Bump past blank lines
		while((*p == '\r') || (*p == '\n')) p++;
	}
	
	return 1;
}

// Export an address
long CEudoraAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	const char* write = nil;
	
	// Write header
	::fwrite(cEudoraAlias, 1, cEudoraAliasLen, mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write nick-name (adl)
	if (!addr->mNickName || (::strlen(addr->mNickName) == 0))
		write = cEudoraAnon;
	else
		write = addr->mNickName;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write user name
	if (addr->mName && *addr->mName)
		AddressOut(addr->mName);
	
	// Write email address
	write = addr->mEmail;
	::fwrite(write, 1, ::strlen(write), mExportFile);

	::fwrite(os_endl, 1, os_endl_len, mExportFile);
	
	return 1;
}

// Export a group
long CEudoraAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	const char* write;
	
	// Write header
	::fwrite(cEudoraAlias, 1, cEudoraAliasLen, mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write nick-name
	if (!grp->mNickName || (::strlen(grp->mNickName) == 0))
		write = cEudoraAnon;
	else
		write = grp->mNickName;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Loop over all addresses
	if (grp->mAddresses)
	{
		bool first = true;
		const char** addr = grp->mAddresses;
		while(*addr)
		{
			// Write ", " before address if not first
			if (!first)
				::fwrite(cEudoraCommaSpace, 1, cEudoraCommaSpaceLen, mExportFile);
			else
				first = false;

			// Write address
			::fwrite(*addr, 1, ::strlen(*addr), mExportFile);
			addr++;
		}
	}

	::fwrite(os_endl, 1, os_endl_len, mExportFile);

	return 1;
}

const char cRFC822_QUOTE[] = " ()<>@,;:\\\".[]/?=";
const char cRFC822_ESCAPE[] = "\\\"";

// Do quote etc of friendly name
void CEudoraAdbkIOPluginDLL::AddressOut(const char* str)
{
	unsigned char* p = (unsigned char*) (char*) str;

	// Check for specials
	if (::strpbrk(str, cRFC822_QUOTE))
	{
		::fwrite("\"", 1, 1, mExportFile);

		// May need further quoting
		if (!::strpbrk(str, cRFC822_ESCAPE))
			::fwrite(str, 1, ::strlen(str), mExportFile);
		else
		{
			const char* p = str;
			do
			{
				switch(*p)
				{
				// Quote these
				case '\\':
				case '\"':
					::fwrite("\\", 1, 1, mExportFile);
				// Fall through
				default:
					::fwrite(p, 1, 1, mExportFile);
				}
			} while (*++p);
		}
		::fwrite("\"", 1, 1, mExportFile);
	}
	else
		::fwrite(str, 1, ::strlen(str), mExportFile);;
}

// Parse an address list
CAdbkIOPluginDLL::SAdbkIOPluginAddress* CEudoraAdbkIOPluginDLL::AddressListParse(char* str)
{
	// Make local copy of text to be tokenised
	char* s = str;

	char* p = s;
	while(*p == ' ') p++;
	char* q = s;
	bool more = true;
	SAdbkIOPluginAddress* list = nil;
	long list_num = 0;

	// Check each character - need to balance "É" & (É)
	while(more)
	{
		switch (*p)
		{

			case '"':
				// Cache for adl
				p++;

				// Find match and terminate and step over
				while(*p)
				{
					if (*p == '\\')
					{
						p++;
						if (*p) p++;
					}
					else if (*p != '"') p++;
					else break;
				}
				if (*p)
					p++;
				else
					more = false;
				break;

			case '(':
				long level;
				level = 0;
				// Find match and step over
				while(*p && ((*p != ')') || (level > 1)))
				{
					// Balance nested brackets
					switch (*p)
					{

						case '(':
							level++;
							break;

						case ')':
							level--;
							break;

						default:;
					}
					p++;
				}
				if (*p)
					p++;
				else
					more = false;
				break;

			case '\0':
				// Force exit next time
				more = false;

				// Check that there is something to copy
				if (p==q) break;

				// Fall through to get address
			case '\r':
			case '\n':
			case ',':
				// Tie off
				*p++ = '\0';

				// Is there anyone there?
				if (!::strlen(q))
				{
					// Do nothing
				}
				// Check for full email address (includes an @)
				else
				{
					// Add new item to list
					if (!list)
					{
						list_num = 1;
						list = (SAdbkIOPluginAddress*) ::malloc((list_num + 1) * sizeof(SAdbkIOPluginAddress));
					}
					else
						list = (SAdbkIOPluginAddress*) ::realloc(list, (++list_num + 1) * sizeof(SAdbkIOPluginAddress));
					
					// Fill in current fields
					list[list_num - 1].mNumFields = 10;
					list[list_num - 1].mNickName = nil;
					list[list_num - 1].mName = nil;
					list[list_num - 1].mEmail = nil;
					list[list_num - 1].mCompany = nil;
					list[list_num - 1].mAddress = nil;
					list[list_num - 1].mPhoneWork = nil;
					list[list_num - 1].mPhoneHome = nil;
					list[list_num - 1].mFax = nil;
					list[list_num - 1].mURL = nil;
					list[list_num - 1].mNotes = nil;
					ParseAddress(&list[list_num - 1], q);

					// Fill in terminator field
					list[list_num].mNumFields = 0;
				}

				// Strip leading space, CR's, LF's & ','s and point to next bit
				while((*p == ' ') || (*p == '\r') || (*p == '\n') || (*p == ',')) p++;
				q = p;

				// Wipe name
				break;

			default:
				// Just advance
				p++;
		}
	}

	return list;

}

// Parse address from text
void CEudoraAdbkIOPluginDLL::ParseAddress(SAdbkIOPluginAddress* addr, const char* txt)
{
	// Only if something present
	if (!txt)
		return;

	// Make local copy of text for tokenising
	const char* s = txt;

	char* p = (char*) s;
	while(*p == ' ') p++;
	char* q = p;

	while (true)
	{

		switch (*q)
		{

			case '\r':
			case '\n':
			case '\0':
				// Terminate and copy
				*q = 0;
				if (p < q) addr->mEmail = p;
				return;

			case '\"':
			//case '\'':
			{
				// Step until term (remember escape chars)
				p = q++;
				while (*q)
				{
					if (*q == '\\')
					{
						// Bump past escape pair
						q++;
						if (*q) q++;
					}
					else if (*q != *p) q++;
					else break;
				}
				if (!*q)
				{
					return;
				}

				// Need to check for following domain
				char* r = q+1;
				while(*r == ' ')
					r++;

				// If no domain its a quoted name so just copy name
				if (*r != '@')
				{
					// tie off
					*q++ = '\0';
					p++;

					// Unescape name
					//cdstring tmp = p;
					//tmp.FilterOutEscapeChars();
					// Copy name and carry on
					addr->mName = p;

					while(*q == ' ')
						q++;
					p = q;
					break;
				}

				// Got quoted mailbox and domain so set up to fall through to '@' case
				q = r;
			}

			case '@':
			{
				// Goto end of address and terminate
				q += ::strcspn(q, WHITE_SPACE);
				bool finished = false;
				if ((*q == '\r') || (*q == '\n'))
				{
					finished = true;
				}
				else if (*q == '\0')
				{
					finished = true;
				}
				*q = '\0';
				addr->mEmail = p;

				if (!finished)
				{
					// Advance over null and strip spaces
					q++;
					p = ::strgetbrastr(&q);

					if (p) addr->mName = p;
				}

				q++;
				return;
			}

			case '<':
				// Was there anything before?
				if (p != q)
				{
					// Tie off and get name
					*q = '\0';
					addr->mName = p;
				}

				// Step until close
				p = ++q;
				while (*q && (*q != '>')) q++;
				if (!*q)
				{
					return;
				}

				// tie off
				*q++ = '\0';
				addr->mEmail = p;

				return;

			default:
				q++;
		}
	}

	return;
}
