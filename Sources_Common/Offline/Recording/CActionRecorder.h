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


// Header for an action Recorder class

#ifndef __CACTIONRECORDER__MULBERRY__
#define __CACTIONRECORDER__MULBERRY__

#include "CRecordableAction.h"
#include "cdstring.h"
#include "cdmutex.h"
#include "ptrvector.h"
#include "cdfstream.h"

typedef ptrvector<CRecordableAction> CActionQueue;

class char_stream;

class CActionRecorder : public CActionQueue
{
public:
	class StActionRecorder
	{
	public:
		StActionRecorder(CActionRecorder* recorder, unsigned long id)
			{ mRecorder = recorder; mRecorder->StartRecording(id); }
		~StActionRecorder()
			{ mRecorder->StopRecording(); }
	private:
		CActionRecorder* mRecorder;
	};

	CActionRecorder();
	virtual ~CActionRecorder();

	void SetRecording(bool recording)
		{ mRecording = recording; }

	// ID
	unsigned long GetNextID();

	// Stream ops
	void SetDescriptor(const char* fpath)
		{ mDescriptor = fpath; }
	void Open();
	void Close();

	void StartRecording(unsigned long id);
	void StopRecording();

	void WriteItemsToStream(std::ostream& out, bool text = false) const;
	void ReadItemsFromStream(std::istream& in, unsigned long vers);

	// Playback logging
	void SetLog(std::ostream* log)
		{ mLog = log; }

	// Playback processing - handled by sub-class

protected:
	cdmutex		_mutex;
	bool		mRecording;
	cdstring	mDescriptor;
	cdfstream	mStream;
	unsigned long mStartRecord;
	unsigned long mNextID;
	unsigned long mCurrentID;
	std::ostream*	mLog;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	virtual OSType	GetMacFileType() const = 0;
#endif
	virtual CRecordableAction* NewAction() const = 0;
};

#endif
