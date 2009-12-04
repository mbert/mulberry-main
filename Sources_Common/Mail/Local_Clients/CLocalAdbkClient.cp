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


// Code for Local Prefs client class

#include "CLocalAdbkClient.h"

#include "CAddressBook.h"
#include "CGeneralException.h"
#include "CINETCommon.h"
#include "CLocalCommon.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CUnicodeStdLib.h"

#include "diriterator.h"

#if __dest_os == __win32_os
#include "StValueChanger.h"
#elif __dest_os == __linux_os
#include "StValueChanger.h"
#include <JString.h>
#include <string.h>
#endif

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include __stat_header
#include <unistd.h>

#define CHECK_STREAM(x) \
	{ if ((x).fail()) { int err_no = os_errno; CLOG_LOGTHROW(CGeneralException, err_no); throw CGeneralException(err_no); } }

#pragma mark -


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CLocalAdbkClient::CLocalAdbkClient(CAdbkProtocol* adbk_owner)
	: CAdbkClient(adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitAdbkClient();

} // CLocalAdbkClient::CLocalAdbkClient

// Copy constructor
CLocalAdbkClient::CLocalAdbkClient(const CLocalAdbkClient& copy, CAdbkProtocol* adbk_owner)
	: CAdbkClient(copy, adbk_owner)
{
	mOwner = adbk_owner;

	// Init instance variables
	InitAdbkClient();

	mCWD = copy.mCWD;

	mRecorder = copy.mRecorder;

} // CLocalAdbkClient::CLocalAdbkClient

CLocalAdbkClient::~CLocalAdbkClient()
{
	mRecorder = NULL;
	delete mTempAddr;
	mTempAddr = NULL;
	delete mTempGrp;
	mTempGrp = NULL;

} // CLocalAdbkClient::~CLocalAdbkClient

void CLocalAdbkClient::InitAdbkClient()
{
	mRecorder = NULL;
	mRecordID = 0;

	mTempAddr = NULL;
	mTempGrp = NULL;
	mSearchMode = false;

} // CLocalAdbkClient::CLocalAdbkClient

// Create duplicate, empty connection
CINETClient* CLocalAdbkClient::CloneConnection()
{
	// Copy construct this
	return new CLocalAdbkClient(*this, GetAdbkOwner());

} // CLocalAdbkClient::CloneConnection

#pragma mark ____________________________Start/Stop

// Start TCP
void CLocalAdbkClient::Open()
{
	// Reset only
	Reset();
}

// Reset Account info
void CLocalAdbkClient::Reset()
{
	// get CWD from owner
	mCWD = GetAdbkOwner()->GetOfflineCWD();

	// Must append dir delim if not present
	if (mCWD.length() && (mCWD[mCWD.length() - 1] != os_dir_delim))
		mCWD += os_dir_delim;
}

// Release TCP
void CLocalAdbkClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CLocalAdbkClient::Abort()
{
	// Local does nothing

}

#pragma mark ____________________________Login & Logout

// Logon to IMAP server
void CLocalAdbkClient::Logon()
{
	// Local does nothing

	// Must fail if empty CWD
	if (mCWD.empty())
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadResponse);
		throw CINETException(CINETException::err_BadResponse);
	}

} // CLocalAdbkClient::Logon

// Logoff from IMAP server
void CLocalAdbkClient::Logoff()
{
	// Local does nothing


} // CLocalAdbkClient::Logoff

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CLocalAdbkClient::_Tickle(bool force_tickle)
{
	// Local does nothing

} // CLocalAdbkClient::_Tickle

// Handle failed capability response
void CLocalAdbkClient::_PreProcess()
{
	// No special preprocess

} // CLocalAdbkClient::_PreProcess

// Handle failed capability response
void CLocalAdbkClient::_PostProcess()
{
	// No special postprocess

} // CLocalAdbkClient::_PostProcess

#pragma mark ____________________________Address Books

// Operations on address books

