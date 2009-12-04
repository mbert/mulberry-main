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


// Header for an Addressbook Record class

#ifndef __CADBKRECORD__MULBERRY__
#define __CADBKRECORD__MULBERRY__

#include "CActionRecorder.h"

#include "CAdbkAction.h"

#include "SBitFlags.h"

class CAdbkProtocol;
class CAddress;
class CAddressBook;
class CAddressList;
class CGroup;
class CGroupList;

class CAdbkRecord : public CActionRecorder
{
public:
	enum EAdbkAction
	{
		eNone = 0,
		eAll = -1L,

		// Addressbook actions
		eCreate = 1 << 0,
		eDelete = 1 << 1,
		eRename = 1 << 2,
		eChange = 1 << 3,
	
		// Address actions
		eStoreAddress = 1 << 7,
		eChangeAddress = 1 << 8,
		eDeleteAddress = 1 << 9,
		eStoreGroup = 1 << 10,
		eChangeGroup = 1 << 11,
		eDeleteGroup = 1 << 12,
		
		// Combined options
		eAdbkAllowed = eCreate | eDelete | eRename | eChange,
		eAddressAllowed = eStoreAddress | eChangeAddress | eDeleteAddress | eStoreGroup | eChangeGroup | eDeleteGroup
	};
	
	CAdbkRecord();
	virtual ~CAdbkRecord() {}

	void SetNoRecord()
		{ mRecord = eNone; SetRecording(false); }
	void SetFullRecord()
		{ mRecord = eAdbkAllowed + eAddressAllowed; SetRecording(true); }

	// Addressbook actions
	void Create(const CAddressBook* adbk);
	void Delete(const CAddressBook* adbk);
	void Rename(const CAddressBook* adbk, const cdstring& newname);
	void Change(const CAddressBook* adbk);

	// Address actions
	void StoreAddress(const CAddressBook* adbk, const CAddressList* addrs);
	void ChangeAddress(const CAddressBook* adbk, const CAddressList* addrs);
	void DeleteAddress(const CAddressBook* adbk, const CAddressList* addrs);
	void DoAddress(EAdbkAction action, CAdbkAction::EAdbkAction aaction, const CAddressBook* adbk, const CAddressList* addrs);

	void StoreGroup(const CAddressBook* adbk, const CGroupList* grps);
	void ChangeGroup(const CAddressBook* adbk, const CGroupList* grps);
	void DeleteGroup(const CAddressBook* adbk, const CGroupList* grps);
	void DoGroup(EAdbkAction action, CAdbkAction::EAdbkAction aaction, const CAddressBook* adbk, const CGroupList* grps);

	// Playback processing
	bool Playback(CAdbkProtocol* remote, CAdbkProtocol* local);

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual OSType	GetMacFileType() const;
#endif
	virtual CRecordableAction* NewAction() const;

private:
	SBitFlags mRecord;						// Commands to record

	CAdbkProtocol* mPlayRemote;
	CAdbkProtocol* mPlayLocal;

	void CompactPlayback();
	
	void PlaybackItem(CAdbkAction& item);

	void Playback_Create(CAdbkAction& item);
	void Playback_Delete(CAdbkAction& item);
	void Playback_Rename(CAdbkAction& item);
	void Playback_Change(CAdbkAction& item);

	void Playback_StoreAddress(CAdbkAction& item);
	void Playback_ChangeAddress(CAdbkAction& item);
	void Playback_StoreChangeAddress(CAdbkAction& item, bool store);
	void Playback_DeleteAddress(CAdbkAction& item);

	void Playback_StoreGroup(CAdbkAction& item);
	void Playback_ChangeGroup(CAdbkAction& item);
	void Playback_StoreChangeGroup(CAdbkAction& item, bool store);
	void Playback_DeleteGroup(CAdbkAction& item);
};

#endif
