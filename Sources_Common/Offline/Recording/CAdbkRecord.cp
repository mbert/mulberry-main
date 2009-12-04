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

#include "CAdbkRecord.h"

#include "CLocalCommon.h"
#include "CAdbkProtocol.h"
#include "CAddressBook.h"
#include "CLog.h"
#include "CRFC822.h"
#include "CUnicodeStdLib.h"

#include "CVCardAddressBook.h"

#include <memory>

CAdbkRecord::CAdbkRecord()
{
}

#pragma mark ____________________________Addressbook actions

void CAdbkRecord::Create(const CAddressBook* adbk)
{
	// Only if allowed
	if (!mRecord.IsSet(eCreate))
		return;

	push_back(new CAdbkAction(CAdbkAction::eCreate, mCurrentID, adbk->GetName()));
}

void CAdbkRecord::Delete(const CAddressBook* adbk)
{
	// Only if allowed
	if (!mRecord.IsSet(eDelete))
		return;

	push_back(new CAdbkAction(CAdbkAction::eDelete, mCurrentID, adbk->GetName()));
}

void CAdbkRecord::Rename(const CAddressBook* adbk, const cdstring& newname)
{
	// Only if allowed
	if (!mRecord.IsSet(eRename))
		return;

	push_back(new CAdbkAction(CAdbkAction::eRename, mCurrentID, adbk->GetName(), newname));
}

void CAdbkRecord::Change(const CAddressBook* adbk)
{
	// Only if allowed
	if (!mRecord.IsSet(eChange))
		return;

	push_back(new CAdbkAction(CAdbkAction::eChange, mCurrentID, adbk->GetName()));
}

#pragma mark ____________________________Address actions

void CAdbkRecord::StoreAddress(const CAddressBook* adbk, const CAddressList* addrs)
{
	DoAddress(eStoreAddress, CAdbkAction::eStoreAddress, adbk, addrs);
}

void CAdbkRecord::ChangeAddress(const CAddressBook* adbk, const CAddressList* addrs)
{
	DoAddress(eChangeAddress, CAdbkAction::eChangeAddress, adbk, addrs);
}

void CAdbkRecord::DeleteAddress(const CAddressBook* adbk, const CAddressList* addrs)
{
	DoAddress(eDeleteAddress, CAdbkAction::eDeleteAddress, adbk, addrs);
}

void CAdbkRecord::DoAddress(EAdbkAction action, CAdbkAction::EAdbkAction aaction, const CAddressBook* adbk, const CAddressList* addrs)
{
	// Only if allowed
	if (!mRecord.IsSet(action))
		return;

	cdstrvect items;
	items.push_back(adbk->GetName());
	if (addrs)
	{
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
			items.push_back(static_cast<const CAdbkAddress*>(*iter)->GetEntry());
	}

	push_back(new CAdbkAction(aaction, mCurrentID, items));
}

void CAdbkRecord::StoreGroup(const CAddressBook* adbk, const CGroupList* grps)
{
	DoGroup(eStoreGroup, CAdbkAction::eStoreGroup, adbk, grps);
}

void CAdbkRecord::ChangeGroup(const CAddressBook* adbk, const CGroupList* grps)
{
	DoGroup(eChangeGroup, CAdbkAction::eChangeGroup, adbk, grps);
}

void CAdbkRecord::DeleteGroup(const CAddressBook* adbk, const CGroupList* grps)
{
	DoGroup(eDeleteGroup, CAdbkAction::eDeleteGroup, adbk, grps);
}

void CAdbkRecord::DoGroup(EAdbkAction action, CAdbkAction::EAdbkAction aaction, const CAddressBook* adbk, const CGroupList* grps)
{
	// Only if allowed
	if (!mRecord.IsSet(action))
		return;

	cdstrvect items;
	items.push_back(adbk->GetName());
	if (grps)
	{
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
			items.push_back((*iter)->GetEntry());
	}

	push_back(new CAdbkAction(aaction, mCurrentID, items));
}

#pragma mark ____________________________Playback

