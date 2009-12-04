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


// Code for POP3 client class

#include "CPOP3Client.h"

#include "CAddressList.h"
#include "CConnectionManager.h"
#include "CGeneralException.h"
#include "CIdentity.h"
#include "CINETCommon.h"
#include "CIMAPCommon.h"
#include "CLocalMessage.h"
#include "CMailRecord.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CPOP3Common.h"
#include "CRFC822.h"
#include "CRFC822Parser.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStringUtils.h"
#include "CURL.h"

#include "filterbuf.h"

#include __stat_header

#include <algorithm>
#include <cerrno>
#include <memory>

#if __dest_os == __win32_os
#include "StValueChanger.h"
#include <WIN_LStream.h>
#elif __dest_os == __linux_os
#include "StValueChanger.h"
#include "UNX_LStream.h"
#endif

#define CHECK_STREAM(x) \
	{ if ((x).fail()) { int err_no = os_errno; CLOG_LOGTHROW(CGeneralException, err_no); throw CGeneralException(err_no); } }
	
#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CPOP3Client::CPOP3Client(CMboxProtocol* owner)
	: CLocalClient(owner)
{
	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitPOP3Client();
}

// Copy constructor
CPOP3Client::CPOP3Client(const CPOP3Client& copy, CMboxProtocol* owner)
	: CLocalClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitPOP3Client();

	// Always retrieve this info to prevent unnecessary commands
	mDoesUIDL = copy.mDoesUIDL;
	mDoesCAPA = copy.mDoesCAPA;
}

CPOP3Client::~CPOP3Client()
{
	mPOP3INBOX = NULL;

} // CPOP3Client::~CPOP3Client

void CPOP3Client::InitPOP3Client()
{
	mLogType = CLog::eLogPOP3;

	// Set protocol specific items
	mUseTag = false;

	mCommandState = eNone;
	mDoesAPOP = false;
	mDoesUIDL = true;
	mDoesCAPA = true;
	
	// Must reocrd local UIDs in EXPUNGE
	mRecordLocalUIDs = true;
	
	mPOP3INBOX = NULL;
}

// Create duplicate, empty connection
CINETClient* CPOP3Client::CloneConnection()
{
	// Copy construct this
	return new CPOP3Client(*this, GetMboxOwner());

}

// Get default port
tcp_port CPOP3Client::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetMboxOwner()->GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetMboxOwner()->GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return cPOP3ServerPort_SSL;
	else
		return cPOP3ServerPort;
}

// Get name of command for SASL
const char* CPOP3Client::GetAuthCommand() const
{
	return cAUTH;
}

// Get name of command for TLS
const char* CPOP3Client::GetStartTLSCommand() const
{
	// Most protocols use STARTTLS, but POP3 doesn't
	return cSTLS;
}

#pragma mark ____________________________Start/Stop

// Check CWD
void CPOP3Client::CheckCWD()
{
	// Must have INBOX
	_TouchMbox(GetMboxOwner()->GetINBOX());
}

// Release TCP
void CPOP3Client::Close()
{
	// Must do TCP action
	CINETClient::Close();
}

// Program initiated abort
void CPOP3Client::Abort()
{
	// Must do TCP action
	CINETClient::Abort();
}

#pragma mark ____________________________Login & Logout

// Start TLS
void CPOP3Client::DoStartTLS()
{
	{
		StCommandState state(eSTLS, this);

		// Issue STARTTLS command
		INETStartSend("Status::INET::StartTLS", "Error::INET::OSErrStartTLS", "Error::INET::NoBadStartTLS");
		INETSendString(cSTLS);
		INETFinishSend();

		// Now force TLS negotiation
		mStream->TLSSetTLSOn(true, GetMboxOwner()->GetAccount()->GetTLSType());
		mStream->TLSStartConnection();
	}

	// Must redo capability after STARTTLS
	_Capability(true);

}

