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

// CCommAdbkIOPluginDLL.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 19-Apr-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a Communicator address book I/E DLL based plug-in for use in Mulberry.
//
// History:
// 19-Apr-1998: Created initial header and implementation.
// 30-Jun-1998: Created initial ldif import/export
//

#include "CCommAdbkIOPluginDLL.h"
#include "CPluginInfo.h"

#include "cdstring.h"
#include "CStringUtils.h"
#include <stdlib.h>

#if __dest_os == __linux_os
#include <netinet/in.h>
#else
#define LDAP_VERSION2_OLD	1
#define LDAP_VERSION2		2
#define LDAP_VERSION LDAP_VERSION2
#endif

#ifdef __VCPP__
#define ber_len_t size_t
#include "ldif.h"
#else
#define NEEDPROTOS
#include <lber.h>
#if __dest_os == __mac_os_x
#include "ldif.h"
#else
#include <ldif.h>
#endif
#endif
#include <string.h>

// To satisfy openldap linkage
int	ldap_debug;

#pragma mark ____________________________consts

const char* cPluginName = "LDIF Plugin";
const CPluginDLL::EPluginType cPluginType = CPluginDLL::ePluginAddressIO;
const char* cPluginDescription = "LDIF import/export plugin for Mulberry." COPYRIGHT;
const char* cPluginIOName = "LDIF";

const char* cEntryType = "dn";
const char* cNameType = "cn";
const char* cSurNameType = "sn";
const char* cFirstNameType = "givenname";
const char* cObjectClass = "objectclass";
const char* cNotesType = "description";
const char* cCityType = "locality";
const char* cStateType = "st";
const char* cEmailType = "mail";
const char* cTitleType = "title";
const char* cAddress1Type = "postOfficeBox";
const char* cAddress2Type = "streetaddress";
const char* cZipType = "postalcode";
const char* cCountryType = "countryname";
const char* cPhoneWorkType = "telephonenumber";
const char* cFaxType = "facsimiletelephonenumber";
const char* cPhoneHomeType = "homephone";
const char* cCompanyType = "o";
const char* cURLType = "url";
const char* cNickNameType = "xmozillanickname";
const char* cUseHTMLType = "xmozillausehtmlmail";
const char* cConferenceServerType = "xmozillauseconferneceserver";

const char* cMemberType = "member";

const char* cObjectClassTop = "top";
const char* cObjectClassPerson = "person";
const char* cObjectClassGroup = "groupOfNames";

#pragma mark ____________________________CCommAdbkIOPluginDLL

// Constructor
CCommAdbkIOPluginDLL::CCommAdbkIOPluginDLL()
{
}

// Destructor
CCommAdbkIOPluginDLL::~CCommAdbkIOPluginDLL()
{
}

// Initialise plug-in
void CCommAdbkIOPluginDLL::Initialise(void)
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
	::strncpy(mAdbkIOInfo.mTypes, "LDIF Data (*.*) | *.*||", 255);
	mAdbkIOInfo.mTypes[255] = 0;
#elif __dest_os == __linux_os
	::strcpy(mAdbkIOInfo.mTypes, "*");
#endif

}

// Does plug-in need to be registered
bool CCommAdbkIOPluginDLL::UseRegistration(unsigned long* key)
{
	return false;
}

// Can plug-in run as demo
bool CCommAdbkIOPluginDLL::CanDemo(void)
{
	// Must be registered
	return true;
}

// Test for run ability
bool CCommAdbkIOPluginDLL::CanRun(void)
{
	return true;
}

// Returns the name of the plug-in
const char* CCommAdbkIOPluginDLL::GetName(void) const
{
	return cPluginName;
}

// Returns the version number of the plug-in
long CCommAdbkIOPluginDLL::GetVersion(void) const
{
	return cPluginVersion;
}

// Returns the type of the plug-in
CPluginDLL::EPluginType CCommAdbkIOPluginDLL::GetType(void) const
{
	return cPluginType;
}

// Returns manufacturer of plug-in
const char* CCommAdbkIOPluginDLL::GetManufacturer(void) const
{
	return cPluginManufacturer;
}

// Returns description of plug-in
const char* CCommAdbkIOPluginDLL::GetDescription(void) const
{
	return cPluginDescription;
}

