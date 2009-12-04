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


// Header for an Message Record class

#ifndef __CMAILRECORD__MULBERRY__
#define __CMAILRECORD__MULBERRY__

#include "CActionRecorder.h"

#include <map>
#include "CMailAction.h"

#include "templs.h"

#include "SBitFlags.h"

typedef std::set<unsigned long> ulset;

class CMbox;
class CMboxProtocol;
class CMessage;
class CProgress;

class CMailRecord : public CActionRecorder
{
public:
	typedef std::map<unsigned long, std::pair<CMbox*, CMbox*> > CConnectionList;
	typedef std::map<unsigned long, cdstrlong> ulcdstrlongmap;

	enum EMailAction
	{
		eNone = 0,
		eAll = -1L,

		// Mailbox actions
		eCreate = 1 << 0,
		eDelete = 1 << 1,
		eRename = 1 << 2,
		eSubscribe = 1 << 3,
		eUnsubscribe = 1 << 4,
		eSelect = 1 << 5,
		eDeselect = 1 << 6,
	
		// Message actions
		eFlag = 1 << 7,
		eCopy = 1 << 8,
		eAppend = 1 << 9,
		eExpunge = 1 << 10,
		
		// ACL related
		eSetACL = 1 << 12,
		eDeleteACL = 1 << 13,

		// Combined options
		eMailboxAllowed = eCreate | eDelete | eRename | eSubscribe | eUnsubscribe |
							eSelect | eDeselect |
							eSetACL | eDeleteACL,
		eMessageAllowed = eFlag | eCopy | eAppend | eExpunge,
		eExpungeOnlyAllowed = eExpunge
	};
	
	enum
	{
		eException_FailedSafe = -2,
		eException_TooManyAttempts = -3
	};

	CMailRecord();
	virtual ~CMailRecord() {}

	void SetNoRecord()
		{ mRecord = eNone; SetRecording(false); }
	void SetFullRecord()
		{ mRecord = eMailboxAllowed + eMessageAllowed; SetRecording(true); }
	void SetMailboxRecord()
		{ mRecord = eMailboxAllowed; SetRecording(true); }
	void SetMessageRecord()
		{ mRecord = eMessageAllowed; SetRecording(true); }
	void SetExpungeOnlyRecord()
		{ mRecord = eExpungeOnlyAllowed; SetRecording(true); }

	// Mailbox actions
	void Create(const CMbox* mbox);
	void Delete(const CMbox* mbox);
	void Rename(const CMbox* mbox, const cdstring& newname);
	void Subscribe(const CMbox* mbox);
	void Unsubscribe(const CMbox* mbox);
	void Select(const CMbox* mbox);
	void Deselect(const CMbox* mbox);

	// Message actions
	void SetFlags(const ulvector& uids, const ulvector&luids, NMessage::EFlags flags, bool set);
	void CopyTo(const ulmap& copy_uids, const ulmap& copy_luids, const CMbox* mbox);
	void AppendTo(unsigned long uid, NMessage::EFlags flags, const CMbox* mbox);
	void Expunge(const ulvector& uids, const ulvector& luids);
	
	// Tests
	bool ExpungedUID(unsigned long uid, bool local) const;
	void Flush(CMailAction::EMailAction actions);

	// Playback processing
	bool Playback(CMboxProtocol* remote, CMboxProtocol* local, CProgress* progress);

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual OSType	GetMacFileType() const;
#endif
	virtual CRecordableAction* NewAction() const;

private:
	SBitFlags mRecord;						// Commands to record
	ulcdstrlongmap mPendingSelect;

	CConnectionList mConns;
	CMboxProtocol* mPlayRemote;
	CMboxProtocol* mPlayLocal;
	CMboxProtocol* mPlayReuse;

	void CompactPlayback();
	void CompactMissing();
	void CompactUnterminated();
	void CompactMultiple();
	void CompactExpunge();
	void CompactFlags();

	bool CanActionMove(CMailAction* action1, CMailAction* action2) const;
	bool SameFlagAction(const CMailAction* action1, const CMailAction* action2) const;
	bool CompetingFlagAction(const CMailAction* action1, const CMailAction* action2) const;
	bool CompetingCopyAction(const CMailAction* action1, const CMailAction* action2) const;
	bool CompetingAppendAction(const CMailAction* action1, const CMailAction* action2) const;
	void TransferUIDs(CMailAction* from, CMailAction* to) const;
	bool set_contains(const ulvector& set1, const ulvector& set2) const;
	bool set_contains(const ulvector& set1, const ulmap& set2) const;
	void transfer_unique(ulvector& from, ulvector& to) const;

	bool PushSelect(bool add = true);
	
	void PlaybackItem(CMailAction& item, bool& done);

	void Playback_Create(CMailAction& item);
	void Playback_Delete(CMailAction& item);
	void Playback_Rename(CMailAction& item);
	void Playback_Subscribe(CMailAction& item);
	void Playback_Unsubscribe(CMailAction& item);
	void Playback_Select(CMailAction& item);
	void Playback_Deselect(CMailAction& item);

	void Playback_SetFlags(CMailAction& item);

	void Playback_CopyTo(CMailAction& item);
	void Playback_CopyToFull(CMailAction& item, ulvector& uids, const ulmap& local_map);
	void Playback_CopyToPartial(CMailAction& item, unsigned long local_uid);

	void Playback_AppendTo(CMailAction& item);

	void Playback_Expunge(CMailAction& item);
	
	void Playback_RecoverSelects();
	void Playback_RecoverNonfatal(CMailAction& item);
	void Playback_RemoveActions(const cdstring& name, CMbox* mbox);
	void Playback_RemoveExpungeActions(const cdstring& name, unsigned long id);

	void PurgeUIDs(ulvector& uids, ulvector& luids);
	void RemapUIDs(CMbox* local, ulvector& uids, const ulvector& luids);
	bool MapLocalUIDs(CMbox* local, ulvector& uids, ulvector* missing = NULL, ulmap* local_map = NULL);
	void AddLocalUIDs(CMbox* local, const ulvector& luids, ulvector& uids);
	
	unsigned long DescoverUID(CMessage* lmsg, CMbox* rsource);
	void DescoverUIDs(ulvector& uids, ulmap& map, CMbox* rsource, CMbox* rdest);
};

#endif
