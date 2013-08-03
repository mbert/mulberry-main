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

// CPineAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Pine address book IO DLL based plug-in for use in Mulberry.
//
// History:
// 13-Jan-1998: Created initial header and implementation.
//

#include "CPineAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "CStringUtils.h"

#include <string.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#endif

#pragma mark ____________________________consts

const char* cPluginName = "Pine Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "Pine address book import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "Pine";

const char* cPineAnon = "Anonymous";
const char* cPineTab = "\t";
const size_t cPineTabLen = 1;
const char* cPineCommaSpace = ", ";
const size_t cPineCommaSpaceLen = 2;

#pragma mark ____________________________CPineAdbkIOPluginDLL

// Constructor
CPineAdbkIOPluginDLL::CPineAdbkIOPluginDLL()
{
}

// Destructor
CPineAdbkIOPluginDLL::~CPineAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CPineAdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "pine/PC-pine Addressbook (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CPineAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CPineAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CPineAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CPineAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CPineAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CPineAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CPineAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CPineAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CPineAdbkIOPluginDLL::ImportAddresses(char* data)
{
	// Unfold lines in address book first
	UnfoldLines(data);

	char* p = data;
	char* adl;
	char* name;
	char  whole_name[256];
	char* eaddr;

	while (*p)
	{
		// Check for comment and skip
		if (*p == '#')
		{
			while(*p && (*p != '\r') && (*p != '\n')) p++;
			while((*p == '\r') || (*p == '\n')) p++;
			continue;
		}

		// nil out pointers at start
		adl = nil;
		name = nil;
		whole_name[0] = '\0';
		eaddr = nil;

		// ADL will be at start
		adl = p;
		SkipTerm(&p);
		
		// Name will be next
		name = p;
		SkipTerm(&p);

		while(*p == ' ') p++;

		// Get email address, addresses ot group
		eaddr = p;
		char q = SkipTerm(&p);

		// Now decide on group (inside '(...)' or comma separated)
		if ((*eaddr == '(') || ::strchr(eaddr, ','))
		{
			// Got group
			CAdbkIOPluginGroup grp;
			grp.mGroup.mNickName = adl;

			// If no name use nick-name
			if (!::strlen(name))
				name = adl;

			// Create new group
			grp.mGroup.mName = name;
			
			// Add all addresses
			char* r = eaddr;
			if (*eaddr == '(')
				r = ::strgetbrastr(&eaddr);
			while(*r == ' ') r++;
			char* p = r;
			while(r && *r)
			{
				switch(*r)
				{
				case '\"':
				case '\'':
				{
					char c = *r++;
					while(*r && (*r++ != c)) ;
					break;
				}
				case ',':
					// Terminate here and bump past
					*r++ = 0;
					
					// Fall through
				case 0:
					{
						// Add remaining to list
						grp.AddAddress(p);
					}
					
					// Reset to start of next address
					while(*r == ' ') r++;
					p = r;
					break;
				default:
					r++;
				}
			}

			// Add remaining to list
			grp.AddAddress(p);

			// Give group to application
#if __dest_os == __mac_os && !TARGET_API_MAC_CARBON
			CallUniversalProc((UniversalProcPtr) mImportCallback, kAdbkImportCallbackProcInfo, grp.GetGroupData(), 1);
#else
			(*mImportCallback)(grp.GetGroupData(), 1);
#endif
		}
		else
		{
			// Got single address
			// Process into first & last names
			
			// Look for comma
			char* comma = ::strchr(name, ',');

			if (comma)
			{
				char* r = comma+1;
				
				// Strip leading space
				r += ::strspn(r, SPACE);

				// Copy first names first with trailing space
				::strcpy(whole_name, r);
				::strcat(whole_name, SPACE);
				
				// Copy last name up to comma
				::strncat(whole_name, name, comma - name);
			}
			
			// Name in correct order - just copy
			else
				::strcpy(whole_name, name);

			// Add address which may be inside (...)
			char* r = eaddr;
			if (*eaddr == '(')
				r = ::strgetbrastr(&eaddr);

			// Create empty address
			CAdbkIOPluginAddress addr;
			
			// Try to parse in email address which may include name
			addr.mAddress.mEmail = r;
			
			// Always add nick_name
			addr.mAddress.mNickName = adl;
			
			// Add name if not already present
			addr.mAddress.mName = whole_name;

			// Give address to application
#if __dest_os == __mac_os && !TARGET_API_MAC_CARBON
			CallUniversalProc((UniversalProcPtr) mImportCallback, kAdbkImportCallbackProcInfo, addr.GetAddressData(), 0);
#else
			(*mImportCallback)(addr.GetAddressData(), 0);
#endif
		}

		// Step till line end or string end
		if ((q != '\r') && (q != '\n'))
		{
			while (*p && (*p != '\r') && (*p != '\n')) p++;
			q = *p;
		}

		if (!q) break;

		while ((*p == '\r') || (*p == '\n')) p++;
		
	}
	
	return 1;
}

