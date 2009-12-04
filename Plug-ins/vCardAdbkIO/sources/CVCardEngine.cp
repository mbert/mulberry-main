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

// CVCardEngine.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 01-Aug-2002
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a generic vCard I/O engine.
//
// History:
// 01-Aug-2002: Created initial header and implementation.
//

#include "CVCardEngine.h"

#include "CVCard.h"

#include "CStringUtils.h"

// Read a single vCard address from the input stream
bool CVCardEngine::ReadOne(std::istream& in, CAdbkIOPluginDLL::SAdbkIOPluginAddress& addr)
{
	// Create vCard 
	CVCard vCard;
	
	// Read in vCard and look for specific items
	if (vCard.Read(in))
	{
		addr.mName = vCard.CountItems("FN") ? ::strdup(vCard.GetValue("FN")) : NULL;
		addr.mNickName = vCard.CountItems("NICKNAME") ? ::strdup(vCard.GetValue("NICKNAME")) : NULL;
		addr.mEmail = vCard.CountItems("EMAIL", "TYPE", "INTERNET") ? ::strdup(vCard.GetValue("EMAIL", "TYPE", "INTERNET")) :
						(vCard.CountItems("EMAIL") ? ::strdup(vCard.GetValue("EMAIL")) : NULL);

		addr.mCompany = vCard.CountItems("ORG") ? ::strdup(vCard.GetValue("ORG")) : NULL;
		addr.mAddress = vCard.CountItems("ADR", "TYPE", "POSTAL") ? ::strdup(vCard.GetValue("ADR", "TYPE", "POSTAL")) :
							(vCard.CountItems("ADR") ? ::strdup(vCard.GetValue("ADR")) : NULL);

		cdstrmap phonework;
		phonework.insert(cdstrmap::value_type("TYPE", "WORK"));
		phonework.insert(cdstrmap::value_type("TYPE", "VOICE"));
		addr.mPhoneWork = vCard.CountItems("TEL", phonework) ? ::strdup(vCard.GetValue("TEL", phonework)) : NULL;

		cdstrmap phonehome;
		phonehome.insert(cdstrmap::value_type("TYPE", "HOME"));
		phonehome.insert(cdstrmap::value_type("TYPE", "VOICE"));
		addr.mPhoneHome = vCard.CountItems("TEL", phonehome) ? ::strdup(vCard.GetValue("TEL", phonehome)) : NULL;

		addr.mFax = vCard.CountItems("TEL", "TYPE", "FAX") ? ::strdup(vCard.GetValue("TEL", "TYPE", "FAX")) : NULL;

		addr.mURL = vCard.CountItems("URL") ? ::strdup(vCard.GetValue("URL")) : NULL;
		addr.mNotes = vCard.CountItems("NOTE") ? ::strdup(vCard.GetValue("NOTE")) : NULL;
		
		// Make full name valid by copying something
		if (!addr.mName)
		{
			if (addr.mCompany)
				addr.mName = ::strdup(addr.mCompany);
			else if (addr.mNickName)
				addr.mName = ::strdup(addr.mNickName);
			else if (addr.mEmail)
				addr.mName = ::strdup(addr.mEmail);
		}

		return true;
	}
	
	return false;
}

// Write a single vCard address to the output stream
void CVCardEngine::WriteOne(std::ostream& out, const CAdbkIOPluginDLL::SAdbkIOPluginAddress& addr)
{
	// Creat vcard object and add appropriate fields
	CVCard vcard;
	vcard.AddItem("FN", addr.mName);
	if (addr.mNickName && ::strlen(addr.mNickName))
		vcard.AddItem("NICKNAME", addr.mNickName);
	if (addr.mEmail && ::strlen(addr.mEmail))
	{
		CVCardItem& item = vcard.AddItem("EMAIL", addr.mEmail);
		item.AddParam("TYPE", "INTERNET");
		item.AddParam("TYPE", "PREF");
	}

	// Add an "N" item only if name is different than the company
	if (::strcmpnocase(addr.mName, addr.mCompany))
		vcard.AddItem("N", addr.mName);

	if (addr.mCompany && ::strlen(addr.mCompany))
		vcard.AddItem("ORG", addr.mCompany);
	if (addr.mAddress && ::strlen(addr.mAddress))
	{
		CVCardItem& item = vcard.AddItem("ADR", addr.mAddress);
		item.AddParam("TYPE", "POSTAL");
	}

	if (addr.mPhoneWork && ::strlen(addr.mPhoneWork))
	{
		CVCardItem& item = vcard.AddItem("TEL", addr.mPhoneWork);
		item.AddParam("TYPE", "WORK");
		item.AddParam("TYPE", "VOICE");
	}
	if (addr.mPhoneHome && ::strlen(addr.mPhoneHome))
	{
		CVCardItem& item = vcard.AddItem("TEL", addr.mPhoneHome);
		item.AddParam("TYPE", "HOME");
		item.AddParam("TYPE", "VOICE");
	}
	if (addr.mFax && ::strlen(addr.mFax))
	{
		CVCardItem& item = vcard.AddItem("TEL", addr.mFax);
		item.AddParam("TYPE", "FAX");
	}

	if (addr.mURL && ::strlen(addr.mURL))
		vcard.AddItem("URL", addr.mURL);
	if (addr.mNotes && ::strlen(addr.mNotes))
		vcard.AddItem("NOTE", addr.mNotes);
	
	// Write the vCard object
	vcard.Write(out);
}