long CCommAdbkIOPluginDLL::ImportAddresses(char* data)
{
	// ldif code requires \n line ends, so convert all \r's to \n's, and all '\r\n's to '\n'.
	// NB We must not change the line spacing by introducing blank lines as that will result in
	// folded ldif lines failing.
	char* p = data;
	if (p)
	{
		char* q = p;
		while(*p)
		{
			switch(*p)
			{
			case '\r':
				if (*(++p) != '\n')
					*q++ = '\n';
				break;
			default:
				*q++ = *p++;
			}
		}
		*q = 0;
	}

	// Get a line from the buffer
	char* line = NULL;
	CAdbkIOPluginAddress* addr = new CAdbkIOPluginAddress;
	CAdbkIOPluginGroup* grp = new CAdbkIOPluginGroup;
	cdstrvect members;
	cdstring cn;
	cdstring sn;
	cdstring givenname;
	bool got_addr = false;
	bool got_grp = false;
	
	// NB data can be set to NULL by ldif_getline so exit the loop if that happens
	while(data)
	{
		// Check for end of entire data
		line = NULL;
		
		// Punt extra line feeds
		while(*data && ((*data == '\r') || (*data == '\n'))) data++;
		if (!*data)
			break;
#if (LDAP_VERSION == LDAP_VERSION2)
		if ((line = ::ldif_getline(&data)) == NULL)
			break;		
#else
		if ((line = ::str_getline(&data)) == NULL)
			break;		
#endif
		char* type = 0;
		char* value = 0;
		
		// Get next type/value
#if (LDAP_VERSION == LDAP_VERSION2) 
		ber_len_t vlen = 0;
		if (::ldif_parse_line(line, &type, &value, &vlen) != 0)
			break;
#else
		int vlen = 0;
		if (::str_parse_line(line, &type, &value, &vlen) != 0)
			break;
#endif		
		// Look for start of new address or group
		if (::strcmpnocase(type, cEntryType) == 0)
		{
			// Punt existing address
			if (got_addr)
			{
				if (sn.length() || givenname.length())
				{
					cn = givenname;
					if (givenname.length())
						cn += " ";
					cn += sn;
				}
				addr->mAddress.mName = cn;

				(*mImportCallback)(addr->GetAddressData(), 0);
			}
			else if (got_grp)
			{
				if (sn.length() || givenname.length())
				{
					cn = givenname;
					if (givenname.length())
						cn += " ";
					cn += sn;
				}
				grp->mGroup.mName = cn;

				// Now push all members into group
				for(cdstrvect::iterator iter = members.begin(); iter != members.end(); iter++)
					grp->AddAddress((*iter).c_str());
				(*mImportCallback)(grp->GetGroupData(), 1);
			}
			got_addr = false;
			got_grp = false;
			
			// delete and refresh
			delete addr;
			delete grp;
			members.clear();
			addr = new CAdbkIOPluginAddress;
			grp = new CAdbkIOPluginGroup;
			cn = cdstring::null_str;
			sn = cdstring::null_str;
			givenname = cdstring::null_str;
		}
		
		// Look for address or group type
		else if (::strcmpnocase(type, cObjectClass) == 0)
		{
			if (::strcmpnocase(value, cObjectClassPerson) == 0)
				got_addr = true;
			else if (::strcmpnocase(value, cObjectClassGroup) == 0)
				got_grp = true;
		}
		
		// values common to address and group
		else if (::strcmpnocase(type, cNameType) == 0)
			cn = value;
		else if (::strcmpnocase(type, cNickNameType) == 0)
		{
			addr->mAddress.mNickName = value;
			grp->mGroup.mNickName = value;
		}
		
		else if (::strcmpnocase(type, cSurNameType) == 0)
			sn = value;
		else if (::strcmpnocase(type, cFirstNameType) == 0)
			givenname = value;

		// values for address only
		else if (::strcmpnocase(type, cEmailType) == 0)
			addr->mAddress.mEmail = value;
		else if (::strcmpnocase(type, cCompanyType) == 0)
			addr->mAddress.mCompany = value;
		else if (::strcmpnocase(type, cAddress2Type) == 0)
			addr->mAddress.mAddress = value;
		else if (::strcmpnocase(type, cPhoneWorkType) == 0)
			addr->mAddress.mPhoneWork = value;
		else if (::strcmpnocase(type, cPhoneHomeType) == 0)
			addr->mAddress.mPhoneHome = value;
		else if (::strcmpnocase(type, cFaxType) == 0)
			addr->mAddress.mFax = value;
		else if (::strcmpnocase(type, cNotesType) == 0)
			addr->mAddress.mNotes = value;
		
		// values for groups
		else if (::strcmpnocase(type, cMemberType) == 0)
		{
			cdstring item;
			bool angle_quote = false;
			
			// look for friendly name
			if (::strncmp(value, "cn=", 3) == 0)
			{
				// punt past tag
				value += 3;
				
				// might be quoted
				if (*value == '\"')
				{
					item = "\"";
					item += ::strgetquotestr(&value);
					item += "\" ";
					if (item.length() > 3)
						angle_quote = true;
					else
						item = cdstring::null_str;
				}
				else
				{
					// punt to comma
					const char* p = value;
					while(*value && (*value != ',')) value++;
					if (value - p)
					{
						item.assign(p, value - p);
						item += " ";
						angle_quote = true;
					}
				}
				value++;
			}
			
			// look for email address
			if (::strncmp(value, "mail=", 5) == 0)
			{
				// punt past tag
				value += 5;
				
				// copy it and possibly add brackets
				if (angle_quote)
					item += "<";
				item += value;
				if (angle_quote)
					item += ">";				
			}
			
			// add to list
			members.push_back(item);
		}
	}

	// last one
	if (got_addr)
	{
		if (sn.length() || givenname.length())
		{
			cn = givenname;
			if (givenname.length())
				cn += " ";
			cn += sn;
		}
		addr->mAddress.mName = cn;

		(*mImportCallback)(addr->GetAddressData(), 0);
	}
	else if (got_grp)
	{
		if (sn.length() || givenname.length())
		{
			cn = givenname;
			if (givenname.length())
				cn += " ";
			cn += sn;
		}
		grp->mGroup.mName = cn;

		// Now push all members into group
		for(cdstrvect::iterator iter = members.begin(); iter != members.end(); iter++)
			grp->AddAddress((*iter).c_str());
		(*mImportCallback)(grp->GetGroupData(), 1);
	}
	
	// delete
	delete addr;
	delete grp;
	members.clear();

	return 1;
}