// Do authentication to server directly
void CPOP3Client::DoAnonymousAuthentication()
{
	// Do not allow logging of auth details
	StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

	cdstring user_id = cANONYMOUS;
	cdstring pswd = "anonymous@anonymous";

	// Determine whether to use APOP or not
	if (mDoesAPOP && GetMboxOwner()->GetMailAccount()->GetUseAPOP())
		DoAPOP(user_id, pswd);
	else
	{
		// Issue first call
		DoUSER(user_id);
		DoPASS(pswd);
	}
}

// Do authentication to server directly
void CPOP3Client::DoPlainAuthentication()
{
	// Do not allow logging of auth details
	StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

	CAuthenticatorUserPswd* auth = GetMboxOwner()->GetAccount()->GetAuthenticatorUserPswd();
	cdstring user_id = auth->GetUID();
	cdstring pswd = auth->GetPswd();

	// Determine whether to use APOP or not
	if (mDoesAPOP && GetMboxOwner()->GetMailAccount()->GetUseAPOP())
		DoAPOP(user_id, pswd);
	else
	{
		// Issue first call
		DoUSER(user_id);
		DoPASS(pswd);
	}
}

// Logoff from POP3 server
void CPOP3Client::Logoff()
{
	// Only do if connected
	if (mStream->TCPGetState() == CTCPSocket::TCPConnected)
	{
		try
		{
			// Issue logout call
			INETStartSend("Status::INET::LoggingOut", "Error::INET::OSErrLogout", "Error::INET::NoBadLogout");
			INETSendString(cQUIT);
			INETFinishSend();

			// Break connection with server
			mStream->TCPCloseConnection();

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Set status
			CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");
			CLOG_LOGRETHROW;
			throw;
		}

		// Set status
		CStatusWindow::SetIMAPStatus("Status::INET::NotConnected");
	}

	// Do inherited
	CLocalClient::Logoff();

} // CINETClient::Logoff

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CPOP3Client::_Tickle(bool force_tickle)
{
	// Just do default CINETClient tickle if stream connected
	if (mStream->TCPGetState() == CTCPSocket::TCPConnected)
		CINETClient::_Tickle(force_tickle);
}

// Process greeting response
bool CPOP3Client::_ProcessGreeting()
{
	// Receive first info from server
	mStream->qgetline(mLineData, cINETBufferLen);

	// Store in greeting
	mGreeting = mLineData;

	// Write to log file
	mLog.LogEntry(mLineData);

	// Look for specific tag responses
	bool ok = (::strncmp(mGreeting, cPlusOK, 3) == 0);

	// Now look for availability of APOP
	if (ok)
	{
		const char* rspStart =::strchr(mGreeting.c_str(), '<');
		const char* rspEnd = ::strchr(mGreeting.c_str(), '>');
		if ((!rspStart) || (!rspEnd))
			mDoesAPOP = false;
		else
		{
			mDoesAPOP = true;
			mAPOPKey.assign(rspStart, rspEnd - rspStart + 1);
		}
	}

	return ok;
}

