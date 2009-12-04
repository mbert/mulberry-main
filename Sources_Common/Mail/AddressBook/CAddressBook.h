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


// CAddressBook.h

// Class that encapsulates address book functionality (local and remote).
// This is an abstract base class and should be derived for each type
// of address book (eg local, remote IMSP-style, remote-LDAP etc)

#ifndef __CADDRESSBOOK__MULBERRY__
#define __CADDRESSBOOK__MULBERRY__

#include "CAdbkACL.h"
#include "CAdbkAddress.h"
#include "CAddressList.h"
#include "CGroupList.h"

#include "SBitFlags.h"

#include "cdstring.h"
#include "cdmutex.h"
#include "ptrvector.h"

// Classes
namespace xmllib 
{
class XMLDocument;
class XMLNode;
};

namespace vCard 
{
class CVCardAddressBook;
};

class CAdbkProtocol;

class CAddressBook;
typedef ptrvector<CAddressBook> CAddressBookList;

class CAddressBook
{
public:
	enum EFlags
	{
		// Adbk status
		eNone = 					0,
		eIsProtocol =				1L << 0,
		eIsDirectory =				1L << 1,
		eIsAdbk =					1L << 2,
		eHasExpanded =				1L << 3,
		eReadOnly =					1L << 4,
		eOpen =						1L << 5,
		eOpenOnStart =				1L << 6,
		eLookup =					1L << 7,
		eSearch =					1L << 8,
		eAdd =						1L << 9,
		eAutoSync =					1L << 10,
		eIsCached =					1L << 11,
		eSynchronising =			1L << 12,		// Addressbook is being synchronised
		eIsDisplayHierarchy =       1L << 13
		
	};

public:

	static cdmutex _mutex;										// Used for locks

	// std::sort methods
	static bool sort_by_name(const CAddressBook* s1, const CAddressBook* s2);

		CAddressBook();
		CAddressBook(CAdbkProtocol* proto);
		CAddressBook(CAdbkProtocol* proto, CAddressBook* parent, bool is_adbk = true, bool is_dir = false, const cdstring& name = cdstring::null_str);
	virtual ~CAddressBook();

	// Getters
	CAdbkProtocol* GetProtocol() const
	{
		return mProtocol;
	}

	CAddressBook* GetParent() const
	{
		return mParent;
	}

	void AddChild(CAddressBook* child, bool sort = false);
	void AddChildHierarchy(CAddressBook* child, bool sort = false);
	void InsertChild(CAddressBook* child, uint32_t index, bool sort = false);
	CAddressBookList* GetChildren() const
	{
		return mChildren;
	}
	void SortChildren();

	cdstring GetAccountName(bool multi = true) const;
	cdstring GetURL(bool full = false) const;

	void SetName(const cdstring& name);							// Set name
	const cdstring&	GetName() const								// Get full name
		{ return mName; }
	const cdstring&	GetShortName() const							// Get short name
		{ return mShortName; }
	const cdstring&	GetDisplayShortName() const						// Get display short name
		{ return mDisplayName.empty() ? mShortName : mDisplayName; }
	void	NewName(const cdstring& name);						// Tell this and children to adjust names
	
	void SetDisplayName(const cdstring& name)					// Set name
		{ mDisplayName = name; }
	const cdstring&	GetDisplayName() const						// Get full name
		{ return mDisplayName; }
	
	CAddressList* GetAddressList()
		{ return &mAddresses; }
	CGroupList* GetGroupList()
		{ return &mGroups; }

	void	SetFlags(EFlags new_flags, bool add = true)			// Set flags
		{ mFlags.Set(new_flags, add); }
	EFlags	GetFlags() const								// Get flags
		{ return (EFlags) mFlags.Get(); }

	bool IsProtocol() const
	{
		return mFlags.IsSet(eIsProtocol);
	}

	bool IsDisplayHierarchy() const
	{
		return mFlags.IsSet(eIsDisplayHierarchy);
	}
	
	bool IsDirectory() const
	{
		return mFlags.IsSet(eIsDirectory);
	}

	bool IsAdbk() const
	{
		return mFlags.IsSet(eIsAdbk);
	}

	bool HasInferiors() const
	{
		return IsDirectory() && (mChildren != NULL) && (mChildren->size() != 0);
	}

	void SetHasExpanded(bool has_expanded)
	{
		mFlags.Set(eHasExpanded, has_expanded);
	}
	bool HasExpanded() const									// Has expanded children
	{
		return mFlags.IsSet(eHasExpanded);
	}

	bool IsOpen() const
		{ return mFlags.IsSet(eOpen); }
	bool	IsOpenOnStart() const
		{ return mFlags.IsSet(eOpenOnStart); }
	bool	IsLookup() const
		{ return mFlags.IsSet(eLookup); }
	bool	IsSearch() const
		{ return mFlags.IsSet(eSearch); }
	bool	IsAdd() const
		{ return mFlags.IsSet(eAdd); }
	bool	IsAutoSync() const
		{ return mFlags.IsSet(eAutoSync);}
	bool	IsCachedAdbk() const
		{ return mFlags.IsSet(eIsCached);}
	bool	IsSynchronising() const
		{ return mFlags.IsSet(eSynchronising);}

	void SetSize(uint32_t size)
	{
		mSize = size;
	}
	uint32_t GetSize() const
	{
		return mSize;
	}
	void CheckSize();

	uint32_t GetLastSync() const
	{
		return mLastSync;
	}
	void SyncNow() const;

	bool IsCached() const;
	void TestDisconnectCache();

