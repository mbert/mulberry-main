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

#include "CMailRecord.h"

#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CINETClient.h"
#include "CLocalCommon.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CNetworkException.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CRFC822.h"
#include "CUnicodeStdLib.h"

#include <algorithm>
#include <memory>

CMailRecord::CMailRecord()
{
}

#pragma mark ____________________________Mailbox actions

void CMailRecord::Create(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eCreate))
		return;

	push_back(new CMailAction(CMailAction::eCreate, mCurrentID, mbox->GetName(), 0));
}

void CMailRecord::Delete(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eDelete))
		return;

	push_back(new CMailAction(CMailAction::eDelete, mCurrentID, mbox->GetName(), mbox->GetUIDValidity()));
}

void CMailRecord::Rename(const CMbox* mbox, const cdstring& newname)
{
	// Only if allowed
	if (!mRecord.IsSet(eRename))
		return;

	push_back(new CMailAction(CMailAction::eRename, mCurrentID, mbox->GetName(), newname, mbox->GetUIDValidity()));
}

void CMailRecord::Subscribe(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eSubscribe))
		return;

	push_back(new CMailAction(CMailAction::eSubscribe, mCurrentID, mbox->GetName(), mbox->GetUIDValidity()));
}

void CMailRecord::Unsubscribe(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eUnsubscribe))
		return;

	push_back(new CMailAction(CMailAction::eUnsubscribe, mCurrentID, mbox->GetName(), mbox->GetUIDValidity()));
}

void CMailRecord::Select(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eSelect))
		return;

	mPendingSelect.insert(ulcdstrlongmap::value_type(mCurrentID, cdstrlong(mbox->GetName(), mbox->GetUIDValidity())));
}

bool CMailRecord::PushSelect(bool add)
{
	// Find the current pending item
	ulcdstrlongmap::iterator found = mPendingSelect.find(mCurrentID);

	// Only if pending
	if (found == mPendingSelect.end())
		return false;

	// Only if allowed
	if (!mRecord.IsSet(eSelect))
		return true;

	// Add the action and remove the pending item
	if (add)
		push_back(new CMailAction(CMailAction::eSelect, mCurrentID, (*found).second.first, (*found).second.second));
	mPendingSelect.erase(found);
	return true;
}

void CMailRecord::Deselect(const CMbox* mbox)
{
	// Only if allowed
	if (!mRecord.IsSet(eDeselect))
		return;

	// Add deselect action, if no pending select
	if (!PushSelect(false))
		push_back(new CMailAction(CMailAction::eDeselect, mCurrentID, mbox->GetName(), mbox->GetUIDValidity()));
}

#pragma mark ____________________________Message actions

void CMailRecord::SetFlags(const ulvector& uids, const ulvector& luids, NMessage::EFlags flags, bool set)
{
	// Push any pending select
	PushSelect();

	// Only if allowed
	if (!mRecord.IsSet(eFlag))
		return;

	// Do some filtering:
	// Remove any zero UIDs from uids while deleting UIDs
	// in luids that correspond to non-zero UIDs in uids
	ulvector temp_uids = uids;
	ulvector temp_luids = luids;
	PurgeUIDs(temp_uids, temp_luids);

	push_back(new CMailAction(mCurrentID, temp_uids, temp_luids, flags, set));
}

void CMailRecord::CopyTo(const ulmap& uids, const ulmap& luids, const CMbox* mbox)
{
	// Push any pending select
	PushSelect();

	// Only if allowed
	if (!mRecord.IsSet(eCopy))
		return;

	push_back(new CMailAction(CMailAction::eCopy, mCurrentID, mbox->GetName(), uids, luids));
}

void CMailRecord::AppendTo(unsigned long uid, NMessage::EFlags flags, const CMbox* mbox)
{
	// Push any pending select if to the pending mailbox
	PushSelect();

	// Only if allowed
	if (!mRecord.IsSet(eAppend))
		return;

	push_back(new CMailAction(CMailAction::eAppend, mCurrentID, mbox->GetName(), uid, flags));
}

void CMailRecord::Expunge(const ulvector& uids, const ulvector& luids)
{
	// Push any pending select
	PushSelect();

	// Only if allowed
	if (!mRecord.IsSet(eExpunge))
		return;

	// Do some filtering:
	// Remove any zero UIDs from uids while deleting UIDs
	// in luids that correspond to non-zero UIDs in uids
	ulvector temp_uids = uids;
	ulvector temp_luids = luids;
	PurgeUIDs(temp_uids, temp_luids);

	push_back(new CMailAction(CMailAction::eExpunge, mCurrentID, temp_uids, temp_luids));
}

#pragma mark ____________________________Tests

// Look for a UID that's been expunged
bool CMailRecord::ExpungedUID(unsigned long uid, bool local) const
{
	// Look for an expunge action containing this uid
	for(const_iterator iter = begin(); iter != end(); iter++)
	{
		CMailAction* action = static_cast<CMailAction*>(*iter);
		if (action->GetAction() == CMailAction::eExpunge)
		{
			// Look for matching uid
			const ulvector& uids = local ? action->GetExpungeAction().second : action->GetExpungeAction().first;
			if (std::find(uids.begin(), uids.end(), uid) != uids.end())
				return true;
		}
	}

	return false;
}

