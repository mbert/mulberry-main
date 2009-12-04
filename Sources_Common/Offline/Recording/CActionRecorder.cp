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


// Source for an Mail Record class

#include "CActionRecorder.h"

#include "CLocalCommon.h"

 
CActionRecorder::CActionRecorder()
{
	mRecording = false;
	mStartRecord = 0;
	mNextID = 1;
	mCurrentID = 0;
	
	mLog = NULL;
}

CActionRecorder::~CActionRecorder()
{
	if (mLog)
		*mLog << std::flush;
	mLog = NULL;
}

unsigned long CActionRecorder::GetNextID()
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Bump it
	mNextID++;

	// Write next ID
	mStream.seekp(0, std::ios_base::beg);
	mStream.write(reinterpret_cast<const char*>(&mNextID), sizeof(unsigned long));

	return mNextID - 1;
}

#pragma mark ____________________________Stream Ops

void CActionRecorder::Open()
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Only bother if not open
	if (mStream.is_open())
		return;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	StCreatorType file(cMulberryCreator, GetMacFileType());
#endif

	// Open the file stream and read in any existing items
	mStream.open(mDescriptor, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
	if (mStream.fail())
		mStream.open(mDescriptor, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

	// Move to beginning of stream
	mStream.seekg(0, std::ios_base::beg);

	// See if empty file
	if ((mStream.peek() == EOF) || mStream.fail())
	{
		// Clear error
		mStream.clear();

		// Write next ID at start of file
		mStream.seekp(0, std::ios_base::beg);
		mStream.write(reinterpret_cast<const char*>(&mNextID), sizeof(unsigned long));

		// Go back to beginning for input
		mStream.seekg(0, std::ios_base::beg);
	}

	// Get next ID
	mStream.read(reinterpret_cast<char*>(&mNextID), sizeof(unsigned long));

	// Read actions from stream
	ReadItemsFromStream(mStream, 0);
}

void CActionRecorder::Close()
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Write next ID
	mStream.seekp(0, std::ios_base::beg);
	mStream.write(reinterpret_cast<const char*>(&mNextID), sizeof(unsigned long));

	mStream.close();
}

void CActionRecorder::StartRecording(unsigned long id)
{
	// Must lock at start of entire record operation
	_mutex.acquire();

	mCurrentID = id;
	mStartRecord = size();
}

void CActionRecorder::StopRecording()
{
	// Now push out all new items
	if (mStartRecord != size())
	{
		// Always seek to end of file
		mStream.seekp(0, std::ios_base::end);

		// Write out new items
		for(const_iterator iter = begin() + mStartRecord; iter != end(); iter++)
			(*iter)->WriteToStream(mStream);

		mStream << std::flush;
	}

	mCurrentID = 0;

	// Now remove the lock
	_mutex.release();
}

void CActionRecorder::WriteItemsToStream(std::ostream& out, bool text) const
{
	// Write each action to stream
	for(const_iterator iter = begin(); iter != end(); iter++)
		(*iter)->WriteToStream(out, text);
}

void CActionRecorder::ReadItemsFromStream(std::istream& in, unsigned long vers)
{
	clear();

	while(true)
	{
		// See if end
		if ((in.peek() == EOF) || in.fail())
		{
			in.clear();
			break;
		}

		// Read in an action
		CRecordableAction* new_action = NewAction();
		new_action->ReadFromStream(in, vers);
		push_back(new_action);
	}
}
