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

#include "CCalendarRecord.h"

#include "CLocalCommon.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreNode.h"
#include "CLog.h"
#include "CRFC822.h"
#include "CUnicodeStdLib.h"

#include "CICalendar.h"

#include <memory>

CCalendarRecord::CCalendarRecord()
{
}

#pragma mark ____________________________Calendar actions

void CCalendarRecord::Create(const calstore::CCalendarStoreNode& node)
{
	// Only if allowed
	if (!mRecord.IsSet(eCreate))
		return;

	push_back(new CCalendarAction(CCalendarAction::eCreate, mCurrentID, node));
}

void CCalendarRecord::Delete(const calstore::CCalendarStoreNode& node)
{
	// Only if allowed
	if (!mRecord.IsSet(eDelete))
		return;

	push_back(new CCalendarAction(CCalendarAction::eDelete, mCurrentID, node));
}

void CCalendarRecord::Rename(const calstore::CCalendarStoreNode& node, const cdstring& newname)
{
	// Only if allowed
	if (!mRecord.IsSet(eRename))
		return;

	push_back(new CCalendarAction(CCalendarAction::eRename, mCurrentID, node, newname));
}

void CCalendarRecord::Change(const calstore::CCalendarStoreNode& node)
{
	// Only if allowed
	if (!mRecord.IsSet(eChange))
		return;

	push_back(new CCalendarAction(CCalendarAction::eChange, mCurrentID, node));
}

#pragma mark ____________________________Playback

