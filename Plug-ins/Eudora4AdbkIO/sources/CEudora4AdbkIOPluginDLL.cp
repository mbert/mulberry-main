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

// CEudora4AdbkIOPluginDLL.cp
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

#include "CEudora4AdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "CStringUtils.h"
#include "cdstring.h"

#include <stdlib.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Eudora4 Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Eudora 4 address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "Eudora4";

const char* cEudoraAnon = "Anonymous";
const char* cEudoraAlias = "alias";
const size_t cEudoraAliasLen = 5;
const char* cEudoraNote = "note";
const size_t cEudoraNoteLen = 4;
const char* cEudoraName = "name:";
const size_t cEudoraNameLen = 5;
const char* cEudoraAddress = "address:";
const size_t cEudoraAddressLen = 8;
const char* cEudoraPhone = "phone:";
const size_t cEudoraPhoneLen = 6;
const char* cEudoraFax = "fax:";
const size_t cEudoraFaxLen = 4;
const char cEudoraBreak = '\03';
const char* cEudoraCommaSpace = ", ";
const size_t cEudoraCommaSpaceLen = 2;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
const char* cEudoraLineEnd = "\r";
const size_t cEudoraLineEndLen = 1;
#elif __dest_os == __win32_os
const char* cEudoraLineEnd = "\r\n";
const size_t cEudoraLineEndLen = 2;
#elif __dest_os == __linux_os
const char* cEudoraLineEnd = "\n";
const size_t cEudoraLineEndLen = 1;
#endif

#pragma mark ____________________________CEudora4AdbkIOPluginDLL

// Constructor
CEudora4AdbkIOPluginDLL::CEudora4AdbkIOPluginDLL()
{
}

// Destructor
CEudora4AdbkIOPluginDLL::~CEudora4AdbkIOPluginDLL()
{
}

// Initialise plug-in
void CEudora4AdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "Eudora4 Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CEudora4AdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CEudora4AdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CEudora4AdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CEudora4AdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CEudora4AdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CEudora4AdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CEudora4AdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CEudora4AdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CEudora4AdbkIOPluginDLL::ImportAddresses(char* data)
{
	// Sort lines into alias list and notes map
	cdstrvect alias;
	cdstrmap notes;
	char* p = data;
	
	while(*p)
	{
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
			
			// Now add to list
			alias.push_back(p);
	
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
			
			// Set nick-name
			char* name = ::strgetquotestr(&p);
			
			// Now add to map
			notes.insert(cdstrmap::value_type(name, p));
	
			// Set position to next line
			p = q;
		}
		else
			// Unrecognised token: go to end of line
			while(*p && (*p != '\r')) p++;
		
		// Bump past blank lines
		while((*p == '\r') || (*p == '\n')) p++;
	}
	
	// Now do each alias
	for(cdstrvect::iterator iter = alias.begin(); iter != alias.end(); iter++)
	{
		p = const_cast<char*>((*iter).c_str());
	
		// Set nick-name
		char* adl = ::strgetquotestr(&p);
		
		// Create address list from remaining text
		SAdbkIOPluginAddress* list = AddressListParse(p);
		
		// Check for single address
		if (list && (list[0].mNumFields != 0) && (list[1].mNumFields == 0))
		{
			// Look for matching notes
			cdstrmap::iterator found = notes.find(adl);
			if (found != notes.end())
				ParseNotes(&list[0], (*found).second);

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
	}

	return 1;
}