void CLocalAdbkClient::_ListAddressBooks(CAddressBook* root)
{
	StINETClientAction action(this, "Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	InitItemCtr();

	// Node must be protocol or directory
	if (!root->IsProtocol() && !root->IsDirectory())
		return;

	cdstring pattern = mCWD;
	ListAddressBooks(root, pattern);
	
	// Always sort children after adding all of them
	root->SortChildren();
}

// Find all adbks below this path
void CLocalAdbkClient::_FindAllAdbks(const cdstring& path)
{
	StINETClientAction action(this, "Status::IMSP::AddressBooks", "Error::IMSP::OSErrAddressBooks", "Error::IMSP::NoBadAddressBooks");
	InitItemCtr();

	cdstring pattern = mCWD + cWILDCARD;
	ScanDirectory(mCWD, pattern, true);
}

// Create adbk
void CLocalAdbkClient::_CreateAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::CreateAddressBook", "Error::IMSP::OSErrCreateAddressBook", "Error::IMSP::NoBadCreateAddressBook");

	// Must ensure entire path exists
	{
		cdstring convert = LocalFileName(adbk->GetName(), '.', GetAdbkOwner()->IsDisconnected());
		cdstring abdk_name = mCWD;
		cdstring dir_delim = os_dir_delim;
		char* dir = ::strtok(convert.c_str_mod(), dir_delim);
		char* next_dir = ::strtok(NULL, dir_delim);
		while(dir && next_dir)
		{
			abdk_name += dir;
			::chkdir(abdk_name);
			abdk_name += os_dir_delim;
			dir = next_dir;
			next_dir = ::strtok(NULL, dir_delim);
		}
	}

	// See whether one already exists and throw if it does
	if (_TestAdbk(adbk))
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
		throw CINETException(CINETException::err_NoResponse);
	}

	{
		// Get the full path
		cdstring adbk_name;
		GetFileName(adbk, adbk_name);

		FILE* file;
		try
		{
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				StCreatorType file1(cMulberryCreator, cAddressBookFileType);
#endif
				if ((file = ::fopen_utf8(adbk_name, "ab")) != NULL)
					fclose(file);
				else
				{
					CLOG_LOGTHROW(CINETException, CINETException::err_NoResponse);
					throw CINETException(CINETException::err_NoResponse);
				}
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// If anyone fails then delete the lot
			_DeleteAdbk(adbk);

			// throw up
			CLOG_LOGRETHROW;
			throw;
		}
	}

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Create(adbk);
	}
}

// Do touch
bool CLocalAdbkClient::_TouchAdbk(const CAddressBook* adbk)
{
	// Check it exists and create if not
	if (!_TestAdbk(adbk))
	{
		// Do this without recording
		StValueChanger<CAdbkRecord*> value(mRecorder, NULL);
		_CreateAdbk(adbk);
		return true;
	}
	else
		return false;
}

// Do test
bool CLocalAdbkClient::_TestAdbk(const CAddressBook* adbk)
{
	// Get name
	cdstring adbk_name;
	GetFileName(adbk, adbk_name);

	// Check it exists and create if not
	return fileexists(adbk_name);
}

bool CLocalAdbkClient::_AdbkChanged(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
	return false;
}

void CLocalAdbkClient::_UpdateSyncToken(const CAddressBook* adbk)
{
	// Nothing to do for local as this is only used when sync'ing with server
}


// Delete adbk
void CLocalAdbkClient::_DeleteAdbk(const CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddressBook", "Error::IMSP::OSErrDeleteAddressBook", "Error::IMSP::NoBadDeleteAddressBook");

	cdstring adbk_name;
	GetFileName(adbk, adbk_name);

	// Delete address book
	::remove_utf8(adbk_name);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Delete(adbk);
	}
}