// Playback processing
bool CCalendarRecord::Playback(calstore::CCalendarProtocol* remote, calstore::CCalendarProtocol* local)
{
	// Check for forced logging
	bool log_created = false;
	std::auto_ptr<cdofstream> fout;
	if (!mLog && CLog::AllowPlaybackLog())
	{
		cdstring temp_name = mDescriptor + ".log";
		fout.reset(new cdofstream(temp_name, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary));
		SetLog(fout.get());
		log_created = true;
	}


	bool clone_opened = false;
	bool result = true;

	try
	{
		if (mLog)
		{
			*mLog << os_endl << os_endl << "----" << os_endl;
			*mLog << "Starting Calendar Playback: " << CRFC822::GetRFC822Date() << os_endl << std::flush;
		}

		// Read in the entire journal
		Open();
		if (mLog)
			*mLog << "Opened Playback log: " << mDescriptor << os_endl << std::flush;

		// Compact the playback to minimise operations
		CompactPlayback();

		if (size())
		{
			// Open main connection
			remote->Open();
			remote->Logon();

			if (mLog)
				*mLog << "Opened remote server: " << remote->GetAccountName() << os_endl << std::flush;
		}

		// Cache useful items
		mPlayRemote = remote;
		mPlayLocal = local;

		// Now run the play back
		while(size())
		{
			// Play the top item
			PlaybackItem(*static_cast<CCalendarAction*>(*begin()));

			// Done with top element
			erase(begin());
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Playback failed for some reason
		result = false;
	}

	// Close remote connection
	if (remote->IsOpen() && mLog)
		*mLog << "Closed remote server: " << remote->GetAccountName() << os_endl << std::flush;
	remote->Close();

	// Clear it out
	clear();
	mNextID = 1;

	// If no failure clear out the file
	if (result)
	{
		// Close and delete record file
		mStream.close();
		::remove_utf8(mDescriptor);

		if (mLog)
			*mLog << "Closed Playback log (cleared): " << mDescriptor << os_endl << std::flush;
	}
	else
	{
		// Close and delete existing
		mStream.close();
		::remove_utf8(mDescriptor);

		// Create/open new one
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		StCreatorType file(cMulberryCreator, cMailboxRecordType);
#endif
		mStream.open(mDescriptor, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

		// Add next id tag at start
		mStream.seekp(0, std::ios_base::beg);
		mStream.write(reinterpret_cast<const char*>(&mNextID), sizeof(unsigned long));

		// Write out items
		for(const_iterator iter = begin(); iter != end(); iter++)
			(*iter)->WriteToStream(mStream);

		mStream << std::flush;
		
		Close();

		if (mLog)
			*mLog << "Closed Playback log (not cleared): " << mDescriptor << os_endl << std::flush;
	}

	if (mLog)
		*mLog << os_endl << (result ? "Playback complete (no error): " : "Playback complete (error): ") << CRFC822::GetRFC822Date() << os_endl << os_endl << std::flush;

	if (log_created)
		SetLog(NULL);

	return result;
}

void CCalendarRecord::PlaybackItem(CCalendarAction& action)
{
	try
	{
		switch(action.GetAction())
		{

		// Addressbook actions
		case CCalendarAction::eCreate:
			Playback_Create(action);
			break;

		case CCalendarAction::eDelete:
			Playback_Delete(action);
			break;

		case CCalendarAction::eRename:
			Playback_Rename(action);
			break;

		case CCalendarAction::eChange:
			Playback_Change(action);
			break;

		default:;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Fail silently for now
	}
}

void CCalendarRecord::Playback_Create(CCalendarAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<calstore::CCalendarStoreNode> node(new calstore::CCalendarStoreNode(mPlayRemote, mPlayRemote->GetStoreRoot(), action.GetCalendarAction().mIsDir, false, false, action.GetCalendarAction().mName));

		// Create on server
		mPlayRemote->CreateCalendar(*node.get());

		if (mLog)
			*mLog << "  Create Calendar: " << action.GetCalendarAction().mName << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Create Calendar: " << action.GetCalendarAction().mName << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CCalendarRecord::Playback_Delete(CCalendarAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<calstore::CCalendarStoreNode> node(new calstore::CCalendarStoreNode(mPlayRemote, mPlayRemote->GetStoreRoot(), action.GetCalendarAction().mIsDir, false, false, action.GetCalendarAction().mName));

		// Delete on server
		mPlayRemote->DeleteCalendar(*node.get());

		if (mLog)
			*mLog << "  Delete Calendar: " << action.GetCalendarAction().mName << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Delete Calendar: " << action.GetCalendarAction().mName << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CCalendarRecord::Playback_Rename(CCalendarAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<calstore::CCalendarStoreNode> node(new calstore::CCalendarStoreNode(mPlayRemote, mPlayRemote->GetStoreRoot(), action.GetCalendarActionRename().first.mIsDir, false, false, action.GetCalendarActionRename().first.mName));

		// Rename on server
		mPlayRemote->RenameCalendar(*node.get(), action.GetCalendarActionRename().second);

		if (mLog)
			*mLog << "  Rename Calendar from: " << action.GetCalendarActionRename().first.mName <<
					" to: " << action.GetCalendarActionRename().second << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Rename Calendar from: " << action.GetCalendarActionRename().first.mName <<
					" to: " << action.GetCalendarActionRename().second << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CCalendarRecord::Playback_Change(CCalendarAction& action)
{
	std::auto_ptr<calstore::CCalendarStoreNode> node;
	std::auto_ptr<iCal::CICalendar> cal;

	try
	{
		// Create temp node and calendar
		node.reset(new calstore::CCalendarStoreNode(mPlayRemote, mPlayRemote->GetStoreRoot(), action.GetCalendarAction().mIsDir, false, false, action.GetCalendarAction().mName));
		cal.reset(new iCal::CICalendar);
		node->SetCalendar(cal.get());

#if 0
		// Read in calendar from local store
		mPlayLocal->ReadFullCalendar(*node.get(), *cal.get());

		// Write to server (will result in sync if required)
		mPlayRemote->WriteFullCalendar(*node.get(), *cal.get());

		// Write back to local store since changes have been propagated
		mPlayLocal->WriteFullCalendar(*node.get(), *cal.get());
#else
		// Just opening the calendar will cause sync
		mPlayRemote->OpenCalendar(*node.get(), *cal.get());
#endif

		// Clean-up
		node->SetCalendar(NULL);

		if (mLog)
			*mLog << "  Change Calendar: " << action.GetCalendarAction().mName << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Change Calendar: " << action.GetCalendarAction().mName << " failed" << os_endl << std::flush;
		
		// Clean-up
		if (node.get() != NULL)
			node->SetCalendar(NULL);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Playback processing
void CCalendarRecord::CompactPlayback()
{
	// I don't know what to do here yet
	
	// Step 1: compact all change actions into one
	CompactChange();

	// Step 2: remove all non-create actions on the same calendar prior to a delete
	// 			and remove all actions if create/delete pair exists
	CompactDelete();
}

// Compact multiple changes into one
void CCalendarRecord::CompactChange()
{
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		CCalendarAction* action1 = static_cast<CCalendarAction*>(*iter1);
		if (action1->GetAction() != CCalendarAction::eChange)
			continue;
		for(iterator iter2 = iter1 + 1; iter2 != end(); iter2++)
		{
			CCalendarAction* action2 = static_cast<CCalendarAction*>(*iter1);
			if (action1->GetCalendarName() == action2->GetCalendarName())
			{
				// Remove the duplicate
				erase(iter2);
				iter2--;
			}
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: multiple changes." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "1.txt");
		WriteItemsToStream(fout, true);
	}
}

// Compact actions when delete is present
void CCalendarRecord::CompactDelete()
{
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		CCalendarAction* action1 = static_cast<CCalendarAction*>(*iter1);

		// Only processing delete actions after the first one
		if ((action1->GetAction() != CCalendarAction::eDelete) || (iter1 == begin()))
			continue;
		
		// Look at all previous ones back to the beginning
		for(iterator iter2 = iter1 - 1; iter2 != begin(); iter2--)
		{
			CCalendarAction* action2 = static_cast<CCalendarAction*>(*iter1);

			// Only worried about ones on the same calendar
			if (action1->GetCalendarName() == action2->GetCalendarName())
			{
				// If we have a create operation then delete that and the delete delete action
				// and stop processing this one
				if (action2->GetAction() == CCalendarAction::eCreate)
				{
					erase(iter1);
					iter1--;
					erase(iter2);
					iter1--;
					break;
				}
				// Remove any change operations as they are pointless when the calendar is being deleted at the end
				else if (action2->GetAction() == CCalendarAction::eChange)
				{
					erase(iter2);
					iter1--;
				}
			}
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: no actions before delete." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "2.txt");
		WriteItemsToStream(fout, true);
	}
}

#pragma mark ____________________________Stream Ops

#if __dest_os == __mac_os || __dest_os == __mac_os_x
OSType CCalendarRecord::GetMacFileType() const
{
	return cMailboxRecordType;
}
#endif

CRecordableAction* CCalendarRecord::NewAction() const
{
	return new CCalendarAction();
}
