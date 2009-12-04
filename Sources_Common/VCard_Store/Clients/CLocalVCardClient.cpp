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

/* 
	CLocalVCardClient.cpp

	Author:			
	Description:	<describe the CLocalVCardClient class here>
*/

#include "CLocalVCardClient.h"

#include "CAddressAccount.h"
#include "CAdbkProtocol.h"
#include "CAdbkRecord.h"
#include "CAddressBook.h"
#include "CGeneralException.h"
#include "CLocalCommon.h"

#include "CVCardMapper.h"
#include "CVCardSearch.h"

#include "CVCardAddressBook.h"
#include "CVCardDefinitions.h"
#include "CVCardVCard.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include "diriterator.h"
#include "cdfstream.h"

#include <memory>
#include __stat_header

const char* cVCardExtension = ".vcf";

using namespace vcardstore; 

CLocalVCardClient::CLocalVCardClient(CAdbkProtocol* adbk_owner)
	: CAdbkClient(adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitLocalClient();
}

CLocalVCardClient::CLocalVCardClient(const CLocalVCardClient& copy, CAdbkProtocol* adbk_owner)
	: CAdbkClient(copy, adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitLocalClient();

	mCWD = copy.mCWD;

	mRecorder = copy.mRecorder;
}

CLocalVCardClient::~CLocalVCardClient()
{
	mRecorder = NULL;
}

void CLocalVCardClient::InitLocalClient()
{
	// Protocol that can disconnect will always be cached
	mCaching = GetAdbkOwner()->CanDisconnect();

	mRecorder = NULL;
	mRecordID = 0;
}

// Create duplicate, empty connection
CINETClient* CLocalVCardClient::CloneConnection()
{
	// Copy construct this
	return new CLocalVCardClient(*this, GetAdbkOwner());

}

bool CLocalVCardClient::IsCaching() const
{
	return mCaching;
}

#pragma mark ____________________________Start/Stop

// Start TCP
void CLocalVCardClient::Open()
{
	// Do account reset
	Reset();
}

// Reset acount info
void CLocalVCardClient::Reset()
{
	// get CWD from owner
	mCWD = GetAdbkOwner()->GetOfflineCWD();

	// Must append dir delim if not present
	if (mCWD.length() && (mCWD[mCWD.length() - 1] != os_dir_delim))
		mCWD += os_dir_delim;

	// May need to check for INBOX on POP3
	CheckCWD();
}

// Check CWD
void CLocalVCardClient::CheckCWD()
{
	// Local does nothing

}

// Release TCP
void CLocalVCardClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CLocalVCardClient::Abort()
{
}

// Forced close
void CLocalVCardClient::Forceoff()
{
}

#pragma mark ____________________________Login & Logout

void CLocalVCardClient::Logon()
{
	// Local does nothing
	
	// Must fail if empty CWD
	if (mCWD.empty())
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}
}

void CLocalVCardClient::Logoff()
{
	// Nothing to do for local
}

#pragma mark ____________________________Handle Errors

// Descriptor for object error context
const char*	CLocalVCardClient::INETGetErrorDescriptor() const
{
	return "Address Book: ";
}

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CLocalVCardClient::_Tickle(bool force_tickle)
{
	// Local does nothing

}

void CLocalVCardClient::_FindAllAdbks(const cdstring& path)
{
	StINETClientAction action(this, "Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	InitItemCtr();

	// Get name for new file
	cdstring fpath = MapDirName(path);

	// Directory scan starting at the root
	ListAddressBooks(NULL, fpath);
}

void CLocalVCardClient::_ListAddressBooks(CAddressBook* root)
{
	// Node must be protocol or directory
	if (!root->IsProtocol() && !root->IsDirectory())
		return;

	// Get name for new file
	cdstring fpath = MapDirName(root);

	// Directory scan starting at the root
	ListAddressBooks(root, fpath);
	
	// Always sort children after adding all of them
	root->SortChildren();
}

void CLocalVCardClient::ListAddressBooks(CAddressBook* root, const cdstring& path)
{
	// Directory scan
	diriterator _dir(path, true, cVCardExtension);
	const char* fname = NULL;
	while(_dir.next(&fname))
	{
		// Get the full path of the found item
		cdstring fpath(path);
		::addtopath(fpath, fname);

		// Get the relative path which will be the name of the node.
		// This is relative to the root path for this client.
		cdstring rpath(fpath, mCWD.length());
		
		// Strip off trailing .vcf
		if (rpath.compare_end(cVCardExtension))
			rpath.erase(rpath.length() - 4);

		// Provide feedback
		BumpItemCtr("Status::IMSP::AddressBookFind");

		// Add adress book to list
		CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), root, !_dir.is_dir(), _dir.is_dir(), rpath);
		root->AddChild(adbk);

		// Scan into directories
		if (_dir.is_dir())
		{
			// Use new node as the root
			ListAddressBooks(adbk, fpath);
			
			// Always mark adbk as having been expanded
			adbk->SetHasExpanded(true);
			
			// Always sort the children after adding all of them
			adbk->SortChildren();
		}
	}
}