// Rename adbk
void CLocalAdbkClient::_RenameAdbk(const CAddressBook* old_adbk, const cdstring& new_adbk)
{
	StINETClientAction action(this, "Status::IMSP::RenameAddressBook", "Error::IMSP::OSErrRenameAddressBook", "Error::IMSP::NoBadRenameAddressBook");

	cdstring adbk_name_old;
	GetFileName(old_adbk, adbk_name_old);

	cdstring convert = LocalFileName(new_adbk, '.', GetAdbkOwner()->IsDisconnected());
	cdstring adbk_name_new;
	GetFileName(convert, adbk_name_new);

	// Rename mailbox
	bool adbk_done = false;
	try
	{
		if (moverename_file(adbk_name_old, adbk_name_new))
		{
			CLOG_LOGTHROW(CGeneralException, os_errno);
			throw CGeneralException(os_errno);
		}
		else
			adbk_done = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must rename back to original
		if (adbk_done)
			moverename_file(adbk_name_new, adbk_name_old);

		// throw up
		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Rename(old_adbk, new_adbk);
	}
}

void CLocalAdbkClient::_SizeAdbk(CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::Checking", "Error::IMSP::OSErrCheck", "Error::IMSP::NoBadCheck", adbk->GetName());

	// Does it exist on disk?
	if (_TestAdbk(adbk))
	{
		cdstring adbk_name;
		GetFileName(adbk, adbk_name);

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

		adbk->SetSize(size);
	}
	else
		// Not cached => size = 0
		adbk->SetSize(0);
}

// Operations with addresses

// Find all addresses in adbk
void CLocalAdbkClient::_ReadFullAddressBook(CAddressBook* adbk)
{
	_FindAllAddresses(adbk);
}

// Write all addresses in adbk
void CLocalAdbkClient::_WriteFullAddressBook(CAddressBook* adbk)
{
	
}

// Find all addresses in adbk
void CLocalAdbkClient::_FindAllAddresses(CAddressBook* adbk)
{
	StINETClientAction action(this, "Status::IMSP::FetchAddress", "Error::IMSP::OSErrFetchAddress", "Error::IMSP::NoBadFetchAddress");
	InitItemCtr();

	// Cache actionable address book
	mActionAdbk = adbk;

	try
	{
		// Fetch all addresses
		ScanAddressBook(adbk);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

}

// Fetch named address
void CLocalAdbkClient::_FetchAddress(CAddressBook* adbk,
								const cdstrvect& names)
{
}

// Store address
void CLocalAdbkClient::_StoreAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	Change(adbk, addrs, NULL, NULL, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->StoreAddress(adbk, addrs);
	}
}

// Store group
void CLocalAdbkClient::_StoreGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	Change(adbk, NULL, grps, NULL, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->StoreGroup(adbk, grps);
	}
}

// Change address
void CLocalAdbkClient::_ChangeAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	Change(adbk, addrs, NULL, addrs, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->ChangeAddress(adbk, addrs);
	}
}

// Change group
void CLocalAdbkClient::_ChangeGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::StoreAddress", "Error::IMSP::OSErrStoreAddress", "Error::IMSP::NoBadStoreAddress");

	// Cache actionable address book
	mActionAdbk = adbk;

	Change(adbk, NULL, grps, NULL, grps);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->ChangeGroup(adbk, grps);
	}
}

// Delete address
void CLocalAdbkClient::_DeleteAddress(CAddressBook* adbk, const CAddressList* addrs)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	Change(adbk, NULL, NULL, addrs, NULL);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->DeleteAddress(adbk, addrs);
	}
}

// Delete group
void CLocalAdbkClient::_DeleteGroup(CAddressBook* adbk, const CGroupList* grps)
{
	StINETClientAction action(this, "Status::IMSP::DeleteAddress", "Error::IMSP::OSErrDeleteAddress", "Error::IMSP::NoBadDeleteAddress");
	InitItemCtr();

	Change(adbk, NULL, NULL, NULL, grps);

	// Record action
	if (mRecorder)
	{
		CAdbkRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->DeleteGroup(adbk, grps);
	}
}

// These must be implemented by specific client

// Resolve address nick-name
void CLocalAdbkClient::_ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
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
void CLocalAdbkClient::_ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
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