// Export an address
long CPineAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	const char* write = nil;
	
	// Write nick-name (adl)
	if (!addr->mNickName || (::strlen(addr->mNickName) == 0))
		write = cPineAnon;
	else
		write = addr->mNickName;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(cPineTab, 1, cPineTabLen, mExportFile);
	
	// Write user name
	if (!addr->mName || (::strlen(addr->mName) == 0))
		write = cPineAnon;
	else
		write = addr->mName;
	char whole_name[256];
	::strcpy(whole_name, write);
	
	// Find last name
	char* last_name = ::strrchr(whole_name, ' ');
	if (last_name)
	{
		// Tie it off and advance to last name
		*last_name = '\0';
		last_name++;
		if (::strlen(last_name) > 0)
		{
			::fwrite(last_name, 1, ::strlen(last_name), mExportFile);
			::fwrite(cPineCommaSpace, 1, cPineCommaSpaceLen, mExportFile);
		}
	}
	
	// Do first name
	::fwrite(whole_name, 1, ::strlen(whole_name), mExportFile);
	::fwrite(cPineTab, 1, cPineTabLen, mExportFile);

	// Write email address mailbox@host
	write = addr->mEmail;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(os_endl, 1, os_endl_len, mExportFile);
	
	return 1;
}

// Export a group
long CPineAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	const char* write;
	
	// Write group nick-name
	if (!grp->mNickName || (::strlen(grp->mNickName) == 0))
		write = cPineAnon;
	else
		write = grp->mNickName;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(cPineTab, 1, cPineTabLen, mExportFile);

	// Write group name
	if (!grp->mName || (::strlen(grp->mName) == 0))
		write = cPineAnon;
	else
		write = grp->mName;
	::fwrite(write, 1, ::strlen(write), mExportFile);
	::fwrite(cPineTab, 1, cPineTabLen, mExportFile);

	::fwrite("(", 1, 1, mExportFile);
	
	// Loop over all addresses
	if (grp->mAddresses)
	{
		bool first = true;
		const char** addr = grp->mAddresses;
		while(*addr)
		{
			// Write ", " before address if not first
			if (!first)
				::fwrite(cPineCommaSpace, 1, cPineCommaSpaceLen, mExportFile);
			else
				first = false;

			// Write address
			::fwrite(*addr, 1, ::strlen(*addr), mExportFile);
			addr++;
		}
	}

	::fwrite(")", 1, 1, mExportFile);
	::fwrite(os_endl, 1, os_endl_len, mExportFile);

	return 1;
}

// Unfold lines
void CPineAdbkIOPluginDLL::UnfoldLines(char* text)
{
	char* p = text;
	char* q = text;

	while(*p)
	{
		// Snoop for possible LWSP
		if ((*p == '\r') || (*p == '\n'))
		{
			// Bump over CR, LF & CRLFs
			char* r = p;
			while((*r == '\r') || (*r == '\n'))
				r++;

			// Check for LWSP-char
			//if ((*r == ' ') || (*r == '\t'))
			if (*r == ' ')
			{
				// Bump over spaces following CR or CRLF
				//while((*r == ' ') || (*r == '\t')) r++;
				while(*r == ' ') r++;
				p = r;
			}
			// Copy all CR, LF & CRLFs
			else
				while((*p == '\r') || (*p == '\n'))
					*q++ = *p++;
		}
		// Copy in -> out
		else
			*q++ = *p++;
	}

	*q = '\0';
}

// Skip chars for termination
char CPineAdbkIOPluginDLL::SkipTerm(char** txt)
{
	// Step up to terminator
	while ((**txt) && (**txt != '\t') && (**txt != '\r') && (**txt != '\n')) (*txt)++;
	
	char q = **txt;

	// null terminate and advance
	if ((**txt == '\r') && (*(*txt + 1) == '\n'))
	{
		**txt = '\0';
		(*txt) += 2;
	}
	else if ((**txt == '\t') || (**txt == '\r') || (**txt == '\n'))
	{
		**txt = '\0';
		(*txt)++;
	}
	
	return q;
}