void CLocalVCardClient::_CreateAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook");

	// Get name for new file
	cdstring fpath = (adbk->IsAdbk() ? MapAdbkName(adbk) : MapDirName(adbk));
	
	// Make sure it does not already exist
	if (::fileexists(fpath) || ::direxists(fpath))
	{
		//throw CGeneralException(-1, "Address Book file/directory exists");
		throw CGeneralException(-1);
	}
	
	// Must ensure entire path exists
	{
		cdstring convert = LocalFileName(adbk->GetName(), GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		if (adbk->IsAdbk())
			convert += cVCardExtension;
		cdstring adbk_name = mCWD;
		cdstring dir_delim = os_dir_delim;
		char* dir = ::strtok(convert.c_str_mod(), dir_delim);
		char* next_dir = ::strtok(NULL, dir_delim);
		while(dir && next_dir)
		{
			adbk_name += dir;
			::chkdir(adbk_name);
			adbk_name += os_dir_delim;
			dir = next_dir;
			next_dir = ::strtok(NULL, dir_delim);
		}
	}

	if (!adbk->IsAdbk() && adbk->IsDirectory())
	{
		if (__mkdir(fpath, S_IRWXU))
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not create directory");
			throw CGeneralException(_errno);
		}
	}
	else
	{
		FILE* file;
		if ((file = ::fopen_utf8(fpath, "ab")) != NULL)
			::fclose(file);
		else
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not create address book file");
			throw CGeneralException(_errno);
		}
		
		// Always clear any existing cache file as a precaution
		if (IsCaching())
		{
			cdstring cpath = MapAdbkCacheName(adbk);
			if (::fileexists(cpath))
				::remove_utf8(cpath);
		}
	}

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Create(adbk);
	}
}

void CLocalVCardClient::_DeleteAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddressBook", "Error::IMSP::OSErrDeleteAddressBook", "Error::IMSP::NoBadDeleteAddressBook");

	// Get name for new file
	cdstring fpath_adbk = MapAdbkName(adbk);
	cdstring fpath_dir = MapDirName(adbk);
	
	// See what kind of object
	bool delete_adbk = adbk->IsAdbk();
	bool delete_dir = adbk->IsDirectory();

	if (delete_adbk)
	{
		// Make sure it already exists
		if (::fileexists(fpath_adbk))
		{
			if (::remove_utf8(fpath_adbk) != 0)
			{
				int _errno = os_errno;
				//throw CGeneralException(_errno, "Could not delete address book file");
				throw CGeneralException(_errno);
			}
			
			// Always clear any existing cache file
			if (IsCaching())
			{
				cdstring cpath = MapAdbkCacheName(adbk);
				if (::fileexists(cpath))
					::remove_utf8(cpath);
			}
		}
		else if (!delete_dir)
		{
			//throw CGeneralException(-1, "File being deleted does not exist");
			throw CGeneralException(-1);
		}
	}

	if (delete_dir)
	{
		// Make sure it already exists
		if (!::direxists(fpath_dir))
		{
			// Make sure it is empty
			if (::count_dir_contents(fpath_dir) != 0)
			{
				//throw CGeneralException(-1, "Directory being deleted is not empty");
				throw CGeneralException(-1);
			}

			if (::delete_dir(fpath_dir) != 0)
			{
				int _errno = os_errno;
				//throw CGeneralException(_errno, "Could not delete directory");
				throw CGeneralException(_errno);
			}
		}
		else if (!delete_adbk)
		{
			//throw CGeneralException(-1, "Directory being deleted does not exist");
			throw CGeneralException(-1);
		}
	}

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Delete(adbk);
	}
}

