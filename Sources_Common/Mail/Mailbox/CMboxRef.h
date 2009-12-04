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


// CMboxRef : class to handle URL references to mailboxes

#ifndef __CMBOXREF__MULBERRY__
#define __CMBOXREF__MULBERRY__

#include "CTreeNode.h"
#include "CMboxRefFwd.h"

#include "SBitFlags.h"

class CMbox;
class CMboxList;

class CMboxRef : public CMboxNode
{
public:
	CMboxRef() {}
	CMboxRef(const char* acct_name, char dir, bool directory = false);
	explicit CMboxRef(const CMbox* mbox);
	CMboxRef(const CMboxRef& copy);
	virtual ~CMboxRef() {}

	CMboxRef& operator=(const CMboxRef& copy);	// Assignment with same type
	CMboxRef& operator=(const CMbox& mbox);		// Assignment with mbox

	int operator==(const CMboxRef& comp) const;			// Compare with same type
	int operator==(const CMbox& mbox) const;			// Compare with mbox

	virtual cdstring GetAccountName(bool multi = true) const;	// Get complete mailbox name
	virtual void SetName(const char* acct);

	virtual const cdstring& GetAccountOnlyName() const			// Get name of account only
		{ return mAccountName; }

	virtual CMbox* ResolveMbox(bool force = false) const;		// Resolve to single mailbox (create may be required)
	virtual void ResolveMbox(CMboxList& mboxes,					// Resolve all mailboxes
								bool dynamic,
								bool no_dir = false) const;

	virtual void SetHasInferiors(bool inferiors)
		{ mFlags.Set(NMboxRef::eHasInferiors, inferiors); }
	virtual bool HasInferiors() const							// Check HasInferiors flag
		{ return mFlags.IsSet(NMboxRef::eHasInferiors);}

	virtual void SetIsExpanded(bool expanded)
		{ mFlags.Set(NMboxRef::eIsExpanded, expanded); }
	virtual bool IsExpanded() const							// Check IsExpanded flag
		{ return mFlags.IsSet(NMboxRef::eIsExpanded);}

	virtual void SetDirectory(bool directory)
		{ mFlags.Set(NMboxRef::eIsDirectory, directory); }
	virtual bool IsDirectory() const
		{ return mFlags.IsSet(NMboxRef::eIsDirectory); }
	virtual bool IsHierarchy() const
		{ return false; }

	virtual bool HasFailed() const
		{ return mFlags.IsSet(NMboxRef::eFailedResolve); }

	
protected:
	SBitFlags mFlags;
	cdstring mAccountName;

	virtual void	SetShortName();					// Set short name from full name
};


class CWildcardMboxRef : public CMboxRef
{
public:
	CWildcardMboxRef(const char* acct_name, char dir);
	CWildcardMboxRef(const CWildcardMboxRef& copy)
		: CMboxRef(copy) {}
	virtual ~CWildcardMboxRef() {}

	CWildcardMboxRef& operator=(const CWildcardMboxRef& copy);	// Assignment with same type

	int operator==(const CWildcardMboxRef& comp) const;			// Compare with same type
	int operator==(const CMbox& mbox) const;					// Compare with mbox

	virtual	bool IsWildcard() const
		{ return true; }

	virtual CMbox* ResolveMbox(bool force = false) const		// Wildcard cannot match single
		{ return nil; }
	virtual void ResolveMbox(CMboxList& mboxes,					// Resolve all mailboxes
								bool dynamic,
								bool no_dir = false) const;
};

#endif