// Remove matching actions
void CMailRecord::Flush(CMailAction::EMailAction actions)
{
	// Look for a matching action
	for(iterator iter = begin(); iter != end();)
	{
		CMailAction* action = static_cast<CMailAction*>(*iter);
		if (action->GetAction() & actions)
		{
			size_type n = iter - begin();
			erase(iter);
			iter = begin() + n;
		}
		else
			iter++;
	}

	// Now write out entire record to disk
	bool was_open = mStream.is_open();
	if (was_open)
		mStream.close();
	mStream.open(mDescriptor, std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
	mStream.seekp(sizeof(unsigned long), std::ios_base::beg);
	WriteItemsToStream(mStream);
	mStartRecord = size();
	if (!was_open)
		mStream.close();
}

#pragma mark ____________________________Playback

// Playback processing
bool CMailRecord::Playback(CMboxProtocol* remote, CMboxProtocol* local, CProgress* progress)
{
	// Check for forced logging
	bool log_created = false;
	std::auto_ptr<cdofstream> fout;
	if (!mLog && CLog::AllowPlaybackLog())
	{
		cdstring temp_name = mDescriptor + ".log";
		fout.reset(new cdofstream(temp_name, std::ios_base::app | std::ios_base::binary));
		SetLog(fout.get());
		log_created = true;
	}

	bool result = true;

	try
	{
		if (progress)
			progress->Reset();

		if (mLog)
		{
			*mLog << os_endl << os_endl << "----" << os_endl;
			*mLog << "Starting Mailbox Playback: " << CRFC822::GetRFC822Date() << os_endl << std::flush;
		}

		// Read in the entire journal
		Open();
		if (mLog)
			*mLog << "Opened Playback log: " << mDescriptor << os_endl << std::flush;

		// Compact the playback to minimise operations
		CompactPlayback();

		if (size())
		{
			// Set progress totol
			if (progress)
				progress->SetTotal(size());

			// Open main connection
			remote->Open();
			remote->Logon();

			if (mLog)
				*mLog << "Opened remote server: " << remote->GetAccountName() << os_endl << std::flush;
		}
		else
		{
			// Set progress done
			if (progress)
			{
				progress->SetTotal(1);
				progress->BumpCount();
			}
		}

		// Cache useful items
		mPlayRemote = remote;
		mPlayLocal = local;

		// Connection to reuse - cloned from existing
		mPlayReuse = static_cast<CMboxProtocol*>(remote->CloneConnection());

		// Now run the play back
		unsigned long attempts = 0;
		while(size())
		{
			// Play the top item
			bool done = true;
			PlaybackItem(*static_cast<CMailAction*>(*begin()), done);

			if (done)
			{
				// Erase top element if done
				erase(begin());
				attempts = 0;
				
				// Bump progress
				if (progress)
					progress->BumpCount();
			}
			else
			{
				// Bump up action repeat count
				attempts++;
				
				// Check for too many attempts
				if (attempts > 1)
				{
					if (mLog)
						*mLog << "  Too many errors while attempting action: " << os_endl << std::flush;

					CLOG_LOGTHROW(CGeneralException, eException_TooManyAttempts);
					throw CGeneralException(eException_TooManyAttempts);
				}
			}
		}
		
		// Done with reusable protocol
		delete mPlayReuse;
		mPlayReuse = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Recovering from Error: " << os_endl << std::flush;

		// Recover currently selected items
		Playback_RecoverSelects();

		// Playback failed for some reason
		result = false;

		// Set progress to total
		if (progress)
			progress->SetCount(0xFFFFFFFF);
	}

	// Close remote connection
	if (remote->IsOpen() && mLog)
		*mLog << "Closed remote server: " << remote->GetAccountName() << os_endl << std::flush;
	remote->Close();

	// If no failure clear out the file
	if (result)
	{
		// Clear it out
		clear();
		mNextID = 1;

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

	// Always remove any pending selects
	mPendingSelect.clear();

	return result;
}

void CMailRecord::PlaybackItem(CMailAction& action, bool& done)
{
	try
	{
		switch(action.GetAction())
		{

		// Mailbox actions
		case CMailAction::eCreate:
			Playback_Create(action);
			break;

		case CMailAction::eDelete:
			Playback_Delete(action);
			break;

		case CMailAction::eRename:
			Playback_Rename(action);
			break;

		case CMailAction::eSubscribe:
			Playback_Subscribe(action);
			break;

		case CMailAction::eUnsubscribe:
			Playback_Unsubscribe(action);
			break;

		// Mailbox connection
		case CMailAction::eSelect:
			Playback_Select(action);
			break;

		case CMailAction::eDeselect:
			Playback_Deselect(action);
			break;

		// Message actions
		case CMailAction::eFlag:
			Playback_SetFlags(action);
			break;

		case CMailAction::eCopy:
			Playback_CopyTo(action);
			break;

		case CMailAction::eAppend:
			Playback_AppendTo(action);
			break;

		case CMailAction::eExpunge:
			Playback_Expunge(action);
			break;

		default:;
		}
	}
	
	// Look for special types of exceptions
	catch (CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Server response error - non-fatal but must clean up
		if ((ex.error() == CINETClient::CINETException::err_NoResponse) ||
			(ex.error() == CINETClient::CINETException::err_BadResponse))
		{
			// Clean out any EXPUNGES pending on the mailbox which generated the error
			Playback_RecoverNonfatal(action);

			if (mLog)
				*mLog << "  Recovering from non-fatal error: removed pending EXPUNGE's" << os_endl << std::flush;
		}

		// See if connection remained up
		else if (ex.disconnected())
		{
			if (mLog)
				*mLog << "  Recovering from connection loss error" << os_endl << std::flush;

			// Throw up - connection went down entirely
			CLOG_LOGRETHROW;
			throw;
		}
		else
		{
			// Redo processing for failed item
			done = false;

			if (mLog)
				*mLog << "  Recovered from non-fatal error" << os_endl << std::flush;
		}
	}
	
	// Look for special errors
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// See if connection remained up
		if (ex.GetErrorCode() == eException_FailedSafe)
		{
			// Clean out any EXPUNGES pending on the mailbox which generated the error
			Playback_RecoverNonfatal(action);

			// Just continue processing
			if (mLog)
				*mLog << "  Recovering from non-fatal error: removed pending EXPUNGE's" << os_endl << std::flush;
		}
		else
		{
			if (mLog)
				*mLog << "  Recovering from fatal error" << os_endl << std::flush;

			// Throw up - connection went down entirely
			CLOG_LOGRETHROW;
			throw;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Recovering from fatal error" << os_endl << std::flush;

		// This is a terminal error - just throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Create(CMailAction& action)
{
	try
	{
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		std::auto_ptr<CMbox> mbox(new CMbox(mPlayRemote, action.GetNameUIDAction().first, dir_delim, NULL));
		mbox->Create();

		// Need to get UIDValidity and sync with local
		{
			mbox->Check();
			CMbox* local = mPlayLocal->FindMbox(action.GetNameUIDAction().first, true);
			
			// Only if local still exists
			if (local)
			{
				local->SetFlags(NMbox::eSynchronising);
				local->Open(NULL, false, false, false);
				local->ChangeUIDValidity(mbox->GetUIDValidity());
				local->CloseSilent();
			}
		}

		if (mLog)
			*mLog << "  Create Mailbox: " << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Create Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Delete(CMailAction& action)
{
	try
	{
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		std::auto_ptr<CMbox> mbox(new CMbox(mPlayRemote, action.GetNameUIDAction().first, dir_delim, NULL));

		// Must check UIDValidity before delete
		mbox->Check();
		bool uidvalidity_override = false;
		if (action.GetNameUIDAction().second && (mbox->GetUIDValidity() != action.GetNameUIDAction().second))
		{
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Playback::UIDVALIDITY_DELETE", mbox->GetName()) == CErrorHandler::Cancel)
			{
				// This deletes the object so release the auto_ptr
				mbox->Remove();
				mbox.release();
				if (mLog)
					*mLog << "* ERROR: Delete Mailbox: " << action.GetNameUIDAction().first << " - UIDValidity does not match" << os_endl << std::flush;
				CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
				throw CGeneralException(eException_FailedSafe);
			}
			uidvalidity_override = true;
		}

		// This delete will NOT delete mbox object
		mbox->Delete();

		if (mLog)
		{
			*mLog << (uidvalidity_override ? "  Delete Mailbox (UIDValidity overridden): " : "  Delete Mailbox: ");
			*mLog << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Delete Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Rename(CMailAction& action)
{
	try
	{
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		std::auto_ptr<CMbox> mbox(new CMbox(mPlayRemote, action.GetRenameAction().first.first, dir_delim, NULL));

		// Must check UIDValidity before delete
		mbox->Check();
		bool uidvalidity_override = false;
		if (action.GetRenameAction().second && (mbox->GetUIDValidity() != action.GetRenameAction().second))
		{
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Playback::UIDVALIDITY_RENAME", mbox->GetName()) == CErrorHandler::Cancel) 
			{
				// This deletes the object so release the auto_ptr
				mbox->Remove();
				mbox.release();
				if (mLog)
					*mLog << "* ERROR: Rename Mailbox: " << action.GetRenameAction().first.first << " - UIDValidity does not match" << os_endl << std::flush;
				CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
				throw CGeneralException(eException_FailedSafe);
			}
			uidvalidity_override = true;
		}

		// Do rename
		mbox->Rename(action.GetRenameAction().first.second);

		if (mLog)
		{
			*mLog << (uidvalidity_override ? "  Rename Mailbox from (UIDValidity overridden): " : "  Rename Mailbox from: ");
			*mLog << action.GetRenameAction().first.first << " to: " <<
					 action.GetRenameAction().first.second << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Rename Mailbox from: " << action.GetRenameAction().first.first <<
					" to: " << action.GetRenameAction().first.second << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Subscribe(CMailAction& action)
{
	try
	{
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		std::auto_ptr<CMbox> mbox(new CMbox(mPlayRemote, action.GetNameUIDAction().first, dir_delim, NULL));

		// Must check UIDValidity before subscribe
		mbox->Check();
		bool uidvalidity_override = false;
		if (action.GetNameUIDAction().second && (mbox->GetUIDValidity() != action.GetNameUIDAction().second))
		{
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Playback::UIDVALIDITY_SUBSCRIBE", mbox->GetName()) == CErrorHandler::Cancel)
			{
				// This deletes the object so release the auto_ptr
				mbox->Remove();
				mbox.release();
				if (mLog)
					*mLog << "* ERROR: Subscribe to Mailbox: " << action.GetNameUIDAction().first << " - UIDValidity does not match" << os_endl << std::flush;
				CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
				throw CGeneralException(eException_FailedSafe);
			}
			uidvalidity_override = true;
		}

		mbox->Subscribe(false);			// No update of UI on subscribe

		if (mLog)
		{
			*mLog << (uidvalidity_override ? "  Subscribe to Mailbox (UIDValidity overridden): " : "  Subscribe to Mailbox: ");
			*mLog << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Subscribe to Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Unsubscribe(CMailAction& action)
{
	try
	{
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		std::auto_ptr<CMbox> mbox(new CMbox(mPlayRemote, action.GetNameUIDAction().first, dir_delim, NULL));

		// Must check UIDValidity before unsubscribe
		mbox->Check();
		bool uidvalidity_override = false;
		if (action.GetNameUIDAction().second && (mbox->GetUIDValidity() != action.GetNameUIDAction().second))
		{
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Playback::UIDVALIDITY_UNSUBSCRIBE", mbox->GetName()) == CErrorHandler::Cancel)
			{
				// This deletes the object so release the auto_ptr
				mbox->Remove();
				mbox.release();
				if (mLog)
					*mLog << "* ERROR: Unsubscribe from Mailbox: " << action.GetNameUIDAction().first << " - UIDValidity does not match" << os_endl << std::flush;
				CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
				throw CGeneralException(eException_FailedSafe);
			}
			uidvalidity_override = true;
		}


		mbox->Unsubscribe(false);		// No update of UI on unsubscribe

		if (mLog)
		{
			*mLog << (uidvalidity_override ? "  Unsubscribe from Mailbox (UIDValidity overridden): " : "  Unsubscribe from Mailbox: ");
			*mLog << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Unsubscribe from Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Select(CMailAction& action)
{
	try
	{
		// Create remote mailbox object
		char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
		CMbox* rsource = new CMbox(mPlayRemote, action.GetNameUIDAction().first, dir_delim, NULL);

		// Create local mailbox object
		// NB Must handle the case of a missing local mailbox
		CMbox* lsource = mPlayLocal->FindMbox(action.GetNameUIDAction().first, true);

		//  Add to ID map
		mConns.insert(CConnectionList::value_type(action.GetID(), std::pair<CMbox*, CMbox*>(rsource, lsource)));

		// Open for sync (reuse connection if available)
		rsource->SetFlags(NMbox::eSynchronising);
		
		// Open it - this might fail for varous reasons - be prepared!
		try
		{
			if (mPlayReuse->GetCurrentMbox())
				rsource->Open(NULL, false);
			else
				rsource->Open(mPlayReuse, false);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Failed during select - must remove this from connection list
			mConns.erase(action.GetID());

			// Remove will NOT delete mbox object - do it manually
			delete rsource;

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}

		// Need to sync UIDValidities
		{
			// NB Must handle the case of a missing local mailbox
			if (lsource)
			{
				lsource->SetFlags(NMbox::eSynchronising);
				lsource->Open(NULL, false);
			}

			// Now check UIDValidities
			if (lsource && !lsource->GetUIDValidity())
			{
				// Reset the UIDValidity for touched mailbox
				lsource->ChangeUIDValidity(rsource->GetUIDValidity());

				if (mLog)
					*mLog << "  Select Mailbox (new local): " << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
			}
			else if (!lsource || (lsource->GetUIDValidity() != rsource->GetUIDValidity()))
			{
				// ERROR: UIDValidity does not match or missing local mailbox
				if (CPreferences::sPrefs->mUIDValidityMismatch.GetValue())
				{
					if (lsource)
						CErrorHandler::PutStopAlertRsrcStr("Alerts::Playback::UIDVALIDITY_SELECT", lsource->GetName());
					else
						CErrorHandler::PutStopAlertRsrcStr("Alerts::Playback::LOCALMAILBOX_MISSING", rsource->GetName());
				}

				// Reset UIDValidity before deselecting
				if (lsource)
					lsource->ChangeUIDValidity(rsource->GetUIDValidity());

				// Force removal of connection
				//Playback_Deselect(action);	<- now done in Playback_RemoveActions if required

				// Must remove playback actions on this mailbox
				Playback_RemoveActions(action.GetNameUIDAction().first, lsource);

				if (mLog)
				{
					*mLog << "* ERROR: Select Mailbox: " << action.GetNameUIDAction().first;
					if (lsource)
						*mLog << " - UIDValidity does not match" << os_endl << std::flush;
					else
						*mLog << " - local mailbox missing" << os_endl;
					*mLog << "  Select Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
				}
			}
			else
			{
				if (mLog)
					*mLog << "  Select Mailbox: " << action.GetNameUIDAction().first << " succeeded" << os_endl << std::flush;
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Select Mailbox: " << action.GetNameUIDAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Deselect(CMailAction& action)
{
	cdstring name;
	CMbox* rsource = NULL;
	CMbox* lsource = NULL;
	try
	{
		// Look for matching connection
		CConnectionList::const_iterator conn = mConns.find(action.GetID());
		if (conn == mConns.end())
		{
			CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
			throw CGeneralException(eException_FailedSafe);
		}

		// Get mailbox objects and pull from list
		rsource = (*conn).second.first;
		lsource = (*conn).second.second;
		name = rsource->GetName();
		mConns.erase(action.GetID());

		// Close (don't kill reuse if first)
		if (rsource->GetMsgProtocol() == mPlayReuse)
		{
			CMboxProtocol* reuse = NULL;
			rsource->CloseMboxOnly(&reuse, true);
		}
		else
			rsource->CloseSilent();

		// Remove will NOT delete mbox object - do it manually
		delete rsource;
		//rsource->Remove();

		if (lsource)
			lsource->CloseSilent();

		if (mLog)
			*mLog << "  Deselect Mailbox: " << name << " succeeded" << os_endl << std::flush;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Always close local mailbox
		if (lsource)
			lsource->CloseSilent();

		if (mLog)
			*mLog << "  Deselect Mailbox: " << name << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}	
}

void CMailRecord::Playback_SetFlags(CMailAction& action)
{
	cdstring name;
	try
	{
		// Look for matching connection
		CConnectionList::const_iterator conn = mConns.find(action.GetID());
		if (conn == mConns.end())
		{
			CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
			throw CGeneralException(eException_FailedSafe);
		}

		// Only do if uids exist (may have been compacted to zero)
		if (action.GetFlagAction().mUids.first.size() || action.GetFlagAction().mUids.second.size())
		{
			// Get mailbox object
			CMbox* rsource = (*conn).second.first;
			CMbox* lsource = (*conn).second.second;
			name = rsource->GetName();

			// Get all non-zero server UIDs - remapping from local
			ulvector uids = action.GetFlagAction().mUids.first;
			RemapUIDs(lsource, uids, action.GetFlagAction().mUids.second);

			// Set remote flags
			if (uids.size())
			{
				rsource->SetFlagMessage(uids, true,
									static_cast<NMessage::EFlags>(action.GetFlagAction().mFlags),
									action.GetFlagAction().mSet);

				if (mLog)
				{
					*mLog << "  Set flags Mailbox: " << name << " succeeded" << os_endl;
					if (action.GetFlagAction().mSet)
						*mLog << "    Setting flags: ";
					else
						*mLog << "    Removing flags:";
					if (action.GetFlagAction().mFlags & NMessage::eAnswered)
						*mLog << " \\Answered";
					if (action.GetFlagAction().mFlags & NMessage::eFlagged)
						*mLog << " \\Flagged";
					if (action.GetFlagAction().mFlags & NMessage::eDeleted)
						*mLog << " \\Deleted";
					if (action.GetFlagAction().mFlags & NMessage::eSeen)
						*mLog << " \\Seen";
					if (action.GetFlagAction().mFlags & NMessage::eDraft)
						*mLog << " \\Draft";
					if (action.GetFlagAction().mFlags & NMessage::eMDNSent)
						*mLog << " $MDNSent";
					*mLog << os_endl;

					*mLog << "    Set flags UIDs: ";
					for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
					{
						if (iter != uids.begin())
							*mLog << ", ";
						*mLog << *iter;
					}
					*mLog << os_endl << std::flush;
				}
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Set flags Mailbox: " << name << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_CopyTo(CMailAction& action)
{
	cdstring name;
	try
	{
		// Look for matching connection
		CConnectionList::const_iterator conn = mConns.find(action.GetID());
		if (conn == mConns.end())
		{
			CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
			throw CGeneralException(eException_FailedSafe);
		}

		// Get mailbox object
		CMbox* rsource = (*conn).second.first;
		CMbox* lsource = (*conn).second.second;
		name = rsource->GetName();

		// Merge server UIDs and local UIDs
		ulmap suids = action.GetCopyAction().second.first;
		ulmap luids;
		for(ulmap::const_iterator iter = action.GetCopyAction().second.second.begin();
			iter != action.GetCopyAction().second.second.end(); iter++)
		{
			// See if local UID now has a remote UID
			if (lsource)
			{
				ulvector temp;
				temp.push_back((*iter).first);
				lsource->MapLocalUIDs(temp);
				if (temp.size())
					suids.insert(ulmap::value_type(temp.front(), (*iter).second));
				else
					luids.insert(*iter); 
			}
			else
				luids.insert(*iter);
		}

		// Do server UID copy
		if (suids.size())
		{
			// Get source server uids
			ulvector uids;
			for(ulmap::const_iterator iter = suids.begin(); iter != suids.end(); iter++)
				uids.push_back((*iter).first);

			// Copy all available via server
			Playback_CopyToFull(action, uids, suids);

			// Do copy log now
			if (mLog)
			{
				*mLog << "  Copy from Mailbox: " << name << 
						" to Mailbox: " << action.GetCopyAction().first << " succeeded" << os_endl;
				*mLog << "    Copy messages UIDs: ";
				for(ulmap::const_iterator iter = suids.begin(); iter != suids.end(); iter++)
				{
					if (iter != suids.begin())
						*mLog << ", ";
					*mLog << (*iter).first;
				}
				*mLog << os_endl << std::flush;
			}
		}

		// Look for local messages that are not on the server (why?)
		if (luids.size())
		{
			// Do copy-append log now
			if (mLog)
			{
				*mLog << "  Copy-append from Mailbox: " << name << 
						" to Mailbox: " << action.GetCopyAction().first << " starting" << os_endl;
			}

			// Find each one that's missing
			for(ulmap::const_iterator iter = luids.begin(); iter != luids.end(); iter++)
				Playback_CopyToPartial(action, (*iter).second);

			// Do copy-append log now
			if (mLog)
			{
				*mLog << "  Copy-append from Mailbox: " << name << 
						" to Mailbox: " << action.GetCopyAction().first << " succeeded" << os_endl;
			}

		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Copy from Mailbox: " << name << 
					" to Mailbox: " << action.GetCopyAction().first << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_CopyToFull(CMailAction& action, ulvector& uids, const ulmap& suids)
{
	// Get mailbox object
	CMbox* rsource = (*mConns.find(action.GetID())).second.first;

	// Get remote destination
	char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
	CMbox* rdest = new CMbox(mPlayRemote, action.GetCopyAction().first, dir_delim, NULL);

	// Set flags
	ulmap copy_map;
	try
	{
		rsource->CopyMessage(uids, true, rdest, copy_map);

		// See if no UIDPLUS support
		if (copy_map.empty())
			DescoverUIDs(uids, copy_map, rsource, rdest);
		
		// Must remap UIDs in local destination
		CMbox* ldest = mPlayLocal->FindMbox(action.GetCopyAction().first, true);
		if (ldest)
		{
			ldest->SetFlags(NMbox::eSynchronising);
			ldest->Open(NULL, false);

			for(ulmap::const_iterator iter = copy_map.begin(); iter != copy_map.end(); iter++)
			{
				unsigned long src_uid = (*iter).first;
				unsigned long dest_uid = (*iter).second;
				
				// Lookup local uid of source message
				ulmap::const_iterator found1 = suids.find(src_uid);
				if (found1 != suids.end())
				{
					unsigned long dest_local_uid = (*found1).second;
					ldest->RemapUID(dest_local_uid, dest_uid);
				}
			}
			
			ldest->CloseSilent();
		}

		// Remove once done
		rdest->Remove();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		rdest->Remove();
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_CopyToPartial(CMailAction& action, unsigned long local_uid)
{
	// Now fake an append
	CMailAction temp(CMailAction::eAppend, 0, action.GetCopyAction().first, local_uid);
	Playback_AppendTo(temp);
}

void CMailRecord::Playback_AppendTo(CMailAction& action)
{
	cdstring name;
	try
	{
		// Get mailbox object
		CMbox* rsource = NULL;
		CMbox* lsource = NULL;
		bool remove_rsource = false;
		bool open_lsource = false;

		// See if existing connection in place
		if (mConns.count(action.GetID()))
		{
			rsource = (*mConns.find(action.GetID())).second.first;
			lsource = (*mConns.find(action.GetID())).second.second;
		}
		else
		{
			// Create temp mailbox
			char dir_delim = mPlayRemote->GetMailAccount()->GetDirDelim();
			rsource = new CMbox(mPlayRemote, action.GetAppendAction().mName, dir_delim, NULL);
			remove_rsource = true;

			// Source must be open
			lsource = mPlayLocal->FindMbox(action.GetAppendAction().mName, true);
			open_lsource = true;
		}
		name = rsource->GetName();

		// Source must exist
		if (lsource)
		{
			if (open_lsource)
			{
				lsource->SetFlags(NMbox::eSynchronising);
				lsource->Open(NULL, false);
			}
			ulvector seq_all;
			seq_all.push_back(0);
			lsource->GetMsgProtocol()->FetchItems(seq_all, false, CMboxProtocol::eUID);

			// Get local message from source and cache it
			unsigned long new_uid = 0;
			CMessage* msg = lsource->GetMessageLocalUID(action.GetAppendAction().mUid);

			// May have already been expunged
			if (msg)
			{
				// Make sure we have message cache data
				msg->CacheMessage();
				msg->ReadCache();
				
				// Force flags to what they were at the time of the expunge
				// without actually changing them in the mailbox
				msg->GetFlags().Set(NMessage::eServerFlags, false);
				msg->GetFlags().Set(action.GetAppendAction().mFlags & NMessage::eServerFlags);

				// Now do append
				try
				{
					rsource->AppendMessage(msg, new_uid, false, false);

					// See if no UIDPLUS support
					if (!new_uid)
						new_uid = DescoverUID(msg, rsource);

					// Do UID map in local
					if (new_uid)
						lsource->RemapUID(action.GetAppendAction().mUid, new_uid);
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Remove if required
					if (remove_rsource)
						rsource->Remove();

					// Done with local mailbox
					if (open_lsource)
						lsource->CloseSilent();

					CLOG_LOGRETHROW;
					throw;
				}
			}
			

			// Remove if required
			if (remove_rsource)
				rsource->Remove();

			// Done with local mailbox
			if (open_lsource)
				lsource->CloseSilent();

			if (mLog)
			{
				if (msg)
				{
					*mLog << "  Append to Mailbox: " << name << " succeeded" << os_endl;
					*mLog << "    Append message new UID: " << new_uid << os_endl;
					*mLog << "    Append with flags: ";
					if (action.GetAppendAction().mFlags & NMessage::eAnswered)
						*mLog << " \\Answered";
					if (action.GetAppendAction().mFlags & NMessage::eFlagged)
						*mLog << " \\Flagged";
					if (action.GetAppendAction().mFlags & NMessage::eDeleted)
						*mLog << " \\Deleted";
					if (action.GetAppendAction().mFlags & NMessage::eSeen)
						*mLog << " \\Seen";
					if (action.GetAppendAction().mFlags & NMessage::eDraft)
						*mLog << " \\Draft";
					if (action.GetAppendAction().mFlags & NMessage::eMDNSent)
						*mLog << " $MDNSent";
					*mLog << os_endl;
				}
				else
					*mLog << "  Append to Mailbox: " << name << " ignored - missing local message" << os_endl;
				*mLog  << std::flush;
			}
		}
		else
		{
			if (mLog)
				*mLog << "  Append to Mailbox: " << name << " ignored - missing local mailbox" << os_endl << std::flush;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Append to Mailbox: " << name << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

void CMailRecord::Playback_Expunge(CMailAction& action)
{
	cdstring name;
	try
	{
		// Look for matching connection
		CConnectionList::const_iterator conn = mConns.find(action.GetID());
		if (conn == mConns.end())
		{
			CLOG_LOGTHROW(CGeneralException, eException_FailedSafe);
			throw CGeneralException(eException_FailedSafe);
		}

		// Only do if uids exist (may have been compacted to zero)
		if (action.GetExpungeAction().first.size() || action.GetExpungeAction().second.size())
		{
			// Get mailbox object
			CMbox* rsource = (*conn).second.first;
			CMbox* lsource = (*conn).second.second;
			name = rsource->GetName();

			// Get all non-zero server UIDs - remapping from local
			ulvector uids = action.GetExpungeAction().first;
			RemapUIDs(lsource, uids, action.GetExpungeAction().second);

			// Expunge (if allowed by ACL)
			if (uids.size() && !rsource->IsReadOnly())
			{
				// Do UID expunge
				rsource->ExpungeMessage(uids, true);
			}

			if (mLog)
			{
				if (rsource->IsReadOnly())
				{
					*mLog << "* ERROR: Expunge Mailbox: " << name << " - read only" << os_endl;
					*mLog << "  Expunge Mailbox: " << name << " failed" << os_endl;
				}
				else
				{
					*mLog << "  Expunge Mailbox: " << name << " succeeded" << os_endl;
					if (rsource->GetProtocol()->DoesExpungeMessage())
						*mLog << "    Expunging UIDs: ";
					else
						*mLog << "    Expunging UIDs via sequence numbers: ";
					for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
					{
						if (iter != uids.begin())
							*mLog << ", ";
						*mLog << *iter;
					}
					*mLog << os_endl;
				}
				*mLog << std::flush;
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mLog)
			*mLog << "  Expunge Mailbox: " << name << " failed" << os_endl << std::flush;
		
		CLOG_LOGRETHROW;
		throw;
	}
}

// Recover outstanding selects
void CMailRecord::Playback_RecoverSelects()
{
	// Recover open connections
	for(CConnectionList::iterator iter1 = mConns.begin(); iter1 != mConns.end(); iter1++)
	{
		// Get mailbox objects and pull from list
		unsigned long id = (*iter1).first;
		CMbox* rsource = (*iter1).second.first;
		CMbox* lsource = (*iter1).second.second;
		cdstring name = rsource->GetName();

		// Look for a suitable place to add this item back in
		for(iterator iter2 = begin(); iter2 != end(); iter2++)
		{
			// Check for matching action
			CMailAction* action1 = static_cast<CMailAction*>(*iter2);
			if (id == action1->GetID())
			{
				// Double-check its not a select
				if (action1->GetAction() != CMailAction::eSelect)
				{
					// Add a new action at this point
					unsigned long pos = iter2 - begin();
					insert(iter2, new CMailAction(CMailAction::eSelect, id, rsource->GetName(), rsource->GetUIDValidity()));
					iter2 = begin() + pos;
					break;
				}
				else
					break;
			}
		}

		// See if rsource connection still in place
		if (rsource->GetMsgProtocol() && rsource->GetMsgProtocol()->IsLoggedOn())
		{
			rsource->CloseSilent();

			// Remove will NOT delete mbox object - do it manually
			delete rsource;
		}
		else
			// Just remove the mailbox - connection already dead
			rsource->Remove();
			
		if (lsource)
			lsource->CloseSilent();

		if (mLog)
			*mLog << "    Recover Mailbox Select: " << name << " succeeded" << os_endl << std::flush;
	}
	
	// Clear out all connections
	mConns.clear();
}

// Recover from non-fatal error
void CMailRecord::Playback_RecoverNonfatal(CMailAction& action)
{

	switch(action.GetAction())
	{
	// Message actions
	case CMailAction::eFlag:
	case CMailAction::eCopy:
	case CMailAction::eAppend:
	case CMailAction::eExpunge:
		{
			// Look for matching connection
			CConnectionList::const_iterator conn = mConns.find(action.GetID());
			
			// Must fail hard if connection is missing
			if (conn == mConns.end())
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Get remote mailbox
			CMbox* rsource = (*conn).second.first;

			// Remove all EXPUNGE's for this item
			Playback_RemoveExpungeActions(rsource->GetName(), action.GetID());
		}
		break;
	default:;
	}
}

// Remove all actions with the corresponding name
void CMailRecord::Playback_RemoveActions(const cdstring& name, CMbox* mbox)
{
	// List of messages appended that should not be erased
	ulvector appended_uids;

	for(unsigned long i = 0; i < size(); i++)
	{
		CMailAction* action = static_cast<CMailAction*>(at(i));

		switch(action->GetAction())
		{

		// Mailbox actions
		case CMailAction::eCreate:
		case CMailAction::eDelete:
		case CMailAction::eSubscribe:
		case CMailAction::eUnsubscribe:
			// Check for matching names
			if (action->GetNameUIDAction().first == name)
			{
				// Pull it
				if (i)
				{
					erase(begin() + i);
					i--;
				}
			}
			break;

		case CMailAction::eRename:
			// Check for matching names
			if (action->GetRenameAction().first.first == name)
			{
				// Pull it
				if (i)
				{
					erase(begin() + i);
					i--;
				}
			}
			break;

		// Mailbox connection
		case CMailAction::eSelect:
			// Check for matching names
			if (action->GetNameUIDAction().first == name)
			{
				// Get its ID
				unsigned long id = action->GetID();
				bool remaining = false;

				// Remove non-appends with this ID
				for(unsigned long j = i + 1; j < size(); j++)
				{
					CMailAction* erase_action = static_cast<CMailAction*>(at(j));

					// Check for matching ID
					if (erase_action->GetID() == id)
					{
						bool do_erase = true;
						switch(erase_action->GetAction())
						{
							case CMailAction::eDeselect:
								// Only remove if no appends during this session and its not the very first (current) select
								do_erase = !remaining && i;
								break;

							// Message actions that must be removed
							case CMailAction::eFlag:
							case CMailAction::eCopy:
							case CMailAction::eExpunge:
								do_erase = erase_action->RemoveUIDs(appended_uids);
								break;

							case CMailAction::eAppend:
								do_erase = false;
								appended_uids.push_back(erase_action->GetNameUIDAction().second);
								break;

							default:;
						}

						// Remove action if required
						if (do_erase)
						{
							erase(begin() + j);
							j--;
						}
						else
							remaining = true;
					}
				}

				// Pull it if no appends pending
				if (!remaining && i)
				{
					erase(begin() + i);
					i--;
				}
			}
			break;

		case CMailAction::eCopy:
			// Look for copy into this mailbox
			if (action->GetCopyAction().first == name)
			{
				// Copy all the destination local UIDs
				for(ulmap::const_iterator iter = action->GetCopyAction().second.first.begin();
						iter != action->GetCopyAction().second.first.end(); iter++)
					appended_uids.push_back((*iter).second);
				for(ulmap::const_iterator iter = action->GetCopyAction().second.second.begin();
						iter != action->GetCopyAction().second.second.end(); iter++)
					appended_uids.push_back((*iter).second);
			}
			break;
		
		case CMailAction::eAppend:
			// Look for append into this mailbox
			if (action->GetNameUIDAction().first == name)
				// Add all appended messages
				appended_uids.push_back(action->GetNameUIDAction().second);
			break;

		default:;
		}
	}

	// Expunge all messages except ones appended while disconnected
	if (mbox && mbox->GetNumberFound())
	{
		if (!mbox->IsFullOpen())
		{
			mbox->CloseSilent();
			mbox->SetFlags(NMbox::eSynchronising);
			mbox->Open(NULL, false);
		}

		// Convert local UIDs to remote
		// Get all message UIDs in local mailbox
		ulvector seq_all;
		seq_all.push_back(0);
		if (mbox->GetNumberFound())
			mbox->GetMsgProtocol()->FetchItems(seq_all, false, CMboxProtocol::eUID);

		ulvector uid_all_nonzero;
		for(unsigned long i = 1; i <= mbox->GetNumberFound(); i++)
		{
			if (mbox->GetMessage(i)->GetUID())
				uid_all_nonzero.push_back(mbox->GetMessage(i)->GetUID());
		}

		// Subtract UIDs of messages appended to the mailbox
		//ulvector uid_remove;
		//std::set_difference(uid_all.begin(), uid_all.end(), appended_uids.begin(), appended_uids.end(), back_inserter<ulvector>(uid_remove));

		// Remove messages that were originally in the mailbox before recording began
		if (uid_all_nonzero.size())
			mbox->ExpungeMessage(uid_all_nonzero, true);
	}

}

// Remove all EXPUNGE actions with the corresponding name or id
void CMailRecord::Playback_RemoveExpungeActions(const cdstring& name, unsigned long id)
{
	// Don't look at the current item
	for(unsigned long i = 1; i < size(); i++)
	{
		CMailAction* action = static_cast<CMailAction*>(at(i));

		switch(action->GetAction())
		{
		// Mailbox connection
		case CMailAction::eSelect:
			// Check for matching names
			if (action->GetNameUIDAction().first == name)
			{
				// Get its ID
				unsigned long erase_id = action->GetID();

				// Remove non-appends with this ID
				for(unsigned long j = i + 1; j < size(); j++)
				{
					CMailAction* erase_action = static_cast<CMailAction*>(at(j));

					// Check for matching ID
					if ((erase_action->GetID() == erase_id) &&
						(erase_action->GetAction() == CMailAction::eExpunge))
					{
						erase(begin() + j);
						j--;
					}
				}
			}
			break;

		case CMailAction::eExpunge:
			// Look for matching EXPUNGE and erase
			if (action->GetID() == id)
			{
				erase(begin() + i);
				i--;
			}
			break;
		
		default:;
		}
	}
}

#pragma mark ____________________________Compacting

// Playback processing
void CMailRecord::CompactPlayback()
{
	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "0.txt");
		WriteItemsToStream(fout, true);
	}

	// Optimisations to apply:
	
	// look for missing selects
	CompactMissing();

	// look for unterminated selects
	CompactUnterminated();

	// merge multiple selects into one
	CompactMultiple();

	// use single expunge
	CompactExpunge();

	// merge similar flag changes
	CompactFlags();

	if (mLog)
		*mLog << "  Compacting Playback log." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "6.txt");
		WriteItemsToStream(fout, true);
	}
}

// Look for missing selects
void CMailRecord::CompactMissing()
{
	// Look for missing selects
	ulset selects;
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		// Check action ids
		CMailAction* action1 = static_cast<CMailAction*>(*iter1);
		switch(action1->GetAction())
		{
		case CMailAction::eSelect:
			// Add action to set
			selects.insert(action1->GetID());
			break;
		case CMailAction::eDeselect:
			// Remove action from set
			selects.erase(action1->GetID());
			break;
		
		// These actions require a select before hand
		case CMailAction::eFlag:
		case CMailAction::eCopy:
		case CMailAction::eExpunge:
			// Check that select exists
			if (!selects.count(action1->GetID()))
			{
				// Look for a matching deselect so we can get the mailbox UID
				bool found = false;
				for(iterator iter2 = iter1 + 1; iter2 != end(); iter2++)
				{
					CMailAction* action2 = static_cast<CMailAction*>(*iter2);
					if ((action2->GetID() == action1->GetID()) && (action2->GetAction() == CMailAction::eDeselect))
					{
						// Got matching deselect, so insert a select

						// Add select
						unsigned long pos = iter1 - begin();
						insert(iter1, new CMailAction(CMailAction::eSelect, action2->GetID(), action2->GetNameUIDAction().first, action2->GetNameUIDAction().second));
						iter1 = begin() + pos;
						
						// Now add to selection set to prevent further additions
						selects.insert(action1->GetID());
						found = true;
						break;
					}
				}
				
				// If not found, remove this playback item, otherwise we'll have an error
				// This condition should be reported to the user, but for now we will just log it
				if (!found)
				{
					// Do log before we delete the action
					if (mLog)
					{
						*mLog << "  Compacting Playback log: missing SELECT caused ";
						switch(action1->GetAction())
						{
							case CMailAction::eFlag:
								*mLog << "FLAG";
								break;
							case CMailAction::eCopy:
								*mLog << "COPY";
								break;
							case CMailAction::eExpunge:
								*mLog << "EXPUNGE";
								break;
							default:;
						}
						*mLog << " action removal." << os_endl << std::flush;
					}

					// Remove current item
					iter1 = erase(iter1);
					iter1--;
				}
			}
			break;

		default:;
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: missing SELECTs." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "1.txt");
		WriteItemsToStream(fout, true);
	}
}

// Compact unterminated selects
void CMailRecord::CompactUnterminated()
{
	// Look for unterminated selects
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		// Look for a select
		CMailAction* action1 = static_cast<CMailAction*>(*iter1);
		if (action1->GetAction() == CMailAction::eSelect)
		{
			//  Try to find matching deselect
			unsigned long id = action1->GetID();
			iterator last = iter1;

			for(iterator iter2 = iter1 + 1; iter2 != end(); iter2++)
			{
				CMailAction* action2 = static_cast<CMailAction*>(*iter2);
				if (action2->GetID() == id)
				{
					if (action2->GetAction() == CMailAction::eDeselect)
					{
						// Got matching deselect - all OK
						last = end();
						break;
					}
					else
						last = iter2;
				}
			}

			// See if unterminated
			if (last != end())
			{
				// See if any commands within select
				if (last != iter1)
				{
					// Add deselect after last
					unsigned long pos = iter1 - begin();
					insert(last + 1, new CMailAction(CMailAction::eDeselect, id, action1->GetNameUIDAction().first, action1->GetNameUIDAction().second));
					iter1 = begin() + pos;
				}
				else
				{
					// Just delete the select
					unsigned long pos = iter1 - begin();
					erase(iter1);
					iter1 = begin() + (pos - 1);
				}
			}
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: unterminated SELECTs." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "2.txt");
		WriteItemsToStream(fout, true);
	}
}

// Compact multiple selects
void CMailRecord::CompactMultiple()
{
	// Look for multiple selects
	long total_selects = 0;
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		// Look for a deselect
		CMailAction* action1 = static_cast<CMailAction*>(*iter1);
		if (action1->GetAction() == CMailAction::eDeselect)
		{
			// Bump down count of SELECTs
			total_selects--;

			//  Try to find another select
			unsigned long id = action1->GetID();
			cdstring select_name = action1->GetNameUIDAction().first;

			for(iterator iter2 = iter1 + 1; iter2 != end(); iter2++)
			{
				CMailAction* action2 = static_cast<CMailAction*>(*iter2);
				if (action2->GetAction() == CMailAction::eSelect)
				{
					// Bump up count of SELECTs
					total_selects++;

					// See if its the same mailbox
					if (select_name == action2->GetNameUIDAction().first)
					{
						// Replace second action id with first
						unsigned long replace_id = action2->GetID();
						for(iterator iter3 = iter2; iter3 != end(); iter3++)
						{
							CMailAction* action3 = static_cast<CMailAction*>(*iter3);
							if (action3->GetID() == replace_id)
								action3->mID = id;
						}
						
						// Delete select
						erase(iter2);
						
						// Delete deselect
						iter1 = erase(iter1);
						iter1--;

						// Bump down count of SELECTs
						total_selects--;
						break;
					}
					// Do not allow merge of action if more than two others are in progress
					else if (total_selects > 2)
						break;
				}
				else if (action2->GetAction() == CMailAction::eDeselect)
					// Bump down count of SELECTs
					total_selects--;
			}
		}
		else if (action1->GetAction() == CMailAction::eSelect)
			// Bump up count of SELECTs
			total_selects++;
	}

	if (mLog)
		*mLog << "  Compacting Playback log: multiple SELECTs." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "3.txt");
		WriteItemsToStream(fout, true);
	}
}


// Compact expunges changes
void CMailRecord::CompactExpunge()
{
	// Look for mutiple expunges
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		// Look for a expunge
		CMailAction* action1 = static_cast<CMailAction*>(*iter1);
		if (action1->GetAction() == CMailAction::eExpunge)
		{
			ulvector add = action1->GetExpungeAction().first;
			ulvector ladd = action1->GetExpungeAction().second;
			CMailAction* action_prev = action1;

			//  Try to find another expunge
			for(iterator iter2 = iter1 + 1; iter2 != end(); iter2++)
			{
				CMailAction* action2 = static_cast<CMailAction*>(*iter2);
				if ((action2->GetAction() == CMailAction::eExpunge) &&
					(action2->GetID() == action1->GetID()))
				{
					// Merge current with previous
					add.insert(add.end(), action2->GetExpungeAction().first.begin(), action2->GetExpungeAction().first.end());
					ladd.insert(ladd.end(), action2->GetExpungeAction().second.begin(), action2->GetExpungeAction().second.end());
					
					// Remove uids from previous
					const_cast<ulvector&>(action_prev->GetExpungeAction().first).clear();
					const_cast<ulvector&>(action_prev->GetExpungeAction().second).clear();
					
					action_prev = action2;
				}
			}
			
			// Now move sorted accumulated uids into last expunge
			if (action_prev != action1)
			{
				std::sort(add.begin(), add.end());
				std::sort(ladd.begin(), ladd.end());
				const_cast<ulvector&>(action_prev->GetExpungeAction().first) = add;
				const_cast<ulvector&>(action_prev->GetExpungeAction().second) = ladd;
			}
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: single EXPUNGE." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "4.txt");
		WriteItemsToStream(fout, true);
	}
}

// Compact flag changes
void CMailRecord::CompactFlags()
{
	// Look for similar flag changes
	for(iterator iter1 = begin(); iter1 != end(); iter1++)
	{
		// Look for a select
		CMailAction* action1 = static_cast<CMailAction*>(*iter1);
		if (action1->GetAction() == CMailAction::eSelect)
		{
			//  Try to find matching items - stop at deselect
			unsigned long id = action1->GetID();

			// Allow flag actions to rise as far as possible and merge with others
			iterator iter2 = iter1 + 1;
			for(; iter2 != end(); iter2++)
			{
				// Get action
				CMailAction* action2 = static_cast<CMailAction*>(*iter2);
				
				// Check ID
				if (action2->GetID() != id)
					continue;
				
				// Look for deselect and finish
				if (action2->GetAction() == CMailAction::eDeselect)
					break;
				
				// Look at actions
				switch(action2->GetAction())
				{
				case CMailAction::eFlag:
					// Action must not contain any local uids if it is to rise
					if (!action2->GetFlagAction().mUids.second.size())
					{
						// Scan back to select item and merge
						iterator iter3 = iter2;
						while((--iter3 != iter1) && CanActionMove(action2, static_cast<CMailAction*>(*iter3))) {}
					}
					break;
				default:;
				}
			}

			// Allow flag actions to sink as far as possible and merge with others
			for(iterator iter3 = iter2 - 1; iter3 != iter1; iter3--)
			{
				// Get action
				CMailAction* action2 = static_cast<CMailAction*>(*iter3);
				
				// Check ID
				if (action2->GetID() != id)
					continue;
				
				// Look at actions
				switch(action2->GetAction())
				{
				case CMailAction::eFlag:
					{
						// Scan forward to select item and merge
						iterator iter4 = iter3;
						while((++iter4 != iter2) && CanActionMove(action2, static_cast<CMailAction*>(*iter4))) {}
					}
					break;
				default:;
				}
			}
		}
	}

	if (mLog)
		*mLog << "  Compacting Playback log: similar FLAGs." << os_endl << std::flush;

	if (CLog::AllowPlaybackLog() && CLog::AllowAuthenticationLog())
	{
		cdofstream fout(mDescriptor + "5.txt");
		WriteItemsToStream(fout, true);
	}
}

bool CMailRecord::CanActionMove(CMailAction* action1, CMailAction* action2) const
{
	// Actions must be same id
	if (action1->GetID() != action2->GetID())
		return true;

	// Original action must not be empty
	if (!action1->GetFlagAction().mUids.first.size() && !action1->GetFlagAction().mUids.second.size())
		return false;

	// Check action ids
	switch(action2->GetAction())
	{
	case CMailAction::eFlag:
		// If they are the same then we transfer the uids over
		if (SameFlagAction(action1, action2))
		{
			TransferUIDs(action1, action2);
			return false;
		}
		// See whether flag action competes (e.g. set vs unset of same message)
		else if (CompetingFlagAction(action1, action2))
			return false;
		break;
	case CMailAction::eCopy:
		// Check for competing copy action
		if (CompetingCopyAction(action1, action2))
			return false;
		break;
	case CMailAction::eAppend:
		if (CompetingAppendAction(action1, action2))
			return false;
		break;
	case CMailAction::eExpunge:
		// Cannot move delete flag operations above or below an expunge
		if (action1->GetFlagAction().mFlags & NMessage::eDeleted)
			return false;
		break;
	default:;
	}
	
	return true;
}

bool CMailRecord::SameFlagAction(const CMailAction* action1, const CMailAction* action2) const
{
	// Check that flags are the same, that set/unset is the same and that the target
	// action2 is not empty
	return (action1->GetFlagAction().mFlags == action2->GetFlagAction().mFlags) &&
			(action1->GetFlagAction().mSet == action2->GetFlagAction().mSet) &&
			(action2->GetFlagAction().mUids.first.size() || action2->GetFlagAction().mUids.second.size());
}

bool CMailRecord::CompetingFlagAction(const CMailAction* action1, const CMailAction* action2) const
{
	// Check that flags are the same, that set/unset is different
	// action2 is not empty
	if ((action1->GetFlagAction().mFlags == action2->GetFlagAction().mFlags) &&
			(action1->GetFlagAction().mSet != action2->GetFlagAction().mSet))
	{
		// Look at any overlap in uids
		return set_contains(action1->GetFlagAction().mUids.first, action2->GetFlagAction().mUids.first) ||
				set_contains(action1->GetFlagAction().mUids.second, action2->GetFlagAction().mUids.second);
	}
	
	return false;
}

bool CMailRecord::CompetingCopyAction(const CMailAction* action1, const CMailAction* action2) const
{
	// Look at any overlap in uids
	return set_contains(action1->GetFlagAction().mUids.first, action2->GetCopyAction().second.first) ||
			set_contains(action1->GetFlagAction().mUids.second, action2->GetCopyAction().second.second);
}

bool CMailRecord::CompetingAppendAction(const CMailAction* action1, const CMailAction* action2) const
{
	// Look at any overlap in uids
	unsigned long luid = action2->GetNameUIDAction().second;
	ulvector::const_iterator found = std::find(action1->GetFlagAction().mUids.second.begin(),
											action1->GetFlagAction().mUids.second.end(),
											luid);
	return (found != action1->GetFlagAction().mUids.second.end());
}

void CMailRecord::TransferUIDs(CMailAction* from, CMailAction* to) const
{
	transfer_unique(const_cast<CMailAction::SFlagAction&>(from->GetFlagAction()).mUids.first,
					const_cast<CMailAction::SFlagAction&>(to->GetFlagAction()).mUids.first);
	transfer_unique(const_cast<CMailAction::SFlagAction&>(from->GetFlagAction()).mUids.second,
					const_cast<CMailAction::SFlagAction&>(to->GetFlagAction()).mUids.second);
}

bool CMailRecord::set_contains(const ulvector& set1, const ulvector& set2) const
{
	ulvector::const_iterator first1 = set1.begin();
	ulvector::const_iterator last1 = set1.end();
	ulvector::const_iterator first2 = set2.begin();
	ulvector::const_iterator last2 = set2.end();
	
	while (first1 != last1 && first2 != last2)
	{
		if (*first2 < *first1)
			++first2;
		else if (*first1 < *first2)
			++first1;
		else
			return true;
	}
	return false;
}

bool CMailRecord::set_contains(const ulvector& set1, const ulmap& set2) const
{
	ulvector::const_iterator first1 = set1.begin();
	ulvector::const_iterator last1 = set1.end();
	ulmap::const_iterator first2 = set2.begin();
	ulmap::const_iterator last2 = set2.end();
	
	while (first1 != last1 && first2 != last2)
	{
		if ((*first2).first < *first1)
			++first2;
		else if (*first1 < (*first2).first)
			++first1;
		else
			return true;
	}
	return false;
}

void CMailRecord::transfer_unique(ulvector& from, ulvector& to) const
{
	// Find unique items
	for(ulvector::const_iterator iter = from.begin(); iter != from.end(); iter++)
	{
		ulvector::const_iterator found = std::find(to.begin(), to.end(), *iter);
		if (found == to.end())
			to.push_back(*iter);
	}
	
	// Sort then remove old items
	std::sort(to.begin(), to.end());
	from.clear();
}

#pragma mark ____________________________Playback Utils

// Remove zero UIDs
void CMailRecord::PurgeUIDs(ulvector& uids, ulvector& luids)
{
	// Point iterators to first non-zero server UID
	ulvector::iterator iter1 = uids.begin();
	ulvector::iterator iter2 = luids.begin();
	while((iter1 != uids.end()) && !*iter1)
	{
		/// Bump iters
		iter1++;
		iter2++;
	}
	
	// Erase unwanted items
	uids.erase(uids.begin(), iter1);
	luids.erase(iter2, luids.end());
}

// Add any local uids after mapping to server UIDs
void CMailRecord::RemapUIDs(CMbox* local, ulvector& uids, const ulvector& luids)
{
	// Check for local only UIDs
	if (luids.size())
		// Map local to remote UIDs and add
		AddLocalUIDs(local, luids, uids);
}

// Convert local uids to remote uids
bool CMailRecord::MapLocalUIDs(CMbox* local, ulvector& uids, ulvector* missing, ulmap* local_map)
{
	if (!local)
	{
		uids.clear();
		return false;
	}
	
	local->MapLocalUIDs(uids, missing, local_map);

	return true;
}

// Convert local uids to remote uids and add to other list
void CMailRecord::AddLocalUIDs(CMbox* local, const ulvector& luids, ulvector& uids)
{
	// Must have a mailbox
	if (!local)
		return;
	
	ulvector temp = luids;
	local->MapLocalUIDs(temp);
	if (temp.size())
	{
		uids.insert(uids.end(), temp.begin(), temp.end());
		std::sort(uids.begin(), uids.end());
	}
}

// Descover the UID of a message on the server
unsigned long CMailRecord::DescoverUID(CMessage* msg, CMbox* rsource)
{
	// See if message has Message-ID
	if (!msg->GetEnvelope()->GetMessageID().empty())
	{
		// Do descovery based on message-id
		CSearchItem search_spec(CSearchItem::eHeader, cdstring("Message-ID"), msg->GetEnvelope()->GetMessageID());
		ulvector results;
		rsource->Search(&search_spec, &results, true, true);
		
		// Choose last matching UID
		if (results.size())
			return results.back();
	}

	// Must fail this - no decent way to do it!
	return 0;
}

void CMailRecord::DescoverUIDs(ulvector& uids, ulmap& map, CMbox* rsource, CMbox* rdest)
{
	// See which messages are not currently cached with a Message-ID
	ulvector must_cache;
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		CMessage* msg = rsource->GetMessageUID(*iter);
		if (!msg || !msg->GetEnvelope())
		{
			// Add it to the list of items to fetch
			must_cache.push_back(*iter);
		}
	}
	
	// Must fetch UIDs of copied messages
	if (must_cache.size())
		rsource->GetMsgProtocol()->FetchItems(must_cache, true, static_cast<CMboxProtocol::EFetchItems>(CMboxProtocol::eUID | CMboxProtocol::eEnvelope));

	// For each message in the map - descover it
	for(ulvector::const_iterator iter = uids.begin(); iter != uids.end(); iter++)
	{
		CMessage* msg = rsource->GetMessageUID(*iter);
		if (msg && msg->GetEnvelope())
		{
			unsigned long uid = DescoverUID(msg, rdest);
			if (uid)
				map.insert(ulmap::value_type(*iter, uid));
		}
	}
}

#pragma mark ____________________________Stream Ops

#if __dest_os == __mac_os || __dest_os == __mac_os_x
OSType CMailRecord::GetMacFileType() const
{
	return cMailboxRecordType;
}
#endif

CRecordableAction* CMailRecord::NewAction() const
{
	return new CMailAction();
}
