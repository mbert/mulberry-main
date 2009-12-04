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


// Header for an Calendar Record class

#ifndef __CCalendarRecord__MULBERRY__
#define __CCalendarRecord__MULBERRY__

#include "CActionRecorder.h"

#include "CCalendarAction.h"

#include "SBitFlags.h"

namespace calstore
{
class CCalendarProtocol;
class CCalendarStoreNode;
};

class CCalendarRecord : public CActionRecorder
{
public:
	enum ECalendarAction
	{
		eNone = 0,
		eAll = -1L,

		// Calendar actions
		eCreate = 1 << 0,
		eDelete = 1 << 1,
		eRename = 1 << 2,
		eChange = 1 << 3,
	
		// Combined options
		eCalendarAllowed = eCreate | eDelete | eRename | eChange
	};
	
	CCalendarRecord();
	virtual ~CCalendarRecord() {}

	void SetNoRecord()
		{ mRecord = eNone; SetRecording(false); }
	void SetFullRecord()
		{ mRecord = eCalendarAllowed; SetRecording(true); }

	// Calendar actions
	void Create(const calstore::CCalendarStoreNode& node);
	void Delete(const calstore::CCalendarStoreNode& node);
	void Rename(const calstore::CCalendarStoreNode& node, const cdstring& newname);
	void Change(const calstore::CCalendarStoreNode& node);

	// Playback processing
	bool Playback(calstore::CCalendarProtocol* remote, calstore::CCalendarProtocol* local);

protected:
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual OSType	GetMacFileType() const;
#endif
	virtual CRecordableAction* NewAction() const;

private:
	SBitFlags mRecord;						// Commands to record

	calstore::CCalendarProtocol* mPlayRemote;
	calstore::CCalendarProtocol* mPlayLocal;

	void CompactPlayback();
	void CompactChange();
	void CompactDelete();
	
	void PlaybackItem(CCalendarAction& item);

	void Playback_Create(CCalendarAction& item);
	void Playback_Delete(CCalendarAction& item);
	void Playback_Rename(CCalendarAction& item);
	void Playback_Change(CCalendarAction& item);
};

#endif
