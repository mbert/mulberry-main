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


// CAdbkIOPlugin.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 13-Jan-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for a DLL based addressbook IO for use in Mulberry.
// This class must be overridden for a real plug-ins. This class provides the basic
// mechanics for the addressbook IO DLL being called by Mulberry.
//
// History:
// CD:	13-Jan-1998:	Created initial header and implementation.
//

#include "CAdbkIOPlugin.h"

#include "CAdbkAddress.h"
#include "CAddressList.h"
#include "CGroup.h"
#include "CGroupList.h"

#include <stdlib.h>
#include <string.h>

static CAddressList* import_addrs = nil;
static CGroupList* import_grps = nil;

#ifdef __cplusplus
extern "C" {
#endif

long ImportCallback(void* data, long group)
{
	if (group)
	{
		CAdbkIOPlugin::SAdbkIOPluginGroup* igrp = (CAdbkIOPlugin::SAdbkIOPluginGroup*) data;

		// Create new object
		CGroup* grp = new CGroup(igrp->mName, igrp->mNickName);

		// Loop over all addresses
		if (igrp->mAddresses)
		{
			const char** addr = igrp->mAddresses;
			while(*addr)
			{
				// Add remaining to list
				cdstring aStr(*addr++);
				grp->GetAddressList().push_back(aStr);
			}
		}

		// Add to list
		import_grps->push_back(grp);
	}
	else
	{
		CAdbkIOPlugin::SAdbkIOPluginAddress* iaddr = (CAdbkIOPlugin::SAdbkIOPluginAddress*) data;

		// Create new object
		CAdbkAddress* addr = new CAdbkAddress;

		// Set nick_name
		addr->SetADL(iaddr->mNickName);

		// Parse RFC822 address - may set name
		addr->ParseAddress(iaddr->mEmail);

		// Add name if not already present
		if (!addr->GetName().length())
			addr->SetName(iaddr->mName);

		// Set extra fields
		addr->SetCompany(iaddr->mCompany);
		addr->SetAddress(iaddr->mAddress, CAdbkAddress::eDefaultAddressType);
		addr->SetPhone(iaddr->mPhoneWork, CAdbkAddress::eWorkPhoneType);
		addr->SetPhone(iaddr->mPhoneHome, CAdbkAddress::eHomePhoneType);
		addr->SetPhone(iaddr->mFax, CAdbkAddress::eFaxType);
		addr->SetURL(iaddr->mURL);
		addr->SetNotes(iaddr->mNotes);

		// Add new address to list
		import_addrs->push_back(addr);
	}

	return 1;
}

#ifdef __cplusplus
}
#endif

#pragma mark ____________________________CAdbkIOPlugin

// Process information from plugin
void CAdbkIOPlugin::ProcessInfo(SPluginInfo* info)
{
	// Do default
	CPlugin::ProcessInfo(info);

	// Get authentication type ID from info
	mAdbkIOInfo = *((SAdbkIOPluginInfo*) info->mData);
}

// Set callback into Mulberry
void CAdbkIOPlugin::SetCallback(ImportCallbackProcPtr proc)
{
	CallPlugin(eAdbkIOSetCallback, (void*) proc);
}

// Do the actual import
long CAdbkIOPlugin::DoImport(const fspec file_spec, CAddressList& addrs, CGroupList& grps)
{
	// Make sure its loaded
	LoadPlugin();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	cdstring fpath(file_spec->GetPath());
#else
	cdstring fpath(file_spec);
#endif
	ImportCallbackProcPtr proc = ImportCallback;

	// First set callback
	SetCallback((ImportCallbackProcPtr) proc);

	// Store references to lists
	import_addrs = &addrs;
	import_grps = &grps;

	// Do import
	long result = CallPlugin(eAdbkIODoImport, (void*) fpath.c_str());

	// Unload after import
	UnloadPlugin();

	return result;
}