void CLocalVCardClient::_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk)
{
	StINETClientAction action(this, "Status::IMSP::RenameAddressBook", "Error::IMSP::OSErrRenameAddressBook", "Error::IMSP::NoBadRenameAddressBook");

	// See what kind of object
	bool rename_adbk = old_adbk->IsAdbk();
	bool rename_dir = old_adbk->IsDirectory();

	if (rename_dir)
	{
		// Get name for new directory
		cdstring fpath_old = MapDirName(old_adbk);
		cdstring convert = LocalFileName(new_adbk, GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		cdstring fpath_new = MapDirName(convert);
		
		// Make sure old already exists
		if (!::direxists(fpath_old))
		{
			// Make sure new does not already exist
			if (::direxists(fpath_new))
			{
				//throw CGeneralException(-1, "Directory being renamed to exists");
				throw CGeneralException(-1);
			}

			if (::rename_utf8(fpath_old, fpath_new) != 0)
			{
				int _errno = os_errno;
				//throw CGeneralException(_errno, "Could not rename directory");
				throw CGeneralException(_errno);
			}
		}
		else if (!rename_adbk)
		{
			//throw CGeneralException(-1, "Directory being renamed does not exist");
			throw CGeneralException(-1);
		}
	}

	if (rename_adbk)
	{
		// Get name for new file
		cdstring fpath_old = MapAdbkName(old_adbk);
		cdstring convert = LocalFileName(new_adbk, GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		cdstring fpath_new = MapAdbkName(convert);
		
		// Make sure old already exists
		if (!::fileexists(fpath_old))
		{
			// Make sure new does not already exist
			if (::fileexists(fpath_new))
			{
				//throw CGeneralException(-1, "Address Book file being renamed to exists");
				throw CGeneralException(-1);
			}

			if (::rename_utf8(fpath_old, fpath_new) != 0)
			{
				int _errno = os_errno;
				//throw CGeneralException(_errno, "Could not rename address book file");
				throw CGeneralException(_errno);
			}
			
			// Always rename any existing cache file
			if (IsCaching())
			{
				cdstring cpath_old = MapAdbkCacheName(old_adbk);
				cdstring cpath_new = MapAdbkCacheName(convert);
				if (::fileexists(cpath_old))
				{
					// Delete existing cache file as a precaution
					if (::fileexists(cpath_new))
						::remove_utf8(cpath_new);
					
					// Rename cache file
					::rename_utf8(cpath_old, cpath_new);
				}
			}
		}
		else if (!rename_dir)
		{
			//throw CGeneralException(-1, "Address Book file being renamed does not exist");
			throw CGeneralException(-1);
		}
	}

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Rename(old_adbk, new_adbk);
	}
}

void CLocalVCardClient::_SizeAdbk(CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::Checking", "Error::IMSP::OSErrCheck", "Error::IMSP::NoBadCheck", adbk->GetName());

	// Does it exist on disk?
	if (_TestAdbk(adbk))
	{
		cdstring adbk_name = MapAdbkName(adbk);

		// Get sizes
		unsigned long size = 0;
		struct stat adbk_finfo;
		if (::stat_utf8(adbk_name, &adbk_finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}
		else
			size += adbk_finfo.st_size;

		// Add cache file if present
		if (IsCaching())
		{
			cdstring cpath = MapAdbkCacheName(adbk);
			if (!::stat_utf8(cpath, &adbk_finfo))
				size += adbk_finfo.st_size;
		}

		adbk->SetSize(size);
	}
	else
		// Not cached => size = 0
		adbk->SetSize(0);
}

bool CLocalVCardClient::_TestAdbk(const CAddressBook* adbk)
{
	// Get name for new file
	cdstring fpath_adbk = MapAdbkName(adbk);
	
	// See what kind of object
	bool test_adbk = adbk->IsAdbk();

	// Make sure it already exists
	return test_adbk &&::fileexists(fpath_adbk);
}

bool CLocalVCardClient::_TouchAdbk(const CAddressBook* adbk)
{
	if (!_TestAdbk(adbk))
	{
		StValueChanger<CAdbkRecord*> value(mRecorder, NULL);
		_CreateAdbk(adbk);
		return true;
	}
	else
		return false;
}

bool CLocalVCardClient::_AdbkChanged(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
	return false;
}

void CLocalVCardClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
}

// Operations with addresses

// Find all addresses in adbk
void CLocalVCardClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	// Get name for new file
	cdstring fpath = MapAdbkName(adbk);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Address Book file does not exist");
		throw CGeneralException(-1);
	}
	
	// Read address book from file
	cdifstream is(fpath);
	adbk->GetVCardAdbk()->Parse(is);
	
	// Get read-only state
	adbk->GetVCardAdbk()->SetReadOnly(!::filereadwriteable(fpath));
	
	// Load cache file
	if (IsCaching())
	{
		cdstring cpath = MapAdbkCacheName(adbk);
		cdifstream cis(cpath);
		adbk->GetVCardAdbk()->ParseCache(cis);
	}
	
	// Now map VCards into internal addresses
	vcardstore::MapFromVCards(adbk);
}

