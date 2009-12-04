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


// Header for tree node abstract base class

#ifndef __CTREENODE__MULBERRY__
#define __CTREENODE__MULBERRY__

#include "cdstring.h"

class CTreeNode
{
public:
	CTreeNode()
		{ mWDLevel = -1; mDirDelim = 0; }
	CTreeNode(const CTreeNode& copy);
	virtual ~CTreeNode() {}

	CTreeNode& operator=(const CTreeNode& copy);					// Assignment with same type

	virtual void	SetName(const char* name)					// Set mailbox name
		{ mName = name; SetShortName(); }
	const cdstring&	GetName() const							// Get full mailbox name
		{ return mName; }
	virtual cdstring GetAccountName(bool multi = true) const = 0;	// Get complete mailbox name
	const char*	GetShortName() const						// Get short mailbox name
		{ return mShortName; }

	void SetWDLevel(long level)									// Set WD index
		{ mWDLevel = level; }
	unsigned long GetWDLevel() const						// Get WD index
		{ return mWDLevel; }

	void	SetDirDelim(char delim)
		{ mDirDelim = delim; }
	char	GetDirDelim() const
		{ return mDirDelim; }

	virtual void	SetHasInferiors(bool inferiors) = 0;
	virtual void	SetIsExpanded(bool expanded) = 0;
	virtual bool	IsDirectory() const = 0;
	virtual bool	IsHierarchy() const = 0;
	virtual bool	HasInferiors() const = 0;
	virtual bool	IsExpanded() const = 0;									// Is expanded

	virtual	bool IsWildcard() const
		{ return false; }

protected:
	// Used when in list
	cdstring			mName;						// Full name of mailbox
	const char*			mShortName;					// Pointer to short name
	long				mWDLevel;					// Level in hierarchy
	char				mDirDelim;					// Directory delimiter

	virtual void	SetShortName();					// Set short name from full name
};

typedef CTreeNode CMboxNode;

#endif