// Export an address
long CEudora4AdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	const char* write = nil;
	
	// Write header
	::fwrite(cEudoraAlias, 1, cEudoraAliasLen, mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write nick-name (adl)
	if (!addr->mNickName || !*addr->mNickName)
	{
		// Try email instead
		if (!addr->mEmail || !*addr->mEmail)
			write = cEudoraAnon;
		else
			write = addr->mEmail;
	}
	else
		write = addr->mNickName;
	if (::strchr(write, ' '))
		::fwrite("\"", 1, 1, mExportFile);
	::fwrite(write, 1, ::strlen(write), mExportFile);
	if (::strchr(write, ' '))
		::fwrite("\"", 1, 1, mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write email address
	if (addr->mEmail && *addr->mEmail)
	{
		const char* email = addr->mEmail;
		::fwrite(email, 1, ::strlen(email), mExportFile);
	}

	::fwrite(cEudoraLineEnd, 1, cEudoraLineEndLen, mExportFile);
	
	// Now write a note if required
	if ((addr->mName && *addr->mName) ||
		(addr->mAddress && *addr->mAddress) ||
		(addr->mPhoneWork && *addr->mPhoneWork) ||
		(addr->mPhoneHome && *addr->mPhoneHome) ||
		(addr->mFax && *addr->mFax) ||
		(addr->mNotes && *addr->mNotes))
	{
		
		// Write header
		::fwrite(cEudoraNote, 1, cEudoraNoteLen, mExportFile);
		::fwrite(" ", 1, 1, mExportFile);

		// Write nick-name (adl) (already determined)
		if (::strchr(write, ' '))
			::fwrite("\"", 1, 1, mExportFile);
		::fwrite(write, 1, ::strlen(write), mExportFile);
		if (::strchr(write, ' '))
			::fwrite("\"", 1, 1, mExportFile);
		::fwrite(" ", 1, 1, mExportFile);

		// Do fax
		if (addr->mFax && *addr->mFax)
		{
			::fwrite("<", 1, 1, mExportFile);
			::fwrite(cEudoraFax, 1, cEudoraFaxLen, mExportFile);
			::fwrite(addr->mFax, 1, ::strlen(addr->mFax), mExportFile);
			::fwrite(">", 1, 1, mExportFile);
		}

		// Do phone
		if ((addr->mPhoneWork && *addr->mPhoneWork) ||
			(addr->mPhoneHome && *addr->mPhoneHome))
		{
			const char* text = (addr->mPhoneWork && *addr->mPhoneWork) ? addr->mPhoneWork : addr->mPhoneHome;
			::fwrite("<", 1, 1, mExportFile);
			::fwrite(cEudoraPhone, 1, cEudoraPhoneLen, mExportFile);
			::fwrite(text, 1, ::strlen(text), mExportFile);
			::fwrite(">", 1, 1, mExportFile);
		}

		// Do address
		if (addr->mAddress && *addr->mAddress)
		{
			// Convert CRLFs
			char* text  = ConvertFromCRLF(addr->mAddress);

			::fwrite("<", 1, 1, mExportFile);
			::fwrite(cEudoraAddress, 1, cEudoraAddressLen, mExportFile);
			::fwrite(text, 1, ::strlen(text), mExportFile);
			::fwrite(">", 1, 1, mExportFile);
			
			delete text;
		}

		// Do name
		if (addr->mName && *addr->mName)
		{
			::fwrite("<", 1, 1, mExportFile);
			::fwrite(cEudoraName, 1, cEudoraNameLen, mExportFile);
			::fwrite(addr->mName, 1, ::strlen(addr->mName), mExportFile);
			::fwrite(">", 1, 1, mExportFile);
		}

		// Do notes
		if (addr->mNotes && *addr->mNotes)
		{
			char* text  = ConvertFromCRLF(addr->mNotes);
			::fwrite(text, 1, ::strlen(text), mExportFile);
			delete text;
		}

		::fwrite(cEudoraLineEnd, 1, cEudoraLineEndLen, mExportFile);
	}
	return 1;
}

// Export a group
long CEudora4AdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	const char* write;
	
	// Write header
	::fwrite(cEudoraAlias, 1, cEudoraAliasLen, mExportFile);
	::fwrite(" ", 1, 1, mExportFile);

	// Write nick-name
	if (!grp->mNickName || (::strlen(grp->mNickName) == 0))
	{
		// Try email instead
		if (!grp->mName || !*grp->mName)
			write = cEudoraAnon;
		else
			write = grp->mName;
	}
	else
		write = grp->mNickName;
	if (::strchr(write, ' '))
		::fwrite("\"", 1, 1, mExportFile);
	::fwrite(write, 1, ::strlen(write), mExportFile);
	if (::strchr(write, ' '))
		::fwrite("\"", 1, 1, mExportFile);
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

	::fwrite(cEudoraLineEnd, 1, cEudoraLineEndLen, mExportFile);

	// Now write a note if required
	if (grp->mName && *grp->mName)
	{
		
		// Write header
		::fwrite(cEudoraNote, 1, cEudoraNoteLen, mExportFile);
		::fwrite(" ", 1, 1, mExportFile);

		// Write nick-name (adl) (already determined)
		if (::strchr(write, ' '))
			::fwrite("\"", 1, 1, mExportFile);
		::fwrite(write, 1, ::strlen(write), mExportFile);
		if (::strchr(write, ' '))
			::fwrite("\"", 1, 1, mExportFile);
		::fwrite(" ", 1, 1, mExportFile);

		// Do name
		if (grp->mName && *grp->mName)
		{
			::fwrite("<", 1, 1, mExportFile);
			::fwrite(cEudoraName, 1, cEudoraNameLen, mExportFile);
			::fwrite(grp->mName, 1, ::strlen(grp->mName), mExportFile);
			::fwrite(">", 1, 1, mExportFile);
		}

		::fwrite(cEudoraLineEnd, 1, cEudoraLineEndLen, mExportFile);
	}
	return 1;
}

