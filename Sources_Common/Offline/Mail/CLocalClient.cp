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


// Code for Local client class

#include "CLocalClient.h"
#include "CINETCommon.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CAttachmentList.h"
#include "CEnvelope.h"
#include "CIMAPCommon.h"
#include "CMailControl.h"
#include "CMailRecord.h"
#include "CMbox.h"
#include "CLocalAttachment.h"
#include "CLocalCommon.h"
#include "CLocalMessage.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CRFC822Parser.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"

#include "diriterator.h"
#include "filterbuf.h"
#include "mimefilters.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x

#include "CStringResources.h"

#include <UProfiler.h>

#elif __dest_os == __win32_os

#include <WIN_LStream.h>
#include "StValueChanger.h"

#elif __dest_os == __linux_os
#include "UNX_LStream.h"
#include "StValueChanger.h"
#include <JString.h>
#include <string.h>

#endif

#include <algorithm>
#include <cerrno>
#include __stat_header
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strstream>

#define CHECK_STREAM(x) \
	{ if ((x).fail()) { int err_no = os_errno; CLOG_LOGTHROW(CGeneralException, err_no); throw CGeneralException(err_no); } }

const unsigned long cIndexVers = 0x0000000B;
const unsigned long cIndexType_Mask = ~0x00030000;
#define sIndexWriteLength (sizeof(CLocalClient::SIndexRecord) - sizeof(unsigned long))

const unsigned long cSearchBufferSize = 8192;
const unsigned long cWorkBufferSize = 4096;

#pragma mark ____________________________SIndexHeader

void CLocalClient::SIndexHeader::write(std::ostream& out) const
{
	out.seekp(0);
	::WriteHost(out, Version());
	::WriteHost(out, MboxModified());
	::WriteHost(out, CacheModified());
	::WriteHost(out, LastSync());
	::WriteHost(out, IndexSize());
	::WriteHost(out, UIDValidity());
	::WriteHost(out, UIDNext());
	::WriteHost(out, LastUID());
	::WriteHost(out, LocalUIDNext());
}

void CLocalClient::SIndexHeader::write_LastSync(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mLastSync), std::ios_base::beg);
	::WriteHost(out, LastSync());
}

void CLocalClient::SIndexHeader::write_IndexSize(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mIndexSize), std::ios_base::beg);
	::WriteHost(out, IndexSize());
}

void CLocalClient::SIndexHeader::write_UIDValidity(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mUIDValidity), std::ios_base::beg);
	::WriteHost(out, UIDValidity());
}

void CLocalClient::SIndexHeader::write_UIDNext(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mUIDNext), std::ios_base::beg);
	::WriteHost(out, UIDNext());
}

void CLocalClient::SIndexHeader::write_LastUID(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mLastUID), std::ios_base::beg);
	::WriteHost(out, LastUID());
}

void CLocalClient::SIndexHeader::write_LocalUIDNext(std::ostream& out) const
{
	out.seekp(offsetof(SIndexHeader, mLocalUIDNext), std::ios_base::beg);
	::WriteHost(out, LocalUIDNext());
}

void CLocalClient::SIndexHeader::read(std::istream& in)
{
	in.seekg(0);
	::ReadHost(in, Version());
	::ReadHost(in, MboxModified());
	::ReadHost(in, CacheModified());
	::ReadHost(in, LastSync());
	::ReadHost(in, IndexSize());
	::ReadHost(in, UIDValidity());
	::ReadHost(in, UIDNext());
	::ReadHost(in, LastUID());
	::ReadHost(in, LocalUIDNext());
	
	// Need to remove line end type from version
	mVersion &= cIndexType_Mask;
}

#pragma mark ____________________________SIndexRecord

void CLocalClient::SIndexRecord::write(std::ostream& out) const
{
	::WriteHost(out, Cache());
	::WriteHost(out, Index());
	::WriteHost(out, Flags());
	::WriteHost(out, UID());
	::WriteHost(out, LocalUID());
	::WriteHost(out, MessageStart());
}

void CLocalClient::SIndexRecord::write_Flags(std::ostream& out) const
{
	out.seekp(offsetof(SIndexRecord, mFlags), std::ios_base::cur);
	::WriteHost(out, Flags());
}

void CLocalClient::SIndexRecord::write_UID(std::ostream& out) const
{
	out.seekp(offsetof(SIndexRecord, mUID), std::ios_base::cur);
	::WriteHost(out, UID());
}

void CLocalClient::SIndexRecord::read(std::istream& in)
{
	::ReadHost(in, Cache());
	::ReadHost(in, Index());
	::ReadHost(in, Flags());
	::ReadHost(in, UID());
	::ReadHost(in, LocalUID());
	::ReadHost(in, MessageStart());
}

#pragma mark ____________________________CLocalClient

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CLocalClient::CLocalClient(CMboxProtocol* owner)
	: CMboxClient(owner),
	  mAppendStart(0)
{
	// Init instance variables
	InitLocalClient();
}

// Copy constructor
CLocalClient::CLocalClient(const CLocalClient& copy, CMboxProtocol* owner)
	: CMboxClient(copy, owner),
	  mAppendStart(0)
{
	// Init instance variables
	InitLocalClient();

	mCWD = copy.mCWD;

	mRecorder = copy.mRecorder;
}

CLocalClient::~CLocalClient()
{
	mRecorder = NULL;
	mProcessMessage = NULL;
	mAppendMailbox = NULL;
	mAppendCache = NULL;
	mAppendIndex = NULL;
	mSearchBuffer = NULL;

}

void CLocalClient::InitLocalClient()
{
	mRecorder = NULL;
	mRecordID = 0;
	mSeparateUIDs = false;
	mRecordLocalUIDs = false;
	mProcessMessage = NULL;

	mEndl = eEndl_Auto;

	mMboxNew = 0;
	mInPostProcess = false;
	mMboxUpdate = false;
	mMboxReset = false;
	mMboxReload = false;
	mMboxReadWrite = false;

	mAppendStart = 0;
	mAppendMailbox = NULL;
	mAppendCache = NULL;
	mAppendIndex = NULL;
	
	mSearchBuffer = NULL;
	mWorkBuffer.reserve(cWorkBufferSize);
	
	mProgress = NULL;

	mPendingAbort = false;

}

// Create duplicate, empty connection
CINETClient* CLocalClient::CloneConnection()
{
	// Copy construct this
	return new CLocalClient(*this, GetMboxOwner());

}

#pragma mark ____________________________Start/Stop

// Start TCP
void CLocalClient::Open()
{
	// Do account reset
	Reset();
}

// Reset acount info
void CLocalClient::Reset()
{
	// get CWD from owner
	mCWD = GetMboxOwner()->GetOfflineCWD();

	// Must append dir delim if not present
	if (mCWD.length() && (mCWD[mCWD.length() - 1] != os_dir_delim))
		mCWD += os_dir_delim;

	// Only worried about this for original not clone
	if (!GetMboxOwner()->IsCloned())
	{
		// May need to check for INBOX on POP3
		CheckCWD();
	}

	// Set index mapping state
	if (GetMboxOwner()->GetMailAccount()->GetServerType() == CINETAccount::eIMAP)
		mSeparateUIDs = true;
	else
		// Get endl state for a local account
		mEndl = GetMboxOwner()->GetMailAccount()->GetEndl();
}

// Check CWD
void CLocalClient::CheckCWD()
{
	// Local does nothing

}

// Release TCP
void CLocalClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CLocalClient::Abort()
{
	// Set abort flag
	mPendingAbort = true;
	
	// Force mailbox and cache streams to error states to trigger throw
	if (mMailbox.is_open())
		mMailbox.setstate(std::ios_base::failbit);
	if (mCache.is_open())
		mCache.setstate(std::ios_base::failbit);
}

// Forced close
void CLocalClient::Forceoff()
{
	// Clean out streams and all cached data
	// Do not allow close to fail
	try
	{
		// Clear error states on streams first
		// We may get here as the result of a stream error but want to
		// be able to close down without errors
		mMailbox.clear();
		mCache.clear();
		mIndex.clear();

		// Close the mailbox and cache files
		mMailbox.close();
		mCache.close();
		mIndex.close();
	}

	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
}

#pragma mark ____________________________Login & Logout

// Logon to IMAP server
void CLocalClient::Logon()
{
	// Local does nothing
	
	// Must fail if empty CWD
	if (mCWD.empty())
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

}

// Logoff from IMAP server
void CLocalClient::Logoff()
{
	// Local does nothing

}

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CLocalClient::_Tickle(bool force_tickle)
{
	// Local does nothing

}

// Handle failed capability response
void CLocalClient::_PreProcess()
{
	// Clear mailbox change
	mMboxNew = 0;
	mMboxUpdate = false;
	mMboxReset = false;
	mMboxReload = false;

}

// Handle failed capability response
void CLocalClient::_PostProcess()
{
	// Save new message count while doing an update
	unsigned long new_msgs = mMboxNew;

	// Do any GUI update of mailbox
	if (mMboxReload && (GetCurrentMbox()))
	{
		// Signal update about to occur
		CMailControl::MboxAbouttoUpdate(GetCurrentMbox());

		// Renumber and resort after change
		GetCurrentMbox()->RenumberMessages();
		//GetCurrentMbox()->LoadMessages();		<-- Done in UpdateMbox

		{
			// Prevent re-entrant calls to MboxReset etc during the caching process
			StValueChanger<bool> _preserve(mInPostProcess, true);
			GetMboxOwner()->UpdateMbox(GetCurrentMbox());
		}

		// Do filtering before doing other updates so that messages that are
		// filtered can be accounted for before the other checks are done
		if (new_msgs && !GetCurrentMbox()->IsBeingOpened())
			CMailControl::MboxNewMessages(GetCurrentMbox(), new_msgs);

		if (!mInPostProcess)
			CMailControl::MboxReset(GetCurrentMbox());
	}

	// Do any GUI update of mailbox
	else if (!mInPostProcess && (GetCurrentMbox() != NULL))
	{
		// Do filtering before doing other updates so that messages that are
		// filtered can be accounted for before the other checks are done
		if (new_msgs && !GetCurrentMbox()->IsBeingOpened())
			CMailControl::MboxNewMessages(GetCurrentMbox(), new_msgs);

		else if (mMboxReset)
			CMailControl::MboxReset(GetCurrentMbox());
		else if (mMboxUpdate)
			CMailControl::MboxUpdate(GetCurrentMbox());
	}

	// Recover new message count
	mMboxNew = new_msgs;

}

#pragma mark ____________________________Mailboxes

// Create a new mbox
void CLocalClient::_CreateMbox(CMbox* mbox)
{
	StINETClientAction action(this, "Status::IMAP::Creating", "Error::IMAP::OSErrCreate", "Error::IMAP::NoBadCreate", mbox->GetName());
	StLocalProcess process(this);

	try
	{
		// Must ensure entire path exists
		{
			cdstring convert = LocalFileName(mbox->GetName(), mbox->GetDirDelim(), GetMboxOwner()->IsDisconnected());
			cdstring mbox_name = mCWD;
			cdstring dir_delim = os_dir_delim;
			char* dir = ::strtok(convert.c_str_mod(), dir_delim);
			char* next_dir = ::strtok(NULL, dir_delim);
			while(dir && next_dir)
			{
				mbox_name += dir;
				::chkdir(mbox_name);
				mbox_name += os_dir_delim;
				dir = next_dir;
				next_dir = ::strtok(NULL, dir_delim);
			}
		}

		// Check for directory and append dir delim
		if (mbox->IsDirectory())
		{
			cdstring convert = LocalFileName(mbox->GetName(), mbox->GetDirDelim(), GetMboxOwner()->IsDisconnected());
			cdstring mbox_name = mCWD + convert;
			if (__mkdir(mbox_name, S_IRWXU))
			{
				int err_no = os_errno;
				CLOG_LOGTHROW(CGeneralException, err_no);
				throw CGeneralException(err_no);
			}
		}
		else
		{
			cdstring mbox_name;
			cdstring cache_name;
			cdstring index_name;
			GetNames(mbox, mbox_name, cache_name, index_name);

			FILE* file;
			try
			{
				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					StCreatorType file1(cMulberryCreator, cMailboxType);
#endif
					if ((file = ::fopen_utf8(mbox_name, "ab")) != NULL)
						::fclose(file);
					else
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
				}
				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					StCreatorType file1(cMulberryCreator, cMailboxCacheType);
#endif
					if ((file = ::fopen_utf8(cache_name, "ab")) != NULL)
						::fclose(file);
					else
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
				}
				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					StCreatorType file1(cMulberryCreator, cMailboxIndexType);
#endif
					if ((file = ::fopen_utf8(index_name, "ab")) != NULL)
						::fclose(file);
					else
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
				}

				// Now do reconstruct to force index header into index file
				Reconstruct(mbox);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// If anyone fails then delete the lot
				_DeleteMbox(mbox);

				// throw up
				CLOG_LOGRETHROW;
				throw;
			}
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Create(mbox);
	}

}

// Do touch
void CLocalClient::_TouchMbox(CMbox* mbox)
{
	// Check it exists and create if not
	if (!_TestMbox(mbox))
	{
		// Do this without recording
		StValueChanger<CMailRecord*> value(mRecorder, NULL);
		_CreateMbox(mbox);
	}
}

// Do test
bool CLocalClient::_TestMbox(CMbox* mbox)
{
	// Must have INBOX
	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

	// Check it exists and create if not
	return ::fileexists(mbox_name);
}

// Rebuild cache
void CLocalClient::_RebuildMbox(CMbox* mbox)
{
	// Reconstruct contents
	Reconstruct(mbox);
}

// Do open mailbox
void CLocalClient::_OpenMbox(CMbox* mbox)
{
	// This does nothing - mailbox is really opened when it is selected
	// This routine is useful for local mailboxes on disks

}

// Do close mailbox
void CLocalClient::_CloseMbox(CMbox* mbox)
{
	StLocalProcess process(this);

	// Do not allow close to fail
	try
	{
		// Force recent flag off on all messages if read-write
		if (!mbox->IsReadOnly())
			ClearRecent();

		// Clear error states on streams first
		// We may get here as the result of a stream error but want to
		// be able to close down without errors
		mMailbox.clear();
		mCache.clear();
		mIndex.clear();

		// Close the mailbox and cache files
		mMailbox.close();
		mCache.close();

		// Update the index header (if writeable) and close
		if (mMboxReadWrite)
			SyncIndexHeader(mbox, mIndex);
		mIndex.close();
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);
	}

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Deselect(mbox);
		mRecordID = 0;
	}

}