// Write all addresses in adbk
void CLocalVCardClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	// Get name for new file
	cdstring fpath = MapAdbkName(adbk);
	cdstring cpath = MapAdbkCacheName(adbk);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Address Book file does not exist");
		throw CGeneralException(-1);
	}
	
	// Make sure its writeable
	if (!::filereadwriteable(fpath))
	{
		//throw CGeneralException(-1, "Address Book file not writeable");
		throw CGeneralException(-1);
	}
	
	// Transactional write
	cdstring fpath_tmp(fpath);
	fpath_tmp += ".tmp";
	cdstring cpath_tmp(cpath);
	cpath_tmp += ".tmp";
	
	// Create the file and write address book to it
	{
		cdofstream os(fpath_tmp);
		adbk->GetVCardAdbk()->Generate(os, IsCaching());
	}
	
	// Write cache file
	if (IsCaching())
	{
		cdofstream cos(cpath_tmp);
		adbk->GetVCardAdbk()->GenerateCache(cos);
	}

	// Remove old and rename the new
	::remove_utf8(fpath);
	::rename_utf8(fpath_tmp, fpath);
	::remove_utf8(cpath);
	::rename_utf8(cpath_tmp, cpath);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Change(adbk);
	}
}

// Find all addresses in adbk
void CLocalVCardClient::_FindAllAddresses(CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress");
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all addresses
		//ScanAddressBook(adbk);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

}

// Fetch named address
void CLocalVCardClient::_FetchAddress(CAddressBook* adbk,
								const cdstrvect& names)
{
}

// Store address
void CLocalVCardClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	Append(adbk, addrs, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->StoreAddress(adbk, addrs);
	}
}

// Store group
void CLocalVCardClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	//Change(adbk, NULL, grps, NULL, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->StoreGroup(adbk, grps);
	}
}

// Change address
void CLocalVCardClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	//Change(adbk, addrs, NULL, addrs, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->ChangeAddress(adbk, addrs);
	}
}

// Change group
void CLocalVCardClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	//Change(adbk, NULL, grps, NULL, grps);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->ChangeGroup(adbk, grps);
	}
}

// Delete address
void CLocalVCardClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	//Change(adbk, NULL, NULL, addrs, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->DeleteAddress(adbk, addrs);
	}
}

// Delete group
void CLocalVCardClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	//Change(adbk, NULL, NULL, NULL, grps);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->DeleteGroup(adbk, grps);
	}
}

// These must be implemented by specific client