// Playback processing
bool CAdbkRecord::Playback(CAdbkProtocol* remote, CAdbkProtocol* local)
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
			*mLog << "Starting Addressbook Playback: " << CRFC822::GetRFC822Date() << os_endl << std::flush;
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
			PlaybackItem(*static_cast<CAdbkAction*>(*begin()));

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

void CAdbkRecord::PlaybackItem(CAdbkAction& action)
{
	try
	{
		switch(action.GetAction())
		{

		// Addressbook actions
		case CAdbkAction::eCreate:
			Playback_Create(action);
			break;

		case CAdbkAction::eDelete:
			Playback_Delete(action);
			break;

		case CAdbkAction::eRename:
			Playback_Rename(action);
			break;

		case CAdbkAction::eChange:
			Playback_Change(action);
			break;

		// Address actions
		case CAdbkAction::eStoreAddress:
			Playback_StoreAddress(action);
			break;

		case CAdbkAction::eDeleteAddress:
			Playback_DeleteAddress(action);
			break;

		case CAdbkAction::eStoreGroup:
			Playback_StoreGroup(action);
			break;

		case CAdbkAction::eDeleteGroup:
			Playback_DeleteGroup(action);
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

void CAdbkRecord::Playback_Create(CAdbkAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetName()));

		// Create on server
		mPlayRemote->CreateAdbk(adbk.get());

		if (mLog)
			*mLog << "  Create Addressbook: " << action.GetName() << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Create Addressbook: " << action.GetName() << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_Delete(CAdbkAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetName()));

		// Create on server
		mPlayRemote->DeleteAdbk(adbk.get());

		if (mLog)
			*mLog << "  Delete Addressbook: " << action.GetName() << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Delete Addressbook: " << action.GetName() << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_Change(CAdbkAction& action)
{
	std::auto_ptr<CAddressBook> node;
	std::auto_ptr<vCard::CVCardAddressBook> adbk;

	try
	{
		// Create temp node and calendar
		node.reset(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetName()));
		adbk.reset(new vCard::CVCardAddressBook);
		node->SetVCardAdbk(adbk.get());

#if 0
		// Read in calendar from local store
		mPlayLocal->ReadFullAddressBook(*node.get());

		// Write to server (will result in sync if required)
		mPlayRemote->WriteFullAddressBook(*node.get());

		// Write back to local store since changes have been propagated
		mPlayLocal->WriteFullAddressBook(*node.get());
#else
		// Just opening the calendar will cause sync
		mPlayRemote->OpenAdbk(node.get());
#endif

		// Clean-up
		node->SetVCardAdbk(NULL);

		if (mLog)
			*mLog << "  Change Address Book: " << action.GetName() << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Change Address Book: " << action.GetName() << " failed" << os_endl << std::flush;
		
		// Clean-up
		if (node.get() != NULL)
			node->SetVCardAdbk(NULL);

		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_Rename(CAdbkAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetNamePair().first));

		// Create on server
		mPlayRemote->RenameAdbk(adbk.get(), action.GetNamePair().second);

		if (mLog)
			*mLog << "  Rename Addressbook from: " << action.GetNamePair().first <<
					" to: " << action.GetNamePair().second << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Rename Addressbook from: " << action.GetNamePair().first <<
					" to: " << action.GetNamePair().second << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_StoreAddress(CAdbkAction& action)
{
	Playback_StoreChangeAddress(action, true);
}

void CAdbkRecord::Playback_ChangeAddress(CAdbkAction& action)
{
	Playback_StoreChangeAddress(action, false);
}

void CAdbkRecord::Playback_StoreChangeAddress(CAdbkAction& action, bool store)
{
	CAddressBook* local = NULL;
	bool local_open = false;
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetList().front()));

		// Get address from local object
		local = mPlayLocal->GetNode(action.GetList().front());

		// Can only do if local exists
		if (local)
		{
			// Cache addresses from local address book by opening it
			local->Open();
			local_open = true;
			
			// Add addresses to list
			CAddressList addrs;
			addrs.set_delete_data(false);
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				// Find address in local address book
				CAdbkAddress* addr = local->FindAddressEntry(*iter);
				if (addr)
					addrs.push_back(addr);
			}

			// Create on server if address found
			if (addrs.size())
			{
				if (store)
					mPlayRemote->AddAddress(adbk.get(), &addrs);
				else
					mPlayRemote->ChangeAddress(adbk.get(), &addrs);
			}
			
			// Close local after storing address as address object will be cleared on close
			local->Close();
		}

		if (mLog)
		{
			*mLog << (store ? "  Store address: " :  "  Change address: ");
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean-up close
		if (local && local_open)
			local->Close();

		if (mLog)
		{
			*mLog << (store ? "  Store address: " :  "  Change address: ");
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " failed" << os_endl << std::flush;
		}
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_DeleteAddress(CAdbkAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetList().front()));

		// Create dummy addresses for delete
		CAddressList addrs;
		for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			addrs.push_back(new CAdbkAddress(*iter, NULL, NULL));

		// Delete on server
		mPlayRemote->RemoveAddress(adbk.get(), &addrs);

		if (mLog)
		{
			*mLog << "  Remove address: ";
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
		{
			*mLog << "  Remove address: ";
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " failed" << os_endl << std::flush;
		}
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_StoreGroup(CAdbkAction& action)
{
	Playback_StoreChangeGroup(action, true);
}

void CAdbkRecord::Playback_ChangeGroup(CAdbkAction& action)
{
	Playback_StoreChangeGroup(action, false);
}

void CAdbkRecord::Playback_StoreChangeGroup(CAdbkAction& action, bool store)
{
	CAddressBook* local = NULL;
	bool local_open = false;
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetList().front()));

		// Get address from local object
		local = mPlayLocal->GetNode(action.GetList().front());

		// Can only do if local exists
		if (local)
		{
			// Cache addresses from local address book by opening it
			local->Open();
			local_open = true;

			// Add groups to list
			CGroupList grps;
			grps.set_delete_data(false);
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				// Find group in local address book
				CGroup* grp = local->FindGroupEntry(*iter);
				if (grp)
					grps.push_back(grp);
			}

			// Create on server if address found
			if (grps.size())
			{
				if (store)
					mPlayRemote->AddGroup(adbk.get(), &grps);
				else
					mPlayRemote->ChangeGroup(adbk.get(), &grps);
			}
			
			// Close local after storing address as address object will be cleared on close
			local->Close();
		}

		if (mLog)
		{
			*mLog << (store ? "  Store group: " :  "  Change group: ");
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean-up close
		if (local && local_open)
			local->Close();

		if (mLog)
		{
			*mLog << (store ? "  Store group: " :  "  Change group: ");
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " failed" << os_endl << std::flush;
		}
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkRecord::Playback_DeleteGroup(CAdbkAction& action)
{
	try
	{
		// Create temp remote object
		std::auto_ptr<CAddressBook> adbk(new CAddressBook(mPlayRemote, mPlayRemote->GetStoreRoot(), true, false, action.GetList().front()));

		// Create dummy groups for delete
		CGroupList grps;
		for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
		{
			grps.push_back(new CGroup(NULL, NULL));
			grps.back()->SetEntry(*iter);
		}

		// Delete on server
		mPlayRemote->RemoveGroup(adbk.get(), &grps);

		if (mLog)
		{
			*mLog << "  Remove group: ";
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
		{
			*mLog << "  Remove group: ";
			for(cdstrvect::const_iterator iter = action.GetList().begin() + 1; iter != action.GetList().end(); iter++)
			{
				if (iter != action.GetList().begin() + 1)
					*mLog << ", ";
				*mLog << *iter;
			}
			*mLog <<" in Addressbook: " << action.GetList().front() << " failed" << os_endl << std::flush;
		}
		
		CLOG_LOGRETHROW;
		throw;
	}
}

// Playback processing
void CAdbkRecord::CompactPlayback()
{
	// I don't know what to do here yet
}

#pragma mark ____________________________Stream Ops

#if __dest_os == __mac_os || __dest_os == __mac_os_x
OSType CAdbkRecord::GetMacFileType() const
{
	return cMailboxRecordType;
}
#endif

CRecordableAction* CAdbkRecord::NewAction() const
{
	return new CAdbkAction();
}