// Convert CRLFs -> 0x03
char* CEudora4AdbkIOPluginDLL::ConvertFromCRLF(const char* str)
{
	// Duplicate it
	char* text = ::strdup(str);
	
	// Convert it
	char* p = text;
	char* q = p;
	
	while(*p)
	{
		switch(*p)
		{
		case '\r':
			*q++ = cEudoraBreak;
			p++;
			
			// Sniff next char
			if (*p == '\n')
				p++;
			break;
		case '\n':
			*q++ = cEudoraBreak;
			p++;
			break;
		default:
			*q++ = *p++;
		}
	}
	*q = 0;
	
	return text;
}

// Convert 0x03 -> CRLFs
char* CEudora4AdbkIOPluginDLL::ConvertToCRLF(const char* str)
{
	// Count the number
	long ctr = 0;
	const char* c = str;
	while(*c)
	{
		if (*c == cEudoraBreak)
			ctr++;
		c++;
	}

	// Create space for new string
	char* text = new char[::strlen(str) + ctr + 1];
	
	// Convert it
	const char* p = str;
	char* q = text;
	
	while(*p)
	{
		switch(*p)
		{
		case cEudoraBreak:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			*q++ = '\r';
#elif __dest_os == __win32_os
			*q++ = '\r';
			*q++ = '\n';
#elif __dest_os == __linux_os
			*q++ = '\n';
#endif
			p++;
			break;
		default:
			*q++ = *p++;
		}
	}
	*q = 0;
	
	return text;
}

const char cRFC822_QUOTE[] = " ()<>@,;:\\\".[]/?=";
const char cRFC822_ESCAPE[] = "\\\"";

// Do quote etc of friendly name
void CEudora4AdbkIOPluginDLL::AddressOut(const char* str)
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
CAdbkIOPluginDLL::SAdbkIOPluginAddress* CEudora4AdbkIOPluginDLL::AddressListParse(char* str)
{
	// Make local copy of text to be tokenised
	char* s = str;

	char* p = s;
	while(*p == ' ') p++;
	char* q = s;
	char* adl = nil;
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
		{
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
		}

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
			adl = nil;
			break;

		default:
			// Just advance
			p++;
		}
	}

	return list;

}

// Parse address from text
void CEudora4AdbkIOPluginDLL::ParseAddress(SAdbkIOPluginAddress* addr, const char* txt)
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

// Parse address from text
void CEudora4AdbkIOPluginDLL::ParseNotes(SAdbkIOPluginAddress* addr, const char* txt)
{
	char* p = const_cast<char*>(txt);
	while(*p == ' ') p++;
	
	while(*p)
	{
		// Look for special field
		if (*p == '<')
		{
			p++;
			if (::strncmp(p, cEudoraName, cEudoraNameLen) == 0)
			{
				p += cEudoraNameLen;
				while(*p == ' ') p++;
				addr->mName = p;
				while(*p && (*p != '>')) p++;
				if (*p) *p++ = 0;
			}
			if (::strncmp(p, cEudoraAddress, cEudoraAddressLen) == 0)
			{
				p += cEudoraAddressLen;
				while(*p == ' ') p++;
				addr->mAddress = p;
				while(*p && (*p != '>')) p++;
				if (*p) *p++ = 0;
				addr->mAddress = ConvertToCRLF(addr->mAddress);
			}
			if (::strncmp(p, cEudoraPhone, cEudoraPhoneLen) == 0)
			{
				p += cEudoraPhoneLen;
				while(*p == ' ') p++;
				addr->mPhoneWork = p;
				while(*p && (*p != '>')) p++;
				if (*p) *p++ = 0;
			}
			if (::strncmp(p, cEudoraFax, cEudoraFaxLen) == 0)
			{
				p += cEudoraFaxLen;
				while(*p == ' ') p++;
				addr->mFax = p;
				while(*p && (*p != '>')) p++;
				if (*p) *p++ = 0;
			}
		}
		else
		{
			// Have a note
			addr->mNotes = p;
			while(*p && (*p != '\r')) p++;
			if (*p) *p++ = 0;
			addr->mNotes = ConvertToCRLF(addr->mNotes);
		}
	}
}
