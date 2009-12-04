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


// Header for an Calendar Action class

#ifndef __CCalendarAction__MULBERRY__
#define __CCalendarAction__MULBERRY__

#include "CRecordableAction.h"

#include "cdstring.h"

namespace calstore
{
class CCalendarStoreNode;
};

class CCalendarAction : public CRecordableAction
{
public:
	struct SCalendarAction
	{
		SCalendarAction()
			{ mIsDir = false; }
		SCalendarAction(const calstore::CCalendarStoreNode& node);
		SCalendarAction(const SCalendarAction& copy)
			{ mIsDir = copy.mIsDir; mName = copy.mName; }

		bool			mIsDir;
		cdstring		mName;
	};
	typedef std::pair<SCalendarAction, cdstring> SCalendarActionRename;

	enum ECalendarAction
	{
		eNone = 0,
		
		// Calendar actions
		eCreate,			// SCalendarAction*
		eDelete,			// SCalendarAction*
		eRename,			// SCalendarActionRename*
		eChange				// SCalendarAction*
	};
	

	CCalendarAction()
		{ mAction = eNone; mID = 0; mData = NULL; }
	CCalendarAction(ECalendarAction action, unsigned long id)
		{ mAction = action; mID = id; mData = NULL; }
	CCalendarAction(ECalendarAction action, unsigned long id, const calstore::CCalendarStoreNode& node)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SCalendarAction(node)); }
	CCalendarAction(ECalendarAction action, unsigned long id, const calstore::CCalendarStoreNode& node, const cdstring& name)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SCalendarActionRename(SCalendarAction(node), name)); }
	virtual ~CCalendarAction()
		{ _tidy(); }

	CCalendarAction(const CCalendarAction& copy)
		{ _copy(copy); }

	CCalendarAction& operator=(const CCalendarAction& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	int operator==(const CCalendarAction& test) const;								// Compare with same type
	
	ECalendarAction GetAction() const
		{ return mAction; }
	unsigned long GetID() const
		{ return mID; }
	const cdstring& GetCalendarName() const;
	const SCalendarAction& GetCalendarAction() const
		{ return *reinterpret_cast<const SCalendarAction*>(mData); }
	const SCalendarActionRename& GetCalendarActionRename() const
		{ return *reinterpret_cast<const SCalendarActionRename*>(mData); }

	virtual void WriteToStream(std::ostream& out, bool text = false) const;
	virtual void ReadFromStream(std::istream& in, unsigned long vers);

private:
	ECalendarAction mAction;
	unsigned long mID;
	void* mData;

	void _copy(const CCalendarAction& copy);
	void _tidy();

	void WriteCalendarAction(std::ostream& out, const SCalendarAction& action) const;
	void ReadCalendarAction(std::istream& in, SCalendarAction& action);
};

#endif