// Do Selection
void CLocalClient::_SelectMbox(CMbox* mbox, bool examine)
{
	StINETClientAction action(this, "Status::IMAP::Selecting", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mbox->GetName());
	StLocalProcess process(this);

	try
	{
		// Make sure examine flag is set
		mbox->SetFlags(NMbox::eReadOnly, examine);
		
		// Open the cache files (always open mailbox and cache in read-only mode)
		SIndexHeader header;
		OpenCache(mbox, mMailbox, mCache, mIndex, header, mIndexList, &mIndexMapping, true);

		// Set read-write based on whether we were able to open it as such
		mbox->SetFlags(NMbox::eReadOnly, !mMboxReadWrite);

		// Set allowed flags changes based on read-write state
		mbox->SetAllowedFlags(mMboxReadWrite ? NMessage::eIMAPFlags : NMessage::eNone);
		
		// Get mailbox info from index
		CheckFromIndex(mbox, mIndexList);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		// Get new recording ID on select
		mRecordID = mRecorder->GetNextID();
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Select(mbox);
	}

}

// Deselect mbox
void CLocalClient::_Deselect(CMbox* mbox)
{
	// Same as close for local mailboxes
	_CloseMbox(mbox);

}

// Check mbox
void CLocalClient::_CheckMbox(CMbox* mbox, bool fast)
{
	StINETClientAction action(this, "Status::IMAP::Checking", "Error::IMAP::OSErrCheck", "Error::IMAP::NoBadCheck", mbox->GetName());
	StLocalProcess process(this);

	try
	{
		if (GetCurrentMbox() == mbox)

			// Just check existing index
			CheckFromIndex(mbox, mIndexList);

		else
		{
			// NB Can never get here if the mailbox is open in another connection!

			SIndexList index_list;

			{
				// Get the files
				cdfstream mailbox;
				cdfstream cache;
				cdfstream index;

				SIndexHeader header;
				OpenCache(mbox, mailbox, cache, index, header, index_list, NULL, true);
			}

			// Get mailbox info from index
			CheckFromIndex(mbox, index_list);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Get size of mailbox
void CLocalClient::_MailboxSize(CMbox* mbox)
{
	StINETClientAction action(this, "Status::IMAP::Checking", "Error::IMAP::OSErrCheck", "Error::IMAP::NoBadCheck", mbox->GetName());

	try
	{
		cdstring mbox_name;
		cdstring cache_name;
		cdstring index_name;
		GetNames(mbox, mbox_name, cache_name, index_name);

		// Get sizes
		unsigned long size = 0;
		struct stat mbox_finfo;
		if (::stat_utf8(mbox_name, &mbox_finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}
		else
			size += mbox_finfo.st_size;

		if (!::stat_utf8(cache_name, &mbox_finfo))
			size += mbox_finfo.st_size;

		if (!::stat_utf8(index_name, &mbox_finfo))
			size += mbox_finfo.st_size;

		mbox->SetSize(size);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Clear (EXPUNGE) current mbox
bool CLocalClient::_ExpungeMbox(bool closing)
{
	StINETClientAction action(this, "Status::IMAP::Expunging", "Error::IMAP::OSErrExpunge", "Error::IMAP::NoBadExpunge", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Get message numbers of all deleted messages
		ulvector indices;
		indices.reserve(mIndexList.size());	// Optimisation for large mailbox
		unsigned long index = 0;
		for(SIndexList::const_iterator iter = mIndexList.begin(); iter != mIndexList.end(); iter++, index++)
		{
			if ((*iter).Flags() & NMessage::eDeleted)
				indices.push_back(index);
		}

		// Now expunge them
		ExpungeMessage(indices);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	
	return false;

}

// Delete mbox
void CLocalClient::_DeleteMbox(CMbox* mbox)
{
	StINETClientAction action(this, "Status::IMAP::DeletingMbox", "Error::IMAP::OSErrDelete", "Error::IMAP::NoBadDelete", mbox->GetName());
	StLocalProcess process(this);

	try
	{
		// Look for uncached disconnected mailbox
		bool missing_mbox = false;
		if (GetMboxOwner()->IsDisconnected())
			missing_mbox = !_TestMbox(mbox);

		// For recording purposes we must have mailbox's UIDValidity cached
		// BEFORE file is removed
		if (mRecorder && !missing_mbox && (!mbox->HasStatus() || !mbox->GetUIDValidity()))
			mbox->Check();

		// Only do if present
		if (!missing_mbox)
		{
			cdstring dir_name;
			cdstring mbox_name;
			cdstring cache_name;
			cdstring index_name;
			GetNames(mbox, mbox_name, cache_name, index_name);

			// Adjust path for directory
			dir_name = mbox_name;
			dir_name.c_str_mod()[dir_name.length() - 4] = 0;
			
			// See what kind of object
			bool delete_dirc = false;
			bool delete_mbox = false;
			if (mbox->IsDirectory())
				delete_dirc = true;
			else if (mbox->IsHierarchy())
			{
				delete_dirc = true;
				delete_mbox = true;
			}
			else
				delete_mbox = true;

			// Delete the mailbox first
			if (delete_mbox)
			{
				// Delete mailbox
				::remove_utf8(mbox_name);
				::remove_utf8(cache_name);
				::remove_utf8(index_name);
			}

			if (delete_dirc)
			{
				// See if directory contains anything
				int contents = count_dir_contents(dir_name);
				
				// Delete it only if it is empty
				if (!contents)
					delete_dir(dir_name);
			}
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Delete(mbox);
	}

}

// Rename mbox
void CLocalClient::_RenameMbox(CMbox* mbox_old, const char* mbox_new)
{
	StINETClientAction action(this, "Status::IMAP::Renaming", "Error::IMAP::OSErrRename", "Error::IMAP::NoBadRename", mbox_old->GetName());
	StLocalProcess process(this);

	try
	{
		// Look for uncached disconnected mailbox
		bool missing_mbox = false;
		if (GetMboxOwner()->IsDisconnected())
			missing_mbox = !_TestMbox(mbox_old);

		// For recording purposes we must have mailbox's UIDValidity cached
		// BEFORE file is removed
		if (mRecorder && !missing_mbox && (!mbox_old->HasStatus() || !mbox_old->GetUIDValidity()))
			mbox_old->Check();

		// Only do if present
		if (!missing_mbox)
		{
			// Rename for mailbox directory
			if (mbox_old->IsHierarchy())
			{
				cdstring wd_name = mCWD;
				wd_name += LocalFileName(mbox_old->GetName(), mbox_old->GetDirDelim(), GetMboxOwner()->IsDisconnected());

				cdstring wd_new_name = mCWD;
				wd_new_name += LocalFileName(mbox_new, mbox_old->GetDirDelim(), GetMboxOwner()->IsDisconnected());

				// NB Sometimes a local mailbox is marked as not /NoInferiors but we haven't actually created
				// the local directory - though the .mbx etc files exist. In this case we should not generate an error
				// as the rename can just operate on the .mbx files.
				if (::direxists(wd_name) && ::moverename_file(wd_name, wd_new_name))
				{
					int err_no = os_errno;
					CLOG_LOGTHROW(CGeneralException, err_no);
					throw CGeneralException(err_no);
				}
			}
			
			// Rename mailbox if its not a directory
			if (!mbox_old->IsDirectory())
			{
				cdstring mbox_name_old;
				cdstring cache_name_old;
				cdstring index_name_old;
				GetNames(mbox_old, mbox_name_old, cache_name_old, index_name_old);

				cdstring convert = LocalFileName(mbox_new, mbox_old->GetDirDelim(), GetMboxOwner()->IsDisconnected());
				cdstring mbox_name_new;
				cdstring cache_name_new;
				cdstring index_name_new;
				GetNames(convert, mbox_name_new, cache_name_new, index_name_new);

				// Rename mailbox
				bool mbox_done = false;
				bool cache_done = false;
				bool index_done = false;
				try
				{
					if (::moverename_file(mbox_name_old, mbox_name_new))
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
					else
						mbox_done = true;
					if (::moverename_file(cache_name_old, cache_name_new))
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
					else
						cache_done = true;
					if (::moverename_file(index_name_old, index_name_new))
					{
						int err_no = os_errno;
						CLOG_LOGTHROW(CGeneralException, err_no);
						throw CGeneralException(err_no);
					}
					else
						index_done = true;
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Must rename back to original
					if (mbox_done)
						::moverename_file(mbox_name_new, mbox_name_old);
					if (cache_done)
						::moverename_file(cache_name_new, cache_name_old);
					if (index_done)
						::moverename_file(index_name_new, index_name_old);

					// throw up
					CLOG_LOGRETHROW;
					throw;
				}
			}
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Rename(mbox_old, mbox_new);
	}

}

// Subscribe mbox
void CLocalClient::_SubscribeMbox(CMbox* mbox)
{
	// No meaning for local mailboxes

	// For recording purposes we must have mailbox's UIDValidity cached
	// BEFORE file is removed
	if (mRecorder && (!mbox->HasStatus() || !mbox->GetUIDValidity()))
		mbox->Check();

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Subscribe(mbox);
	}

}

// Unsubscribe mbox
void CLocalClient::_UnsubscribeMbox(CMbox* mbox)
{
	// No meaning for local mailboxes

	// For recording purposes we must have mailbox's UIDValidity cached
	// BEFORE file is removed
	if (mRecorder && (!mbox->HasStatus() || !mbox->GetUIDValidity()))
		mbox->Check();

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Unsubscribe(mbox);
	}

}

// Get namespace
void CLocalClient::_Namespace(CMboxProtocol::SNamespace* names)
{
	// No meaning for local mailboxes

}

// Find all subscribed mboxes
void CLocalClient::_FindAllSubsMbox(CMboxList* mboxes)
{
	// No meaning for local mailboxes

}

// Find all mboxes
void CLocalClient::_FindAllMbox(CMboxList* mboxes)
{
	StINETClientAction action(this, "Status::IMAP::FindingAll", "Error::IMAP::OSErrFindAll", "Error::IMAP::NoBadFindAll");
	StLocalProcess process(this);
	
	try
	{
		InitItemCtr();

		// Get the WD (NULL => do hierarchy character descovery)
		if (!mboxes)
		{
			if (GetMboxOwner()->GetMailAccount()->GetDirDelim() != os_dir_delim)
			{
				GetMboxOwner()->GetMailAccount()->SetDirDelim(os_dir_delim);
				CPreferences::sPrefs->mMailAccounts.SetDirty(true);
			}
		}
		else
		{
			// Always set dir-delim in mailbox list
			mboxes->SetDirDelim(os_dir_delim);

			// Do pattern search
			cdstring pattern = mCWD + mboxes->GetRoot();
			ScanDirectory(mCWD, pattern, mboxes, true);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Starting multiple append
void CLocalClient::_StartAppend(CMbox* mbox)
{
	StINETClientAction action(this, "Status::IMAP::Appending", "Error::IMAP::OSErrAppend", "Error::IMAP::NoBadAppend", mbox->GetName());

	try
	{
		if (GetCurrentMbox() == mbox)
		{
			// Must have read-write streams
			SwitchCache(mbox, mIndex, mMailbox, mCache, false);

			// Use existing streams
			mAppendMailbox = &mMailbox;
			mAppendCache = &mCache;
			mAppendIndex = &mIndex;
		}
		else
		{
			// NB Can never get here if the mailbox is open in another connection!
			// File streams
			mAppendMailbox = new cdfstream;
			mAppendCache = new cdfstream;
			mAppendIndex = new cdfstream;

			// May need to reconstruct
			ValidateCache(mbox, *mAppendMailbox, *mAppendCache, *mAppendIndex, false);
		}

		// Seek to end of mailbox stream
		mAppendMailbox->seekp(0, std::ios_base::end);

		// Now at starting point for this message
		mAppendStart = mAppendMailbox->tellp();
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		// Clean-up
		if (GetCurrentMbox() == mbox)
		{
			try
			{
				// Must have read-only streams
				SwitchCache(mbox, mIndex, mMailbox, mCache, true);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

			}
		}
		else
		{
			// Close files if open
			if (mAppendMailbox)
				mAppendMailbox->close();
			if (mAppendCache)
				mAppendCache->close();
			if (mAppendIndex)
				mAppendIndex->close();

			delete mAppendMailbox;
			delete mAppendCache;
			delete mAppendIndex;
		}
		mAppendMailbox = NULL;
		mAppendCache = NULL;
		mAppendIndex = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		// Clean-up
		if (GetCurrentMbox() == mbox)
		{
			try
			{
				// Must have read-only streams
				SwitchCache(mbox, mIndex, mMailbox, mCache, true);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

			}
		}
		else
		{
			// Close files if open
			if (mAppendMailbox)
				mAppendMailbox->close();
			if (mAppendCache)
				mAppendCache->close();
			if (mAppendIndex)
				mAppendIndex->close();

			delete mAppendMailbox;
			delete mAppendCache;
			delete mAppendIndex;
		}
		mAppendMailbox = NULL;
		mAppendCache = NULL;
		mAppendIndex = NULL;

		CLOG_LOGRETHROW;
		throw;
	}
}

// Stopping multiple append
void CLocalClient::_StopAppend(CMbox* mbox)
{
	StINETClientAction action(this, "Status::IMAP::Appending", "Error::IMAP::OSErrAppend", "Error::IMAP::NoBadAppend", mbox->GetName());

	try
	{
		// Delete those created
		if (GetCurrentMbox() == mbox)
		{
			// Must have read-only streams
			SwitchCache(mbox, mIndex, mMailbox, mCache, true);
		}
		else
		{
			// Close mailbox and cache files and write index header
			if (mAppendMailbox)
				mAppendMailbox->close();
			if (mAppendCache)
				mAppendCache->close();
			if (mAppendIndex)
			{
				SyncIndexHeader(mbox, *mAppendIndex);
				mAppendIndex->close();
			}

			delete mAppendMailbox;
			delete mAppendCache;
			delete mAppendIndex;
		}

		mAppendMailbox = NULL;
		mAppendCache = NULL;
		mAppendIndex = NULL;
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Append to mbox
void CLocalClient::_AppendMbox(CMbox* mbox, CMessage* theMsg, unsigned long& new_uid, bool dummy_files)
{
	StINETClientAction action(this, "Status::IMAP::Appending", "Error::IMAP::OSErrAppend", "Error::IMAP::NoBadAppend", mbox->GetName());
	StLocalProcess process(this);

	// Check whether already started
	bool do_start = (mAppendMailbox == NULL);

	try
	{
		if (do_start)
			_StartAppend(mbox);

		if (GetCurrentMbox() == mbox)
			new_uid = AppendMessage(mbox, theMsg, true, false, NULL, dummy_files);
		else
			// Now append
			new_uid = AppendMessage(mbox, theMsg, false, false, NULL, dummy_files);

		if (do_start)
			_StopAppend(mbox);

		// Record action
		if (mRecorder)
		{
			CMailRecord::StActionRecorder record(mRecorder, mRecordID);
			mRecorder->AppendTo(new_uid, static_cast<NMessage::EFlags>(theMsg->GetFlags().Get() & NMessage::eServerFlags), mbox);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		if (do_start)
			_StopAppend(mbox);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		if (do_start)
			_StopAppend(mbox);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Search messages on the server
void CLocalClient::_SearchMbox(const CSearchItem* spec, ulvector* results, bool uids)
{
	StINETClientAction action(this, "Status::IMAP::Searching", "Error::IMAP::OSErrSearch", "Error::IMAP::NoBadSearch", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		CLocalMessage dummy(reinterpret_cast<CMbox*>(NULL));
		bool use_dummy = !GetCurrentMbox()->IsFullOpen();

		// Create search buffer
		std::auto_ptr<char> search(new char[cSearchBufferSize]);
		StValueChanger<char*> change(mSearchBuffer, search.get());

		//StProfileSection profile("\pSearching", 200, 20);
		for(SIndexList::const_iterator iter = mIndexList.begin(); iter != mIndexList.end(); iter++)
		{
			// If not fully open use a dummy message
			unsigned long seq = (*iter).Sequence();
			CLocalMessage* lmsg = use_dummy ? &dummy :
									dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(seq));

			// Dummy message must have correct message number and be reindexed, cached
			if (use_dummy)
			{
				lmsg->SetMessageNumber(seq);
				lmsg->RefreshIndex();
				lmsg->UncacheMessage();
			}

			// Now do search
			if (lmsg && SearchMessage(lmsg, spec))
				results->push_back(uids ? (*iter).UID() : seq);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Might be cancelled
		if (mPendingAbort)
		{
			mPendingAbort = false;
			mMailbox.clear();
			mCache.clear();
			mIndex.clear();
		}

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Might be cancelled
		if (mPendingAbort)
		{
			mPendingAbort = false;
			mMailbox.clear();
			mCache.clear();
			mIndex.clear();
		}

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set the UIDValidity
void CLocalClient::_SetUIDValidity(unsigned long uidv)
{
	try
	{
		// Set value and write it out
		SIndexHeader header;
		header.UIDValidity() = uidv;
		header.write_UIDValidity(mIndex);

		CHECK_STREAM(mIndex)
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set the UIDNext
void CLocalClient::_SetUIDNext(unsigned long uidn)
{
	try
	{
		// Set value and write it out
		SIndexHeader header;
		header.UIDNext() = uidn;
		header.write_UIDNext(mIndex);

		CHECK_STREAM(mIndex)
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set the last sync
void CLocalClient::_SetLastSync(unsigned long sync)
{
	try
	{
		// Set value and write it out
		SIndexHeader header;
		header.LastSync() = sync;
		header.write_LastSync(mIndex);

		CHECK_STREAM(mIndex)
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

#pragma mark ____________________________Messages

// Fetch sequence number for message with this local UID
unsigned long CLocalClient::_GetMessageLocalUID(unsigned long uid)
{
	ulvector uids;
	uids.push_back(uid);
	ulvector seqs;
	MapFromUIDs(uids, seqs, true);
	
	return seqs.size() ? seqs.front() : 0;
}

// Do fetch envelopes
void CLocalClient::_FetchItems(const ulvector& nums, bool uids, CMboxProtocol::EFetchItems items)
{
	StINETClientAction action(this, "Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Do sequence convert
		ulvector seq;
		ConvertSequence(nums, seq, mIndexList.size());

		// Do possible uid convert
		ulvector actual_nums;
		if (uids)
			MapFromUIDs(seq, actual_nums, false);
		else
			actual_nums = seq;

		// Reset item counter for feedback
		InitItemCtr(actual_nums.size());

		for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		{
			// Provide feedback
			BumpItemCtr("Status::IMAP::MessageFetch");

			// Now read in the envelope and body info
			CLocalMessage* msg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(*iter));
			if (!msg)
				continue;

			FetchMessage(msg, *iter, items);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Get header list from messages
void CLocalClient::_ReadHeaders(const ulvector& nums, bool uids, const cdstring& hdrs)
{
	StINETClientAction action(this, "Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Do sequence convert
		ulvector seq;
		ConvertSequence(nums, seq, mIndexList.size());

		// Do possible uid convert
		ulvector actual_nums;
		if (uids)
			MapFromUIDs(seq, actual_nums, false);
		else
			actual_nums = seq;

		// Filter header components to another stream if required
		cdstrvect hdrsvect;
		if (!hdrs.empty())
		{
			cdstring temp(hdrs);
			char* p = ::strtok(temp.c_str_mod(), " ");
			while(p && *p)
			{
				hdrsvect.push_back(p);
				p = ::strtok(NULL, " ");
			}
		}

		// Reset item counter for feedback
		InitItemCtr(actual_nums.size());

		for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		{
			// Provide feedback
			BumpItemCtr("Status::IMAP::MessageFetch");

			// Get message for this iteration
			CLocalMessage* lmsg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(*iter));
			if (!lmsg)
				continue;

			// Write entire header to stream
			std::ostrstream hdr;
			unsigned long start = lmsg->GetIndexHeaderStart() + lmsg->GetIndexStart();
			unsigned long length = lmsg->GetIndexHeaderLength();
			
			// Always filter crlf
			{
				CStreamFilter filter(new crlf_filterbuf(lendl), &hdr);
				::StreamCopy(mMailbox, filter, start, length);
			}
			hdr << std::ends;

			// Check for specific headers
			if (hdrs.empty())
			{
				// Now grab string from stream and minimise its size
				cdstring temp_hdr(hdr.str());
				hdr.freeze(false);
				lmsg->SetHeader(temp_hdr.grab_c_str());
			}
			else
			{
				// Now grab string from stream and minimise its size
				cdstring temp_hdr(CRFC822::OnlyHeaders(hdr.str(), hdrsvect));
				hdr.freeze(false);
				
				// NB Result of OnlyHeaders may be NULL str but we have to have some
				// pointer in the header variable to prevent another header load
				char* p = temp_hdr.grab_c_str();
				lmsg->SetHeader(p ? p : ::strdup(os_endl));
			}
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Get message header text from server
void CLocalClient::_ReadHeader(CMessage* msg)
{
	StINETClientAction action(this, "Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		CLocalMessage* lmsg = dynamic_cast<CLocalMessage*>(msg);

		std::ostrstream hdr;
		unsigned long start = lmsg->GetIndexHeaderStart() + lmsg->GetIndexStart();
		unsigned long length = lmsg->GetIndexHeaderLength();
		
		// Always filter crlf
		{
			CStreamFilter filter(new crlf_filterbuf(lendl), &hdr);
			::StreamCopy(mMailbox, filter, start, length);
		}
		hdr << std::ends;

		// Now grab string from stream and minimise its size
		cdstring temp_hdr(hdr.str());
		hdr.freeze(false);
		lmsg->SetHeader(temp_hdr.grab_c_str());
		
		// IMPORTANT - if this is called during an append, then this header
		// is part of the message being appended. If the stream position is changed
		// then the append position will be corrupted so we must reset it
		if (mAppendMailbox != NULL)
			mMailbox.seekp(0, std::ios_base::end);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Get attachment data into stream
void CLocalClient::_ReadAttachment(unsigned long msg_num, CAttachment* attach,
									LStream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Always bump down start as its not zero-based
	start--;

	StINETClientAction action(this, "Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		CLocalMessage* lmsg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(msg_num));
		CLocalAttachment* lattach = dynamic_cast<CLocalAttachment*>(attach);

		// Get actual length to read
		unsigned long length = count ? count : (lattach->GetIndexBodyLength() - start);

		// Go to start pos
		start += lattach->GetIndexBodyStart() + lmsg->GetIndexStart();

		// Always filter crlf
		{
			CStreamFilter filter(new crlf_filterbuf(lendl), aStream);
			::StreamCopy(mMailbox, filter, start, length);
		}

		// Must put null
		long temp = 1;
		aStream->PutBytes(cdstring::null_str.c_str(), temp);

		// Set \Seen flag if not peek
		if (!peek && lmsg->IsUnseen())
			lmsg->ChangeFlags(NMessage::eSeen, true);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Copy raw attachment data into stream
void CLocalClient::_CopyAttachment(unsigned long msg_num, CAttachment* attach,
									costream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Always bump down start as its not zero-based
	start--;
	unsigned long length = 0;

	StINETClientAction action(this, "Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		CLocalMessage* lmsg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(msg_num));

		// If no attach read in the entire message body
		if (attach)
		{
			CLocalAttachment* lattach = dynamic_cast<CLocalAttachment*>(attach);

			// Get actual length to read
			length = count ? count : (lattach->GetIndexBodyLength() - start);

			// Adjust to start of stream
			start += lattach->GetIndexBodyStart() + lmsg->GetIndexStart();
		}
		else
		{
			// Get actual length to read
			length = count ? count : (lmsg->GetIndexBodyLength() - start);

			// Adjust to start of stream
			start += lmsg->GetIndexBodyStart() + lmsg->GetIndexStart();
		}

		// Check for filter requirements
		{
			// Look for copy back to self during append
			bool copy_to_same = false;
			std::ostream* f1 = &mMailbox;
			std::ostream* f2 = NULL;
			if (dynamic_cast<CStreamFilter*>(aStream->GetStream()))
				f2 = dynamic_cast<CStreamFilter*>(aStream->GetStream())->GetOStream();
			else
				f2 = aStream->GetStream();
			copy_to_same = (f1 == f2);

			if (copy_to_same)
				::StreamCopyIO(mMailbox, *aStream->GetStream(), start, length);
			else
			{
				CStreamFilter filter(new crlf_filterbuf(aStream->GetEndlType()), aStream->GetStream());
				::StreamCopy(mMailbox, filter, start, length);
			}
		}

		// Set \Seen flag if not peek
		if (!peek && lmsg->IsUnseen())
			lmsg->ChangeFlags(NMessage::eSeen, true);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Set the UID
void CLocalClient::_RemapUID(unsigned long local_uid, unsigned long new_uid)
{
	try
	{
		// Convert local uid to sequence number
		unsigned long seq = 0;
		{
			ulvector local_uids;
			local_uids.push_back(local_uid);
			ulvector seq_nums;
			MapFromUIDs(local_uids, seq_nums, true);
			if (seq_nums.size() != 1)
				return;
			seq = seq_nums.front();
		}

		// Set the new uid in the index cache if different
		if (mIndexList[GetIndex(seq)].UID() != new_uid)
		{
			// Force update of internal cache
			mIndexList[GetIndex(seq)].UID() = new_uid;

			// Force update of disk cache
			mIndex.seekp(sizeof(SIndexHeader) + GetIndex(seq) * sIndexWriteLength);
			mIndexList[GetIndex(seq)].write_UID(mIndex);
			CHECK_STREAM(mIndex)
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Map local to remote UIDs
void CLocalClient::_MapLocalUIDs(const ulvector& uids, ulvector* missing, ulmap* local_map)
{
	// Do mapping
	ulvector temp;
	MapBetweenUIDs(uids, temp, false, missing, local_map);
	
	// Remove any 0 UIDs
	ulvector::iterator remove_end = std::remove_copy(temp.begin(), temp.end(), const_cast<ulvector&>(uids).begin(), 0UL);
	if (remove_end != const_cast<ulvector&>(uids).end())
		const_cast<ulvector&>(uids).erase(remove_end, const_cast<ulvector&>(uids).end());
}

// Set specified flag
void CLocalClient::_SetFlag(const ulvector& nums, bool uids, NMessage::EFlags flags, bool set)
{
	// Determine status info
	const char* status_strid;
	const char* oserr_strid;
	const char* nobad_strid;
	if (flags & NMessage::eAnswered)
	{
		status_strid = "Status::IMAP::MarkingAnswered";
		oserr_strid = "Error::IMAP::OSErrAnsweredMsg";
		nobad_strid = "Error::IMAP::NoBadAnsweredMsg";
	}
	if (flags & NMessage::eFlagged)
	{
		status_strid = "Status::IMAP::MarkingFlagged";
		oserr_strid = "Error::IMAP::OSErrFlaggedMsg";
		nobad_strid = "Error::IMAP::NoBadFlaggedMsg";
	}
	if (flags & NMessage::eDeleted)
	{
		status_strid = (set ? "Status::IMAP::Deleting" : "Status::IMAP::Undeleting");
		oserr_strid = (set ? "Error::IMAP::OSErrDeleteMsg" : "Error::IMAP::OSErrUndeleteMsg");
		nobad_strid = (set ? "Error::IMAP::NoBadDeleteMsg" : "Error::IMAP::NoBadUndeleteMsg");
	}
	if (flags & NMessage::eSeen)
	{
		status_strid = "Status::IMAP::MarkingSeen";
		oserr_strid = "Error::IMAP::OSErrSeenMsg";
		nobad_strid = "Error::IMAP::NoBadSeenMsg";
	}
	if (flags & NMessage::eDraft)
	{
		status_strid = "Status::IMAP::MarkingDraft";
		oserr_strid = "Error::IMAP::OSErrDraftMsg";
		nobad_strid = "Error::IMAP::NoBadDraftMsg";
	}
	if (flags & (NMessage::eMDNSent | NMessage::eError | NMessage::eLabels))
	{
		status_strid = "Status::IMAP::MarkingLabel";
		oserr_strid = "Error::IMAP::OSErrLabelMsg";
		nobad_strid = "Error::IMAP::NoBadLabelMsg";
	}

	StINETClientAction action(this, status_strid, oserr_strid, nobad_strid, GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		bool changed = false;

		// Do sequence convert
		ulvector seq;
		ConvertSequence(nums, seq, mIndexList.size());

		// Do possible uid convert
		ulvector actual_nums;
		if (uids)
			MapFromUIDs(seq, actual_nums, false);
		else
			actual_nums = seq;

		// Do to each message
		for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		{
			// Get the new flags from index cache
			SBitFlags new_flags(mIndexList[GetIndex(*iter)].Flags());
			new_flags.Set(flags, set);

			// Set the new flags in the index cache if different
			if (mIndexList[GetIndex(*iter)].Flags() != new_flags.Get())
			{
				// Force update of internal cache
				mIndexList[GetIndex(*iter)].Flags() = new_flags.Get();

				// Force update of disk cache
				mIndex.seekp(sizeof(SIndexHeader) + GetIndex(*iter) * sIndexWriteLength);
				mIndexList[GetIndex(*iter)].write_Flags(mIndex);
				CHECK_STREAM(mIndex)
				changed = true;
			}

			// Change flags on local message if mailbox is fully open (it may notbe if unseen punt on closed mailbox)
			if (GetCurrentMbox()->IsFullOpen())
			{
				// Get local message
				CLocalMessage* msg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(*iter));

				// Set the new flags - only force update if different
				if (msg->SetFlags(new_flags))
				{
					// Do vsiual update
					CMailControl::MessageChanged(msg);
					mMboxUpdate = true;
				}
			}
		}

		// Flush index file
		if (changed)
		{
			mIndex << std::flush;
			CHECK_STREAM(mIndex)
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}

	// Record action
	if (mRecorder)
	{
		CMailRecord::StActionRecorder record(mRecorder, mRecordID);

		ulvector uid_list;
		ulvector luid_list;
		
		if (uids)
		{
			uid_list = nums;
				
			// Now map from UID to local uids
			MapBetweenUIDs(uid_list, luid_list, true);
		}
		else
		{
			// Now map sequence numbers to uids
			MapToUIDs(nums, uid_list, false);
			MapToUIDs(nums, luid_list, true);
		}

		// Write these out
		mRecorder->SetFlags(uid_list, luid_list, flags, set);
	}
}

// Copy specified message to specified mailbox
void CLocalClient::_CopyMessage(const ulvector& nums, bool uids, CMbox* mbox_to, ulmap& copy_uids)
{
	// Determine if destination open elsewhere
	CLocalClient* dest_client = mbox_to->IsFullOpen() ?
								static_cast<CLocalClient*>(mbox_to->GetMsgProtocol()->mClient) : NULL;

	// NB Can never get here if the mailbox is open in another connection!

	StINETClientAction action(this, "Status::IMAP::Copying", "Error::IMAP::OSErrCopyMsg", "Error::IMAP::NoBadCopyMsg", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Recorded map items
		ulmap ruids;
		ulmap rluids;

		// Start the append operation
		if (dest_client)
			// Take control of destination mailbox
			dest_client->_StartAppend(mbox_to);
		else
			_StartAppend(mbox_to);

		// Do sequence convert
		ulvector seq;
		ConvertSequence(nums, seq, mIndexList.size());

		// Do possible uid convert
		ulvector actual_nums;
		if (uids)
			MapFromUIDs(seq, actual_nums, false);
		else
			actual_nums = seq;

		// Need to know cached state of messages in mailbox
		bool is_full_open = GetCurrentMbox()->IsFullOpen();

		// Reset item counter for feedback
		InitItemCtr(actual_nums.size());

		// Do to each message
		for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
		{
			// Provide feedback
			BumpItemCtr("Status::IMAP::CopyingProgress");

			// Now read in the envelope and body info
			CLocalMessage* msg = is_full_open ? dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(*iter)) : NULL;

			// Message must exist and be cached
			std::auto_ptr<CLocalMessage> lmsg;
			if (!msg || !msg->IsFullyCached())
			{
				// Create temp local message
				lmsg.reset(new CLocalMessage(reinterpret_cast<CMbox*>(NULL)));
				msg = lmsg.get();
				
				// Read its cache details directly
				msg->CacheMessage();
				msg->SetMessageNumber(*iter);
				FetchMessage(msg, *iter, CMboxProtocol::eSummary);
			}

			// Do append
			unsigned long new_uid = 0;
			if (dest_client)
				new_uid = dest_client->AppendMessage(mbox_to, msg, true, true, this);
			else
				new_uid = AppendMessage(mbox_to, msg, false, true, this);
			
			// Store map info
			copy_uids.insert(ulmap::value_type(mIndexList[GetIndex(*iter)].UID(), new_uid));
			if (mIndexList[GetIndex(*iter)].UID())
				ruids.insert(ulmap::value_type(mIndexList[GetIndex(*iter)].UID(), new_uid));
			else
				rluids.insert(ulmap::value_type(mIndexList[GetIndex(*iter)].LocalUID(), new_uid));
		}

		// Done with append
		if (dest_client)
			// Take control of destination mailbox
			mbox_to->GetMsgProtocol()->mClient->_StopAppend(mbox_to);
		else
			_StopAppend(mbox_to);

		// Record action
		if (mRecorder)
		{
			CMailRecord::StActionRecorder record(mRecorder, mRecordID);
			mRecorder->CopyTo(ruids, rluids, mbox_to);
		}
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Must always stop the append
		if (dest_client)
			// Take control of destination mailbox
			mbox_to->GetMsgProtocol()->mClient->_StopAppend(mbox_to);
		else
			_StopAppend(mbox_to);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Must always stop the append
		if (dest_client)
			// Take control of destination mailbox
			mbox_to->GetMsgProtocol()->mClient->_StopAppend(mbox_to);
		else
			_StopAppend(mbox_to);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Do copy message to stream
void CLocalClient::_CopyMessage(unsigned long msg_num, bool uids, costream* aStream, unsigned long count, unsigned long start)
{
	// Always bump down start as its not zero-based
	start--;

	StINETClientAction action(this, "Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Do possible uid convert
		if (uids)
		{
			ulvector nums;
			nums.push_back(msg_num);
			ulvector actual_nums;
			MapFromUIDs(nums, actual_nums, false);
			if (actual_nums.size() == 0)
				return;
			msg_num = actual_nums.front();
		}

		CLocalMessage* lmsg = dynamic_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage(msg_num));
		
		// Do local message copy via stream copy
		CopyMessage(lmsg, aStream, count, start);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

void CLocalClient::_ExpungeMessage(const ulvector& nums, bool uids)
{
	StINETClientAction action(this, "Status::IMAP::Expunging", "Error::IMAP::OSErrExpunge", "Error::IMAP::NoBadExpunge", GetCurrentMbox()->GetName());
	StLocalProcess process(this);

	try
	{
		// Do sequence convert
		ulvector seq;
		ConvertSequence(nums, seq, mIndexList.size());

		// Do possible uid convert
		ulvector actual_nums;
		if (uids)
			MapFromUIDs(seq, actual_nums, false);
		else
			actual_nums = seq;

		for(ulvector::iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
			*iter = GetIndex(*iter);

		ExpungeMessage(actual_nums);
	}
	catch(std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error
		INETHandleError(ex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		CLOG_LOGRETHROW;
		throw;
	}
}

#pragma mark ____________________________Sort/Thread

// Does server-side sorting
bool CLocalClient::_DoesSort(ESortMessageBy sortby) const
{
	// Never!
	return false;
}

// Do server-side sort
void CLocalClient::_Sort(ESortMessageBy sortby, EShowMessageBy show_by, const CSearchItem* search, ulvector* results, bool uids)
{
}
	
// Does server-side threading
bool CLocalClient::_DoesThreading(EThreadMessageBy threadby) const
{
	// Never!
	return false;
}

// Do server-side thread
void CLocalClient::_Thread(EThreadMessageBy threadby, const CSearchItem* search, threadvector* results, bool uids)
{
}

#pragma mark ____________________________ACLs

// Set acl on server
void CLocalClient::_SetACL(CMbox* mbox, CACL* acl)
{
}

// Delete acl on server
void CLocalClient::_DeleteACL(CMbox* mbox, CACL* acl)
{
}

// Get all acls for mailbox from server
void CLocalClient::_GetACL(CMbox* mbox)
{
}

// Get allowed rights for user
void CLocalClient::_ListRights(CMbox* mbox, CACL* acl)
{
}

// Get current user's rights to mailbox
void CLocalClient::_MyRights(CMbox* mbox)
{
}

#pragma mark ____________________________Quotas

// Set quota root values on server
void CLocalClient::_SetQuota(CQuotaRoot* root)
{
}

// Get quota root values from server
void CLocalClient::_GetQuota(CQuotaRoot* root)
{
}

// Get quota roots for a mailbox
void CLocalClient::_GetQuotaRoot(CMbox* mbox)
{
}

#pragma mark ____________________________Handle Errors

// Handle an error condition
void CLocalClient::INETHandleError(std::exception& ex, const char* err_id, const char* nobad_id)
{
	// Check for network or general exception
	CNetworkException* nex = dynamic_cast<CNetworkException*>(&ex);
	CGeneralException* gex = dynamic_cast<CGeneralException*>(&ex);

	// Display alert to user
	INETDisplayError(ex, err_id, nobad_id);
		
	// Mark as handled
	if (nex)
		nex->sethandled();
}

// Descriptor for object error context
const char*	CLocalClient::INETGetErrorDescriptor() const
{
	return "Mailbox: ";
}

#pragma mark ____________________________Local Ops

void CLocalClient::GetNames(const CMbox* mbox, cdstring& mbox_name, cdstring& cache_name, cdstring& index_name) const
{
	cdstring convert = LocalFileName(mbox->GetName(), mbox->GetDirDelim(), GetMboxOwner()->IsDisconnected());
	GetNames(convert, mbox_name, cache_name, index_name);
}

void CLocalClient::GetNames(const char* mbox, cdstring& mbox_name, cdstring& cache_name, cdstring& index_name) const
{
	cdstring wd_name = mCWD;
	wd_name += mbox;
	mbox_name = wd_name + ".mbx";
	cache_name = wd_name + ".mbc";
	index_name = wd_name + ".mbi";
}

// Open cache files
void CLocalClient::OpenCache(CMbox* mbox, cdfstream& mailbox, cdfstream& cache, cdfstream& index,
								SIndexHeader& index_header, SIndexList& index_list, ulvector* index_mapping, bool examine)
{
	// May need to reconstruct
	ValidateCache(mbox, mailbox, cache, index, examine);

	// Read index into cache
	ReadIndex(index, index_header, index_list, index_mapping);

	// Set items in mailbox
	mbox->SetUIDValidity(index_header.UIDValidity());
	mbox->SetUIDNext(index_header.UIDNext());
	mbox->SetLastSync(index_header.LastSync());
}

// Open cache files
void CLocalClient::ValidateCache(CMbox* mbox, cdfstream& mailbox, cdfstream& cache, cdfstream& index, bool examine)
{
	// Assumes cache and index files are open

	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

	bool valid = true;
	bool recovery = true;

	try
	{
		// Open mailbox
		mailbox.clear();
		mailbox.open(mbox_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (examine ? 0 : std::ios_base::out) | std::ios_base::binary));
		CHECK_STREAM(mailbox)

		// Open the cache files
		cache.clear();
		cache.open(cache_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (examine ? 0 : std::ios_base::out) | std::ios_base::binary));
		CHECK_STREAM(cache)

		// Check whether index file is writeable
		mMboxReadWrite = (::access_utf8(index_name, R_OK | W_OK) == 0);
		
		index.clear();
		index.open(index_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (mMboxReadWrite ? std::ios_base::out : 0) | std::ios_base::binary));

		// Do special check for write permission failure - do not rely on access fn as that
		// seems to not always work with locked volumes (e.g. CDs)
		
		if (mMboxReadWrite && index.fail())
		{
#if __dest_os == __mac_os
			if (os_errno == wrPermErr)
#elif __dest_os == __win32_os
			if (os_errno == ERROR_WRITE_PROTECT)
#elif __dest_os == __linux_os
			if (os_errno == EROFS)
#endif
			{
				// Turn off read-write and retry the fopen
				mMboxReadWrite = false;
				index.clear();
				index.open(index_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (mMboxReadWrite ? std::ios_base::out : 0) | std::ios_base::binary));
			}
		}
		CHECK_STREAM(index)

		// Get modtimes
		struct stat mbox_finfo;
		if (::stat_utf8(mbox_name, &mbox_finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}
		struct stat cache_finfo;
		if (::stat_utf8(cache_name, &cache_finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}
		struct stat index_finfo;
		if (::stat_utf8(index_name, &index_finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}

		// Read index header
		SIndexHeader hdr;
		ReadIndexHeader(index, hdr);

		// Compare version
		if (hdr.Version() != CreateIndexVersion())
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		
#if 1
		// New method - check that modtime on index is newer than the other two
		if (((unsigned long)mbox_finfo.st_mtime > (unsigned long)index_finfo.st_mtime) ||
			((unsigned long)cache_finfo.st_mtime > (unsigned long)index_finfo.st_mtime))
#else	
		// Compare modtimes
		// Allow +/- 1 hour offsets to go without error
		unsigned long mbox_file_time = (unsigned long)GetDefinitiveFileTime(mbox_finfo.st_mtime, mbox_name);
		unsigned long cache_file_time = (unsigned long)GetDefinitiveFileTime(cache_finfo.st_mtime, cache_name);
		if ((hdr.MboxModified() != mbox_file_time) && (hdr.MboxModified() != mbox_file_time + 3600) && (hdr.MboxModified() != mbox_file_time - 3600) ||
			(hdr.CacheModified() != cache_file_time) && (hdr.CacheModified() != cache_file_time + 3600) && (hdr.CacheModified() != cache_file_time - 3600))
#endif
		{
			// Ask user whether to re-parse or not using sync task
			CErrorDialogTask* task = new CErrorDialogTask(CErrorDialog::eErrDialog_Caution,
															"ErrorDialog::Btn::Reparse",
															"ErrorDialog::Btn::Continue",
															"ErrorDialog::Btn::Cancel",
															NULL,
															"ErrorDialog::Text::ReparseLocal",
															3,
															mbox->GetAccountName());
			switch(task->Go())
			{
			case CErrorDialog::eBtn1:
			default:
				{
					// Throw exception and let recovery deal with it
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}
			case CErrorDialog::eBtn2:
				{
					// Rewrite header info then continue
					SyncIndexHeader(mbox, mIndex);
					break;
				}
			case CErrorDialog::eBtn3:
				{
					// Turn of recovery and then throw exception
					recovery = false;

					// Throw exception
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		valid = false;

		// Close all files as reconstruct will be attempted
		mailbox.close();
		cache.close();
		index.close();

		// Now force reconstruct if recoveryt requested
		if (recovery)
		{
			Reconstruct(mbox);

			// Now try reopening the mailbox, cache & index files
			try
			{
				mailbox.clear();
				mailbox.open(mbox_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (examine ? 0 : std::ios_base::out) | std::ios_base::binary));
				CHECK_STREAM(mailbox)

				cache.clear();
				cache.open(cache_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (examine ? 0 : std::ios_base::out) | std::ios_base::binary));
				CHECK_STREAM(cache)

				index.clear();
				index.open(index_name, static_cast<std::ios_base::openmode>(std::ios_base::in | std::ios_base::out | std::ios_base::binary));
				CHECK_STREAM(index)
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Close all
				mailbox.close();
				cache.close();
				index.close();

				// Must throw out of here
				CLOG_LOGRETHROW;
				throw;
			}
		}
		else
		{
			// Must throw out of here
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Change streams from read-only <-> read-write
void CLocalClient::SwitchCache(CMbox* mbox, cdfstream& index, cdfstream& mailbox, cdfstream& cache, bool read_only)
{
	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

	bool valid = true;

	// Close/reopen mailbox
	mailbox.close();
	mailbox.clear();
	mailbox.open(mbox_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (read_only ? 0 : std::ios_base::out) | std::ios_base::binary));
	CHECK_STREAM(mailbox)

	// Close/reopen the cache files
	cache.close();
	cache.clear();
	cache.open(cache_name, static_cast<std::ios_base::openmode>(std::ios_base::in | (read_only ? 0 : std::ios_base::out) | std::ios_base::binary));
	CHECK_STREAM(cache)

	// After switching to read-only from read-write we must re-sync the index
	// as cache/mailbox file modtimes may have changed
	if (read_only)
		SyncIndexHeader(mbox, index);
}

// Reconstruct cache and index
void CLocalClient::Reconstruct(CMbox* mbox)
{
	// Sledge hammer approach for now!

	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

	CRFC822Parser* parser = NULL;
	CMessageList* list = NULL;
	bool saved_uids = false;
	try
	{
		// Parse out entire mailbox file
		parser = new CRFC822Parser;

		{
			//StProfileSection profile("\pReconstruct Parse", 200, 20);
			// Must not be open already
			cdfstream fin(mbox_name, std::ios_base::in | std::ios_base::binary);
			CHECK_STREAM(fin)

			CNetworkAttachProgress progress;
			list = parser->ListFromStream(fin, &progress);
			if (!list)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
		}

		// Now try to sync with any existing index/cache files
		SIndexHeader recovered_index;
		ulvector recovered_localuids;
		bool recovered = ReconstructRecover(mbox, list, recovered_index, recovered_localuids);

		//StProfileSection profile("\pReconstruct Write", 200, 20);

		// Write cache while calculating indices
		SIndexList indices;
		indices.reserve(list->size());	// Optimisation for large mailbox
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			StCreatorType file(cMulberryCreator, cMailboxCacheType);
#endif
			cdfstream fout(cache_name, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
			unsigned long uid = 1;
			for(CMessageList::const_iterator iter = list->begin(); iter != list->end(); iter++, uid++)
			{
				SIndexRecord index_item;

				// Always append items to output
				fout.seekp(0, std::ios_base::end);

				// Write empty index to stream first
				index_item.Index() = fout.tellp();
				static_cast<CLocalMessage*>(*iter)->WriteIndexToStream(fout);

				// Write cache to stream next - this will calculate actual indices
				index_item.Cache() = fout.tellp();
				static_cast<CLocalMessage*>(*iter)->WriteCacheToStream(fout);

				// Reposition at previous index write
				fout.seekp(index_item.Index());
				
				// Write out correct indices
				static_cast<CLocalMessage*>(*iter)->WriteIndexToStream(fout);

				// Determine other index file items
				index_item.UID() = recovered ? (*iter)->GetUID() : uid;
				index_item.LocalUID() = recovered ? recovered_localuids.at(uid - 1) : (mSeparateUIDs ? uid : 0);
				index_item.Flags() = (*iter)->GetFlags().Get() & NMessage::eLocalFlags;
				index_item.MessageStart() = static_cast<CLocalMessage*>(*iter)->GetIndexStart();

				indices.push_back(index_item);
			}
		}

		// Write index file
		{
			// Create output stream
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			StCreatorType file(cMulberryCreator, cMailboxIndexType);
#endif
			cdfstream fout(index_name, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

			// Write out new header
			SIndexHeader header;
			header.Version() = CreateIndexVersion();
			header.MboxModified() = 0;
			header.CacheModified() = 0;
			header.LastSync() = recovered ? recovered_index.LastSync() : 0;
			header.IndexSize() = indices.size();

			if (!saved_uids)
			{
				// Must generate UIDValidity value from scratch for ALL types (local and disconnected)
				cdstring hash_key = mbox->GetAccountName();
				hash_key += CRFC822::GetRFC822Date();
				unsigned long hash;
				hash_key.md5(hash);
				mbox->SetUIDValidity(hash);
			}
			if (!mSeparateUIDs)
				mbox->SetUIDNext((indices.size() ? indices.back().UID() : 0) + 1);

			header.UIDValidity() = recovered ? recovered_index.UIDValidity() : mbox->GetUIDValidity();
			header.UIDNext() = recovered ? recovered_index.UIDNext() : mbox->GetUIDNext();
			header.LastUID() = recovered ? recovered_index.LastUID() : (indices.size() ? indices.back().UID() : 0);
			header.LocalUIDNext() = recovered ? recovered_index.LocalUIDNext() : (mSeparateUIDs ? indices.size() + 1 : 0);
			header.write(fout);

			// Sync header modtimes
			SyncIndexHeader(mbox, fout);

			// Add each index entry
			for(SIndexList::const_iterator iter = indices.begin(); iter != indices.end(); iter++)
				(*iter).write(fout);
		}

		delete parser;
		delete list;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete parser;
		delete list;

		// Failure must result in cache and index deletion
		::remove_utf8(cache_name);
		::remove_utf8(index_name);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Reconstruct cache and index
bool CLocalClient::ReconstructRecover(CMbox* mbox, CMessageList* list, SIndexHeader& index_header, ulvector& local_uids, bool force)
{
	// First check to see if mailbox is ours
	if (!force && (GetCurrentMbox() != mbox))
	{
		// Its not ours - we have to create a fake CLocalClient and use that to do
		// the reconstruct to keep the data in this CLocalClient consistent
		std::auto_ptr<CLocalClient> temp(new CLocalClient(*this, mbox->GetProtocol()));
		
		// Reconstruct recovery via the fake but force it to use the mailbox even though
		// its not really current
		return temp->ReconstructRecover(mbox, list, index_header, local_uids, true);
	}

	bool result = false;

	// Check that cache and index exist
	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

	try
	{
		// Open the cache & index files
		mCache.clear();
		mCache.open(cache_name, static_cast<std::ios_base::openmode>(std::ios_base::in | std::ios_base::binary));
		CHECK_STREAM(mCache)

		mIndex.clear();
		mIndex.open(index_name, static_cast<std::ios_base::openmode>(std::ios_base::in | std::ios_base::binary));
		CHECK_STREAM(mIndex)

		// Read index into memory
		ReadIndex(mIndex, index_header, mIndexList, &mIndexMapping);

		// Must be same size - any change in size => mailbox edit - we can't reliably recover from that
		if (index_header.mIndexSize != list->size())
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}
		
		// Create temp message list with fully cached local messages and read them in
		CMessageList temp;
		for(unsigned long i = 1; i <= index_header.mIndexSize; i++)
		{
			CLocalMessage* lmsg = new CLocalMessage((CMbox*) NULL);
			lmsg->SetMessageNumber(i);
			temp.push_back(lmsg);
			FetchMessage(lmsg, i, CMboxProtocol::eSummary);
		}
		
		// Now compare new with old creating a mapping between the two
		
		// Create message-id -> list position mappings for faster compares
		// NB Use multimap as message-ids may be duplicated
		typedef std::multimap<cdstring, unsigned long> msgidmap;
		msgidmap mapid;
		unsigned long index = 0;
		for(CMessageList::const_iterator iter = temp.begin(); iter != temp.end(); iter++, index++)
			mapid.insert(msgidmap::value_type((*iter)->GetEnvelope()->GetMessageID(), index));
		
		// Look at each new message and find one and only one match
		ulvector new2old;
		for(CMessageList::const_iterator iter1 = list->begin(); iter1 != list->end(); iter1++)
		{
			// Do message id lookup
			msgidmap::iterator start = mapid.lower_bound((*iter1)->GetEnvelope()->GetMessageID());
			msgidmap::iterator stop = mapid.upper_bound((*iter1)->GetEnvelope()->GetMessageID());
			
			// Must have a match otherwise message is missing and we can't recover
			if (start == mapid.end())
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			
			// Compare with all possible message-id matches looking for one that's the same
			bool matched = false;
			for(msgidmap::iterator iter2 = start; iter2 != stop; iter2++)
			{
				// See if message content is an exact match
				unsigned long old_index = (*iter2).second;
				const CLocalMessage* lmsg = static_cast<const CLocalMessage*>(temp.at(old_index));
				if (lmsg->RecoverCompare(*static_cast<const CLocalMessage*>(*iter1)))
				{
					// Got a match - add it to mapping list
					new2old.push_back(old_index);
					
					// Remove this item from old map so we don;t find it again
					mapid.erase(iter2);
					
					// Mark the match and exit inner loop
					matched = true;
					break;
				}
			}
			
			// Must have one match otherwise message is missing and we can't recover
			if (!matched)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

		// Now transfer old data to new as everything matches
		index = 0;
		for(CMessageList::iterator iter = list->begin(); iter != list->end(); iter++, index++)
		{
			// Get old message
			const CLocalMessage* lmsg = static_cast<const CLocalMessage*>(temp.at(new2old[index]));
			
			// Copy over flags
			(*iter)->GetFlags() = lmsg->GetFlags();
			
			// Copy over UIDs
			(*iter)->SetUID(lmsg->GetUID());
			local_uids.push_back(mIndexList[GetIndex(new2old[index] + 1)].LocalUID());	// NB Add 1 to new2old value to convert to sequence number
		}
		
		// Recovered!
		result = true;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
	
	// Close out cache files
	mCache.close();
	mIndex.close();

	// Clear out cached data
	mIndexList.clear();
	mIndexMapping.clear();

	return result;
}

// Update index header
void CLocalClient::UpdateIndexSize(cdfstream& index, unsigned long size)
{
	// Set size and write it out
	SIndexHeader header;
	header.IndexSize() = size;
	header.write_IndexSize(index);

	CHECK_STREAM(index)
}

// Update index header
void CLocalClient::UpdateIndexLastUID(cdfstream& index, unsigned long luid)
{
	// Set size and write it out
	SIndexHeader header;
	header.LastUID() = luid;
	header.write_LastUID(index);

	CHECK_STREAM(index)
}

// Update index header
void CLocalClient::SyncIndexHeader(CMbox* mbox, cdfstream& index)
{
	// Assumes cache and index files are open

	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(mbox, mbox_name, cache_name, index_name);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Disk cache means file info not yet updated => flush the volume buffer
	FlushLocalVolume(mbox_name);
#endif

	// Get modtimes on all files
	struct stat mbox_finfo;
	if (::stat_utf8(mbox_name, &mbox_finfo))
	{
		int err_no = os_errno;
		CLOG_LOGTHROW(CGeneralException, err_no);
		throw CGeneralException(err_no);
	}
	struct stat cache_finfo;
	if (::stat_utf8(cache_name, &cache_finfo))
	{
		int err_no = os_errno;
		CLOG_LOGTHROW(CGeneralException, err_no);
		throw CGeneralException(err_no);
	}
	struct stat index_finfo;
	if (::stat_utf8(index_name, &index_finfo))
	{
		int err_no = os_errno;
		CLOG_LOGTHROW(CGeneralException, err_no);
		throw CGeneralException(err_no);
	}

	// Read header to fill in fields
	SIndexHeader header;
	header.read(index);

	// Is it different? Compare the cached modtimes (the old method) and compare the index modtime
	// against the other two files - it its older then we must touch it
	if ((header.MboxModified() != (unsigned long)GetDefinitiveFileTime(mbox_finfo.st_mtime, mbox_name)) ||
		(header.CacheModified() != (unsigned long)GetDefinitiveFileTime(cache_finfo.st_mtime, cache_name)) ||
		((unsigned long)mbox_finfo.st_mtime > (unsigned long)index_finfo.st_mtime) ||
		((unsigned long)cache_finfo.st_mtime > (unsigned long)index_finfo.st_mtime))
	{
		// Change mod times and write header back
		header.MboxModified() = (unsigned long)GetDefinitiveFileTime(mbox_finfo.st_mtime, mbox_name);
		header.CacheModified() = (unsigned long)GetDefinitiveFileTime(cache_finfo.st_mtime, cache_name);
		header.write(index);
		
		index << std::flush;
	}

	CHECK_STREAM(index)
}

// Create index version
unsigned long CLocalClient::CreateIndexVersion()
{
	unsigned long result = cIndexVers;
	
	return result;
}

bool CLocalClient::uid_index_sort(SIndexRecord* rec1, SIndexRecord* rec2)
{
	// Some UIDs might be zero!
	if (rec1->UID() && rec2->UID())
		// Non-zero UIDs - straight compare
		return rec1->UID() < rec2->UID();
	else if (!rec1->UID() && !rec2->UID())
		// Both UIDs are zero - compare local UIDs
		return rec1->LocalUID() < rec2->LocalUID();
	else
		// One non-zero UID sorts above zero UID
		return rec1->UID();
}

// Read index file from disk
void CLocalClient::ReadIndex(cdfstream& in, SIndexHeader& header, SIndexList& index, ulvector* seq)
{
	// Read in the header
	ReadIndexHeader(in, header);

	// Used for sequence number mapping
	SIndexRefList sort_list;
	if (seq)
		sort_list.reserve(header.IndexSize());	// Optimisation for large mailbox

	// Read in index
	index.clear();
	index.reserve(header.IndexSize());	// Optimisation for large mailbox
	for(unsigned long index_pos = 0; index_pos < header.IndexSize(); index_pos++)
	{
		SIndexRecord index_item;
		index_item.read(in);
		index_item.Sequence() = seq ? index_pos : index_pos + 1;

		// Check that no error occurred
		CHECK_STREAM(in)

		index.push_back(index_item);
		if (seq)
			sort_list.push_back(&index.back());
	}

	// Map to ascending sequence numbers
	if (seq)
	{
		seq->clear();
		seq->reserve(index.size());			// Optimisation for large mailbox
		std::stable_sort(sort_list.begin(), sort_list.end(), uid_index_sort);
		unsigned long seq_pos = 1;
		for(SIndexRefList::iterator iter = sort_list.begin(); iter != sort_list.end(); iter++, seq_pos++)
		{
			seq->push_back((*iter)->Sequence());
			(*iter)->Sequence() = seq_pos;
		}
	}
}

// Read index file from disk
void CLocalClient::ReadIndexHeader(cdfstream& in, SIndexHeader& header)
{
	// Read header
	header.read(in);

	// Check that no error occurred
	CHECK_STREAM(in)
}

// Write back single index entry
void CLocalClient::WriteIndex(cdfstream& out, unsigned long index)
{
	mIndex.seekp(sizeof(SIndexHeader) + index * sIndexWriteLength);
	mIndexList[index].write(mIndex);
	CHECK_STREAM(mIndex)
}

// Write back single index entry
void CLocalClient::WriteIndexFlag(cdfstream& out, unsigned long index)
{
	mIndex.seekp(sizeof(SIndexHeader) + index * sIndexWriteLength);
	mIndexList[index].write_Flags(mIndex);
	CHECK_STREAM(mIndex)
}

// Read in this message's index
void CLocalClient::ReadMessageIndex(CLocalMessage* lmsg, ulvector* indices)
{
	// Get seek position for this index
	unsigned long index = GetIndex(lmsg);
	std::istream::pos_type pos_index = mIndexList[index].Index();

	// Get length of index
	unsigned long length = mIndexList[index].Cache() - pos_index;

	// If it fits in work buffer read into buffer, otherwise read direct from stream
	mCache.seekg(pos_index);
	if (length < cWorkBufferSize)
	{
		// Read into work buffer
		mCache.read(mWorkBuffer.c_str_mod(), length);
		CHECK_STREAM(mCache)
		mWorkBuffer[length] = 0;

		// Make work buffer a stream and read it in
		std::istrstream stream(mWorkBuffer.c_str(), length);
		lmsg->ReadIndexFromStream(stream, indices, 0);
	}
	else
	{
		// Seek to position and read index then cache
		lmsg->ReadIndexFromStream(mCache, indices, 0);
		CHECK_STREAM(mCache)
	}

	// Must update message start
	lmsg->SetIndexStart(mIndexList[index].MessageStart());
}

// Read in this message's cache
void CLocalClient::ReadMessageCache(CLocalMessage* lmsg)
{
	// Get seek position for this index
	unsigned long index = GetIndex(lmsg);
	std::istream::pos_type pos_cache = mIndexList[index].Cache();

	// Determine if direct streaming or buffer
	bool use_buffer = false;
	unsigned long length = 0;

	// See if another message after this
	if (index + 1 < mIndexList.size())
	{
		// Get length of cache
		length = mIndexList[index + 1].Index() - pos_cache;
		
		// Use buffer if it fits
		use_buffer = (length < cWorkBufferSize);
	}

	// If it fits in work buffer read into buffer, otherwise read direct from stream
	mCache.seekg(pos_cache);
	if (use_buffer)
	{
		// Read into work buffer
		mCache.read(mWorkBuffer.c_str_mod(), length);
		CHECK_STREAM(mCache)
		mWorkBuffer[length] = 0;

		// Make work buffer a stream and read it in
		std::istrstream stream(mWorkBuffer.c_str(), length);
		lmsg->ReadCacheFromStream(stream, 0);
	}
	else
	{
		// Seek to position and read index then cache
		lmsg->ReadCacheFromStream(mCache, 0);
		CHECK_STREAM(mCache)
	}
}

// Clear all recent flags
void CLocalClient::ClearRecent()
{
	for(unsigned long i = 0; i < mIndexList.size(); i++)
	{
		if (mIndexList[i].Flags() & NMessage::eRecent)
		{
			mIndexList[i].Flags() = mIndexList[i].Flags() & ~NMessage::eRecent;
			mIndex.seekp(sizeof(SIndexHeader) + i * sIndexWriteLength);
			mIndexList[i].write_Flags(mIndex);
			CHECK_STREAM(mIndex)
		}
	}
}

// Map sequence to nums
void CLocalClient::ConvertSequence(const ulvector& sequence, ulvector& nums, unsigned long total)
{
	bool to_end = false;
	unsigned long last_num = 0;
	nums.reserve(sequence.size());	// Optimisation for large mailbox
	for(ulvector::const_iterator iter = sequence.begin(); iter != sequence.end(); iter++)
	{
		if (!*iter)
			to_end = true;
		else
			nums.push_back(*iter);
		last_num = *iter;
	}

	if (to_end && (last_num != total))
	{
		nums.reserve(nums.size() + (total - last_num));	// Optimisation for large mailbox
		for(unsigned long num = last_num + 1; num <= total; num++)
			nums.push_back(num);
	}

}

// Map message numbers to (local) uids
void CLocalClient::MapToUIDs(const ulvector& nums, ulvector& uids, bool local) const
{
	for(ulvector::const_iterator iter = nums.begin(); iter != nums.end(); iter++)
		uids.push_back(local ? mIndexList[GetIndex(*iter)].LocalUID() : mIndexList[GetIndex(*iter)].UID());
}

// Map message (local) uids to numbers
void CLocalClient::MapFromUIDs(const ulvector& uids, ulvector& nums, bool local) const
{
	nums.reserve(uids.size());	// Optimisation for large mailbox
	for(unsigned long i = 0; i < mIndexList.size(); i++)
	{
		ulvector::const_iterator found = std::find(uids.begin(), uids.end(), local ? mIndexList[i].LocalUID() : mIndexList[i].UID());
		if (found != uids.end())
			nums.push_back(mIndexList[i].Sequence());
	}
}

// Map between uids and local uids
void CLocalClient::MapBetweenUIDs(const ulvector& uids, ulvector& luids, bool to_local, ulvector* missing, ulmap* local_map) const
{
	luids.reserve(uids.size());	// Optimisation for large mailbox
	for(unsigned long i = 0; i < mIndexList.size(); i++)
	{
		unsigned long find_it = to_local ? mIndexList[i].UID() : mIndexList[i].LocalUID();
		ulvector::const_iterator found = std::find(uids.begin(), uids.end(), find_it);
		if (found != uids.end())
		{
			unsigned long found_it = to_local ? mIndexList[i].LocalUID() : mIndexList[i].UID();
			luids.push_back(found_it);
			
			// Check for missing item
			if (missing && !luids.back())
				missing->push_back(find_it);

			// Add item to reverse lookup map if key is non-zero
			if (local_map && luids.back())
				local_map->insert(ulmap::value_type(found_it, *found));
		}
	}
	
	std::sort(luids.begin(), luids.end());
}

unsigned long CLocalClient::GetIndex(unsigned long seq) const
{
	return mIndexMapping[seq - 1];
}

void CLocalClient::ScanDirectory(const char* path, const cdstring& pattern, CMboxList* mbox_list, bool first)
{
	// Create lists for directories and mailboxes
	std::auto_ptr<cdstrvect> dirs(new cdstrvect);
	std::auto_ptr<cdstrvect> mboxes(new cdstrvect);

	// Iterate over all .mbx files/directories in directory
	// but not ones that are hidden
	// No longer require Mac OS creator/type match
	diriterator iter(path, true, ".mbx");
	iter.set_return_hidden_files(false);
	const char* p = NULL;
	while(iter.next(&p))
	{
		// Get full path
		cdstring fpath = path;
		fpath += p;

		// Check for directory
		if (iter.is_dir())
		{
			// Add to list of hits
			dirs->push_back(fpath);
		}
		else
		{
			// Remove suffix
			fpath[fpath.length() - 4] = 0;

			// Don't process top level INBOX
			if (!first || ::strcmpnocase(p, "INBOX.mbx"))
			{

				// Add to list of hits if pattern match
				if (fpath.PatternDirMatch(pattern, os_dir_delim))
					mboxes->push_back(fpath);
			}
		}
	}

	// Now process each set

	// Add each mailbox but look to see if its also a directory
	for(cdstrvect::iterator iter = mboxes->begin(); iter != mboxes->end(); iter++)
	{
		// Look in list of dirs
		cdstrvect::iterator found = std::find(dirs->begin(), dirs->end(), *iter);
		if (found == dirs->end())
		{
			// Add as mailbox without hierarchy
			AddMbox(*iter, mbox_list, NMbox::eNoInferiors);
		}
		else
		{
			// Pull directory
			dirs->erase(found);

			// Add as mailbox with hierarchy
			AddMbox(*iter, mbox_list, NMbox::eHasExpanded);

			// Now recurse after adding directory suffix
			cdstring temp_path = *iter;
			temp_path += os_dir_delim;
			ScanDirectory(temp_path, pattern, mbox_list);
		}
	}

	// Add each remaining directory
	for(cdstrvect::iterator iter = dirs->begin(); iter != dirs->end(); iter++)
	{
		// Add as directory if pattern match
		if ((*iter).PatternDirMatch(pattern, os_dir_delim))
			AddMbox(*iter, mbox_list, static_cast<NMbox::EFlags>(NMbox::eNoSelect | NMbox::eHasInferiors));

		// Now recurse after adding directory suffix
		cdstring temp_path = *iter;
		temp_path += os_dir_delim;
		ScanDirectory(temp_path, pattern, mbox_list);
	}
}

void CLocalClient::AddMbox(const char* path_name, CMboxList* mbox_list, NMbox::EFlags flags)
{
	// Provide feedback
	BumpItemCtr("Status::IMAP::MailboxFind");

	// Get name of mbox from full path
	cdstring mbox_name = &path_name[mCWD.length()];

	// Always create
	CMbox* mbox = new CMbox(GetMboxOwner(), mbox_name, os_dir_delim, mbox_list, false);
	mbox->SetFlags(flags);

	// Add to its list - maybe deleted if duplicate
	mbox = mbox->AddMbox();
}

// Do fetch envelopes
void CLocalClient::FetchMessage(CLocalMessage* msg, unsigned long seq, CMboxProtocol::EFetchItems items)
{
	if (items & (CMboxProtocol::eSummary | CMboxProtocol::eEnvelope))
	{
		// Read in message cache before index to ensure attachments are present for indexing
		ReadMessageCache(msg);

		// Read index after cache as we need attachments created before attachment indiced can be read in
		ReadMessageIndex(msg);

		// UID & flags stored elsewhere
		msg->SetUID(mIndexList[GetIndex(seq)].UID());

		// Change flags on cached message - only update if changed
		SBitFlags new_flags(mIndexList[GetIndex(seq)].Flags());
		if (msg->SetFlags(new_flags))
		{
			// Only if message actually owned
			if (msg->GetMbox())
			{
				CMailControl::MessageChanged(msg);
				mMboxUpdate = true;
			}
		}
	}
	else
	{
		if (items & CMboxProtocol::eUID)
			msg->SetUID(mIndexList[GetIndex(seq)].UID());
		if (items & CMboxProtocol::eSize)
		{
			if (!msg->HasIndex())
				ReadMessageIndex(msg);	// <-- will set the message size
		}
		if (items & CMboxProtocol::eFlags)
		{
			// Change flags on cached message - only update if changed
			SBitFlags new_flags(mIndexList[GetIndex(seq)].Flags());
			if (msg->SetFlags(new_flags))
			{
				// Only if message actually owned
				if (msg->GetMbox())
				{
					CMailControl::MessageChanged(msg);
					mMboxUpdate = true;
				}
			}
		}
	}
}

void CLocalClient::ExpungeMessage(ulvector& indices)
{
	// Only bother if we actually have something to do
	if (indices.size() == 0)
		return;

	// This is real complicated!

	// Get names of temp files
	cdstring mbox_name;
	cdstring cache_name;
	cdstring index_name;
	GetNames(GetCurrentMbox(), mbox_name, cache_name, index_name);

	cdstring mbox_temp = mbox_name + ".1";
	cdstring cache_temp = cache_name + ".1";
	cdstring index_temp = index_name + ".1";
	cdstring mbox_old = mbox_name + ".2";
	cdstring cache_old = cache_name + ".2";
	cdstring index_old = index_name + ".2";

	try
	{
		// Create temp file streams (for Mac OS must adjust file creator/type as these files will be created)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		StCreatorType file1(cMulberryCreator, cMailboxType);
#endif
		cdfstream temp_mbox(mbox_temp, std::ios::out | std::ios::binary);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		file1.Reset();
		StCreatorType file2(cMulberryCreator, cMailboxCacheType);
#endif
		cdfstream temp_cache(cache_temp, std::ios::out | std::ios_base::binary | std::ios::trunc);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		file2.Reset();
		StCreatorType file3(cMulberryCreator, cMailboxIndexType);
#endif
		cdfstream temp_index(index_temp, std::ios::out | std::ios_base::binary | std::ios::trunc);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		file3.Reset();
#endif

		unsigned long msg_start_offset = 0;
		unsigned long cache_offset = 0;
		std::istream::pos_type start_copy = 0;

		// Copy in initial index header
		SIndexHeader header;
		header.read(mIndex);
		CHECK_STREAM(mIndex)
		header.write(temp_index);

		// Make sure list of indices to remove is sorted
		std::sort(indices.begin(), indices.end());

		// Copy each index record
		ulvector recorded;	// Used for recording
		recorded.reserve(indices.size());	// Optimisation for large mailbox
		ulvector lrecorded;	// Used for recording
		lrecorded.reserve(indices.size());	// Optimisation for large mailbox

		ulvector seq;	// Used for mailbox removal
		seq.reserve(indices.size());	// Optimisation for large mailbox

		unsigned long pos = 0;
		for(SIndexList::const_iterator iter = mIndexList.begin(); iter != mIndexList.end(); iter++, pos++)
		{
			if (pos == indices.front())
			{
				// Pop off the front
				indices.erase(indices.begin());

				// Just bump offsets by cache/message length
				if (iter != mIndexList.end() - 1)
				{
					cache_offset += (*(iter + 1)).Index() - (*iter).Index();
					msg_start_offset += (*(iter + 1)).MessageStart() - (*iter).MessageStart();
				}

				// Flag message removal
				mMboxReset = GetCurrentMbox()->IsFullOpen();
				mMboxUpdate = true;

				// Add sequence number to list to be removed
				if (mMboxReset)
					seq.push_back((*iter).Sequence());

				// Adjust index size
				header.IndexSize()--;

				// Add to recorder array
				recorded.push_back(mRecordLocalUIDs ? 0 : (*iter).UID());
				lrecorded.push_back((*iter).LocalUID());
			}
			else
			{
				// Copy index entry intact, but with offsets applied
				SIndexRecord new_record = *iter;
				new_record.Cache() -= cache_offset;
				new_record.Index() -= cache_offset;
				new_record.MessageStart() -= msg_start_offset;
				new_record.write(temp_index);

				// Must update start in local message (NB may not be cached so might come back as NULL)
				CLocalMessage* lmsg = static_cast<CLocalMessage*>(GetCurrentMbox()->GetMessage((*iter).Sequence()));
				if (lmsg)
					lmsg->SetIndexStart(new_record.MessageStart());

				// Copy cache entry intact
				unsigned long length_cache = 0xFFFFFFFF;
				if (iter != mIndexList.end() - 1)
					length_cache = (*(iter + 1)).Index() - (*iter).Index();
				else
				{
					mCache.seekp((*iter).Index());
					length_cache = ::StreamLength(mCache);
				}
				::StreamCopy(mCache, temp_cache, (*iter).Index(), length_cache);

				// Just bump offset by message length
				unsigned long start_msg = (*iter).MessageStart();
				unsigned long length_msg = 0xFFFFFFFF;
				if (iter != mIndexList.end() - 1)
					length_msg = (*(iter + 1)).MessageStart() - start_msg;
				else
				{
					mMailbox.seekp(start_msg);
					length_msg = ::StreamLength(mMailbox);
				}

				// Copy mailbox entry intact
				::StreamCopy(mMailbox, temp_mbox, start_msg, length_msg);
			}
		}

		// Now do mailbox removal
		if (mMboxReset)
		{
			// Sort sequence numbers to make this easier
			std::sort(seq.begin(), seq.end());

			// Remove each squence number, but adjust for ones removed
			unsigned long offset = 0;
			for(ulvector::const_iterator iter = seq.begin(); iter != seq.end(); iter++, offset++)
				GetCurrentMbox()->RemoveMessage(*iter - offset);
		}

		// Now close and rename old ones
		mMailbox.close();
		CHECK_STREAM(mMailbox)
		mCache.close();
		CHECK_STREAM(mCache)
		mIndex.close();
		CHECK_STREAM(mIndex)
		::rename_utf8(mbox_name, mbox_old);
		::rename_utf8(cache_name, cache_old);
		::rename_utf8(index_name, index_old);

		// Rename new ones
		temp_mbox.close();
		temp_cache.close();
		temp_index.close();
		::rename_utf8(mbox_temp, mbox_name);
		::rename_utf8(cache_temp, cache_name);
		::rename_utf8(index_temp, index_name);

		// Remove old ones
		::remove_utf8(mbox_old);
		::remove_utf8(cache_old);
		::remove_utf8(index_old);

		// Force update of index header
		mIndex.clear();
		mIndex.open(index_name, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
		CHECK_STREAM(mIndex)
		UpdateIndexSize(mIndex, header.IndexSize());
		SyncIndexHeader(GetCurrentMbox(), mIndex);

		// Now select it to reopen/recache
		mMailbox.clear();
		mMailbox.open(mbox_name, std::ios_base::in | std::ios_base::binary);
		CHECK_STREAM(mMailbox)
		mCache.clear();
		mCache.open(cache_name, std::ios_base::in | std::ios_base::binary);
		CHECK_STREAM(mCache)

		// Read index into cache
		ReadIndex(mIndex, header, mIndexList, &mIndexMapping);
		CHECK_STREAM(mIndex)

		// Record action
		if (mRecorder)
		{
			CMailRecord::StActionRecorder record(mRecorder, mRecordID);
			mRecorder->Expunge(recorded, lrecorded);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove temp files
		::remove_utf8(mbox_temp);
		::remove_utf8(cache_temp);
		::remove_utf8(index_temp);
	}
}

void CLocalClient::CheckFromIndex(CMbox* mbox, const SIndexList& index)
{
	bool changed = false;

	// EXISTS
	unsigned long size = index.size();
	unsigned long old_size = mbox->GetNumberFound();

	// Force update of window if change
	if (size != old_size)
	{
		if (GetCurrentMbox() == mbox)
		{
			mMboxReload = mbox->IsFullOpen();
			mMboxUpdate = true;
			mMboxNew += size - old_size;
		}
		else
			changed = true;
	}
	mbox->SetNumberFound(size);

	// Count number of recent messages
	unsigned long num = 0;
	for(unsigned long i = 0; i != size; i++)
		if (index[i].Flags() & NMessage::eRecent) num++;

	// Force update of window if change
	if (num != mbox->GetNumberRecent())
	{
		if (GetCurrentMbox() == mbox)
		{
			mMboxReset = mbox->IsFullOpen();
			mMboxUpdate = true;
		}
		else
			changed = true;
	}
	mbox->SetNumberRecent(num);

	// Count number of unseen messages
	num = 0;
	for(unsigned long i = 0; i != size; i++)
		if (!(index[i].Flags() & NMessage::eSeen)) num++;

	// Force update of window if change
	if (num != mbox->GetNumberUnseen())
	{
		if (GetCurrentMbox() == mbox)
			mMboxUpdate = true;
		else
			changed = true;
	}
	mbox->SetNumberUnseen(num);

	// Force immediate UI update if changed
	if (changed)
		// Do update
		CMailControl::MboxUpdate(mbox);
}

void CLocalClient::CopyMessage(const CLocalMessage* lmsg, costream* aStream, unsigned long count, unsigned long start)
{
	// Get actual length to read (adjust to bump past the UNIX header tag)
	unsigned long length = count ? count : (lmsg->GetIndexLength() - lmsg->GetIndexHeaderStart() - start);

	// Adjust to start of stream
	start += lmsg->GetIndexStart() + lmsg->GetIndexHeaderStart();

	// Check for filter requirements
	{
		// Look for copy back to self during append
		bool copy_to_same = false;
		std::ostream* f1 = &mMailbox;
		std::ostream* f2 = NULL;
		if (dynamic_cast<CStreamFilter*>(aStream->GetStream()))
			f2 = dynamic_cast<CStreamFilter*>(aStream->GetStream())->GetOStream();
		else
			f2 = aStream->GetStream();
		copy_to_same = (f1 == f2);

		if (copy_to_same)
			::StreamCopyIO(mMailbox, *aStream->GetStream(), start, length);
		else
		{
			CStreamFilter filter(new crlf_filterbuf(aStream->GetEndlType()), aStream->GetStream());
			::StreamCopy(mMailbox, filter, start, length);
		}
	}
}

unsigned long CLocalClient::AppendMessage(CMbox* mbox, CMessage* msg, bool add,
											bool copying, CLocalClient* copier, bool dummy_files)
{
	// Need to from-stuff the stream as written to the mailbox
	CStreamFilter from_stuff(new fromstuff_filterbuf(true), mAppendMailbox);

	// Create stream type object
	costream stream_out(&from_stuff, mEndl);

	// Seek to end of mailbox stream
	mAppendMailbox->seekp(0, std::ios_base::end);

	// Now at starting point for append
	std::istream::pos_type old_start = mAppendMailbox->tellp();

	// Must add line between any previous message and this one
	if (old_start)
	{
		*mAppendMailbox << stream_out.endl();
		CHECK_STREAM(*mAppendMailbox)
	}

	// Now at starting point for this message
	std::istream::pos_type start = mAppendMailbox->tellp();

	// Add UNIX mailbox header
	*mAppendMailbox << "From ";
	if (msg->GetEnvelope()->GetFrom()->size())
		*mAppendMailbox << msg->GetEnvelope()->GetFrom()->front()->GetMailAddress();
	else
		*mAppendMailbox << "Mulberry_Local";
	*mAppendMailbox << " " << CRFC822::GetUNIXDate(msg->GetInternalDate(), msg->GetInternalZone()) << stream_out.endl();
	CHECK_STREAM(*mAppendMailbox)

	// Now at starting point for the message header
	std::istream::pos_type start_hdr = mAppendMailbox->tellp();

	// Catch errors and rollback the append
	try
	{
		// Stream message into file or do direct copy
		if (!copying)
			// (indicate we want a copy of the original message)
			msg->WriteToStream(stream_out, dummy_files, NULL, true);
		else
			// Must be local message being copied locally
			copier->CopyMessage(dynamic_cast<CLocalMessage*>(msg), &stream_out, 0, 0);
		from_stuff.flush();
		mAppendMailbox->flush();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Force flush both streams to get data in sync before rollback
		try
		{
			from_stuff.flush();
			mAppendMailbox->flush();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

		}

		// Rollback append mailbox
		RollbackAppend(mbox, mAppendMailbox, old_start);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	CHECK_STREAM(*mAppendMailbox)

	CLocalMessage* lmsg = NULL;
	CLocalMessage* temp = NULL;
	long index_offset = 0;

	// No longer use cache from a local message as we cannot be sure what the endl type is
	// and whether that will get converted by the filter to something else and thus invalidate
	// the cached indices

	// If its already local and cached just use existing index info (not sub-message)
	bool local_copy = false;
#if 0
	if (dynamic_cast<CLocalMessage*>(msg) && msg->IsFullyCached() && !msg->IsSubMessage())
	{
		// Check that endls match
		if (copier ||
			msg->GetMbox() && (dynamic_cast<CLocalClient*>(msg->GetMbox()->GetProtocol()->mClient)->mEndl == mEndl))
		{
			lmsg = dynamic_cast<CLocalMessage*>(msg);

			// Adjust index offset since UNIX header line may change in size
			index_offset = (start_hdr - start) - lmsg->GetIndexHeaderStart();
			
			local_copy = true;
		}
	}
#endif

	// Now always reparse the message when its appended
	if (!local_copy)
	{
		// Reposition
		mAppendMailbox->seekp(start);

		// Parse it
		CRFC822Parser parser;
		CNetworkAttachProgress progress;
		lmsg = temp = parser.MessageFromStream(*mAppendMailbox, &progress);

		// Parser will leave it in failed state
		mAppendMailbox->clear();
	}

	// Read index header
	SIndexHeader hdr;
	ReadIndexHeader(*mAppendIndex, hdr);
	
	// Determine new UID based on synchronise status
	unsigned long new_uid = 0;
	unsigned long new_local_uid = 0;
	if (mSeparateUIDs)
	{
		// Copy incoming message's UID is synchronising
		new_uid = mbox->IsSynchronising() ? msg->GetUID() : 0;
		new_local_uid = hdr.LocalUIDNext();
		
		// Need to update next local UID
		hdr.LocalUIDNext()++;
		hdr.write_LocalUIDNext(*mAppendIndex);
	}
	else
	{
		// Just use 
		new_uid = hdr.LastUID() + 1;
		new_local_uid = 0;
	}

	// Store new index here
	SIndexRecord index_item;

	// Add to end of cache file
	mAppendCache->seekp(0, std::ios_base::end);

	// Write empty indices first
	index_item.Index() = mAppendCache->tellp();
	lmsg->WriteIndexToStream(*mAppendCache, index_offset);
	CHECK_STREAM(*mAppendCache)

	// Now write cache - this will calculate actual indices
	index_item.Cache() = mAppendCache->tellp();
	lmsg->WriteCacheToStream(*mAppendCache);
	CHECK_STREAM(*mAppendCache)

	// Move back to indices write pos and write out correct indices
	mAppendCache->seekp(index_item.Index());
	lmsg->WriteIndexToStream(*mAppendCache, index_offset);
	CHECK_STREAM(*mAppendCache)

	index_item.Flags() = (msg->GetFlags().Get() & NMessage::eLocalFlags) | NMessage::eRecent;	// New messages always recent
	index_item.UID() = new_uid;
	index_item.LocalUID() = new_local_uid;
	index_item.MessageStart() = start;

	// Done with any temporary message
	delete temp;

	// Add index to internal cache
	if (add)
	{
		// Update sequence number
		index_item.Sequence() = mIndexList.size() + 1;
		
		// Add item to index list
		mIndexList.push_back(index_item);
		mIndexMapping.push_back(index_item.Sequence() - 1);
	}

	// Write new index total
	unsigned long new_size;
	if (add)
		new_size = mIndexList.size();
	else
		new_size = hdr.IndexSize() + 1;

	// Force updates of mailbox and cache and write new index header
	UpdateIndexSize(*mAppendIndex, new_size);

	// Write out new last uid
	if (hdr.LastUID() < new_uid)
		UpdateIndexLastUID(*mAppendIndex, new_uid);

	// Write new indices at end
	mAppendIndex->seekp(0, std::ios_base::end);
	index_item.write(*mAppendIndex);
	CHECK_STREAM(*mAppendCache)

	// Return local uid if recording uids
	return mSeparateUIDs ? new_local_uid : new_uid;
}

void CLocalClient::RollbackAppend(CMbox* mbox, cdfstream* stream, std::istream::pos_type old_start)
{
	// Rollback append mailbox
	try
	{
		// The OS independent way to do this is to rewrite the file as
		// there is no stdio function to set the EOF marker on a file
		
		// Get names of temp files
		cdstring mbox_name;
		cdstring cache_name;
		cdstring index_name;
		GetNames(mbox, mbox_name, cache_name, index_name);

		cdstring mbox_temp = mbox_name + ".1";
		cdstring mbox_old = mbox_name + ".2";
	
		// Create temp file streams (for Mac OS must adjust file creator/type as these files will be created)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		StCreatorType file1(cMulberryCreator, cMailboxType);
#endif
		cdfstream temp_mbox(mbox_temp, std::ios::out | std::ios::binary);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		file1.Reset();
#endif

		// Now copy from original (clearing errors first) to temp upto the start position of this append
		stream->close();
		stream->clear();
		stream->open(mbox_name, std::ios_base::in | std::ios_base::binary);
		CHECK_STREAM(*stream)
		::StreamCopy(*stream, temp_mbox, 0, old_start);
		
		// Close both streams
		stream->close();
		temp_mbox.close();
		
		// Rename
		::rename_utf8(mbox_name, mbox_old);
		::rename_utf8(mbox_temp, mbox_name);
		::remove_utf8(mbox_old);
		
		// Now reopen the stream
		stream->clear();
		stream->open(mbox_name, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
		CHECK_STREAM(*stream)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

#pragma mark ____________________________Searching

bool CLocalClient::SearchMessage(const CLocalMessage* lmsg, const CSearchItem* spec)
{
	switch(spec->GetType())
	{
	case CSearchItem::eAll:
		return true;

	case CSearchItem::eAnd:
		{
			const CSearchItemList* list = static_cast<const CSearchItemList*>(spec->GetData());
			bool result = true;
			for(CSearchItemList::const_iterator iter = list->begin(); result && (iter != list->end()); iter++)
				result = SearchMessage(lmsg, *iter);
			return result;
		}

	case CSearchItem::eAnswered:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eAnswered;

	case CSearchItem::eBcc:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search for address
		return AddressSearch(lmsg, spec);

	case CSearchItem::eBefore:
		return DateCompare(InternalDateRead(lmsg), spec->ResolveDate(), -1);

	case CSearchItem::eBody:
		// Do text only search
		return TextSearch(lmsg, *static_cast<const cdstring*>(spec->GetData()), lmsg->GetIndexStart(), false);

	case CSearchItem::eCC:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search for address
		return AddressSearch(lmsg, spec);

	case CSearchItem::eDeleted:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eDeleted;

	case CSearchItem::eDraft:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eDraft;

	case CSearchItem::eFlagged:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eFlagged;

	case CSearchItem::eFrom:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search for address
		return AddressSearch(lmsg, spec);

	case CSearchItem::eHeader:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Do text only search
		return HeaderSearch(lmsg,
							static_cast<const cdstrpair*>(spec->GetData())->first,
							static_cast<const cdstrpair*>(spec->GetData())->second,
							lmsg->GetIndexHeaderStart() + lmsg->GetIndexStart());

	case CSearchItem::eKeyword:
		return false;

	case CSearchItem::eLabel:
	{
		unsigned long index = reinterpret_cast<unsigned long>(spec->GetData());
		if (index < NMessage::eMaxLabels)
			return mIndexList[GetIndex(lmsg)].Flags() & (NMessage::eLabel1 << index);
		else
			return false;
	}

	case CSearchItem::eLarger:
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));
		return lmsg->GetIndexLength() > reinterpret_cast<unsigned long>(spec->GetData());

	case CSearchItem::eNew:
		return (mIndexList[GetIndex(lmsg)].Flags() & NMessage::eRecent) &&
				!(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eSeen);

	case CSearchItem::eNot:
		return !SearchMessage(lmsg, reinterpret_cast<const CSearchItem*>(spec->GetData()));

	case CSearchItem::eNumber:
		{
			const ulvector* nums = static_cast<const ulvector*>(spec->GetData());
			return find(nums->begin(), nums->end(), lmsg->GetMessageNumber()) != nums->end();
		}

	case CSearchItem::eOld:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eRecent);

	case CSearchItem::eOn:
		return DateCompare(InternalDateRead(lmsg), spec->ResolveDate(), 0);

	case CSearchItem::eOr:
		{
			const CSearchItemList* list = static_cast<const CSearchItemList*>(spec->GetData());
			bool result = false;
			for(CSearchItemList::const_iterator iter = list->begin(); !result && (iter != list->end()); iter++)
				result = SearchMessage(lmsg, *iter);
			return result;
		}
	case CSearchItem::eRecent:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eRecent;

	case CSearchItem::eSeen:
		return mIndexList[GetIndex(lmsg)].Flags() & NMessage::eSeen;

	case CSearchItem::eSender:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search for address
		return AddressSearch(lmsg, spec);

	case CSearchItem::eSentBefore:
		return DateCompare(DateRead(lmsg), spec->ResolveDate(), -1);

	case CSearchItem::eSentOn:
		return DateCompare(DateRead(lmsg), spec->ResolveDate(), 0);

	case CSearchItem::eSentSince:
		return DateCompare(DateRead(lmsg), spec->ResolveDate(), 1);

	case CSearchItem::eSince:
		return DateCompare(InternalDateRead(lmsg), spec->ResolveDate(), 1);

	case CSearchItem::eSmaller:
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));
		return lmsg->GetIndexLength() < reinterpret_cast<unsigned long>(spec->GetData());

	case CSearchItem::eSubject:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search - hdr encoding aware
		return StreamSearch1522(mCache,
							lmsg->GetEnvelopeIndex().GetSubjectIndex() + mIndexList[GetIndex(lmsg)].Cache(),
							lmsg->GetEnvelopeIndex().GetSubjectLength(),
							*static_cast<const cdstring*>(spec->GetData()));

	case CSearchItem::eText:
		// Search header then body
		return TextSearch(lmsg, *static_cast<const cdstring*>(spec->GetData()), lmsg->GetIndexStart(), true);

	case CSearchItem::eTo:
		// Must have index
		if (!lmsg->HasIndex())
			ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

		// Search for address
		return AddressSearch(lmsg, spec);

	case CSearchItem::eUID:
		{
			const ulvector* uids = static_cast<const ulvector*>(spec->GetData());
			return std::find(uids->begin(), uids->end(), mIndexList[GetIndex(lmsg)].UID()) != uids->end();
		}

	case CSearchItem::eUnanswered:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eAnswered);

	case CSearchItem::eUndeleted:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eDeleted);

	case CSearchItem::eUndraft:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eDraft);

	case CSearchItem::eUnflagged:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eFlagged);

	case CSearchItem::eUnkeyword:
		return false;

	case CSearchItem::eUnseen:
		return !(mIndexList[GetIndex(lmsg)].Flags() & NMessage::eSeen);

	case CSearchItem::eCorrespondent:
		{
			CSearchItem temp(*spec);
			temp.SetType(CSearchItem::eFrom);
			if (SearchMessage(lmsg, &temp))
				return true;
		}
		// Fall through to recipients

	case CSearchItem::eRecipient:
		{
			CSearchItem temp(*spec);
			temp.SetType(CSearchItem::eTo);
			if (SearchMessage(lmsg, &temp))
				return true;

			temp.SetType(CSearchItem::eCC);
			if (SearchMessage(lmsg, &temp))
				return true;

			temp.SetType(CSearchItem::eBcc);
			if (SearchMessage(lmsg, &temp))
				return true;
		}	
		return false;

	case CSearchItem::eNamedStyle:
		{
			// Get named style from prefs
			const cdstring* temp = static_cast<const cdstring*>(spec->GetData());
			const CSearchStyle* style = CPreferences::sPrefs->mSearchStyles.GetValue().FindStyle(*temp);

			// Add style to this list
			if (style)
				return SearchMessage(lmsg, style->GetSearchItem());
			else
				return false;
		}

	default:
		return false;
	}
}

bool CLocalClient::AddressSearch(const CLocalMessage* lmsg, const CSearchItem* spec)
{
	cdstrvect items;

	// Check for smart address
	if (spec->GetData() == NULL)
	{
		// Do default identity
		cdstring addr_txt = CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom();
		CAddress addr(addr_txt);
		items.push_back(addr.GetMailAddress());

		// Now do each smart address
		const cdstrvect& addrs = CPreferences::sPrefs->mSmartAddressList.GetValue();
		items.insert(items.end(), addrs.begin(), addrs.end());
	}
	else
		// Just use provided data
		items.push_back(*static_cast<const cdstring*>(spec->GetData()));
	
	// Do search over each item
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		bool result = false;
		
		switch(spec->GetType())
		{
		case CSearchItem::eBcc:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetBccIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetBccLength(),
								*iter);
			break;

		case CSearchItem::eCC:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetCCIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetCCLength(),
								*iter);
			break;

		case CSearchItem::eFrom:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetFromIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetFromLength(),
								*iter);
			break;

// Reply To search not done right now
/*		case CSearchItem::eReplyTo:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetReplyToIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetReplyToLength(),
								*iter);
			break;
*/
		case CSearchItem::eSender:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetSenderIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetSenderLength(),
								*iter);
			break;

		case CSearchItem::eTo:
			// Search - hdr encoding aware
			result = StreamSearch1522(mCache,
								lmsg->GetEnvelopeIndex().GetToIndex() + mIndexList[GetIndex(lmsg)].Cache(),
								lmsg->GetEnvelopeIndex().GetToLength(),
								*iter);
			break;

		default:;
		}
		
		if (result)
			return true;
	}
	
	return false;
}

bool CLocalClient::HeaderSearch(const CLocalMessage* lmsg, const cdstring& hdr, const cdstring& txt, unsigned long start)
{
	unsigned long ctr = start - 1;								// Subtract first line end (it will be the UNIX From line)
	unsigned long length = lmsg->GetIndexHeaderLength() + 1;	// Account for first line end

	// Header to match
	cdstring actual_hdr = '\r';
	actual_hdr += hdr + ":";

	// Get buffer for header
	cdstring hdrs;
	char* buf = NULL;
	if (length < cSearchBufferSize)
		buf = mSearchBuffer;
	else
	{
		hdrs.reserve(length + 1);
		buf = hdrs.c_str_mod();
	}
	
	// Read entire header into buffer
	mMailbox.seekg(ctr);
	mMailbox.read(buf, length);
	buf[length] = 0;

	// Convert all \n to \r for tests
	char* p = buf;
	while(*p)
	{
		if (*p == '\n')
			*p = '\r';
		p++;
	}
	
	// Now look for matching header
	p = ::strstrnocase(buf, actual_hdr);
	
	// Leave if not found
	if (!p)
		return false;
	
	// Punt over the header
	p += actual_hdr.length();

	// Unfold lines
	CRFC822::UnfoldLines(p);

	// Now punt to end of line and terminate
	char* q = p;
	while(*q && (*q != '\r') && (*q != '\n'))
		q++;
	*q = 0;
	
	// Copy line and convert
	cdstring line(p);
	CRFC822::TextFrom1522(line);

	// Now do search again
	p = ::strstrnocase(line, txt);
	
	return (p != NULL);
}

bool CLocalClient::TextSearch(const CLocalMessage* lmsg, const cdstring& txt, unsigned long start, bool do_header)
{
	// If cached do search via cached attachment indices
	if (lmsg->IsFullyCached())
	{
		return (do_header &&
					StreamSearch(mMailbox, lmsg->GetIndexStart() + lmsg->GetIndexHeaderStart(),
									lmsg->GetIndexHeaderLength(), txt, e7bitEncoding)) ||
				TextAttachmentSearch(dynamic_cast<const CLocalAttachment*>(lmsg->GetBody()), txt, lmsg->GetIndexStart());
	}
	else
	{
		// Read in text attachment indices
		ulvector indices;
		ReadMessageIndex(const_cast<CLocalMessage*>(lmsg), &indices);

		// Do text only search
		return (do_header &&
					StreamSearch(mMailbox, lmsg->GetIndexStart() + lmsg->GetIndexHeaderStart(),
									lmsg->GetIndexHeaderLength(), txt, e7bitEncoding)) ||
				TextIndexSearch(indices, txt, lmsg->GetIndexStart());
	}
}

bool CLocalClient::TextAttachmentSearch(const CLocalAttachment* lattach, const cdstring& txt, unsigned long start)
{
	if (!lattach)
		return false;
	else if (lattach->IsText())
		return StreamSearch(mMailbox, start + lattach->GetIndexBodyStart(), lattach->GetIndexBodyLength(), txt, lattach->GetContent().GetTransferEncoding());
	else if (lattach->IsMessage() && lattach->GetMessage())
		return TextAttachmentSearch(dynamic_cast<const CLocalAttachment*>(lattach->GetMessage()->GetBody()), txt, start);
	else if (lattach->IsMultipart() && lattach->GetParts())
	{
		for(CAttachmentList::const_iterator iter = lattach->GetParts()->begin(); iter != lattach->GetParts()->end(); iter++)
		{
			if (TextAttachmentSearch(dynamic_cast<const CLocalAttachment*>(*iter), txt, start))
				return true;
		}
	}

	return false;
}

bool CLocalClient::TextIndexSearch(const ulvector& indices, const cdstring& txt, unsigned long start)
{
	for(ulvector::const_iterator iter = indices.begin(); iter != indices.end(); iter++)
	{
		if (StreamSearch(mMailbox, start + *iter++, *iter++, txt, static_cast<EContentTransferEncoding>(*iter)))
			return true;
	}

	return false;
}

bool CLocalClient::DateCompare(time_t date1, time_t date2, int comp) const
{
	// Convert each date to 12.00am
	struct tm* date_spec1 = ::localtime(&date1);
	date_spec1->tm_sec = 0;
	date_spec1->tm_min = 0;
	date_spec1->tm_hour = 0;
	date1 = ::mktime(date_spec1);

	struct tm* date_spec2 = ::localtime(&date2);
	date_spec2->tm_sec = 0;
	date_spec2->tm_min = 0;
	date_spec2->tm_hour = 0;
	date2 = ::mktime(date_spec2);

	if (comp == 0)
		return (date1 == date2);
	else if (comp < 0)
		return date1 < date2;
	else if (comp > 0)
		return date2 < date1;
	else
		return false;
}

time_t CLocalClient::DateRead(const CLocalMessage* lmsg)
{
	// Must have index
	if (!lmsg->HasIndex())
		ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

	mCache.seekg(mIndexList[GetIndex(lmsg)].Cache() + lmsg->GetEnvelopeIndex().GetDateIndex());
	time_t date;
	::ReadHost(mCache, date);
	return date;
}

time_t CLocalClient::InternalDateRead(const CLocalMessage* lmsg)
{
	// Must have index
	if (!lmsg->HasIndex())
		ReadMessageIndex(const_cast<CLocalMessage*>(lmsg));

	mCache.seekg(mIndexList[GetIndex(lmsg)].Cache());
	time_t date;
	::ReadHost(mCache, date);
	return date;
}

bool CLocalClient::StreamSearch(std::istream& in, unsigned long start, unsigned long length, const cdstring& txt, EContentTransferEncoding cte)
{
	// Only bother if something to search
	if (!length)
		return false;

	cdstring pattern = txt;

	// Set initial stream pos
	in.seekg(start);

	std::auto_ptr<CStreamFilter> filter;

	// May need to filter
	switch(cte)
	{
	case eNoTransferEncoding:
	case e7bitEncoding:
	case e8bitEncoding:
		// Do nothing
		break;
	case eQuotedPrintableEncoding:
		// Convert from QP
		filter.reset(new CStreamFilter (new mime_qp_filterbuf(false)));
		filter->SetBuffer(mSearchBuffer, cSearchBufferSize);
		break;
	case eBase64Encoding:
		// Convert from base64
		filter.reset(new CStreamFilter (new mime_base64_filterbuf(false)));
		filter->SetBuffer(mSearchBuffer, cSearchBufferSize);
		break;
	default:;
	}
	
	// Loop reading into buffer
	//unsigned long comp_len = pattern.length();
	unsigned long pat_pos = 0;
	while(length)
	{
		// Determine length to read into buffer
		unsigned long read_length = std::min(length, cSearchBufferSize);
		unsigned long search_length = read_length;

		// Read from stream
		in.read(mSearchBuffer, search_length);
		CHECK_STREAM(in)

		// Look for filter
		if (filter.get())
		{
			filter->write(mSearchBuffer, read_length);
			filter->sync();
			search_length = filter->GetWritten();
		}

		// Do brute search
		bool result = SearchBuffer(mSearchBuffer, search_length, pattern, pat_pos, true);
		if (result)
			return true;
		
		// Adjust length
		length -= read_length;
	}
	
	// Must have failed if we get here
	return false;
}

bool CLocalClient::SearchBuffer(const char* str, unsigned long n, const char* pat, unsigned long& pat_pos, bool crlf_convert)
{
	const unsigned char* s1 = (const unsigned char *) str;
	const unsigned char* p1 = (const unsigned char *) pat;
	const unsigned char* p2 = p1 + pat_pos;
	
	unsigned char firstp = tolower(*p1);
	unsigned char firstc = tolower(*p2);

	while(n--)
	{
		// Compare first character with current
		unsigned char c1 = tolower(*s1++);
		if (c1 == firstc)
		{
			// Initialise for compare over remaining pattern
			size_t m = n;
			const unsigned char* s2 = s1;
			p2++;

			// Remember last space
			bool was_space = false;
			unsigned char c2 = ' ';

			// Do inner loop compare over remaining pattern
			while (m--)
			{
				c1 = tolower(*s2++);
				
				// Convert CRLFs -> space if required
				if (crlf_convert)
				{
					if ((c1 == '\r') || (c1 == '\n'))
						c1 = ' ';
				}

				// Now do comparison but punt over space runs
				bool is_space = isspace(c1);
				if (!is_space || !was_space)
				{
					c2 = tolower(*p2++);
					pat_pos++;
					if (c1 != c2)
						break;
					was_space = is_space;
				}
			}

			// End of pattern => match
			if (!c2)
				return true;
			
			// End of search buffer => partial match
			if (!m && (c1 == c2))
				return false;
			
			// Reset first character
			firstc = firstp;
			p2 = p1;
			pat_pos = 0;
		}
	}

	return false;
}

#if 0
void CLocalClient::KMPInit(const cdstring& txt, long M, long* next)
{
	// Init KMP next table
	const char* p = txt.c_str();
	long i;
	long j;
	next[0] = -1;
	for(i = 0, j = -1; i < M; i++, j++, next[i] = j)
	{
		while((j >= 0) && (p[i] != p[j]))
			j = next[j];
	}
}

bool CLocalClient::KMPSearch(unsigned long length, const cdstring& txt, long M, const long* next)
{
	const char* p = txt.c_str();
	long i;
	long j;
	long N = length;
	const char* q = mSearchBuffer;
	bool was_space = false;

	// KMP search
	for(i = 0, j = 0; (j < M) && (i < N); i++, j++)
	{
		// Convert line ends
		char c = *q++;
		
		if ((c == '\r') || (c == '\n'))
		{
			// Ignore space runs
			if (was_space)
			{
				j--;
				continue;
			}
			else
				c = ' ';
		}

		was_space = isspace(c);

		// Do compare
		while((j >= 0) && (tolower(c) != tolower(p[j])))
			j = next[j];
	}

	return (j == M);
}
#endif

bool CLocalClient::StreamSearch1522(std::istream& in, unsigned long start, unsigned long length, const cdstring& txt)
{
	// Only bother if something to search
	if (!length)
		return false;

	// Read stream into temp buffer
	cdstring temp;
	temp.reserve(length + 1);

	// Read stream into temp buffer
	in.seekg(start);
	in.read(temp.c_str_mod(), length);

	// Do 1522 decode
	CRFC822::TextFrom1522(temp);
	
	// Now do comparison
	return ::strstrnocase(temp, txt);
}