// Export an address
long CCommAdbkIOPluginDLL::ExportAddress(SAdbkIOPluginAddress* addr)
{
	// dn
	cdstring value = "cn=";
	value += addr->mName;
	value += ",mail=";
	value += addr->mEmail;
	WriteTypeValue(cEntryType, value.c_str());
	
	// cn
	WriteTypeValue(cNameType, addr->mName);

	// sn
	const char* p = ::strrchr(addr->mName, ' ');
	if (p)
	{
		value.assign(addr->mName, p - addr->mName);
		p++;
	}
	else
	{
		value = cdstring::null_str;
		p = addr->mName;
	}
	WriteTypeValue(cSurNameType, p);
	
	// givenname
	if (value.length())
		WriteTypeValue(cFirstNameType, value.c_str());
	
	// objectclass x2
	WriteTypeValue(cObjectClass, cObjectClassTop);
	WriteTypeValue(cObjectClass, cObjectClassPerson);
	
	// description
	value = addr->mNotes;
	WriteTypeValue(cNotesType, value.c_str(), true);
	
	// no locality, st
	
	// mail
	WriteTypeValue(cEmailType, addr->mEmail);

	// no title, postOfficeBox
	
	// streetaddress
	value = addr->mAddress;
	WriteTypeValue(cAddress2Type, value.c_str(), true);

	// no postalcode, countryname
	
	// telephonenumber
	WriteTypeValue(cPhoneWorkType, addr->mPhoneWork);
	
	// facsimiletelephonenumber
	WriteTypeValue(cFaxType, addr->mFax);

	// homephone
	WriteTypeValue(cPhoneHomeType, addr->mPhoneHome);
	
	// o
	WriteTypeValue(cCompanyType, addr->mCompany);
	
	// xmozillanickname
	WriteTypeValue(cNickNameType, addr->mNickName);
	
	// xmozillausehtmlmail
	WriteTypeValue(cUseHTMLType, "FALSE");

	// xmozillauseconferneceserver
	WriteTypeValue(cConferenceServerType, "0");

	::fwrite(os_endl, 1, os_endl_len, mExportFile);

	return 1;
}

// Export a group
long CCommAdbkIOPluginDLL::ExportGroup(SAdbkIOPluginGroup* grp)
{
	// dn
	cdstring value = "cn=";
	value += grp->mName;
	WriteTypeValue(cEntryType, value.c_str());
	
	// cn
	WriteTypeValue(cNameType, grp->mName);

	// objectclass x2
	WriteTypeValue(cObjectClass, cObjectClassTop);
	WriteTypeValue(cObjectClass, cObjectClassGroup);
	
	// Write each address as a member
	const char** p = grp->mAddresses;
	while(*p)
	{
		// split into email and name
		const char *q = ::strrchr(*p, ' ');
		cdstring email = (q ? q + 1 : *p);
		if (email[(cdstring::size_type)0] == '<')
			email = cdstring(&email.c_str()[1], email.length() - 2);
		cdstring name = (q ? cdstring(*p, q - *p) : cdstring::null_str);
		name.unquote();

		// member
		value = "cn=";
		value += name;
		value += ",mail=";
		value += email;
		WriteTypeValue(cMemberType, value.c_str());
		
		// punt past last
		p++;
	}

	// xmozillanickname
	WriteTypeValue(cNickNameType, grp->mNickName);
	
	// no description

	::fwrite(os_endl, 1, os_endl_len, mExportFile);


	return 1;
}

void CCommAdbkIOPluginDLL::WriteTypeValue(const char* type, const char* value, bool process_lines)
{
#if LDAP_VERSION == LDAP_VERSION2
	ber_len_t vlen = ::strlen(value);
#else
	size_t vlen = ::strlen(value);
#endif
	if (value && vlen)
	{
		// May need to precess line ends
		if (process_lines)
		{
			char* p = (char*) value;
			char* q = p;
			while(*p)
			{
				// Replace CR/LFs with single space
				if ((*p != '\r') && (*p != '\n'))
					*q++ = *p++;
				else
				{
					*q++ = ' ';
					if ((*p == '\r') && (*(p+1) == '\n'))
						p += 2;
					else
						p++;
				}
			}
		}
#if LDAP_VERSION == LDAP_VERSION2
		const char* write = ::ldif_put(LDIF_PUT_VALUE, (char*) type, (char*) value, vlen);
#else
		const char* write = ::ldif_type_and_value((char*) type, (char*) value, vlen);
#endif
		::fwrite(write, 1, ::strlen(write), mExportFile);
		delete write;
	}
}