// Resolve address nick-name
void CLocalVCardClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all matching addresses
		CAdbkAddress::CAddressFields fields;
		fields.push_back(CAdbkAddress::eNickName);
		SearchAddressBook(adbk, nick_name, fields, NULL, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Resolve group nick-name
void CLocalVCardClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all matching addresses
		//SearchAddressBook(adbk, nick_name, CAdbkAddress::eNickName, NULL, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Search for addresses
void CLocalVCardClient::_SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	StINETClientAction action(this, "Status::IMSP::SearchAddress", "Error::IMSP::OSErrSearchAddress", "Error::IMSP::NoBadSearchAddress");

	// Cache actionable address book - addresses actually go into list provided
	mActionAdbk = adbk;

	try
	{
		cdstring matchit(name);
		CAdbkAddress::ExpandMatch(match, matchit);

		// Fetch all addresses
		SearchAddressBook(adbk, matchit, fields, &addr_list, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Set acl on server
void CLocalVCardClient::_SetACL(CAddressBook* adbk, CACL* acl)
{
}

// Delete acl on server
void CLocalVCardClient::_DeleteACL(CAddressBook* adbk, CACL* acl)
{
}

// Get all acls for adbk from server
void CLocalVCardClient::_GetACL(CAddressBook* adbk)
{
}

// Get current user's rights to adbk
void CLocalVCardClient::_MyRights(CAddressBook* adbk)
{
}

#pragma mark ____________________________Local Ops

void CLocalVCardClient::Append(CAddressBook* adbk, const CAddressList* addrs, const CGroupList* grps)
{
	// Just append vCards to file
	
	// Get name for new file
	cdstring fpath = MapAdbkName(adbk);
	cdstring cpath = MapAdbkCacheName(adbk);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Address Book file does not exist");
		throw CGeneralException(-1);
	}
	
	// Make sure its writeable
	if (!::filereadwriteable(fpath))
	{
		//throw CGeneralException(-1, "Address Book file not writeable");
		throw CGeneralException(-1);
	}
	
	// Create the file and write address book to it
	{
		cdofstream os(fpath, std::ios_base::out | std::ios_base::app);
		vCard::CVCardAddressBook vadbk;
		
		if (addrs != NULL)
		{
			for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
			{
				std::auto_ptr<vCard::CVCardVCard> vcard(vcardstore::GenerateVCard(vadbk.GetRef(), static_cast<CAdbkAddress*>(*iter)));
				vadbk.GenerateOne(os, *vcard);
			}
		}
	}
	
	// Write cache file
	//if (IsCaching())
	//{
	//	cdofstream cos(cpath);
	//	adbk->GetVCardAdbk()->GenerateCache(cos);
	//}
}

#pragma mark ____________________________Utils

cdstring CLocalVCardClient::MapAdbkName(const CAddressBook* adbk) const
{
	if (adbk->IsProtocol())
		return mCWD;
	else
	{
		cdstring convert = LocalFileName(adbk->GetName(), GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		return MapAdbkName(convert);
	}
}

cdstring CLocalVCardClient::MapDirName(const CAddressBook* adbk) const
{
	if (adbk->IsProtocol())
		return mCWD;
	else
	{
		cdstring convert = LocalFileName(adbk->GetName(), GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		return MapDirName(convert);
	}
}

cdstring CLocalVCardClient::MapAdbkName(const cdstring& node_name) const
{
	// Create path from address book name
	cdstring result(mCWD);
	::addtopath(result, node_name);
	
	// Always add ".vcf" for actual address books
	result += cVCardExtension;

	return result;
}

cdstring CLocalVCardClient::MapDirName(const cdstring& node_name) const
{
	// Create path from address book name
	cdstring result(mCWD);
	::addtopath(result, node_name);

	return result;
}

cdstring CLocalVCardClient::MapAdbkCacheName(const CAddressBook* adbk) const
{
	if (adbk->IsProtocol())
		return mCWD;
	else
	{
		cdstring convert = LocalFileName(adbk->GetName(), GetAdbkOwner()->GetDirDelim(), GetAdbkOwner()->IsDisconnectedCache());
		return MapAdbkCacheName(convert);
	}
}

cdstring CLocalVCardClient::MapAdbkCacheName(const cdstring& node_name) const
{
	// Create path from address book name
	cdstring result(mCWD);
	::addtopath(result, node_name);
	
	// Always add ".xml" for actual address book cache files
	result += ".xml";

	return result;
}

void CLocalVCardClient::SearchAddressBook(CAddressBook* adbk, const cdstring& pattern, const CAdbkAddress::CAddressFields& fields,
											CAddressList* addr_list, CGroupList* grp_list)
{
	// Map adbk field to vCard property
	cdstrvect prop_names;
	for(CAdbkAddress::CAddressFields::const_iterator iter = fields.begin(); iter != fields.end(); iter++)
	{
		switch(*iter)
		{
		case CAdbkAddress::eName:
			prop_names.push_back(vCard::cVCardProperty_FN);
			break;
		case CAdbkAddress::eNickName:
			prop_names.push_back(vCard::cVCardProperty_NICKNAME);
			break;
		case CAdbkAddress::eEmail:
			prop_names.push_back(vCard::cVCardProperty_EMAIL);
			break;
		case CAdbkAddress::eCompany:
			prop_names.push_back(vCard::cVCardProperty_ORG);
			break;
		case CAdbkAddress::eAddress:
			prop_names.push_back(vCard::cVCardProperty_ADR);
			break;
		case CAdbkAddress::ePhoneWork:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::ePhoneHome:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::eFax:
			prop_names.push_back(vCard::cVCardProperty_TEL);
			break;
		case CAdbkAddress::eURL:
			prop_names.push_back(vCard::cVCardProperty_URL);
			break;
		case CAdbkAddress::eNotes:
			prop_names.push_back(vCard::cVCardProperty_NOTE);
			break;
		}
	}

	// Get name for new file
	cdstring fpath = MapAdbkName(adbk);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Address Book file does not exist");
		throw CGeneralException(-1);
	}
	
	// Read address book from file
	cdifstream is(fpath);
	vCard::CVCardAddressBook temp;
	temp.Parse(is);

	vcardstore::SearchVCards(temp, pattern, prop_names, adbk, addr_list);

}
