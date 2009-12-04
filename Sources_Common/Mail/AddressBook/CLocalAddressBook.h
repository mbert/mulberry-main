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


// CLocalAddressBook.h

// Class to handle remote address books

#ifndef __CLOCALADDRESSBOOK__MULBERRY__
#define __CLOCALADDRESSBOOK__MULBERRY__

#include "CAddressBook.h"

// Classes
class CAdbkProtocol;

class CLocalAddressBook: public CAddressBook
{
public:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		explicit CLocalAddressBook(PPx::FSObject* fspec);
		explicit CLocalAddressBook(PPx::FSObject* fspec, const char* name);
#else
		explicit CLocalAddressBook(const char* fname);
		explicit CLocalAddressBook(const char* fname, const char* name);
#endif

	virtual ~CLocalAddressBook();

	virtual cdstring GetAccountName() const;
	virtual cdstring GetURL(bool full = false) const;

	// Opening/closing
	virtual void	New();									// New visual address book on source
	virtual void	Open();									// Open visual address book from source
	virtual void	Read();									// Read in addresses
	virtual void	Save();									// Save addresses
	virtual void	Close();								// Close visual address book

	virtual void	Rename(cdstring& new_name);					// Rename
	virtual void	Delete();								// Delete

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual void	SetSpec(PPx::FSObject* fspec)
			{ mSpecified = (fspec != nil);
				if (mSpecified) mFileSpec = *fspec; }
	virtual const PPx::FSObject& GetSpec() const
		{ return mFileSpec; }
	virtual bool IsSpecified() const
		{ return mSpecified; }
#else
	virtual const cdstring& GetFileName() const
		{ return mFileName; }
#endif

	// ACLs - Not used for local address books
	void	CheckMyRights() {}

	void	AddACL(const CAdbkACL* acl) {}
	void	SetACL(CAdbkACL* acl) {}
	void	DeleteACL(CAdbkACL* acl) {}
	void	CheckACLs() {}

private:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	bool			mSpecified;
	PPx::FSObject	mFileSpec;											// Local file
#else
	cdstring		mFileName;
#endif
};

#endif