// Check version of server
void CPOP3Client::_Capability(bool after_tls)
{
	// Only if known to be supported
	if (!mDoesCAPA)
		return;

	// Try capability command - do not allow failure
	try
	{
		StCommandState state(eCAPA, this);

		// Prevent changing status which is already set
		StINETClientSilent silent(this);

		// Clear out existng capabilities before issuing command again
		_InitCapability();

		// Issue first call
		INETStartSend(NULL, NULL, NULL, cdstring::null_str, false);
		INETSendString(cCAPA, eQueueNoFlags, false);
		INETFinishSend(false);

		_ProcessCapability();
	}

	// Trap NO or BAD response
	catch (CINETException& ex)
	{
		CLOG_LOGCATCH(CINETException&);

		if ((ex.error() == CINETException::err_NoResponse) ||
			(ex.error() == CINETException::err_BadResponse))
		{
			_NoCapability();
		}
		else
		{
			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
	}

}

// Initialise capability flags to empty set
void CPOP3Client::_InitCapability()
{
	// Init capabilities to empty set
	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CPOP3Client::_ProcessCapability()
{
	// Process response to look for extensions
	mSTARTTLSAllowed = mLastResponse.CheckUntagged(cSTLS, true);

	// For now do nothing special with extensions

	// Add all capabilities to capability line
	mCapability = cdstring::null_str;

	bool first = true;
	for(cdstrvect::const_iterator iter = mLastResponse.untag_msgs.begin(); iter != mLastResponse.untag_msgs.end(); iter++)
	{
		if (first)
			first = false;
		else
			mCapability += ", ";
		mCapability += *iter;
	}

}

// Handle failed capability response
void CPOP3Client::_NoCapability()
{
	// Flag to prevent clones doing this
	mDoesCAPA = false;
}

// Parse text sent by server (advance pointer to next bit to be parsed)
void CPOP3Client::_ParseResponse(char** txt, CINETClientResponse* response)
{
	// Depends on command state
	switch(mCommandState)
	{
	case eSTAT:
		POP3ParseSTATResponse(txt, response);
		break;
	case eLIST:
		POP3ParseLISTResponse(txt, response);
		break;
	case eRETR:
		POP3ParseRETRResponse(txt, response);
		break;
	case eTOP:
		POP3ParseTOPResponse(txt, response);
		break;
	case eUIDL:
		POP3ParseUIDLResponse(txt, response);
		break;
	case eCAPA:
		POP3ParseCAPAResponse(txt, response);
		break;
	default:;
	}
}

#pragma mark ____________________________Handle Response

// Parse text sent by server (advance pointer to next bit to be parsed)
void CPOP3Client::INETParseResponse(char** txt, CINETClientResponse* response)
{
	// Clear response message first
	response->tag_msg = cdstring::null_str;

	// Look for specific tag responses
	if (::stradvtokcmp(txt, cPlusOK) == 0)
		response->code = cTagOK;
	else if (::stradvtokcmp(txt, cMinusERR) == 0)
		response->code = cTagNO;
	else
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}
		//response->code = cResponseError;

	// Copy remaining tag message
	if (response->code != cResponseError)
		// Copy the response message text
		response->tag_msg = *txt;

	// Now do any further POP3 processing
	if (response->code == cTagOK)
		_ParseResponse(txt, response);
}

#pragma mark ____________________________Mailboxes

// Do open mailbox
void CPOP3Client::_OpenMbox(CMbox* mbox)
{
	// Do inherited
	CLocalClient::_OpenMbox(mbox);

	// Check for INBOX and turn on recording
	if (!::strcmpnocase(mbox->GetName(), cINBOX))
	{
		// Create new recorder
		if (!mRecorder)
			mRecorder = new CMailRecord;
		cdstring record_name;
		GetRecorderName(mbox, record_name);
		mRecorder->SetDescriptor(record_name);

		// Record only expunges of remote UIDs
		mRecorder->SetExpungeOnlyRecord();

		// Read in existing items
		mRecorder->Open();
	}
}

// Do close mailbox
void CPOP3Client::_CloseMbox(CMbox* mbox)
{
	// Do inherited
	CLocalClient::_CloseMbox(mbox);

	// Check for INBOX and turn off recording
	if (!::strcmpnocase(mbox->GetName(), cINBOX))
	{
		if (mRecorder)
			mRecorder->Close();
		delete mRecorder;
		mRecorder = NULL;
	}
}

// Check mbox
void CPOP3Client::_CheckMbox(CMbox* mbox, bool fast)
{
	// Check for INBOX and do POP3 if connected - but not if fast
	if (!fast && !::strcmpnocase(mbox->GetName(), cINBOX) &&
		CConnectionManager::sConnectionManager.IsConnected())
		DoPOP3(mbox);

	CLocalClient::_CheckMbox(mbox);

} // CPOP3Client::_CheckMbox

#pragma mark ____________________________Messages

#pragma mark ____________________________ACLs

#pragma mark ____________________________Quotas

#pragma mark ____________________________Others

#pragma mark ____________________________Handle Errors

#pragma mark ____________________________Local Ops

// Do POP3 check
void CPOP3Client::DoPOP3(CMbox* mbox)
{
	bool was_current = (mbox == GetCurrentMbox());
	bool was_open = (mStream->TCPGetState() == CTCPSocket::TCPOpenNoSocket);
	bool read_write = false;

	try
	{
		// Grab the global lock on the mailbox to prevent it being
		// opened while messages are being downloaded
		CMbox::mbox_lock _glock(CMbox::_smutex, mbox);
		mPOP3INBOX = mbox;

		// If this is not the current one then force it current
		if (!was_current)
		{
			_OpenMbox(mbox);

			// Open Read-Write, but force it read-only to prevent _CloseMbox resetting recent flag
			_SelectMbox(mbox, false);
			mbox->SetFlags(NMbox::eReadOnly);
		}

		// Must block the protocol to prevent other user actions
		// from trying to access the streams
		cdmutex::lock_cdmutex _plock(GetMboxOwner()->_get_mutex());

		// Make sure its open first
		if (!was_open)
			CINETClient::Open();

		// Make sure its logged in to
		CINETClient::Logon();

		// Get UIDLs
		ulvector uidls;
		if (mDoesUIDL)
			DoUIDL(uidls);
		
		// If UIDL supported then map UIDLs
		ulvector msgs;
		ulvector deletes;
		if (mDoesUIDL)
			// Find new messages on server
			NewUIDLs(uidls, msgs, deletes);
		else
		{
			// Do STAT to get number of new messages
			unsigned long num_msgs;
			unsigned long size;
			DoSTAT(num_msgs, size);
			
			// Download and (possibly) delete all messages
			for(unsigned long num = 1; num <= num_msgs; num++)
			{
				msgs.push_back(num);
				uidls.push_back(0);

				// Always delete if not leave on server
				if (!GetMboxOwner()->GetMailAccount()->GetLeaveOnServer())
					deletes.push_back(num);
			}

			// Must flush the recorder of expunges
			if (mRecorder)
				mRecorder->Flush(CMailAction::eExpunge);
		}

		// Get all message sizes
		ulvector sizes;
		DoLIST(sizes);
		
		// See if size restriction required
		if (GetMboxOwner()->GetMailAccount()->GetUseMaxSize())
		{
			// Now check each size and see if over limit
			unsigned long num = 1;
			for(ulvector::const_iterator iter = sizes.begin(); iter != sizes.end(); iter++, num++)
			{
				// Check limit
				if (*iter > GetMboxOwner()->GetMailAccount()->GetMaxSize() * 1024)
				{
					// See if this message number is marked for download
					ulvector::iterator found = std::find(msgs.begin(), msgs.end(), num);
					if (found != msgs.end())
					{
						// Remove it from download list
						msgs.erase(found);
						
						// See if it was also marked for deletion
						found = std::find(deletes.begin(), deletes.end(), num);
						if (found != deletes.end())
							// Remove from deletion list
							deletes.erase(found);
					}
				}
			}
		}

		// Get any new messages
		if (msgs.size())
		{
			// Must have read-write streams
			SwitchCache(mbox, mIndex, mMailbox, mCache, false);
			read_write = true;

			// Now POP all the messages
			PopMessages(mbox, msgs, uidls, sizes);

			// Must have read-only streams
			SwitchCache(mbox, mIndex, mMailbox, mCache, true);
			read_write = false;
		}

		// Now delete the unwanted
		if (deletes.size())
		{
			// Start action
			StINETClientAction status(this, "Status::IMAP::Deleting", NULL, NULL, mbox->GetName());

			// Prevent loop from changing status
			StINETClientSilent silent(this);

			// Reset item counter for feedback
			InitItemCtr(deletes.size());

			for(ulvector::const_iterator iter = deletes.begin(); iter != deletes.end(); iter++)
			{
				// Bump item counter
				BumpItemCtr("Status::IMAP::DeleteCount");

				// Delete it
				DoDELE(*iter);
			}
		}

		// Always logout
		Logoff();

		// Must close if not previously open
		if (!was_current)
			_CloseMbox(mbox);
		
		mPOP3INBOX = NULL;
	}
	catch (std::exception& ex)
	{
		CLOG_LOGCATCH(std::exception&);

		// Handle error alert to user
		INETHandleError(ex, mErrorID, mNoBadID);

		try
		{
			// Always logout
			Logoff();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must not allow this to cause subsequent failure
		}

		// Must have read-only streams
		if (read_write)
			SwitchCache(mbox, mIndex, mMailbox, mCache, true);

		// Must close if not previously open
		if (!was_current)
			_CloseMbox(mbox);

		mPOP3INBOX = NULL;

		// No longer throw our of here
		// If the connect attempt fails then the POP3 operation fails, 
		// but that shouldn't cause problems with the POP3 INBOX later on
		//CLOG_LOGRETHROW;
		//throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handle error alert to user
		CGeneralException gex(-1);
		INETHandleError(gex, mErrorID, mNoBadID);

		try
		{
			// Always logout
			Logoff();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must not allow this to cause subsequent failure
		}

		// Must have read-only streams
		if (read_write)
			SwitchCache(mbox, mIndex, mMailbox, mCache, true);

		// Must close if not previously open
		if (!was_current)
			_CloseMbox(mbox);

		mPOP3INBOX = NULL;

		// No longer throw our of here
		// If the connect attempt fails then the POP3 operation fails, 
		// but that shouldn't cause problems with the POP3 INBOX later on
		//CLOG_LOGRETHROW;
		//throw;
	}
}

void CPOP3Client::NewUIDLs(const ulvector& uidls, ulvector& msgs, ulvector& deletes)
{
	// Look at each UIDL
	unsigned long msg = 1;
	for(ulvector::const_iterator iter1 = uidls.begin(); iter1 != uidls.end(); iter1++, msg++)
	{
		// Does it appear in the mailboxes index?
		bool found = false;
		for(SIndexList::const_iterator iter2 = mIndexList.begin(); iter2 != mIndexList.end(); iter2++)
		{
			if ((*iter2).LocalUID() == *iter1)
			{
				// Mark as found
				found = true;

				// Check whether its been on server past its sell-by date
				if (GetMboxOwner()->GetMailAccount()->GetLeaveOnServer() &&
					GetMboxOwner()->GetMailAccount()->GetDoTimedDelete())
				{
					// Get cache stream pos for envelope
					std::istream::pos_type pos_cache = (*iter2).Cache();

					// Now read in the envelope and body info
					CLocalMessage lmsg(static_cast<CMbox*>(NULL));

					// Seek to position and read cache
					mCache.seekg(pos_cache);
					lmsg.ReadCacheFromStream(mCache, 0);

					// Get current time in UTC
					time_t utc_now = ::time(NULL);
					utc_now = ::mktime(::gmtime(&utc_now));

					// Now compare UTC dates (use internal/popped date) and delete if over sell-by date
					if (::difftime(utc_now, lmsg.GetUTCInternalDate()) / (60 * 60 * 24) >= GetMboxOwner()->GetMailAccount()->GetDeleteAfter())
						deletes.push_back(msg);
				}
				break;
			}
		}

		// Add if not found
		if (!found)
		{
			// Check whether its been expunged
			if (GetMboxOwner()->GetMailAccount()->GetLeaveOnServer() &&
				GetMboxOwner()->GetMailAccount()->GetDeleteExpunged())
			{
				// Is this local UID in the list of expunged items
				if (mRecorder && mRecorder->ExpungedUID(*iter1, true))
					deletes.push_back(msg);
				else
					msgs.push_back(msg);
			}
			else
				// New message => download it
				msgs.push_back(msg);
		}

		// Always delete if not leave on server
		if (!GetMboxOwner()->GetMailAccount()->GetLeaveOnServer())
			deletes.push_back(msg);
	}

	// Must flush the recorder of expunges
	if (mRecorder)
		mRecorder->Flush(CMailAction::eExpunge);
}

void CPOP3Client::PopMessages(CMbox* mbox, const ulvector& msgs, const ulvector& uidls, const ulvector& sizes)
{
	// Need to from-stuff the stream as written to the mailbox
	CStreamFilter from_stuff(new fromstuff_filterbuf(true), &mMailbox);

	// Create stream type object
	costream stream_out(&from_stuff, mEndl);
	mRcvOStream = &stream_out;

	// Prevent loop from changing status
	StINETClientSilent silent(this);

	// Reset item counter for feedback
	InitItemCtr(msgs.size());

	// Create network status item for % progress counter
	CNetworkAttachProgress progress;
	StValueChanger<CProgress*> _progress(mProgress, &progress);

	// Read in each one
	for(ulvector::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		// Bump item counter
		BumpItemCtr("Status::IMAP::MessageFetch");

		// Start progress
		progress.Reset();
		progress.SetTotal(sizes[*iter - 1]);

		// Seek to end of mailbox stream
		mMailbox.seekp(0, std::ios_base::end);

		// Now at starting point for append
		std::istream::pos_type old_start = mMailbox.tellp();

		// Must add line between any previous message and this one
		if (old_start)
		{
			mMailbox << stream_out.endl();
			CHECK_STREAM(mMailbox)
		}

		// Now at starting point for this message
		std::istream::pos_type start = mMailbox.tellp();

		// Add UNIX mailbox header
		mMailbox << "From ";
		mMailbox << "Mulberry_Local";
		mMailbox << " " << CRFC822::GetUNIXDate() << stream_out.endl();
		CHECK_STREAM(mMailbox)

		// Catch errors and rollback the append
		try
		{
			// Read message into stream
			DoRETR(*iter);
			from_stuff.flush();
			mMailbox.flush();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Force flush both streams to get data in sync before rollback
			try
			{
				from_stuff.flush();
				mMailbox.flush();
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

			}

			// Rollback append mailbox
			RollbackAppend(mbox, &mMailbox, old_start);

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
		
		// Check that stream is safe
		CHECK_STREAM(mMailbox)

		// Reposition
		mMailbox.seekp(start);

		// Parse it
		CRFC822Parser parser;
		CNetworkAttachProgress progress;
		std::auto_ptr<CLocalMessage> lmsg(parser.MessageFromStream(mMailbox, &progress));

		// Parser will leave it in failed state
		mMailbox.clear();

		// Read index header
		SIndexHeader hdr;
		ReadIndexHeader(mIndex, hdr);

		// Store new index here
		SIndexRecord index_item;

		// Add to end of cache file
		mCache.seekg(0, std::ios_base::end);

		// Write empty indices first
		index_item.Index() = mCache.tellp();
		lmsg->WriteIndexToStream(mCache, 0);
		CHECK_STREAM(mCache)

		// Now write cache - this will calculate actual indices
		index_item.Cache() = mCache.tellp();
		lmsg->WriteCacheToStream(mCache);
		CHECK_STREAM(mCache)

		// Move back to indices write pos and write out correct indices
		mCache.seekp(index_item.Index());
		lmsg->WriteIndexToStream(mCache, 0);
		CHECK_STREAM(mCache)

		index_item.Flags() = NMessage::eRecent;	// POP'd messages always recent
		index_item.UID() = hdr.LastUID() + 1;
		index_item.LocalUID() = uidls[*iter - 1];
		index_item.Sequence() = mIndexList.size() + 1;
		index_item.MessageStart() = start;

		// Add index to internal cache
		mIndexList.push_back(index_item);
		mIndexMapping.push_back(index_item.Sequence() - 1);

		// Write new index total
		unsigned long new_size = mIndexList.size();

		// Write new index size
		UpdateIndexSize(mIndex, new_size);

		// Write out new last uid
		if (hdr.LastUID() < index_item.UID())
			UpdateIndexLastUID(mIndex, index_item.UID());

		// Write new indices at end
		mIndex.seekg(0, std::ios_base::end);
		index_item.write(mIndex);
		CHECK_STREAM(mIndex)
	}
}

void CPOP3Client::GetRecorderName(const CMbox* mbox, cdstring& record_name) const
{
	record_name = mCWD + mbox->GetName();
	record_name += ".mbr";
}

#pragma mark ____________________________Commands

void CPOP3Client::DoSTAT(unsigned long& msgs, unsigned long& size)
{
	msgs = 0;
	size = 0;

	StCommandState state(eSTAT, this);

	// Issue first call
	INETStartSend("Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mPOP3INBOX->GetName());
	INETSendString(cSTAT);
	INETFinishSend();

	// Get the number of messages and size
	char* p = mLastResponse.tag_msg;
	msgs = ::strtoul(p, &p, 10);
	size = ::strtoul(p, &p, 10);
}

void CPOP3Client::DoLIST(ulvector& sizes)
{
	StCommandState state(eLIST, this);

	// Issue first call
	INETStartSend("Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mPOP3INBOX->GetName());
	INETSendString(cLIST);
	INETFinishSend();

	// Now add each size
	for(cdstrvect::iterator iter = mLastResponse.untag_msgs.begin(); iter != mLastResponse.untag_msgs.end(); iter++)
	{
		const char* p = (*iter).c_str();

		// Punt over number
		while(*p == ' ') p++;
		while(*p && (*p != ' ')) p++;
		while(*p == ' ') p++;

		// Append number
		sizes.push_back(::strtoul(p, NULL, 10));
	}
}

void CPOP3Client::DoRETR(unsigned long msg)
{
	StCommandState state(eRETR, this);

	// Issue first call
	INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", mPOP3INBOX->GetName());
	INETSendString(cRETR);
	INETSendString(cSpace);
	INETSendString(cdstring(msg));
	INETFinishSend();
}

void CPOP3Client::DoDELE(unsigned long msg)
{
	StCommandState state(eDELE, this);

	// Issue first call
	INETStartSend("Status::IMAP::Deleting", "Error::IMAP::OSErrDeleteMsg", "Error::IMAP::NoBadDeleteMsg", mPOP3INBOX->GetName());
	INETSendString(cDELE);
	INETSendString(cSpace);
	INETSendString(cdstring(msg));
	INETFinishSend();
}

void CPOP3Client::DoTOP(unsigned long msg)
{
	StCommandState state(eTOP, this);

	// Issue first call
	INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", mPOP3INBOX->GetName());
	INETSendString(cTOP);
	INETSendString(cSpace);
	INETSendString(cdstring(msg));
	INETFinishSend();
}

void CPOP3Client::DoUIDL(ulvector& uidls)
{
	StCommandState state(eUIDL, this);

	try
	{
		// Issue first call
		INETStartSend("Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mPOP3INBOX->GetName(), false);
		INETSendString(cUIDL, false);
		INETFinishSend(false);

		// Now convert each UIDL into a hash
		for(cdstrvect::iterator iter = mLastResponse.untag_msgs.begin(); iter != mLastResponse.untag_msgs.end(); iter++)
		{
			const char* p = (*iter).c_str();

			// Punt over number
			while(*p == ' ') p++;
			while(*p && (*p != ' ')) p++;
			while(*p == ' ') p++;

			// Grab text UIDL and convert to MD5 hash
			cdstring uidl = p;
			unsigned long hash;
			uidl.md5(hash);
			uidls.push_back(hash);
		}
	}
	catch (CINETException& ex)
	{
		CLOG_LOGCATCH(CINETException&);

		// This command is optional so fail gracefully
		mDoesUIDL = false;
	}
}

void CPOP3Client::DoUSER(const cdstring& user)
{
	StCommandState state(eUSER, this);

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Issue first call (silent status)
	INETStartSend(NULL, NULL, NULL);
	INETSendString(cUSER);
	INETSendString(cSpace);
	INETSendString(user);
	INETFinishSend();
}

void CPOP3Client::DoPASS(const cdstring& pass)
{
	StCommandState state(ePASS, this);

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Issue first call
	INETStartSend(NULL, NULL, NULL);
	INETSendString(cPASS);
	INETSendString(cSpace);
	INETSendString(pass);
	INETFinishSend();
}

void CPOP3Client::DoAPOP(const cdstring& user, const cdstring& pass)
{
	StCommandState state(eAPOP, this);

	// Calculate digest
	cdstring convert = mAPOPKey + pass;

	// do MD5 - must use lowercase hex characters
	cdstring digest;
	convert.md5(digest);
	::strlower(digest.c_str_mod());

	// Prevent changing status which is already set
	StINETClientSilent silent(this);

	// Issue first call
	INETStartSend(NULL, NULL, NULL);
	INETSendString(cAPOP);
	INETSendString(cSpace);
	INETSendString(user);
	INETSendString(cSpace);
	INETSendString(digest);
	INETFinishSend();
}

#pragma mark ____________________________Parsing

// Read a set of lines from POP3 server
void CPOP3Client::GetResponseLines(CINETClientResponse* response)
{
	// Read lines until terminator
	bool done = false;
	while(!done)
	{
		// Now get a line of text
		char* txt = INETGetLine();

		// Write to log file
		mLog.AddEntry(txt);

		// Look for line end
		if (!::strcmp(txt, "."))
			done = true;
		else
			// Add as untagged response
			response->AddUntagged(txt);

		// Clean up any long lines
		if (mLongLine)
		{
			delete mLongLine;
			mLongLine = NULL;
		}
	}
}

// Read a set of lines from POP3 server into stream
void CPOP3Client::GetResponseStream()
{
	// Read lines until terminator
	bool done = false;
	unsigned long progress_total = 0;
	unsigned long line_ctr = 0;
	while(!done)
	{
		// Now get a line of text
		char* txt = INETGetLine();
		line_ctr++;

		// Write to log file
		mLog.AddEntry(txt);

		// Look for line end
		if (!::strcmp(txt, "."))
			done = true;
		else
		{
			// Need to bump past leading '.' which is doubled
			if (*txt == '.') txt++;

			// Write data to stream
			if (mRcvStream)
			{
				// Write to LStream
				long len = ::strlen(txt);
				mRcvStream->PutBytes(txt, len);
				progress_total += len;
				len = net_endl_len;
				mRcvStream->PutBytes(net_endl, len);
				progress_total += 2;

				// Bump progress every 128 lines
				if (mProgress && ((line_ctr & 0x0000007F) == 0))
					mProgress->SetPercentageValue(progress_total);
			}
			else if (mRcvOStream)
			{
				unsigned long len = ::strlen(txt);
				mRcvOStream->Stream().write(txt, len);
				progress_total += len;
				len = mRcvOStream->endl_len();
				mRcvOStream->Stream().write(mRcvOStream->endl(), len);
				progress_total += 2;

				// Bump progress every 128 lines
				if (mProgress && ((line_ctr & 0x0000007F) == 0))
					mProgress->SetPercentageValue(progress_total);
			}
		}

		// Clean up any long lines
		if (mLongLine)
		{
			delete mLongLine;
			mLongLine = NULL;
		}
	}
}

// Status of maildrop
void CPOP3Client::POP3ParseSTATResponse(char** txt, CINETClientResponse* response)
{
	// Single line response only
}

// List of all messages
void CPOP3Client::POP3ParseLISTResponse(char** txt, CINETClientResponse* response)
{
	// Get lines from server
	GetResponseLines(response);
}

// Get a messages
void CPOP3Client::POP3ParseRETRResponse(char** txt, CINETClientResponse* response)
{
	// Get stream from server
	GetResponseStream();
}

// Get a message header
void CPOP3Client::POP3ParseTOPResponse(char** txt, CINETClientResponse* response)
{
	// Get stream from server
	GetResponseStream();
}

// Get messages uidls
void CPOP3Client::POP3ParseUIDLResponse(char** txt, CINETClientResponse* response)
{
	// Get lines from server
	GetResponseLines(response);
}

// Get messages uidls
void CPOP3Client::POP3ParseCAPAResponse(char** txt, CINETClientResponse* response)
{
	// Get lines from server
	GetResponseLines(response);
}