	CAddressBook* FindNode(const cdstring& path, bool discover = false) const;
	CAddressBook* FindNodeOrCreate(const cdstring& path) const
	{
		return FindNode(path, true);
	}

	void RemoveFromParent();
	void Clear();

	uint32_t GetRow() const;
	uint32_t CountDescendants() const;
	uint32_t GetParentOffset() const;
	const CAddressBook* GetSibling() const;
	void GetInsertRows(uint32_t& parent_row, uint32_t& sibling_row) const;

	// VCard stuff
	const vCard::CVCardAddressBook* GetVCardAdbk() const
	{
		return mVCardAdbk;
	}
	vCard::CVCardAddressBook* GetVCardAdbk()
	{
		return mVCardAdbk;
	}
	void SetVCardAdbk(vCard::CVCardAddressBook* adbk)
	{
		mVCardAdbk = adbk;
	}

	// Operations on address books
	void MoveAddressBook(const CAddressBook* dir, bool sibling);
	void CopyAddressBook(CAddressBook* node);
	void CopyAddressBookContents(CAddressBook* node);

	// Opening/closing
	void	Open();									// Open visual address book from source
	void	Close();								// Close visual address book
	void	ClearContents();						// Clear addresses

	// Manipulation
	void	Rename(cdstring& new_name);				// Rename
	void	Delete();								// Delete
	void	Empty();								// Empty

	void	Synchronise(bool fast);					// Synchronise to local
	void	ClearDisconnect();						// Clear disconnected cache

	// Searching
	CAdbkAddress* FindAddress(const char* name);
	CAdbkAddress* FindAddress(const CAddress* addr);
	CAdbkAddress* FindAddressEntry(const char* name);
	CGroup* FindGroup(const char* name);
	CGroup* FindGroupEntry(const char* name);

	// Adding/removing items
	void	AddAddress(CAddress* addr, bool sorted = false);
	void	AddAddress(CAddressList* addrs, bool sorted = false);
	void	AddUniqueAddresses(CAddressList& add);							// Add unique addresses from list
	void	UpdateAddress(CAddress* addr, bool sorted = false);				// Address changed
	void	UpdateAddress(CAddressList* addrs, bool sorted = false);		// Address changed
	void	UpdateAddress(CAddressList* old_addrs, CAddressList* new_addrs, bool sorted = false);		// Address changed
	void	RemoveAddress(CAddress* addr);
	void	RemoveAddress(CAddressList* addrs);

	void	AddGroup(CGroup* grp, bool sorted = false);
	void	AddGroup(CGroupList* grps, bool sorted = false);
	void	AddUniqueGroups(CGroupList& add);								// Add unique groups from list
	void	UpdateGroup(CGroup* grp, bool sorted = false);					// Group changed
	void	UpdateGroup(CGroupList* grps, bool sorted = false);				// Group changed
	void	UpdateGroup(CGroupList* old_grps, CGroupList* new_grps, bool sorted = false);		// Address changed
	void	RemoveGroup(CGroup* grp);
	void	RemoveGroup(CGroupList* grps);

	void	MakeUniqueEntry(CAdbkAddress* addr) const;	// Give this address a unique entry

	// Reading/writing
	void	ImportAddresses(char* txt);
	void	ImportAddress(char* txt, bool add,
									CAdbkAddress** addr,
									CGroup** grp, bool add_entry = false);
	char*	ExportAddress(const CAdbkAddress* addr) const;
	char*	ExportGroup(const CGroup* grp) const;

	// Lookup
	bool	FindNickName(const char* nick_name,
										CAdbkAddress*& addr,
										bool cache_only = false);	// Find address from nick-name

	bool	FindGroupName(const char* grp_name,
										CGroup*& grp,
										bool cache_only = false);			// Find group from group-name

	void	SearchAddress(const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list);	// Do search

	// ACLs
	void	SetMyRights(SACLRight rights)				// Set user's rights to this mailbox
		{ mMyRights = rights; }
	SACLRight	GetMyRights() const
		{ return mMyRights; }
	void	CheckMyRights();							// Get user's rights from server

	void	AddACL(const CAdbkACL* acl);				// Add ACL to list
	void	SetACL(CAdbkACL* acl);						// Set ACL on server
	void	DeleteACL(CAdbkACL* acl);					// Delete ACL on server
	CAdbkACLList*	GetACLs() const
		{ return mACLs; }
	void	CheckACLs();								// Get ACLs from server

	void WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent, bool is_root = false) const;
	void ReadXML(const xmllib::XMLNode* node, bool is_root = false);

protected:
	CAdbkProtocol*		mProtocol;
	CAddressBook*		mParent;
	CAddressBookList*	mChildren;
	SBitFlags			mFlags;						// Flags state
	cdstring			mName;						// Full path name of item
	cdstring			mShortName;					// Pointer to the last part of the path name
	cdstring			mDisplayName;				// Display name of item
	uint32_t			mSize;						// Disk size
	mutable uint32_t	mLastSync;					// Last sync time
	CAddressList		mAddresses;
	CGroupList			mGroups;
	SACLRight			mMyRights;					// User's rights on this address book
	CAdbkACLList*		mACLs;						// List of ACLs on this address book
	unsigned long		mRefCount;					// Reference count of opens
	vCard::CVCardAddressBook*	mVCardAdbk;					// The VCard address book

			char SkipTerm(char** txt, cdstring& copy);

			bool OpenCount();						// Determine whether to open based on ref count
			bool CloseCount();						// Determine whether to close based on ref cunt

			void SetShortName();					// Set short name from full name
			void ParentRenamed();				// Tell children to adjust names when parent moves
};

#endif
