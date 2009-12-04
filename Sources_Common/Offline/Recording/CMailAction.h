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


// Header for an Mail Action class

#ifndef __CMAILACTION__MULBERRY__
#define __CMAILACTION__MULBERRY__

#include "CRecordableAction.h"

#include "CMessageFwd.h"

#include "cdstring.h"
#include "templs.h"

class CMailAction : public CRecordableAction
{
	friend class CMailRecord;

public:
	typedef std::pair<ulvector, ulvector> SUIDs;
	typedef std::pair<ulmap, ulmap> SMaps;
	struct SFlagAction
	{
		SFlagAction()
			{ mFlags = 0; mSet = false; }
		SFlagAction(const ulvector& uids, const ulvector& luids, unsigned long flags, bool set)
			{ mUids.first = uids; mUids.second = luids; mFlags = flags; mSet = set; }
		SFlagAction(const SFlagAction& copy)
			{ mUids = copy.mUids; mFlags = copy.mFlags; mSet = copy.mSet; }

		SUIDs			mUids;
		unsigned long	mFlags;
		bool			mSet;
	};

	struct SAppendAction
	{
		SAppendAction()
			{ mUid = 0; mFlags = 0; }
		SAppendAction(const cdstring& mbox, unsigned long uid, unsigned long flags)
			{ mName = mbox; mUid = uid; mFlags = flags; }
		SAppendAction(const SAppendAction& copy)
			{ mName = copy.mName; mUid = copy.mUid; mFlags = copy.mFlags; }

		cdstring		mName;
		unsigned long	mUid;
		unsigned long	mFlags;
	};

	typedef std::pair<const cdstring, unsigned long> SNameUIDAction;
	typedef std::pair<const cdstrpair, unsigned long> SRenameAction;
	typedef std::pair<const cdstring, SMaps > SCopyAction;
	typedef SUIDs SExpungeAction;

	enum EMailAction
	{
		eNone = 0,
		
		// Mailbox actions
		eCreate,			// SNameUIDAction*
		eDelete,			// SNameUIDAction*
		eRename,			// SRenameAction*
		eSubscribe,			// SNameUIDAction*
		eUnsubscribe,		// SNameUIDAction*
		
		// Mailbox connection
		eSelect,			// SNameUIDAction*
		eDeselect,			// SNameUIDAction*
		
		// Message actions
		eFlag,				// SFlagAction*
		eCopy,				// SCopyAction*
		eAppend,			// SAppendAction*
		eExpunge			// SExpungeAction*
	};
	

	CMailAction()
		{ mAction = eNone; mID = 0; mData = NULL; }
	CMailAction(EMailAction action, unsigned long id)
		{ mAction = action; mID = id; mData = NULL; }
	CMailAction(EMailAction action, unsigned long id, const cdstring& name, unsigned long uid)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SNameUIDAction(name, uid)); }
	CMailAction(EMailAction action, unsigned long id, const cdstring& name1, const cdstring& name2, unsigned long uid)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SRenameAction(cdstrpair(name1, name2), uid)); }
	CMailAction(unsigned long id, const ulvector& uids, const ulvector& luids, NMessage::EFlags flags, bool set)
		{ mAction = eFlag; mID = id; mData = static_cast<void*>(new SFlagAction(uids, luids, static_cast<unsigned long>(flags), set)); }
	CMailAction(EMailAction action, unsigned long id, const cdstring& name, const ulmap& copy_uids, const ulmap& copy_luids)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SCopyAction(name, SMaps(copy_uids, copy_luids))); }
	CMailAction(EMailAction action, unsigned long id, const cdstring& name, unsigned long uid, NMessage::EFlags flags)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SAppendAction(name, uid, static_cast<unsigned long>(flags))); }
	CMailAction(EMailAction action, unsigned long id, const ulvector& uids, const ulvector& luids)
		{ mAction = action; mID = id; mData = static_cast<void*>(new SExpungeAction(uids, luids)); }
	virtual ~CMailAction()
		{ _tidy(); }

	CMailAction(const CMailAction& copy)
		{ _copy(copy); }

	CMailAction& operator=(const CMailAction& copy)							// Assignment with same type
		{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }
	
	EMailAction GetAction() const
		{ return mAction; }
	unsigned long GetID() const
		{ return mID; }
	const SNameUIDAction& GetNameUIDAction() const
		{ return *reinterpret_cast<const SNameUIDAction*>(mData); }
	const SRenameAction& GetRenameAction() const
		{ return *reinterpret_cast<const SRenameAction*>(mData); }
	const SFlagAction& GetFlagAction() const
		{ return *reinterpret_cast<const SFlagAction*>(mData); }
	const SCopyAction& GetCopyAction() const
		{ return *reinterpret_cast<const SCopyAction*>(mData); }
	const SAppendAction& GetAppendAction() const
		{ return *reinterpret_cast<const SAppendAction*>(mData); }
	const SExpungeAction& GetExpungeAction() const
		{ return *reinterpret_cast<const SExpungeAction*>(mData); }

	bool RemoveUIDs(const ulvector& leave);

	virtual void WriteToStream(std::ostream& out, bool text = false) const;
	virtual void ReadFromStream(std::istream& in, unsigned long vers);

private:
	EMailAction mAction;
	unsigned long mID;
	void* mData;

	void _copy(const CMailAction& copy);
	void _tidy();
	
	void WriteUIDS(std::ostream& out, const ulvector& uids, bool text) const;
	void ReadUIDS(std::istream& in, unsigned long vers, ulvector& uids);

	void WriteUIDMap(std::ostream& out, const ulmap& uids, bool text) const;
	void ReadUIDMap(std::istream& in, unsigned long vers, ulmap& uids);
};

#endif
