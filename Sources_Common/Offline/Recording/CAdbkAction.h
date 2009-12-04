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


// Header for an Addressbook Action class

#ifndef __CADBKACTION__MULBERRY__
#define __CADBKACTION__MULBERRY__

#include "CRecordableAction.h"

#include "cdstring.h"

class CAdbkAction : public CRecordableAction
{
public:
	enum EAdbkAction
	{
		eNone = 0,
		
		// Addressbook actions
		eCreate,			// cdstring*
		eDelete,			// cdstring*
		eRename,			// cdstrpair*
		eChange,			// cdstring*
		
		// Address actions
		eStoreAddress,			// cdstrvect*
		eChangeAddress,			// cdstrvect*
		eDeleteAddress,			// cdstrvect*
		eStoreGroup,			// cdstrvect*
		eChangeGroup,			// cdstrvect*
		eDeleteGroup			// cdstrvect*
	};
	

	CAdbkAction()
		{ mAction = eNone; mID = 0; mData = NULL; }
	CAdbkAction(EAdbkAction action, unsigned long id)
		{ mAction = action; mID = id; mData = NULL; }
	CAdbkAction(EAdbkAction action, unsigned long id, const cdstring& name)
		{ mAction = action; mID = id; mData = static_cast<void*>(new cdstring(name)); }
	CAdbkAction(EAdbkAction action, unsigned long id, const cdstring& name1, const cdstring& name2)
		{ mAction = action; mID = id; mData = static_cast<void*>(new cdstrpair(name1, name2)); }
	CAdbkAction(EAdbkAction action, unsigned long id, const cdstrvect& list)
		{ mAction = action; mID = id; mData = static_cast<void*>(new cdstrvect(list)); }
	virtual ~CAdbkAction()
		{ _tidy(); }

	CAdbkAction(const CAdbkAction& copy)
		{ _copy(copy); }

	CAdbkAction& operator=(const CAdbkAction& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }
	
	EAdbkAction GetAction() const
		{ return mAction; }
	unsigned long GetID() const
		{ return mID; }
	const cdstring& GetName() const
		{ return *reinterpret_cast<const cdstring*>(mData); }
	const cdstrpair& GetNamePair() const
		{ return *reinterpret_cast<const cdstrpair*>(mData); }
	const cdstrvect& GetList() const
		{ return *reinterpret_cast<const cdstrvect*>(mData); }

	virtual void WriteToStream(std::ostream& out, bool text = false) const;
	virtual void ReadFromStream(std::istream& in, unsigned long vers);

private:
	EAdbkAction mAction;
	unsigned long mID;
	void* mData;

	void _copy(const CAdbkAction& copy);
	void _tidy();
};

#endif