// Do the actual export - pure virtual - must do in subclass
long CAdbkIOPlugin::DoExport(const fspec file_spec, CAddressList& addrs, CGroupList& grps)
{
	// Make sure its loaded
	LoadPlugin();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	cdstring fpath(file_spec->GetPath());
#else
	cdstring fpath(file_spec);
#endif

	// Start export
	long result = CallPlugin(eAdbkIOStartExport, (void*) fpath.c_str());
	if (!result)
		return result;

	// Write single addresses to file
	for(CAddressList::const_iterator iter = addrs.begin(); result && (iter != addrs.end()); iter++)
	{
		CAdbkAddress* exp = static_cast<CAdbkAddress*>(*iter);

		SAdbkIOPluginAddress addr;

		// Set number of fields
		addr.mNumFields = 10;

		// Fill in all fields
		addr.mNickName = exp->GetADL().c_str();
		addr.mName = exp->GetName().c_str();
		cdstring write = exp->GetMailAddress();
		addr.mEmail = write.c_str();
		addr.mCompany = exp->GetCompany().c_str();
		addr.mAddress = exp->GetAddress(CAdbkAddress::eDefaultAddressType).c_str();
		addr.mPhoneWork = exp->GetPhone(CAdbkAddress::eWorkPhoneType).c_str();
		addr.mPhoneHome = exp->GetPhone(CAdbkAddress::eHomePhoneType).c_str();
		addr.mFax = exp->GetPhone(CAdbkAddress::eFaxType).c_str();
		addr.mURL = exp->GetURL().c_str();
		addr.mNotes = exp->GetNotes().c_str();

		// Do import
		result = CallPlugin(eAdbkIOExportAddress, &addr);
	}

	// Write groups to file
	for(CGroupList::const_iterator iter = grps.begin(); result && (iter != grps.end()); iter++)
	{
		CAdbkIOPluginGroup grp;

		// Set number of fields
		grp.mGroup.mNumFields = 3;

		// Fill in all fields
		grp.mGroup.mNickName = (*iter)->GetNickName().c_str();
		grp.mGroup.mName = (*iter)->GetName().c_str();

		// Write each address out
		for(unsigned long i = 0; i < (*iter)->GetAddressList().size(); i++)
		{
			const cdstring& aStr = (*iter)->GetAddressList().at(i);

			grp.AddAddress(aStr.c_str());
		}

		// Do import
		result = CallPlugin(eAdbkIOExportGroup, grp.GetGroupData());
	}

	// Stop export
	result = CallPlugin(eAdbkIOStopExport, nil);

	// Unload after import
	UnloadPlugin();

	return result;
}

#pragma mark ____________________________CAdbkIOPluginGroup

CAdbkIOPlugin::CAdbkIOPluginGroup::CAdbkIOPluginGroup()
{
	mGroup.mNumFields = 3;
	mGroup.mNickName = nil;
	mGroup.mName = nil;
	mGroup.mAddresses = nil;

	mNumAddresses = 0;
}

CAdbkIOPlugin::CAdbkIOPluginGroup::~CAdbkIOPluginGroup()
{
	if (mGroup.mAddresses)
		::free(mGroup.mAddresses);
}

void CAdbkIOPlugin::CAdbkIOPluginGroup::AddAddress(const char* addr)
{
	mNumAddresses++;

	if (!mGroup.mAddresses)
		mGroup.mAddresses = (const char**) ::malloc(sizeof(char*));
	else
		mGroup.mAddresses = (const char**) ::realloc(mGroup.mAddresses, mNumAddresses * sizeof(char*));

	mGroup.mAddresses[mNumAddresses - 1] = addr;
}

CAdbkIOPlugin::SAdbkIOPluginGroup* CAdbkIOPlugin::CAdbkIOPluginGroup::GetGroupData()
{
	// Must add empty address into list for terminator if some exist
	if (mNumAddresses)
		AddAddress(nil);

	return &mGroup;
}