// Search for addresses
void CLocalAdbkClient::_SearchAddress(CAddressBook* adbk,
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
void CLocalAdbkClient::_SetACL(CAddressBook* adbk, CACL* acl)
{
}

// Delete acl on server
void CLocalAdbkClient::_DeleteACL(CAddressBook* adbk, CACL* acl)
{
}

// Get all acls for adbk from server
void CLocalAdbkClient::_GetACL(CAddressBook* adbk)
{
}

// Get current user's rights to adbk
void CLocalAdbkClient::_MyRights(CAddressBook* adbk)
{
}

#pragma mark ____________________________Local Ops

void CLocalAdbkClient::GetFileName(const CAddressBook* adbk, cdstring& name)
{
	cdstring convert = LocalFileName(adbk->GetName(), GetAdbkOwner()->IsDisconnected() ? '.' : os_dir_delim, GetAdbkOwner()->IsDisconnected());
	GetFileName(convert, name);
}

void CLocalAdbkClient::GetFileName(const char* adbk, cdstring& name)
{
	name = mCWD;
	name += adbk;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Mac does not need file extensions
	if (GetAdbkOwner()->IsDisconnected())
#endif
	name += ".mba";
}

void CLocalAdbkClient::ListAddressBooks(CAddressBook* root, const cdstring& path)
{
	// Directory scan
	// Iterate over all .mba files/directories in directory
	// but not ones that are hidden
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Look for '.mba' at end only if disconnected
	// No longer require file creator/type match
	diriterator _dir(path, true, GetAdbkOwner()->IsDisconnected() ? ".mba" : NULL);
#else
	diriterator _dir(path, true, ".mba");
#endif
	_dir.set_return_hidden_files(false);
	const char* fname = NULL;
	while(_dir.next(&fname))
	{
		// Provide feedback
		BumpItemCtr("Status::IMSP::AddressBookFind");

		// Get the full path of the found item
		cdstring fpath(path);
		::addtopath(fpath, fname);

		// Get the relative path which will be the name of the node.
		// This is relative to the root path for this client.
		cdstring rpath(fpath, mCWD.length());
		
		// Strip off trailing .mba
		if (rpath.compare_end(".mba"))
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

void CLocalAdbkClient::ScanDirectory(const char* path, const cdstring& pattern, bool first)
{
	// Create lists for directories and mailboxes
	std::auto_ptr<cdstrvect> dirs(new cdstrvect);
	std::auto_ptr<cdstrvect> adbks(new cdstrvect);

	// Iterate over all .mba files/directories in directory
	// but not ones that are hidden
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Look for '.mba' at end only if disconnected
	// No longer require file creator/type match
	diriterator iter(path, true, GetAdbkOwner()->IsDisconnected() ? ".mba" : NULL);
#else
	diriterator iter(path, true, ".mba");
#endif
	iter.set_return_hidden_files(false);
	const char* p = NULL;
	while(iter.next(&p))
	{
		// Get full path
		cdstring fpath = path;
		fpath += p;

		// Check for directory
		if (iter.is_dir())
		{
			// Add to list of hits
			dirs->push_back(fpath);
		}
		else
		{
			// Remove suffix
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Only have .mba on disconnected address books
			if (GetAdbkOwner()->IsDisconnected())
#endif
			{
				fpath[fpath.length() - 4] = 0;
			}

			// Add to list of hits if pattern match
			if (fpath.PatternDirMatch(pattern, os_dir_delim))
				adbks->push_back(fpath);
		}
	}

	// Now process each set

	// Add each address book but look to see if its also a directory
	for(cdstrvect::iterator iter = adbks->begin(); iter != adbks->end(); iter++)
	{
		// Look in list of dirs
		cdstrvect::iterator found = std::find(dirs->begin(), dirs->end(), *iter);
		if (found == dirs->end())
		{
			// Add as addressbook without hierarchy
			AddAdbk(*iter);
		}
		else
		{
			// Pull directory
			dirs->erase(found);

			// Add as addressbook without hierarchy
			AddAdbk(*iter);

			// Now recurse after adding directory suffix
			cdstring temp_path = *iter;
			temp_path += os_dir_delim;
			ScanDirectory(temp_path, pattern);
		}
	}

	// Add each remaining directory
	for(cdstrvect::iterator iter = dirs->begin(); iter != dirs->end(); iter++)
	{
		// Now recurse after adding directory suffix
		cdstring temp_path = *iter;
		temp_path += os_dir_delim;
		ScanDirectory(temp_path, pattern);
	}
}

void CLocalAdbkClient::AddAdbk(const char* path_name)
{
	// Provide feedback
	BumpItemCtr("Status::IMSP::AddressBookFind");

	// Get name of mbox from full path
	cdstring adbk_name = &path_name[mCWD.length()];

	// Add adress book to list
	CAddressBook* adbk = new CAddressBook(GetAdbkOwner(), GetAdbkOwner()->GetStoreRoot(), true, false, adbk_name);
	GetAdbkOwner()->GetStoreRoot()->AddChild(adbk);
}

// Scan address book for all addresses
void CLocalAdbkClient::ScanAddressBook(CAddressBook* adbk)
{
	cdstring adbk_name;
	GetFileName(adbk, adbk_name);
	
	// Open address book
	mAdbk.clear();
	mAdbk.open(adbk_name, std::ios_base::in | std::ios_base::binary);
	CHECK_STREAM(mAdbk)
	
	mAdbk.seekg(0, std::ios_base::beg);
	
	// Get each line from file
	while(!mAdbk.fail())
	{
		// Get a line - may end if any endl
		cdstring line;
		getline(mAdbk, line, 0);
		
		// Parse out an address
		ParseFetchAddress(line);
	}

	mAdbk.clear();
	mAdbk.close();
}

// Scan address book for matching addresses
void CLocalAdbkClient::SearchAddressBook(CAddressBook* adbk, const cdstring& name, const CAdbkAddress::CAddressFields& fields, CAddressList* addr_list, CGroupList* grp_list)
{
	InitItemCtr();

	cdstring adbk_name;
	GetFileName(adbk, adbk_name);
	
	// Open address book
	mAdbk.clear();
	mAdbk.open(adbk_name, std::ios_base::in | std::ios_base::binary);
	CHECK_STREAM(mAdbk)
	
	mAdbk.seekg(0, std::ios_base::beg);
	
	// Use search mode
	StValueChanger<bool> change(mSearchMode, true);

	// Get each line from file
	while(!mAdbk.fail())
	{
		// Get a line - may end with any endl
		cdstring line;
		getline(mAdbk, line, 0);
		
		// Parse out an address
		mTempAddr = NULL;
		mTempGrp = NULL;
		ParseFetchAddress(line);

		// Look for matching item
		if (mTempAddr)
		{
			if (mTempAddr->Search(name, fields))
			{
				if (addr_list)
					addr_list->push_back(mTempAddr);
				else if (mActionAdbk)
					mActionAdbk->GetAddressList()->push_back(mTempAddr);
				else
					delete mTempAddr;
			}
			else
				delete mTempAddr;
			mTempAddr = NULL;
		}
		else if (mTempGrp)
		{
			if (mTempGrp->Search(name, fields))
			{
				if (grp_list)
					grp_list->push_back(mTempGrp);
				else if (mActionAdbk)
					mActionAdbk->GetGroupList()->push_back(mTempGrp);
				else
					delete mTempGrp;
			}
			else
				delete mTempGrp;
			mTempGrp = NULL;
		}
	}

	mAdbk.clear();
	mAdbk.close();
}


void CLocalAdbkClient::ParseFetchAddress(cdstring& line)
{
	// Do counter bits here as
	BumpItemCtr(mSearchMode ? "Status::IMSP::SearchAddressCount" : (mItemTotal ? "Status::IMSP::FetchAddressCount2" : "Status::IMSP::FetchAddressCount1"));

	// Import from line
	if (mActionAdbk)
		mActionAdbk->ImportAddress(line.c_str_mod(), !mSearchMode, &mTempAddr, &mTempGrp, true);
}

void CLocalAdbkClient::Change(CAddressBook* adbk, const CAddressList* add_addrs, const CGroupList* add_grps,
												const CAddressList* rmv_addrs, const CGroupList* rmv_grps)
{
	// Scan existing address book and copy non-matching items
	cdstring adbk_name;
	GetFileName(adbk, adbk_name);
	
	// Create a temporary file
	cdstring temp_name = adbk_name;
	temp_name += "~";
	cdstring old_name = adbk_name;
	old_name += "~~";

	try
	{
		// Now do removals
		if (rmv_addrs || rmv_grps)
		{
			// Open temp address book
	#if __dest_os == __mac_os || __dest_os == __mac_os_x
			StCreatorType file(cMulberryCreator, cAddressBookFileType);
	#endif
			cdfstream temp_adbk(temp_name, std::ios::out | std::ios::binary);

			// Open address book
			mAdbk.clear();
			mAdbk.open(adbk_name, std::ios_base::in | std::ios_base::binary);
			CHECK_STREAM(mAdbk)
		
			mAdbk.seekg(0, std::ios_base::beg);
			
			// Use search mode
			StValueChanger<bool> change(mSearchMode, true);

			// Get each line from file
			while(!mAdbk.fail())
			{
				// Get a line - may end with any endl
				cdstring line;
				getline(mAdbk, line, 0);
				
				// Parse out an address
				mTempAddr = NULL;
				mTempGrp = NULL;
				cdstring parseit = line;
				ParseFetchAddress(parseit);

				// Look for matching item
				if (mTempAddr)
				{
					bool delete_it = false;
					if (rmv_addrs)
					{
						CAddressList::const_iterator found = std::find_if(rmv_addrs->begin(), rmv_addrs->end(), CAdbkAddress::same_entry(mTempAddr));
						delete_it = (found != rmv_addrs->end());
					}

					// Copy if not the same
					if (!delete_it)
						temp_adbk << line << os_endl;
					
					// Always delete
					delete mTempAddr;
					mTempAddr = NULL;
				}
				else if (mTempGrp)
				{
					bool delete_it = false;
					if (rmv_grps)
					{
						CGroupList::const_iterator found = std::find_if(rmv_grps->begin(), rmv_grps->end(), CGroup::same_entry(mTempGrp));
						delete_it = (found != rmv_grps->end());
					}

					// Copy if not the same
					if (!delete_it)
						temp_adbk << line << os_endl;
					
					// Always delete
					delete mTempGrp;
					mTempGrp = NULL;
				}
			}

			// Now close and rename old ones
			mAdbk.clear();
			mAdbk.close();
			::rename_utf8(adbk_name, old_name);

			// Rename new ones
			temp_adbk.close();
			::rename_utf8(temp_name, adbk_name);

			// Remove old ones
			::remove_utf8(old_name);
		}

		// Now do appends
		if (add_addrs || add_grps)
		{
			mAdbk.clear();
			mAdbk.open(adbk_name, std::ios_base::out | std::ios_base::app | std::ios_base::binary);
			CHECK_STREAM(mAdbk)
			
			// Write the relevant ones
			if (add_addrs)
			{
				for(CAddressList::const_iterator iter = add_addrs->begin(); iter != add_addrs->end(); iter++)
				{
					const CAdbkAddress* addr = static_cast<const CAdbkAddress*>(*iter);
					cdstring out;
					out.steal(adbk->ExportAddress(addr));
					mAdbk << out;
					
					// Update entry to match new name
					const_cast<CAdbkAddress*>(addr)->SetEntry(addr->GetName());
				}
			}
			
			// Write the relevant ones
			if (add_grps)
			{
				for(CGroupList::const_iterator iter = add_grps->begin(); iter != add_grps->end(); iter++)
				{
					const CGroup* grp = static_cast<const CGroup*>(*iter);
					cdstring out;
					out.steal(adbk->ExportGroup(grp));
					mAdbk << out;
					
					// Update entry to match new name
					const_cast<CGroup*>(grp)->SetEntry(grp->GetName());
				}
			}

			mAdbk.clear();
			mAdbk.close();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove temp files
		::remove_utf8(temp_name);
	}
}
