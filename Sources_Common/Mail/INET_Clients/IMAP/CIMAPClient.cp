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


// Code for IMAP client class

#include "CIMAPClient.h"
#include "CINETClientResponses.h"
#include "CINETCommon.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CCharsetManager.h"
#include "CCharSpecials.h"
#include "CEnvelope.h"
#include "CFilter.h"
#include "CLocalMessage.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#endif
#include "CPreferences.h"
#include "CQuotas.h"
#include "CRFC822.h"
#include "CSearchItem.h"
#include "CSequence.h"
#include "CStreamType.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x

//#include <UProfiler.h>
#elif __dest_os == __win32_os

#include <WIN_LStream.h>
#include <WIN_LMemFileStream.h>
#include "StValueChanger.h"

#elif __dest_os == __linux_os

#include "UNX_LStream.h"
#include "UNX_LMemFileStream.h"
#include "StValueChanger.h"

#endif

#include "ctrbuf.h"

#include <cstdio>
#include <cstdlib>
#include <strstream>

#pragma mark -

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CIMAPClient::CIMAPClient(CMboxProtocol* owner)
	: CMboxClient(owner)
{
	// Create the TCP stream
	mStream = new CTCPStream;

	// Init instance variables
	InitIMAPClient();

}

// Copy constructor
CIMAPClient::CIMAPClient(const CIMAPClient& copy, CMboxProtocol* owner)
	: CMboxClient(copy, owner)
{
	// Stream will be copied by parent constructor

	// Init instance variables
	InitIMAPClient();
}

CIMAPClient::~CIMAPClient()
{
	mCurrentWD = NULL;
	mProcessMessage = NULL;
	mThreadResults = NULL;
}

void CIMAPClient::InitIMAPClient()
{
	// Init instance variables
	mVersion = eIMAP2bis;

	mFindingHier = false;
	mFindingSubs = false;
	mMboxNew = 0;
	mInPostProcess = false;
	mMboxUpdate = false;
	mMboxReset = false;
	mMboxReload = false;
	mDummyFiles = false;

	mProcessMessage = NULL;

	mLogType = CLog::eLogIMAP;

	mHasNamespace = false;
	mHasUIDPlus = false;
	mHasUnselect = false;
	mHasSort = false;
	mHasThreadSubject = false;
	mHasThreadReferences = false;
	mThreadResults = NULL;

} // CIMAPClient::CIMAPClient

// Create duplicate, empty connection
CINETClient* CIMAPClient::CloneConnection()
{
	// Copy construct this
	return new CIMAPClient(*this);

} // CIMAPClient::CloneConnection

// Get default port
tcp_port CIMAPClient::GetDefaultPort()
{
	// TCP/IP-based sub-classes must deal with this
	if ((GetMboxOwner()->GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
		(GetMboxOwner()->GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		return cIMAPServerPort_SSL;
	else
		return cIMAPServerPort;
}

#pragma mark ____________________________Protocol

// Initialise capability flags to empty set
void CIMAPClient::_InitCapability()
{
	mVersion = eIMAP2bis;

	GetMboxOwner()->SetHasACL(false);
	GetMboxOwner()->SetHasQuota(false);

	mAsyncLiteral = false;
	mHasNamespace = false;
	mHasUIDPlus = false;
	mHasUnselect = false;
	mHasSort = false;
	mHasThreadSubject = false;
	mHasThreadReferences = false;

	mAuthLoginAllowed = false;
	mAuthPlainAllowed = false;
	mAuthAnonAllowed = false;
	mSTARTTLSAllowed = false;

	mCapability = cdstring::null_str;
}

// Check version of server
void CIMAPClient::_ProcessCapability()
{
	// Process response to look for versions (in decreasing version order to get the latest)
	if (mLastResponse.CheckUntagged(cIMAP4REV1, true))
	{
		mVersion = eIMAP4rev1;
		GetMboxOwner()->SetType(cType_IMAP4rev1);
		GetMboxOwner()->SetHasDisconnected(true);
	}
	else if (mLastResponse.CheckUntagged(cIMAP4, true))
	{
		mVersion = eIMAP4;
		GetMboxOwner()->SetType(cType_IMAP4);
		GetMboxOwner()->SetHasDisconnected(true);
	}
	else
	{
		mVersion = eIMAP2bis;
		GetMboxOwner()->SetType(cType_IMAP2bis);
		GetMboxOwner()->SetHasDisconnected(false);
	}

	// Look for other capabilities
	GetMboxOwner()->SetHasACL(mLastResponse.CheckUntagged(cIMAP_ACL, true));
	GetMboxOwner()->SetHasQuota(mLastResponse.CheckUntagged(cIMAP_QUOTA, true));
	mAsyncLiteral = mLastResponse.CheckUntagged(cIMAP_LITERAL_PLUS, true);
	mHasNamespace = mLastResponse.CheckUntagged(cIMAP_NAMESPACE, true);
	mHasUIDPlus = mLastResponse.CheckUntagged(cIMAP_UIDPLUS, true);
	mHasUnselect = mLastResponse.CheckUntagged(cIMAP_UNSELECT, true);
	mHasSort = mLastResponse.CheckUntagged(cIMAP_SORT, true);
	mHasThreadSubject = mLastResponse.CheckUntagged(cIMAP_THREAD_SUBJECT, true);
	mHasThreadReferences = mLastResponse.CheckUntagged(cIMAP_THREAD_REFERENCES, true);

	mAuthLoginAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHLOGIN, true);
	mAuthPlainAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHPLAIN, true);
	mAuthAnonAllowed = mLastResponse.CheckUntagged(cIMAP_AUTHANON, true);
	mSTARTTLSAllowed = mLastResponse.CheckUntagged(cSTARTTLS, true);
}

// Handle failed capability response
void CIMAPClient::_NoCapability()
{
	// Default to only protocol that does not have CAPABILITY
	mVersion = eIMAP2bis;
} // CIMAPClient::_NoCapability

// Handle failed capability response
void CIMAPClient::_PreProcess()
{
	// Clear mailbox change
	mMboxNew = 0;
	mMboxUpdate = false;
	mMboxReset = false;
	mMboxReload = false;

} // CIMAPClient::_PreProcess

// Handle failed capability response
void CIMAPClient::_PostProcess()
{
	if (mLastResponse.CheckUntagged(cUIDVALIDITY))
	{
		const cdstring& uidv_txt = mLastResponse.GetUntagged(cUIDVALIDITY);

		const char* p = ::strstrnocase(uidv_txt, cUIDVALIDITY);
		if (p)
		{
			p += ::strlen(cUIDVALIDITY);
			unsigned long uidv = ::strtoul(p, NULL, 10);
			if (GetCurrentMbox())
				GetCurrentMbox()->SetUIDValidity(uidv);
		}
	}

	if (mLastResponse.CheckUntagged(cUIDNEXT))
	{
		const cdstring& uidn_txt = mLastResponse.GetUntagged(cUIDNEXT);

		const char* p = ::strstrnocase(uidn_txt, cUIDNEXT);
		if (p)
		{
			p += ::strlen(cUIDNEXT);
			unsigned long uidn = ::strtoul(p, NULL, 10);
			if (GetCurrentMbox())
				GetCurrentMbox()->SetUIDNext(uidn);
		}
	}

	if (mLastResponse.CheckUntagged(cUNSEEN))
	{
		// Don't do this - now do a SEARCH on mailbox open to get the same info
		// New method relies on first unseen NOT being set before the intial search
#if 0
		// Only bother if user wants Unseen only to represent 'new' messages
		if (CPreferences::sPrefs->mUnseenNew.GetValue() && !CPreferences::sPrefs->mRecentNew.GetValue())
		{
			const cdstring& uneen_txt = mLastResponse.GetUntagged(cUNSEEN);

			const char* p = ::strstrnocase(uneen_txt, cUNSEEN);
			if (p)
			{
				p += ::strlen(cUNSEEN);
				unsigned long unseen = ::strtoul(p, NULL, 10);
				if (GetCurrentMbox() &&
					(unseen <= GetCurrentMbox()->GetNumberFound()))	// Add sanity check to cope with ASIP beta bug
					GetCurrentMbox()->SetFirstNew(unseen);
			}
		}
#endif
	}

	if (mLastResponse.CheckUntagged(cPERMANENTFLAGS))
	{
		const cdstring& flags_txt = mLastResponse.GetUntagged(cPERMANENTFLAGS);

		char* txt = ::strstrnocase(flags_txt, cPERMANENTFLAGS);
		if (txt)
		{
			txt += ::strlen(cPERMANENTFLAGS);

			NMessage::EFlags new_flags = NMessage::eNone;

			// Look for bracket
			char* p = ::strmatchbra(&txt);
			if (!p)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}

			while(p && *p)
			{

				// Look for all posibilities
				if (CheckStrAdv(&p,cFLAGRECENT))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eRecent);

				else if (CheckStrAdv(&p,cFLAGANSWERED))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eAnswered);

				else if (CheckStrAdv(&p,cFLAGFLAGGED))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eFlagged);

				else if (CheckStrAdv(&p,cFLAGDELETED))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eDeleted);

				else if (CheckStrAdv(&p,cFLAGSEEN))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eSeen);

				else if (CheckStrAdv(&p,cFLAGDRAFT))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eDraft);

				else if (CheckStrAdv(&p,cFLAGMDNSENT))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eMDNSent);

				else if (CheckStrAdv(&p,cFLAGKEYWORDS))
					new_flags = static_cast<NMessage::EFlags>(new_flags | NMessage::eLabels | NMessage::eMDNSent);

				else
				{
					// Unknown flag - ignore
					while(*p == ' ') p++;
					p = ::strpbrk(p,SPACE);
				}
			}
			if (GetCurrentMbox())
				GetCurrentMbox()->SetAllowedFlags(new_flags);
		}
	}

	// Save last response/new message count while doing an update
	CINETClientResponse save_response = mLastResponse;
	unsigned long new_msgs = mMboxNew;

	// Do any GUI update of mailbox
	if (mMboxReload && (GetCurrentMbox() != NULL))
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

		if (mMboxReset)
			CMailControl::MboxReset(GetCurrentMbox());
		else if (mMboxUpdate)
			CMailControl::MboxUpdate(GetCurrentMbox());
	}

	// Recover last response/new message count
	mLastResponse = save_response;
	mMboxNew = new_msgs;

} // CIMAPClient::_PostProcess

// Handle failed capability response
void CIMAPClient::_ParseResponse(char** txt,
							CINETClientResponse* response)
{
	IMAPParseResponse(txt, response);
} // CIMAPClient::_ParseResponse

#pragma mark ____________________________Mailboxes

// Create a new mbox
void CIMAPClient::_CreateMbox(CMbox* mbox)
{
	// Get full name
	cdstring wd_name = mbox->GetName();

	// Check for directory and append dri delim
	if (mbox->IsDirectory())
		wd_name += mbox->GetDirDelim();

	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send CREATE message to server
	INETStartSend("Status::IMAP::Creating", "Error::IMAP::OSErrCreate", "Error::IMAP::NoBadCreate", mbox->GetName());
	INETSendString(cCREATE);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();

} // CIMAPClient::_CreateMbox

// Do explicit list of mailbox
bool CIMAPClient::_TestMbox(CMbox* mbox)
{
	// Unset flag to stop adding subscribed flag
	mFindingSubs = false;
	InitItemCtr();

	// Get the WD
	mCurrentWD = NULL;
	cdstring wd = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd.ToModifiedUTF7(true);

	// Depends on server version
	INETStartSend("Status::IMAP::FindingAll", "Error::IMAP::OSErrFindAll", "Error::IMAP::NoBadFindAll");
	switch(mVersion)
	{
	case eIMAP2bis:
	default:
		{
			// Send FIND ALL.MAILBOX "xxx" message to server
			INETSendString(cFINDALLMAILBOXES);
			INETSendString(cSpace);
			INETSendString(wd, eQueueProcess);
			break;
		}

	case eIMAP4:
	case eIMAP4rev1:
		{
			// Get start spec
			cdstring list_spec = "\"\"";

			// Send LIST "" "xxx" message to server
			INETSendString(cLIST);
			INETSendString(cSpace);
			INETSendString(list_spec);
			INETSendString(cSpace);
			INETSendString(wd, eQueueProcess);
			break;
		}
	}
	INETFinishSend();
	
	return true;
}

// Rebuild cache
void CIMAPClient::_RebuildMbox(CMbox* mbox)
{
	// IMAP does nothing
}

// Do open mailbox
void CIMAPClient::_OpenMbox(CMbox* mbox)
{
	// This does nothing - mailbox is really opened when it is selected
	// This routine is useful for local mailboxes on disks

} // CIMAPClient::_OpenMbox

// Do close mailbox
void CIMAPClient::_CloseMbox(CMbox* mbox)
{
	// This does nothing - mailbox is never closed
	// This routine is useful for local mailboxes on disks

} // CIMAPClient::_CloseMbox

// Do Selection
void CIMAPClient::_SelectMbox(CMbox* mbox, bool examine)
{
	try
	{
		// Get full name
		cdstring wd_name = mbox->GetName();
		if (mVersion == eIMAP4rev1)
			wd_name.ToModifiedUTF7(true);

		// Issue SELECT/EXAMINE call
		INETStartSend("Status::IMAP::Selecting", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mbox->GetName(), false);
		INETSendString(examine ? cEXAMINE : cSELECT, eQueueNoFlags, false);
		INETSendString(cSpace, eQueueNoFlags, false);
		INETSendString(wd_name, eQueueProcess, false);
		INETFinishSend();

		// Can select - set flag
		mbox->SetFlags(NMbox::eError, false);

		// Check Read/Write status from OK response text
		if (mLastResponse.FindTagged(cREAD_WRITE))
			mbox->SetFlags(NMbox::eReadOnly, false);
		else if (mLastResponse.FindTagged(cREAD_ONLY))
			mbox->SetFlags(NMbox::eReadOnly, true);
		else
			mbox->SetFlags(NMbox::eReadOnly, false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Cannot select - set flag
		mbox->SetFlags(NMbox::eError);

		CLOG_LOGRETHROW;
		throw;
	}

} // CIMAPClient::_SelectMbox

// Deselect mbox
void CIMAPClient::_Deselect(CMbox* mbox)
{
	// This will only be required if the mailbox is already selected
	try
	{
		// Get full name
		cdstring wd_name = mbox->GetName();
		if (mVersion == eIMAP4rev1)
			wd_name.ToModifiedUTF7(true);

		// Use UNSELECT if supported
		if (mHasUnselect)
		{
			INETStartSend("Status::IMAP::Selecting", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mbox->GetName(), false);
			INETSendString(cUNSELECT, eQueueNoFlags, false);
			INETFinishSend(false);
		}
		else
		{
			// Do EXPUNGE/CLOSE

			// See if already read-only
			if (!mbox->IsReadOnly())
			{
				// Send EXAMINE message to server to force read-only
				INETStartSend("Status::IMAP::Selecting", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mbox->GetName(), false);
				INETSendString(cEXAMINE, eQueueNoFlags, false);
				INETSendString(cSpace, eQueueNoFlags, false);
				INETSendString(wd_name, eQueueProcess, false);
				INETFinishSend(false);
			}

			// Send CLOSE message to server
			if (mVersion != eIMAP2bis)
			{
				INETStartSend("Status::IMAP::Selecting", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", mbox->GetName(), false);
				INETSendString(cCLOSE, eQueueNoFlags, false);
				INETFinishSend(false);
			}
		}
	}
	catch(CINETException& ex)
	{
		CLOG_LOGCATCH(CINETException&);

		// Do nothing...expecting NO response
	}
	catch (std::exception& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Handle error
		INETHandleError(ex, "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect");

		CLOG_LOGRETHROW;
		throw;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handle error
		CINETException iex(CINETException::err_INETUnknown);
		INETHandleError(iex, "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect");

		CLOG_LOGRETHROW;
		throw;
	}

} // CIMAPClient::_Deselect

// Check mbox
void CIMAPClient::_CheckMbox(CMbox* mbox, bool fast)
{
	// New mailcheck scheme:
	// If selected do NOOP
	// If not selected use version specific method:
	// IMAP2bis: SELECT
	// IMAP4: EXAMINE
	// IMAP4rev1: STATUS

	if (GetCurrentMbox() == mbox)
	{
		// Send NOOP message to server
		INETStartSend("Status::IMAP::Checking", "Error::IMAP::OSErrCheck", "Error::IMAP::NoBadCheck", mbox->GetName());
		INETSendString(cNOOP);
		INETFinishSend();
	}
	else
	{
		// Depends on server version
		switch(mVersion)
		{
		case eIMAP2bis:
		case eIMAP4:
		default:
			// Make it current without update using EXAMINE
			GetMboxOwner()->SetCurrentMbox(mbox, false, true);
			break;

		case eIMAP4rev1:
			try
			{
				// Cache actionable mailbox
				StValueChanger<CMbox*> _change(mTargetMbox, mbox);

				// Get full name
				cdstring wd_name = mbox->GetName();
				wd_name.ToModifiedUTF7(true);

				// Send STATUS message to server
				INETStartSend("Status::IMAP::Checking", "Error::IMAP::OSErrCheck", "Error::IMAP::NoBadCheck", mbox->GetName());
				INETSendString(cSTATUS);
				INETSendString(cSpace);
				INETSendString(wd_name, eQueueProcess);
				INETSendString(cSpace);
				INETSendString(cSTATUS_CHECK);
				INETFinishSend();
			}
			catch(CINETException& ex)
			{
				CLOG_LOGCATCH(CINETException&);

				// Cannot select - set flag
				mbox->SetFlags(NMbox::eError);

				CLOG_LOGRETHROW;
				throw;
			}

			// Can select - set flag
			mbox->SetFlags(NMbox::eError, false);
			break;
		}
	}

} // CIMAPClient::_CheckMbox

// Clear (EXPUNGE) current mbox
bool CIMAPClient::_ExpungeMbox(bool closing)
{
	// Send EXPUNGE or CLOSE message to server
	bool did_close = false;
	INETStartSend("Status::IMAP::Expunging", "Error::IMAP::OSErrExpunge", "Error::IMAP::NoBadExpunge", GetCurrentMbox()->GetName());
	if (closing && (mVersion != eIMAP2bis))
	{
		INETSendString(cCLOSE);
		did_close = true;
	}
	else
		INETSendString(cEXPUNGE);
	INETFinishSend();

	return did_close;

} // CIMAPClient::_ExpungeMbox

// Delete mbox
void CIMAPClient::_DeleteMbox(CMbox* mbox)
{
	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send DELETE message to server
	INETStartSend("Status::IMAP::DeletingMbox", "Error::IMAP::OSErrDelete", "Error::IMAP::NoBadDelete", mbox->GetName());
	INETSendString(cIMAP_DELETE);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();

} // CIMAPClient::_DeleteMbox

// Rename mbox
void CIMAPClient::_RenameMbox(CMbox* mbox_old, const char* mbox_new)
{
	// Get full names
	cdstring wd_name_old = mbox_old->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name_old.ToModifiedUTF7(true);

	cdstring wd_name_new = mbox_new;
	if (mVersion == eIMAP4rev1)
		wd_name_new.ToModifiedUTF7(true);

	// Send RENAME message to server
	INETStartSend("Status::IMAP::Renaming", "Error::IMAP::OSErrRename", "Error::IMAP::NoBadRename", mbox_old->GetName());
	INETSendString(cRENAME);
	INETSendString(cSpace);
	INETSendString(wd_name_old, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(wd_name_new, eQueueProcess);
	INETFinishSend();

} // CIMAPClient::_RenameMbox

// Subscribe mbox
void CIMAPClient::_SubscribeMbox(CMbox* mbox)
{
	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send SUBSCRIBE MAILBOX message to server
	INETStartSend("Status::IMAP::Subscribing", "Error::IMAP::OSErrSubscribe", "Error::IMAP::NoBadSubscribe", mbox->GetName());
	INETSendString((mVersion == eIMAP2bis) ? cSUBSCRIBEMBOX : cSUBSCRIBEMBOX4);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();

} // CIMAPClient::_SubscribeMbox

// Unsubscribe mbox
void CIMAPClient::_UnsubscribeMbox(CMbox* mbox)
{
	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send UNSUBSCRIBE MAILBOX message to server
	INETStartSend("Status::IMAP::Unsubscribing", "Error::IMAP::OSErrUnsubscribe", "Error::IMAP::NoBadUnsubscribe", mbox->GetName());
	INETSendString((mVersion == eIMAP2bis) ? cUNSUBSCRIBEMBOX : cUNSUBSCRIBEMBOX4);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();

} // CIMAPClient::_UnsubscribeMbox

// Get namespace
void CIMAPClient::_Namespace(CMboxProtocol::SNamespace* names)
{
	// Only bother if extension supported
	if (!mHasNamespace)
		return;

	mNamespace = names;

	try
	{
		// Issue command
		INETStartSend("Status::IMAP::Namespace", "Error::IMAP::OSErrFindAll", "Error::IMAP::NoBadFindAll");
		INETSendString(cNAMESPACE);
		INETFinishSend();

		// Clean up
		mNamespace = NULL;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		mNamespace = NULL;
		CLOG_LOGRETHROW;
		throw;
	}
}

// Find all subscribed mboxes
void CIMAPClient::_FindAllSubsMbox(CMboxList* mboxes)
{
	// Set flag to add subscribed flag to mailboxes found
	mFindingSubs = true;
	mCurrentWD = mboxes;
	InitItemCtr();

	// Depends on server version
	INETStartSend("Status::IMAP::FindingAllSubs", "Error::IMAP::OSErrFindAll", "Error::IMAP::NoBadFindAll");
	switch(mVersion)
	{
	case eIMAP2bis:
	default:
		// Send FIND MAILBOX * message to server
		INETSendString(cFINDMAILBOXES);
		break;

	case eIMAP4:
	case eIMAP4rev1:
		{
			// Get full name
			cdstring lsub_spec;
			lsub_spec = "\"\" \"";
			lsub_spec += cWILDCARD;
			lsub_spec += '\"';

			// Send LSUB "" "*" message to server
			INETSendString(cLSUB);
			INETSendString(cSpace);
			INETSendString(lsub_spec);
			break;
		}
	}
	INETFinishSend();

} // CIMAPClient::_FindAllSubsMbox

// Find all mboxes
void CIMAPClient::_FindAllMbox(CMboxList* mboxes)
{
	// Unset flag to stop adding subscribed flag
	mFindingSubs = false;
	InitItemCtr();

	// Get the WD (NULL => do hierarchy character descovery)
	mCurrentWD = mboxes;
	cdstring wd = (mboxes ? mboxes->GetRoot() : cdstring::null_str);
	if (mVersion == eIMAP4rev1)
		wd.ToModifiedUTF7(true);

	// Turn on/off flag for hierarchy delimiter search
	StValueChanger<bool> _delimiter(mFindingHier, mboxes == NULL);

	// Depends on server version
	INETStartSend("Status::IMAP::FindingAll", "Error::IMAP::OSErrFindAll", "Error::IMAP::NoBadFindAll");
	switch(mVersion)
	{
	case eIMAP2bis:
	default:
		{
			// Convert % to *
			{
				char* p = wd.c_str_mod();
				while(*p)
				{
					if (*p == '%')
						*p = '*';
					p++;
				}
			}

			// Send FIND ALL.MAILBOX "xxx" message to server
			INETSendString(cFINDALLMAILBOXES);
			INETSendString(cSpace);
			INETSendString(wd, eQueueProcess);
			break;
		}

	case eIMAP4:
	case eIMAP4rev1:
		{
			// Get start spec
			cdstring list_spec = "\"\"";

			// Send LIST "" "xxx" message to server
			INETSendString(cLIST);
			INETSendString(cSpace);
			INETSendString(list_spec);
			INETSendString(cSpace);
			INETSendString(wd, eQueueProcess);
			break;
		}
	}
	INETFinishSend();

} // CIMAPClient::_FindAllMbox

// Append to mbox
void CIMAPClient::_AppendMbox(CMbox* mbox, CMessage* theMsg, unsigned long& new_uid, bool dummy_files)
{
	// Need to repeat this operation if it fails but connection is recovered
	unsigned long repeat = 2;
	while(repeat--)
	{
		try
		{
			new_uid = 0;
			mDummyFiles = dummy_files;

			// Get full name
			cdstring wd_name = mbox->GetName();
			if (mVersion == eIMAP4rev1)
				wd_name.ToModifiedUTF7(true);

			// If IMAP4 then set flags or internal date as well
			cdstring flags;
			cdstring internaldate;
			if (mVersion >= eIMAP4)
			{
				flags += '(';
				bool added = false;
				if (theMsg->IsAnswered())
				{
					flags += cFLAGANSWERED;
					added = true;
				}
				if (theMsg->IsFlagged())
				{
					if (added) flags += SPACE;
					flags += cFLAGFLAGGED;
					added = true;
				}
				if (theMsg->IsDeleted())
				{
					if (added) flags += SPACE;
					flags += cFLAGDELETED;
					added = true;
				}
				if (!theMsg->IsUnseen())
				{
					if (added) flags += SPACE;
					flags += cFLAGSEEN;
					added = true;
				}
				if (theMsg->IsDraft())
				{
					if (added) flags += SPACE;
					flags += cFLAGDRAFT;
					added = true;
				}
				if (theMsg->IsMDNSent())
				{
					if (added) flags += SPACE;
					flags += cFLAGMDNSENT;
					added = true;
				}
				for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
				{
					if (theMsg->HasLabel(i))
					{
						if (added) flags += SPACE;
						flags += CPreferences::sPrefs->mIMAPLabels.GetValue()[i];
						added = true;
					}
				}

				// Only add if flags specified
				if (added)
					flags += ")";
				else
					flags = cdstring::null_str;
				
				// Must have non-zero internal date
				if (theMsg->GetInternalDate())
				{
					internaldate = "\"";
					internaldate += CRFC822::GetIMAPDate(theMsg->GetInternalDate(), theMsg->GetInternalZone());
					internaldate += "\"";
				}
			}

			// Send APPEND message to server
			INETStartSend("Status::IMAP::Appending", "Error::IMAP::OSErrAppend", "Error::IMAP::NoBadAppend", mbox->GetName());
			INETSendString(cAPPEND, eQueueNoFlags);
			INETSendString(cSpace, eQueueNoFlags);
			INETSendString(wd_name, eQueueProcess);
			INETSendString(cSpace, eQueueNoFlags);
			if (flags.length())
			{
				INETSendString(flags, eQueueNoFlags);
				INETSendString(cSpace, eQueueNoFlags);
			}
			if (internaldate.length())
			{
				INETSendString(internaldate, eQueueNoFlags);
				INETSendString(cSpace, eQueueNoFlags);
			}
			mProcessMessage = theMsg;
			INETSendString(NULL, eQueueManualLiteral);
			INETFinishSend();

			// Check APPENDUID status from OK response text
			if (mLastResponse.FindTagged(cAPPENDUID))
			{
				cdstring temp = mLastResponse.GetTagged();
				char* p = ::strstrnocase(temp.c_str(), cAPPENDUID);
				if (p)
				{
					p += ::strlen(cAPPENDUID);

					unsigned long uidv = ::strtoul(p, &p, 10);
					unsigned long uid = ::strtoul(p, &p, 10);

					// Must check UIDValidity
					if (mbox->HasStatus() &&
						(mbox->GetUIDValidity() != uidv))
						new_uid = 0;
					else
						new_uid = uid;
				}
			}

			mProcessMessage = NULL;
			
			// Set count to zero to indicate success
			repeat = 0;
		}
		catch(CNetworkException& ex)
		{
			CLOG_LOGCATCH(CNetworkException&);

			// If it didn't recover the connection or no more loops throw out of here
			// otherwise we wll go through the loop one more time
			if (!ex.reconnected() || !repeat)
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}
	}

} // CIMAPClient::_AppendMbox

// Search messages on the server
void CIMAPClient::_SearchMbox(const CSearchItem* spec, ulvector* results, bool uids)
{
	// IMAP2bis can't do UID SEARCH
	if (uids && (mVersion == eIMAP2bis))
		return;

	try
	{
		// Set results pointer
		mCurrentResults = results;

		// Issue SEARCH call
		INETStartSend("Status::IMAP::Searching", "Error::IMAP::OSErrSearch", "Error::IMAP::NoBadSearch", GetCurrentMbox()->GetName());
		INETSendString(uids ? cUIDSEARCH : cSEARCH);

		// Parse out search hierarchy!
		AddSearchItem(spec);
		INETFinishSend();

		// Reset results
		mCurrentResults = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		mCurrentResults = NULL;

		CLOG_LOGRETHROW;
		throw;
	}

} // CIMAPClient::_ReadSize

// Add search item to command line
void CIMAPClient::AddSearchItem(const CSearchItem* spec, bool force_charset)
{
	cdstrboolvect items;
	if (spec)
		spec->GenerateItems(items);
	bool add_charset = false;

	for(cdstrboolvect::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		// Check for non-US-ASCII
		const char* p = (*iter).first.c_str();

		// Check for any to be encoded
		while(!add_charset && *p)
		{
			if (!cUSASCIIChar[(unsigned char) *p++])
				add_charset = true;
		}
	}

	// Add charset if required
	if (add_charset)
	{
		INETSendString(cSpace);
		INETSendString(cSEARCH_CHARSET);
	}
	if (add_charset || force_charset)
	{
		INETSendString(cSpace);
		INETSendString(i18n::CCharsetManager::sCharsetManager.GetNameFromCode(add_charset ? i18n::eUTF8 : i18n::eUSASCII));
	}

	if (!spec)
	{
		INETSendString(cSpace);
		INETSendString(cSEARCH_ALL);
	}
	else
	{
		bool no_space = false;
		for(cdstrboolvect::iterator iter = items.begin(); iter != items.end(); iter++)
		{
			// (*iter).first - the string to add to search spec
			// (*iter).second - indicates whether ATOM or string processing required on the text
			
			// Don't add a space before ")"
			if (!(*iter).second && ((*iter).first == ")"))
				no_space = true;

			if (!no_space)
				INETSendString(cSpace);
			INETSendString((*iter).first, (*iter).second ? eQueueProcess : eQueueNoFlags);

			// No space following a "("
			if (!(*iter).second && ((*iter).first == "("))
				no_space = true;
			else
				no_space = false;
		}
	}
}

#pragma mark ____________________________Messages

// Do fetch envelopes
void CIMAPClient::_FetchItems(const ulvector& nums, bool uids, CMboxProtocol::EFetchItems items)
{
	//StProfileSection profile("\pMulberry Profile", 200, 20);

	// Fetch info for new messages in cache
	CSequence sequence(nums);
	const char* msgnum_txt = sequence.GetSequenceText();

	// Reset item counter for feedback
	InitItemCtr(nums.size());

	// Depends on server version
	INETStartSend("Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDFETCH : cFETCH);
	INETSendString(cSpace);
	INETSendString(msgnum_txt);
	INETSendString(cSpace);
	if (items & CMboxProtocol::eSummary)
	{
		switch(mVersion)
		{
		case eIMAP2bis:
		default:
			INETSendString(cSUMMARY);
			break;

		case eIMAP4:
		case eIMAP4rev1:
			INETSendString(cSUMMARY4);
			break;
		}
	}
	else
	{
		bool spaced = false;
		INETSendString("(");
		if (items & CMboxProtocol::eEnvelope)
		{
			if (spaced) INETSendString(cSpace);
			INETSendString(cENVELOPE);
			spaced = true;
		}
		if (items & CMboxProtocol::eUID)
		{
			if (spaced) INETSendString(cSpace);
			INETSendString(cUID);
			spaced = true;
		}
		if (items & CMboxProtocol::eSize)
		{
			if (spaced) INETSendString(cSpace);
			INETSendString(cRFC822SIZE);
			spaced = true;
		}
		if (items & CMboxProtocol::eFlags)
		{
			if (spaced) INETSendString(cSpace);
			INETSendString(cFLAGS);
			spaced = true;
		}
		INETSendString(")");
	}
	INETFinishSend();

} // CIMAPClient::_FetchItems

// Get header list from messages
void CIMAPClient::_ReadHeaders(const ulvector& nums, bool uids, const cdstring& hdrs)
{
	// Fetch info for new messages in cache
	CSequence sequence(nums);
	const char* msgnum_txt = sequence.GetSequenceText();

	// Reset item counter for feedback
	InitItemCtr(nums.size());

	// Issue FETCH (BODY[#]) call
	cdstring body_spec;
	if (hdrs.empty())
		body_spec = cBODYHEADER;
	else
	{
		body_spec = cBODYHEADERFIELDS;
		body_spec += cSpace;
		body_spec += "(";
		body_spec += hdrs;
		body_spec += ")";
	}
	cdstring body;
	size_t body_reserve = ::strlen(cBODYSECTIONPEEK_OUT) + body_spec.length() + 1;
	body.reserve(body_reserve);
	::snprintf(body.c_str_mod(), body_reserve, cBODYSECTIONPEEK_OUT, body_spec.c_str());

	// Issue FETCH (BODY[#]) call
	INETStartSend("Status::IMAP::Fetching", "Error::IMAP::OSErrSelect", "Error::IMAP::NoBadSelect", GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDFETCH : cFETCH);
	INETSendString(cSpace);
	INETSendString(msgnum_txt);
	INETSendString(cSpace);
	INETSendString(body);
	INETFinishSend();
}

// Get message header text from server
void CIMAPClient::_ReadHeader(CMessage* msg)
{
	// Is it a message/rfc822?
	if (msg->GetOwner())
	{
		// Try to read from server
		// Create text stream
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		LHandleStream data;
#elif __dest_os == __win32_os || __dest_os == __linux_os
		LMemFileStream data;
#endif

		// Get appropriate filter
		CFilter* aFilter = new CFilterEndls;
		aFilter->SetStream(&data);

		// Get section description
		cdstring section;
		msg->GetBody()->GetPartNumber(section);

		if (msg->GetBody()->GetParts())
			// Append header spec if multiple parts
			section += ".0";
		else
			// Replace first part spec if single part
			section[section.length() - 1] = '0';

		try
		{
			// Get owning message
			CMessage* top_message = msg;
			while(top_message->GetOwner())
				top_message = top_message->GetOwner();

			// Read in from section description
			_ReadAttachment(top_message->GetMessageNumber(), section, aFilter);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			if (data.GetMarker())
#elif __dest_os == __win32_os || __dest_os == __linux_os
			if (data.GetLength())
#else
#error __dest_os
#endif
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				StHandleLocker lock(data.GetDataHandle());

				// Make a copy of the text
				msg->SetHeader(::strndup(*data.GetDataHandle(), data.GetMarker()));
#elif __dest_os == __win32_os || __dest_os == __linux_os
				// Must write c-string terminator
				char end = 0;
				data.WriteBlock(&end, 1);

				// Get original data
				msg->SetHeader(data.DetachData());
#else
#error __dest_os
#endif
			}
			else
				// Give it empty string
				msg->SetHeader(NULL);

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up
			delete aFilter;

			CLOG_LOGRETHROW;
			throw;
		}

		// Clean up
		delete aFilter;
	}
	else
	{
		// Issue FETCH (RFC822.HEADER) call
		char msg_spec[256];
		::snprintf(msg_spec, 256, "%ld", msg->GetMessageNumber());

		// Reset item counter for feedback
		InitItemCtr(1);

		INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
		INETSendString(cFETCH);
		INETSendString(cSpace);
		INETSendString(msg_spec);
		INETSendString(cSpace);
		INETSendString(cRFC822HEADER_OUT);
		INETFinishSend();
	}

} // CIMAPClient::_ReadHeader

// Get attachment data into stream
void CIMAPClient::_ReadAttachment(unsigned long msg_num, CAttachment* attach,
									LStream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Get section description
	cdstring attach_spec;
	attach->GetPartNumber(attach_spec);

	// Read in from section description
	_ReadAttachment(msg_num, attach_spec, aStream, peek, count, start);
}

// Get attachment data into stream
void CIMAPClient::_ReadAttachment(unsigned long msg_num, const char* attach_spec,
									LStream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Can only do [*.0] parts (message/rfc822 header) if IMAP4+
	if (mVersion == eIMAP2bis)
	{
		// Look for illegal body spec
		if ((::strcmp(attach_spec, "0") == 0) ||
			(::strcmp(attach_spec + ::strlen(attach_spec) - 2, ".0") == 0))
			return;
	}

	cdstring modified_spec = attach_spec;

	// Use IMAP4rev1 command syntax
	if (mVersion == eIMAP4rev1)
	{
		if (::strcmp(modified_spec, "0") == 0)
			modified_spec = cBODYHEADER;
		else if (::strcmp(modified_spec.c_str() + modified_spec.length() - 2, ".0") == 0)
		{
			((char*) modified_spec.c_str())[modified_spec.length() - 1] = 0;
			modified_spec += cBODYHEADER;
		}
	}

	// Set up streams
	mRcvStream = aStream;
	mRcvOStream = NULL;

	// Reset item counter for feedback
	InitItemCtr(1);

	// Partial required?
	INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	if (!count)
	{
		// Issue FETCH (BODY[#]) call
		char msg[32];
		::snprintf(msg, 32, "%ld", msg_num);
		cdstring body;
		switch(mVersion)
		{
		case eIMAP2bis:
		{
			// BODY - no PEEK
			size_t body_reserve = ::strlen(cBODYSECTION_OUT) + modified_spec.length() + 1;
			body.reserve(body_reserve);
			::snprintf(body.c_str_mod(), body_reserve, cBODYSECTION_OUT, modified_spec.c_str());
			break;
		}
		default:
		{
			// BODY - PEEK
			size_t body_reserve = ::strlen(peek ? cBODYSECTIONPEEK_OUT : cBODYSECTION_OUT) + modified_spec.length() + 1;
			body.reserve(body_reserve);
			::snprintf(body.c_str_mod(), body_reserve, peek ? cBODYSECTIONPEEK_OUT : cBODYSECTION_OUT, modified_spec.c_str());
			break;
		}
		}

		INETSendString(cFETCH);
		INETSendString(cSpace);
		INETSendString(msg);
		INETSendString(cSpace);
		INETSendString(body);
	}
	else
	{
		const char* cmd = NULL;
		const char* templ = NULL;
		
		switch(mVersion)
		{
		case eIMAP2bis:
			// PARTIAL - no PEEK
			cmd = cPARTIAL;
			templ = "%d BODY[%s] %d %d";
			break;

		case eIMAP4:
		default:
			// PARTIAL - PEEK
			cmd = cPARTIAL;
			templ = peek ? "%d BODY.PEEK[%s] %d %d" : "%d BODY[%s] %d %d";
			break;

		case eIMAP4rev1:
			// <> syntax - PEEK
			cmd = cFETCH;
			templ = peek ? "%d BODY.PEEK[%s]<%d.%d>" : "%d BODY[%s]<%d.%d>";
			break;
		}

		// Issue (PARTIAL FETCH) BODY[#]( # #)(<#.#>) call
		cdstring msg;
		size_t msg_reserve = 64 + modified_spec.length();
		msg.reserve(msg_reserve);
		::snprintf(msg.c_str_mod(), msg_reserve, templ, msg_num, modified_spec.c_str(), start, count);

		INETSendString(cmd);
		INETSendString(cSpace);
		INETSendString(msg);
	}

	INETFinishSend();

} // CIMAPClient::_ReadAttachment

// Copy raw attachment data into stream
void CIMAPClient::_CopyAttachment(unsigned long msg_num, CAttachment* attach,
									costream* aStream, bool peek, unsigned long count, unsigned long start)
{
	// Get fetch specifier
	std::ostrstream out;
	if (attach)
	{
		// Get section description
		cdstring attach_spec;
		attach->GetPartNumber(attach_spec);

		// Use IMAP4rev1 command syntax
		if ((mVersion == eIMAP4rev1) &&
			(::strcmp(attach_spec.c_str() + attach_spec.length() - 2, ".0") == 0))
		{
			((char*) attach_spec.c_str())[attach_spec.length() - 1] = 0;
			attach_spec += cBODYHEADER;
		}

		// Do BODY[%s]
		switch(mVersion)
		{
		case eIMAP2bis:
			// BODY - no PEEK
			out << cBODYSECTION_IN << attach_spec << "]";
			break;
		default:
			// BODY - PEEK
			out << (peek ? cBODYSECTIONPEEK_IN : cBODYSECTION_IN) << attach_spec << "]";
			break;
		}
	}
	else
	{
		// No attachment => entire message body
		switch(mVersion)
		{
		case eIMAP2bis:
			out << cRFC822TEXT;
			break;

		case eIMAP4:
		default:
			out << (peek ? cRFC822TEXTPEEK : cRFC822TEXT);
			break;

		case eIMAP4rev1:
			out << (peek ? cBODYPEEKTEXT : cBODYTEXT);
			break;
		}
	}

	// Check for partial start/length
	if (count)
	{
		// Use PARTIAL syntax for older servers
		const char* templ = NULL;
		switch(mVersion)
		{
		case eIMAP2bis:
		case eIMAP4:
		default:
			out << " " << start << " " << count;
			break;

		case eIMAP4rev1:
			out << "<" << start << "." << count << ">";
			break;
		}
	}
	out << std::ends;
	cdstring spec;
	spec.steal(out.str());

	// Set up streams
	mRcvOStream = aStream;
	mRcvStream = NULL;

	// Reset item counter for feedback
	InitItemCtr(1);

	// Partial required?
	INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	char msg[32];
	::snprintf(msg, 32, "%ld", msg_num);
	if (!count)
	{
		// Issue FETCH nn (xx) call
		INETSendString(cFETCH);
		INETSendString(cSpace);
		INETSendString(msg);
		INETSendString(cSpace);
		INETSendString(spec);
	}
	else
	{
		// Determine command based on server type
		const char* cmd = NULL;
		switch(mVersion)
		{
		case eIMAP2bis:
		case eIMAP4:
		default:
			cmd = cPARTIAL;
			break;

		case eIMAP4rev1:
			cmd = cFETCH;
			break;
		}

		// Issue (PARTIAL FETCH) nn xx( # #)(<#.#>) call
		INETSendString(cmd);
		INETSendString(cSpace);
		INETSendString(msg);
		INETSendString(cSpace);
		INETSendString(spec);
	}
	INETFinishSend();

} // CIMAPClient::_CopyAttachment

// Set specified flag
void CIMAPClient::_SetFlag(const ulvector& nums, bool uids, NMessage::EFlags flags, bool set)
{
	const char* status_strid;
	const char* oserr_strid;
	const char* nobad_strid;

	// Send xFLAGS /DELETED message to server
	CSequence sequence(nums);
	const char* msgnum_txt = sequence.GetSequenceText();

	// Form flag string
	cdstring flag;
	if (set)
		flag += cSET_FLAG;
	else
		flag += cUNSET_FLAG;

	// Add each flag
	bool got_one = false;
	if (flags & NMessage::eAnswered)
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGANSWERED;
		got_one = true;

		status_strid = "Status::IMAP::MarkingAnswered";
		oserr_strid = "Error::IMAP::OSErrAnsweredMsg";
		nobad_strid = "Error::IMAP::NoBadAnsweredMsg";
	}
	if (flags & NMessage::eFlagged)
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGFLAGGED;
		got_one = true;

		status_strid = "Status::IMAP::MarkingFlagged";
		oserr_strid = "Error::IMAP::OSErrFlaggedMsg";
		nobad_strid = "Error::IMAP::NoBadFlaggedMsg";
	}
	if (flags & NMessage::eDeleted)
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGDELETED;
		got_one = true;

		status_strid = (set ? "Status::IMAP::Deleting" : "Status::IMAP::Undeleting");
		oserr_strid = (set ? "Error::IMAP::OSErrDeleteMsg" : "Error::IMAP::OSErrUndeleteMsg");
		nobad_strid = (set ? "Error::IMAP::NoBadDeleteMsg" : "Error::IMAP::NoBadUndeleteMsg");
	}
	if (flags & NMessage::eSeen)
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGSEEN;
		got_one = true;

		status_strid = "Status::IMAP::MarkingSeen";
		oserr_strid = "Error::IMAP::OSErrSeenMsg";
		nobad_strid = "Error::IMAP::NoBadSeenMsg";
	}
	if ((flags & NMessage::eDraft) && (mVersion != eIMAP2bis))	// Only IMAP4 and above
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGDRAFT;
		got_one = true;

		status_strid = "Status::IMAP::MarkingDraft";
		oserr_strid = "Error::IMAP::OSErrDraftMsg";
		nobad_strid = "Error::IMAP::NoBadDraftMsg";
	}
	if ((flags & NMessage::eMDNSent) && (mVersion != eIMAP2bis))	// Only IMAP4 and above
	{
		if (got_one)
			flag += ' ';
		flag += cFLAGMDNSENT;
		got_one = true;

		status_strid = "Status::IMAP::MarkingLabel";
		oserr_strid = "Error::IMAP::OSErrLabelMsg";
		nobad_strid = "Error::IMAP::NoBadLabelMsg";
	}
	if ((flags & NMessage::eLabels) && (mVersion != eIMAP2bis))	// Only IMAP4 and above
	{
		// Scan over all labels and add each
		for(int i = 0; i < NMessage::eMaxLabels; i++)
		{
			if (flags & (NMessage::eLabel1 << i))
			{
				if (got_one)
					flag += ' ';
				flag += CPreferences::sPrefs->mIMAPLabels.GetValue()[i];
				got_one = true;
			}
		}

		status_strid = "Status::IMAP::MarkingLabel";
		oserr_strid = "Error::IMAP::OSErrLabelMsg";
		nobad_strid = "Error::IMAP::NoBadLabelMsg";
	}
	flag += cFLAG_END;

	INETStartSend(status_strid, oserr_strid, nobad_strid, GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDSTORE : cSTORE);
	INETSendString(cSpace);
	INETSendString(msgnum_txt);
	INETSendString(cSpace);
	INETSendString(flag);
	INETFinishSend();

} // CIMAPClient::_SetFlag

// Copy specified message to specified mailbox
void CIMAPClient::_CopyMessage(const ulvector& nums, bool uids, CMbox* mbox_to, ulmap& copy_uids)
{
	// Get full name
	cdstring wd_name = mbox_to->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send COPY message to server
	CSequence sequence(nums);
	const char* msgnum_txt = sequence.GetSequenceText();

	INETStartSend("Status::IMAP::Copying", "Error::IMAP::OSErrCopyMsg", "Error::IMAP::NoBadCopyMsg", GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDCOPY : cCOPY, eQueueNoFlags);
	INETSendString(cSpace, eQueueNoFlags);
	INETSendString(msgnum_txt, eQueueNoFlags);
	INETSendString(cSpace, eQueueNoFlags);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();

	// Check COPYUID status from OK response text
	if (mLastResponse.FindTagged(cCOPYUID))
	{
		const cdstring& temp = mLastResponse.GetTagged();
		const char* p = ::strstrnocase(temp, cCOPYUID);
		if (p)
		{
			p += ::strlen(cCOPYUID);

			unsigned long uidv = ::strtoul(p, const_cast<char**>(&p), 10);
			CSequence source;
			CSequence destination;
			source.ParseSequence(&p, nums.size());
			destination.ParseSequence(&p, nums.size());

			// Now add to map
			CSequence::const_iterator iter1 = source.begin();
			CSequence::const_iterator iter2 = destination.begin();
			while((iter1 != source.end()) && (iter2 != destination.end()))
			{
				copy_uids.insert(ulmap::value_type(*iter1, *iter2));
				iter1++;
				iter2++;
			}
		}
	}

} // CIMAPClient::_CopyMessage

// Do copy message to stream
void CIMAPClient::_CopyMessage(unsigned long msg_num, bool uids, costream* aStream, unsigned long count, unsigned long start)
{
	// Set up streams
	mRcvOStream = aStream;
	mRcvStream = NULL;

	// Reset item counter for feedback
	InitItemCtr(1);

	// Partial required?
	INETStartSend("Status::IMAP::FetchingOne", "Error::IMAP::OSErrReadMsg", "Error::IMAP::NoBadReadMsg", GetCurrentMbox()->GetName());
	if (!count)
	{
		// Issue FETCH (RFC822) call
		char msg[32];
		::snprintf(msg, 32, "%ld", msg_num);

		INETSendString(uids ? cUIDFETCH : cFETCH);
		INETSendString(cSpace);
		INETSendString(msg);
		INETSendString(cSpace);
		switch(mVersion)
		{
		case eIMAP2bis:
		default:
			INETSendString(cdstring(cRFC822_OUT));
			break;

		case eIMAP4:
			INETSendString(cdstring(cRFC822PEEK_OUT));
			break;

		case eIMAP4rev1:
			INETSendString(cdstring(cBODYALL_OUT));
			break;
		}
	}
	else
	{
		// Issue PARTIAL RFC822 # # call
		const char* cmd = NULL;
		const char* templ = NULL;
		switch(mVersion)
		{
		case eIMAP2bis:
		default:
			cmd = cPARTIAL;
			templ = "%d RFC822 %d %d";
			break;

		case eIMAP4:
			cmd = cPARTIAL;
			templ = "%d RFC822.PEEK %d %d";
			break;

		case eIMAP4rev1:
			cmd = cFETCH;
			templ = "%d BODY[]<%d.%d>";
			break;
		}
		char msg[256];
		::snprintf(msg, 256, templ, msg_num, start, count);

		INETSendString(cmd);
		INETSendString(cSpace);
		INETSendString(msg);
	}
	INETFinishSend();
}

// Expunge messages
void CIMAPClient::_ExpungeMessage(const ulvector& nums, bool uids)
{
	ulvector actual_uids;
	if (uids)
		actual_uids = nums;
	else
		GetCurrentMbox()->MapUIDs(nums, actual_uids);

	CSequence sequence(actual_uids);
	const char* msgnum_txt = sequence.GetSequenceText();

	INETStartSend("Status::IMAP::Expunging", "Error::IMAP::OSErrExpunge", "Error::IMAP::NoBadExpunge", GetCurrentMbox()->GetName());
	INETSendString(cUIDEXPUNGE);
	INETSendString(cSpace);
	INETSendString(msgnum_txt);
	INETFinishSend();
}

#pragma mark ____________________________Sort/Thread

// Does server-side sorting
bool CIMAPClient::_DoesSort(ESortMessageBy sortby) const
{
	if (mHasSort)
	{
		switch(sortby)
		{
		case cSortMessageTo:
		case cSortMessageFrom:
		case cSortMessageCc:
		case cSortMessageSubject:
		case cSortMessageDateSent:
		case cSortMessageDateReceived:
		case cSortMessageSize:
			return true;
		default:
			return false;
		}
	}
	else
		return false;
}

// Do server-side sort
void CIMAPClient::_Sort(ESortMessageBy sortby, EShowMessageBy show_by, const CSearchItem* search, ulvector* results, bool uids)
{
	StValueChanger<ulvector*> change(mCurrentResults, results);
	
	INETStartSend("Status::IMAP::Sort", "Error::IMAP::OSErrSort", "Error::IMAP::NoBadSort", GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDSORT : cSORT);
	INETSendString(cSpace);
	cdstring temp = "(";
	switch(show_by)
	{
	case cShowMessageAscending:
	default:
		break;
	case cShowMessageDescending:
		temp += cSORT_REVERSE;
		temp += cSpace;
		break;
	}

	switch(sortby)
	{
	case cSortMessageTo:
		temp += cSORT_TO;
		break;
	case cSortMessageFrom:
		temp += cSORT_FROM;
		break;
	case cSortMessageCc:
		temp += cSORT_CC;
		break;
	case cSortMessageSubject:
		temp += cSORT_SUBJECT;
		break;
	case cSortMessageDateSent:
	default:
		temp += cSORT_DATE;
		break;
	case cSortMessageDateReceived:
		temp += cSORT_ARRIVAL;
		break;
	case cSortMessageSize:
		temp += cSORT_SIZE;
		break;
	}
	temp += ")";
	INETSendString(temp);
	// Parse out search hierarchy with charset
	AddSearchItem(search, true);
	INETFinishSend();
}
	
// Does server-side threading
bool CIMAPClient::_DoesThreading(EThreadMessageBy threadby) const
{
	switch(threadby)
	{
	case cThreadMessageAny:
		return mHasThreadSubject || mHasThreadReferences;
	case cThreadMessageSubject:
		return mHasThreadSubject;
	case cThreadMessageReferences:
		return mHasThreadReferences;
	default:
		return false;
	}
}

// Do server-side thread
void CIMAPClient::_Thread(EThreadMessageBy threadby, const CSearchItem* search, threadvector* results, bool uids)
{
	StValueChanger<threadvector*> change(mThreadResults, results);
	
	INETStartSend("Status::IMAP::Thread", "Error::IMAP::OSErrThread", "Error::IMAP::NoBadThread", GetCurrentMbox()->GetName());
	INETSendString(uids ? cUIDTHREAD : cTHREAD);
	INETSendString(cSpace);
	switch(threadby)
	{
	case cThreadMessageSubject:
	default:
		INETSendString(cTHREAD_SUBJECT);
		break;
	case cThreadMessageReferences:
		INETSendString(cTHREAD_REFERENCES);
		break;
	}
	// Parse out search hierarchy with charset
	AddSearchItem(search, true);
	INETFinishSend();
}

#pragma mark ____________________________ACLs

// Set acl on server
void CIMAPClient::_SetACL(CMbox* mbox, CACL* acl)
{
	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);
	cdstring acl_txt = acl->GetFullTextRights();

	// Send SETACL message to server
	INETStartSend("Status::IMAP::SetACL", "Error::IMAP::OSErrSetACL", "Error::IMAP::NoBadSetACL", mbox->GetName());
	INETSendString(cSETACL);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(acl_txt);
	INETFinishSend();
}

// Delete acl on server
void CIMAPClient::_DeleteACL(CMbox* mbox, CACL* acl)
{
	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send DELETEACL message to server
	INETStartSend("Status::IMAP::DeleteACL", "Error::IMAP::OSErrDeleteACL", "Error::IMAP::NoBadDeleteACL", mbox->GetName());
	INETSendString(cDELETEACL);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(acl->GetUID());
	INETFinishSend();

}

// Get all acls for mailbox from server
void CIMAPClient::_GetACL(CMbox* mbox)
{
	// Cache actionable mailbox
	StValueChanger<CMbox*> _change(mTargetMbox, mbox);

	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send GETACL message to server
	INETStartSend("Status::IMAP::GetACL", "Error::IMAP::OSErrGetACL", "Error::IMAP::NoBadGetACL", mbox->GetName());
	INETSendString(cGETACL);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();
}

// Get allowed rights for user
void CIMAPClient::_ListRights(CMbox* mbox, CACL* acl)
{
	// Cache actionable mailbox & ACL
	StValueChanger<CMbox*> _change1(mTargetMbox, mbox);
	StValueChanger<CACL*> _change2(mACL, acl);

	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send LISTRIGHTS message to server
	INETStartSend("Status::IMAP::ListRights", "Error::IMAP::OSErrListRights", "Error::IMAP::NoBadListRights", mbox->GetName());
	INETSendString(cLISTRIGHTS);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETSendString(cSpace);
	INETSendString(acl->GetUID());
	INETFinishSend();
}

// Get current user's rights to mailbox
void CIMAPClient::_MyRights(CMbox* mbox)
{
	// Cache actionable mailbox & ACL
	StValueChanger<CMbox*> _change(mTargetMbox, mbox);

	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send MYRIGHTS message to server
	INETStartSend("Status::IMAP::MyRights", "Error::IMAP::OSErrMyRights", "Error::IMAP::NoBadMyRights", mbox->GetName());
	INETSendString(cMYRIGHTS);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();
}

#pragma mark ____________________________Quotas

// Set quota root values on server
void CIMAPClient::_SetQuota(CQuotaRoot* root)
{
	// Create list of items
	cdstring list = '(';
	bool first = true;
	const CQuotaItemList& items = root->GetItems();
	for(CQuotaItemList::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		// Prepend space if required
		if (!first)
			list += ' ';
		else
			first = false;

		// Add item name
		list += (*iter).GetItem();
		list += ' ';
		list += cdstring((*iter).GetMax());
	}
	list += ')';

	// Send SETQUOTA message to server
	INETStartSend("Status::IMAP::SetQuota", "Error::IMAP::OSErrSetQuota", "Error::IMAP::NoBadSetQuota", GetCurrentMbox()->GetName());
	INETSendString(cSETQUOTA);
	INETSendString(cSpace);
	INETSendString(root->GetName(), eQueueProcess);
	INETSendString(cSpace);
	INETSendString(list);
	INETFinishSend();
}

// Get quota root values from server
void CIMAPClient::_GetQuota(CQuotaRoot* root)
{
	// Send GETQUOTA message to server
	INETStartSend("Status::IMAP::GetQuota", "Error::IMAP::OSErrGetQuota", "Error::IMAP::NoBadGetQuota", GetCurrentMbox()->GetName());
	INETSendString(cGETQUOTA);
	INETSendString(cSpace);
	INETSendString(root->GetName(), eQueueProcess);
	INETFinishSend();
}

// Get quota roots for a mailbox
void CIMAPClient::_GetQuotaRoot(CMbox* mbox)
{
	// Cache actionable mailbox
	StValueChanger<CMbox*> _change(mTargetMbox, mbox);

	// Get full name
	cdstring wd_name = mbox->GetName();
	if (mVersion == eIMAP4rev1)
		wd_name.ToModifiedUTF7(true);

	// Send GETQUOTAROOT message to server
	INETStartSend("Status::IMAP::GetQuotaRoot", "Error::IMAP::OSErrGetQuotaRoot", "Error::IMAP::NoBadGetQuotaRoot", mbox->GetName());
	INETSendString(cGETQUOTAROOT);
	INETSendString(cSpace);
	INETSendString(wd_name, eQueueProcess);
	INETFinishSend();
}

#pragma mark ____________________________Others

// Get length of manually processed literal
int CIMAPClient::GetManualLiteralLength()
{
	// Check for cached mbox message and one that is not local
	if (mProcessMessage->GetMbox() &&
		!mProcessMessage->IsSubMessage() &&
		!dynamic_cast<CLocalMessage*>(mProcessMessage))
		return mProcessMessage->GetSize();
	else
	{
		// Create counting stream
		ctrbuf ctr;
		std::ostream octr(&ctr);

		// Make sure right line endings are used
		costream stream_out(&octr, eEndl_CRLF);

		// Write message to stream (indicate we want a copy of the original message)
		mProcessMessage->WriteToStream(stream_out, mDummyFiles, NULL, true);

		// Return byte count
		return ctr.get_size();
	}
}

// Send manual literal
void CIMAPClient::SendManualLiteral()
{
	// Write message to network stream
	CNetworkAttachProgress progress;

	// Message may not have body if only temporary
	if (mProcessMessage->GetBody())
		progress.SetTotal(mProcessMessage->GetBody()->CountParts());
	else
		progress.SetTotal(1);

	// Make sure right line endings are used
	costream stream_out(mStream, eEndl_CRLF);

	// Write message to stream (indicate we want a copy of the original message)
	mProcessMessage->WriteToStream(stream_out, mDummyFiles, &progress, true);
}

#pragma mark ____________________________Handle Errors

// Force reconnect
void CIMAPClient::INETRecoverReconnect()
{
	CMailControl::MboxServerReconnect(GetMboxOwner());
}

// Force disconnect
void CIMAPClient::INETRecoverDisconnect()
{
	CMailControl::MboxServerDisconnect(GetMboxOwner());
}

// Descriptor for object error context
const char*	CIMAPClient::INETGetErrorDescriptor() const
{
	return "Mailbox: ";
}

#pragma mark ____________________________Parsing

// Parse text sent by server (advance pointer to next bit to be parsed)
void CIMAPClient::IMAPParseResponse(char** txt, CINETClientResponse* response)
{
	// Found a mailbox - do this here to improve performance
	if (::stradvtokcmp(txt,cLIST)==0)
	{
		response->code = cStarLIST;
		IMAPParseListLsub(txt, false);
	}

	// Found a subscribed mailbox - do this here to improve performance
	else if (::stradvtokcmp(txt,cLSUB)==0)
	{
		response->code = cStarLSUB;
		IMAPParseListLsub(txt, true);
	}

	// Found a mailbox - do this here to improve performance
	else if (::stradvtokcmp(txt,cMAILBOX)==0)
	{
		response->code = cStarMAILBOX;
		IMAPParseMailbox(txt, GetMboxOwner()->GetMailAccount()->GetDirDelim(), NMbox::eNoInferiors);
	}

	// Got search spec
	else if (::stradvtokcmp(txt,cSEARCH)==0)
	{
		response->code = cStarSEARCH;
		IMAPParseSearch(txt);
	}

	// Got status
	else if (::stradvtokcmp(txt,cSTATUS)==0)
	{
		response->code = cStarSTATUS;
		IMAPParseStatus(txt);
	}

	// Flags so parse them
	else if (::stradvtokcmp(txt,cFLAGS)==0)
		response->code = cStarFLAGS;

	else if (::stradvtokcmp(txt,cBBOARD)==0)
		response->code = cStarBBOARD;

	// E X T E N S I O N S

	// SORT/THREAD
	else if (::stradvtokcmp(txt,cSORT)==0)
	{
		response->code = cStarSORT;
		IMAPParseSort(txt);
	}
	else if (::stradvtokcmp(txt,cTHREAD)==0)
	{
		response->code = cStarTHREAD;
		IMAPParseThread(txt);
	}

	// ACL
	else if (::stradvtokcmp(txt,cACL)==0)
	{
		response->code = cStarACL;
		IMAPParseACL(txt);
	}

	else if (::stradvtokcmp(txt,cLISTRIGHTS)==0)
	{
		response->code = cStarLISTRIGHTS;
		IMAPParseListRights(txt);
	}

	else if (::stradvtokcmp(txt,cMYRIGHTS)==0)
	{
		response->code = cStarMYRIGHTS;
		IMAPParseMyRights(txt);
	}

	// QUOTA
	else if (::stradvtokcmp(txt,cQUOTA)==0)
	{
		response->code = cStarQUOTA;
		IMAPParseQuota(txt);
	}

	else if (::stradvtokcmp(txt,cQUOTAROOT)==0)
	{
		response->code = cStarQUOTAROOT;
		IMAPParseQuotaRoot(txt);
	}

	// NAMESPACE
	else if (::stradvtokcmp(txt,cNAMESPACE)==0)
	{
		response->code = cStarNAMESPACE;
		IMAPParseNamespace(txt);
	}

	else
		// Handle <n> message
		IMAPParseMessageResponse(txt,response);

} // CIMAPClient::IMAPParseResponse

// Parse <n> message text sent by server
void CIMAPClient::IMAPParseMessageResponse(char** txt, CINETClientResponse* response)
{
	// Get message number and ptr to message text
	char* end;
	unsigned long num = ::strtoul(*txt, &end, 10);
	*txt = end;

	// Look for message data
	if (::stradvtokcmp(txt, cMSGEXISTS)==0)
	{
		// Force update of window if change
		if (GetCurrentMbox())
		{
			unsigned long old = GetCurrentMbox()->GetNumberFound();
			if (num != old)
			{
				mMboxReload = GetCurrentMbox()->IsFullOpen();
				mMboxUpdate = true;
				mMboxNew += num - old;
			}
			GetCurrentMbox()->SetNumberFound(num);
		}

		response->code = cMsgEXISTS;
	}
	else if (::stradvtokcmp(txt, cMSGRECENT)==0)
	{
		// Force update of window if change
		if (GetCurrentMbox())
		{
			unsigned long old = GetCurrentMbox()->GetNumberRecent();
			if (num != old)
			{
				mMboxReset = GetCurrentMbox()->IsFullOpen();
				mMboxUpdate = true;
			}

			GetCurrentMbox()->SetNumberRecent(num);
		}

		response->code = cMsgRECENT;
	}
	else if (::stradvtokcmp(txt, cMSGEXPUNGE)==0)
	{
		// Always force window update
		if (GetCurrentMbox())
		{
			mMboxReset = GetCurrentMbox()->IsFullOpen();
			mMboxUpdate = true;

			if (mMboxReset)
				GetCurrentMbox()->RemoveMessage(num);
		}
		response->code = cMsgEXPUNGE;
	}
	else if (::stradvtokcmp(txt, cMSGSTORE)==0)
	{
		response->code = cMsgSTORE;
		// Protect against index out of bounds which can happen
		// under rare circumstances. This should be fixed properly by
		// making sure EXISTS immediately updates the message list size
		try
		{
			SetMessage(num);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			mCurrent_msg = NULL;
		}
		IMAPParseFetch(txt);
	}
	else if (::stradvtokcmp(txt, cMSGFETCH)==0)
	{
		response->code = cMsgFETCH;
		// Protect against index out of bounds which can happen
		// under rare circumstances. This should be fixed properly by
		// making sure EXISTS immediately updates the message list size
		try
		{
			SetMessage(num);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			mCurrent_msg = NULL;
		}
		IMAPParseFetch(txt);
	}
	else if (::stradvtokcmp(txt, cMSGCOPY)==0)
	{
		response->code = cMsgCOPY;
		// Protect against index out of bounds which can happen
		// under rare circumstances. This should be fixed properly by
		// making sure EXISTS immediately updates the message list size
		try
		{
			SetMessage(num);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			mCurrent_msg = NULL;
		}
	}
	else
		response->code = cResponseError;

} // CIMAPClient::IMAPParseMessageResponse

// Parse list message
void CIMAPClient::IMAPParseListLsub(char** txt, bool lsub)
{
	char* p;
	NMbox::EFlags new_flags = NMbox::eNone;

	// Look for bracket
	p = ::strmatchbra(txt);
	if (!p)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	while(p && *p)
	{

		// Look for all posibilities
		if (CheckStrAdv(&p, cMBOXFLAGMARKED))
			new_flags = (NMbox::EFlags) (new_flags | NMbox::eMarked);

		else if (CheckStrAdv(&p, cMBOXFLAGNOINFERIORS))
			new_flags = (NMbox::EFlags) (new_flags | NMbox::eNoInferiors);

		else if (CheckStrAdv(&p, cMBOXFLAGNOSELECT))
			new_flags = (NMbox::EFlags) (new_flags | NMbox::eNoSelect);

		else if (CheckStrAdv(&p,cMBOXFLAGUNMARKED))
			new_flags = (NMbox::EFlags) (new_flags | NMbox::eUnMarked);

		//else if (CheckStrAdv(&p,cMBOXHASCHILDREN))
		//	new_flags = (NMbox::EFlags) (new_flags | NMbox::eHasExpanded | eHasInferiors);

		//else if (CheckStrAdv(&p,cMBOXHASNOCHILDREN))
		//	new_flags = (NMbox::EFlags) (new_flags | NMbox::eHasExpanded);

		else
		{
			// Unknown flag - ignore
			while(*p == ' ') p++;
			p = ::strpbrk(p, SPACE);
		}
	}

	// Now get directory delim - remove special quote
	char* delim = ::strgetquotestr(txt);

	if (!delim || ((::strlen(delim) > 1) && (::strcmp(delim, cNIL) != 0)))
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}
	else if (::strcmp(delim, cNIL) == 0)
		*delim = 0;

	// Now get rest of name as mailbox
	mFindingSubs = lsub;
	IMAPParseMailbox(txt, *delim, new_flags);

} // CIMAPClient::IMAPParseList

// Parse mailbox message
void CIMAPClient::IMAPParseMailbox(char** txt, char delim, NMbox::EFlags mbox_flags)
{
	CMbox* mbox = NULL;
	bool isWDMbox = false;
	bool dispose_str = false;

	// Remove leading space
	char* mbox_name = *txt;
	while(*mbox_name == ' ') mbox_name++;

	// If quoted get in normal way
	if (*mbox_name == '\"')
		mbox_name = ::strgetquotestr(txt);

	// Look for string literal
	else if (*mbox_name == '{')
	{
		mbox_name = INETParseString(txt);

		// Flag so that it will be disposed of on exit
		dispose_str = true;
	}
	else
	{
		// Look for CR and terminate
		while(**txt && (**txt != '\r'))
			(*txt)++;
		*(*txt)++ = 0;
	}

	// Check for actual name and not doing delimiter descovery
	if (!mFindingHier && *mbox_name)
	{
		{
#if 0
			// Do not do '.' & '..' in unix
			// Do not do mbox that ends with hierarchy character
			register short len = ::strlen(mbox_name);
			register char* p = &mbox_name[len-1];
			register char* q = p;
			if ((*p == GetMboxOwner()->GetMailAccount()->GetDirDelim()) ||
				((len == 1) && (mbox_name[0] == '.'))  ||
				((len == 2) && (mbox_name[0] == '.') && (mbox_name[1] == '.'))  ||
				((len > 1) && (*p-- == '.') && (*p == '/')) ||
				((len > 2) && (*q-- == '.') && (*q-- == '.') && (*q == '/')) ||
				())
			{
				if (dispose_str)
					delete mbox_name;
				return NULL;
			}
			else
#endif
			// Do not do 'INBOX'
			if (::strcmpnocase(mbox_name, cINBOX) == 0)
			{
				if (dispose_str)
					delete mbox_name;

				CMbox* inbox = GetMboxOwner()->GetINBOX();

				// Always reset dir delim and flags
				inbox->SetDirDelim(delim);
				inbox->SetListFlags(mbox_flags);

				return;
			}
		}

		// Display status and bump count
		BumpItemCtr(mFindingSubs ? "Status::IMAP::SubscribeFind" : "Status::IMAP::MailboxFind");

		// Decode modified UTF7 here
		if (mVersion == eIMAP4rev1)
		{
			// Do mUTF7 decode
			char* decoded = cdstring::FromModifiedUTF7(mbox_name, true);
			if (decoded != NULL)
			{
				// Reset mailbox name to the decoded value if it was actually decoded
				if (dispose_str)
					delete mbox_name;
				mbox_name = decoded;
				dispose_str = true;
			}
		}

		// Always reset directory delimiter in mailbox lists
		if (mCurrentWD)
			mCurrentWD->SetDirDelim(delim);

		// Always create
		mbox = new CMbox(GetMboxOwner(), mbox_name, delim, mCurrentWD, mFindingSubs);
		mbox->SetFlags(mbox_flags);

		// Add to its list - maybe deleted if duplicate
		mbox = mbox->AddMbox();
	}
	else
	{
		// No name => doing hierarchy character descovery

		// Synchronise dir delim with account
		if (GetMboxOwner()->GetMailAccount()->GetDirDelim() != delim)
		{
			GetMboxOwner()->GetMailAccount()->SetDirDelim(delim);
			CPreferences::sPrefs->mMailAccounts.SetDirty(true);
		}
	}

	if (dispose_str)
		delete mbox_name;

} // CIMAPClient::IMAPParseMailbox

// Parse search message
void CIMAPClient::IMAPParseSearch(char** txt)
{
	// Add search results to mailbox
	unsigned long msg_num = ::strtoul(*txt, txt, 10);
	while(msg_num && *txt && **txt && (**txt != '\r'))	// NB msgnum == 0 => error
	{
		if (mCurrentResults)
			mCurrentResults->push_back(msg_num);
		msg_num = ::strtoul(*txt, txt, 10);
	}

	// Check for trailing number
	if (msg_num && mCurrentResults)
		mCurrentResults->push_back(msg_num);

} // CIMAPClient::IMAPParseSearch

// Parse status message
void CIMAPClient::IMAPParseStatus(char** txt)
{
	// Get mbox name
	char* mbox_name = INETParseString(txt);
	if (!mbox_name)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}
	{
		// Do mUTF7 decode
		char* decoded = cdstring::FromModifiedUTF7(mbox_name, true);
		if (decoded != NULL)
		{
			// Reset mailbox name to the decoded value if it was actually decoded
			delete mbox_name;
			mbox_name = decoded;
		}
	}

	// Look for mailbox in list
	CMbox* update = mTargetMbox;
	{
		CMboxList list;
		GetMboxOwner()->FindMbox(mbox_name, list);
		delete mbox_name;

		// Only bother with those that have a status info already
		for(CMboxList::iterator iter = list.begin(); iter != list.end(); iter++)
		{
			if (static_cast<CMbox*>(*iter)->HasStatus())
			{
				update = static_cast<CMbox*>(*iter);
				break;
			}
		}
	}

	// Parse out bracket items
	char* q = ::strgetbrastr(txt);

	if (q && update)
	{
		bool changed = false;

		// Loop while q is not empty
		while(*q)
		{
			if (::CheckStrAdv(&q, cSTATUS_MESSAGES))
			{
				// Get a number
				unsigned long msg_num = ::strtoul(q, &q, 10);
				if (update->GetNumberFound() != msg_num)
				{
					update->SetNumberFound(msg_num);
					changed = true;
				}
			}
			else if (::CheckStrAdv(&q, cSTATUS_RECENT))
			{
				// Get a number
				unsigned long msg_recent = ::strtoul(q, &q, 10);
				if (update->GetNumberRecent() != msg_recent)
				{
					update->SetNumberRecent(msg_recent);
					changed = true;
				}
			}
			else if (::CheckStrAdv(&q, cSTATUS_UIDNEXT))
			{
				// Get a number
				unsigned long uid_next = ::strtoul(q, &q, 10);
				if (update->GetUIDNext() != uid_next)
				{
					update->SetUIDNext(uid_next);
					changed = true;
				}
			}
			else if (::CheckStrAdv(&q, cSTATUS_UIDVALIDITY))
			{
				// Get a number
				unsigned long uid_valid = ::strtoul(q, &q, 10);
				if (update->GetUIDValidity() != uid_valid)
				{
					update->SetUIDValidity(uid_valid);
					changed = true;
				}
			}
			else if (::CheckStrAdv(&q, cSTATUS_UNSEEN))
			{
				// Get a number
				unsigned long msg_unseen = ::strtoul(q, &q, 10);
				if (update->GetNumberUnseen() != msg_unseen)
				{
					update->SetNumberUnseen(msg_unseen);
					changed = true;
				}
			}
			// Got an unknown item - just step over it
			else
			{
				// Strip space
				while(*q && (*q == ' ')) q++;

				// Step over tag
				while(*q && (*q != ' ')) q++;

				// Get a number - ignore
				long unknown = ::strtol(q, &q, 10);
			}
		}

		// Force immediate UI update if changed
		if (changed)
			CMailControl::MboxUpdate(update);
	}

} // CIMAPClient::IMAPParseStatus

// Parse IMAP reply
void CIMAPClient::IMAPParseFetch(char** txt)
{
	char*	p = *txt;
	char*	q = p;

	// Strip off leading space/bracket which must be here
	while (*q == ' ')  q++;

	// No match so exit (must have brackets)
	if (*q!='(')
		return;
	else
		q++;

	// Loop while q is not empty
	while(*q)
	{
		if (::CheckStrAdv(&q, cENVELOPE))
		{
			// Do counter bits here as IMAPParseEnvelope also gets called for message/rfc822 parts
			BumpItemCtr("Status::IMAP::MessageFetch");
			IMAPParseEnvelope(&q);
		}
		else if (::CheckStrAdv(&q, cFLAGS))
			IMAPParseFlags(&q);

		else if (::CheckStrAdv(&q, cUID))
			IMAPParseUID(&q);

		else if (::CheckStrAdv(&q, cBODYSECTION_IN))
			IMAPParseBodySection(&q);

		else if (::CheckStrAdv(&q, cBODYSTRUCTURE))		// Must come before cBODY
			IMAPParseBody(&q);

		else if (::CheckStrAdv(&q, cBODY))
			IMAPParseBody(&q);

		else if (::CheckStrAdv(&q, cINTERNALDATE))
			IMAPParseInternalDate(&q);

		else if (::CheckStrAdv(&q, cRFC822HEADER))
			IMAPParseRFC822Header(&q);

		else if (::CheckStrAdv(&q, cRFC822SIZE))
			IMAPParseRFC822Size(&q);

		else if (::CheckStrAdv(&q, cRFC822TEXT))
			IMAPParseRFC822Text(&q);

		// Do this after the others as 'RFC822' is common to some of the above
		else if (::CheckStrAdv(&q,cRFC822))
			IMAPParseRFC822(&q);

		// Got an unknown item - just step over it
		else
			while(*q && (*q != ' ')) q++;

		// Strip off white space and remaining brackets
		while((*q == ' ') || (*q == '\n') || (*q == '\r') || (*q == ')')) q++;
	}

} // CIMAPClient::IMAPParseFetch

// Parse IMAP ENVELOPE reply
void CIMAPClient::IMAPParseEnvelope(char** txt)
{
	char*		p = *txt;
	char*		parsed = NULL;
	CEnvelope*	new_envelope = NULL;
	char*		date = NULL;
	cdstring	subject;
	CAddressList* from_list = NULL;
	CAddressList* sender_list = NULL;
	CAddressList* reply_to_list = NULL;
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	char*		in_reply_to = NULL;
	char*		message_id = NULL;

	try {
		// Step over white space & left brackets
		while((*p == ' ') || (*p == '(')) p++;

		// Get date (not owned by envelope)
		date = INETParseString(&p);

		// Get subject (convert from RFC1522)
		subject.steal(INETParseString(&p));
		CRFC822::TextFrom1522(subject);

		// Get from
		from_list = new CAddressList;
		IMAPParseAddressList(&p, from_list);

		// Get sender
		sender_list = new CAddressList;
		IMAPParseAddressList(&p, sender_list);

		// Get reply_to
		reply_to_list = new CAddressList;
		IMAPParseAddressList(&p, reply_to_list);

		// Get to
		to_list = new CAddressList;
		IMAPParseAddressList(&p, to_list);

		// Get ccList
		cc_list = new CAddressList;
		IMAPParseAddressList(&p, cc_list);

		// Get bcc
		bcc_list = new CAddressList;
		IMAPParseAddressList(&p, bcc_list);

		// Get in_reply_to
		in_reply_to = INETParseString(&p);

		// Get message_id
		message_id = INETParseString(&p);

		// Step over remaining white space & right brackets
		while((*p == ' ') || (*p == ')')) p++;
		*txt = p;

		// Successful parse so replace old envelope
		if (mCurrent_msg)
		{
			// If the message is not currently cached, then cache it
			// Since the server will likely only send ENVELOPE's when requested
			// we can assume that we really want to cache it
			// This is particularly important when accessing messages only by UID as the message
			// won't be cached before the UID is known
			if (!mCurrent_msg->IsCached())
				mCurrent_msg->CacheMessage();

			// Create new envelope and give it to message
			new_envelope = new CEnvelope(date, subject, from_list, sender_list, reply_to_list, to_list, cc_list, bcc_list, in_reply_to, message_id);
			SetEnvelope(new_envelope);
		}

		// Strings not owned
		delete date;
		delete in_reply_to;
		delete message_id;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up
		if (new_envelope)
			delete new_envelope;
		else
		{
			delete from_list;
			delete sender_list;
			delete reply_to_list;
			delete to_list;
			delete cc_list;
			delete bcc_list;
			delete in_reply_to;
			delete message_id;
		}

		// Always delete date
		delete date;

		// Warning this will have left a message without an envelope - dangerous must delete from cache
		if (mCurrent_msg && mCurrent_msg->IsCached())
			GetCurrentMbox()->UncacheMessage(mCurrent_msg->GetMessageNumber());
		mCurrent_msg = NULL;

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

} // CIMAPClient::IMAPParseEnvelope

// Parse IMAP style address list
void CIMAPClient::IMAPParseAddressList(char** txt, CAddressList* addr_list)
{
	char*		p = *txt;
	CAddress*	new_addr = NULL;

	// Check for NULL - if found we have an empty address list so just return
	if (::CheckStrAdv(txt, cNIL))
		return;

	try
	{
		// NB IMAPParseAddress takes care of Address lists enclosing brackets

		// Step over space
		while (*p == ' ') p++;

		// Must start with left bracket
		if (*p != '(')
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}

		// Step over address list left bracket
		p++;

		// An address always starts with left bracket
		while (*p == '(')
		{

			new_addr = new CAddress;

			// Parse address
			IMAPParseAddress(&p, new_addr);

			// Add it to the list if not empty
			if (new_addr->IsEmpty())
				delete new_addr;
			else
				addr_list->push_back(new_addr);
				
			// Must NULL the pointer in case of a throw after exiting the loop
			new_addr = NULL;

			// Step over space
			while (*p == ' ') p++;

		}

		// Address list must end with right bracket
		while (*p == ' ') p++;

		if (*p != ')')
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}

		// Step over right bracket and return updated ptr
		*txt = ++p;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		delete new_addr;

		CLOG_LOGRETHROW;
		throw;
	}

} // CIMAPClient::IMAPParseAddressList

// Parse IMAP style address
void CIMAPClient::IMAPParseAddress(char** txt, CAddress* addr)
{
	char* p = *txt;
	char* q;

	// Step over white space
	while(*p == ' ') p++;

	// Address must start with left bracket (step over)
	if (*p++ != '(')
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Get name (decode from RFC1522)
	q = INETParseString(&p);
	addr->SetName(q);
	CRFC822::TextFrom1522(addr->GetName());
	delete q;

	// Get adl
	q = INETParseString(&p);
	addr->SetADL(q);
	delete q;

	// Get mailbox
	q = INETParseString(&p);
	addr->SetMailbox(q);
	delete q;

	// Get host
	q = INETParseString(&p);
	addr->SetHost(q);
	delete q;

	// Step over remaining white space
	while(*p == ' ') p++;

	// Check for closing right bracket (step over)
	if (*p++ != ')')
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Return updated ptr
	*txt = p;

} // CIMAPClient::IMAPParseAddress

// Parse IMAP BODY/BODYSTRUCTURE reply
void CIMAPClient::IMAPParseBody(char** txt)
{
	char*	p = *txt;

	// Step over white space
	while(*p == ' ') p++;

	// Step over bracket which must be there
	if (*p++ != '(')
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	CAttachment* attach = IMAPParseBodyItem(&p, NULL);

	// Step over remaining white space & right brackets
	while((*p == ' ') || (*p == ')')) p++;
	*txt = p;

	// Successful parse so replace old body
	if (mCurrent_msg && mCurrent_msg->IsCached())
		SetBody(attach);
	else
	{
		delete attach;
		attach = NULL;
	}
}

// Parse IMAP BODY/BODYSTRUCTURE item
CAttachment* CIMAPClient::IMAPParseBodyItem(char** txt, CAttachment* parent)
{
	char* p = *txt;

	// Step over white space
	while(*p == ' ') p++;

	// Look for bracket => multi - recurse
	if (*p == '(')
	{
		// Got a multipart attachment - create new and add to parent
		CAttachment* multipart = new CAttachment(parent);
		multipart->GetContent().SetContent(eContentMultipart, eContentSubMixed);

		do
		{
			// Step over left bracket
			p++;

			// Recurse
			IMAPParseBodyItem(&p, multipart);

			// Step over remaining white space & right brackets
			while((*p == ' ') || (*p == ')')) p++;

			// Check for another multipart item

		} while(*p == '(');

		// Get multipart subtype
		char* parsed = INETParseString(&p);
		multipart->GetContent().SetContentSubtype(parsed);
		delete parsed;	// Not owned by content

		// Look for extended items
		if (*p == ' ')
		{
			p++;

			switch(mVersion)
			{
			case eIMAP2bis:
			default:
				// Look for body extension
				IMAPParseBodyExtension(&p, multipart->GetContent());
				break;

			case eIMAP4:
				// Get parameter
				IMAPParseBodyParameter(&p, multipart->GetContent(), true, false);

				// Rest are optional
				if (*p == ' ')
				{
					p++;

					// Look for body extension
					IMAPParseBodyExtension(&p, multipart->GetContent());
				}
				break;

			case eIMAP4rev1:
				// Get parameter
				IMAPParseBodyParameter(&p, multipart->GetContent(), true, false);

				// Rest are optional
				if (*p == ' ')
				{
					p++;

					// Get disposition
					IMAPParseBodyDisposition(&p, multipart->GetContent());

					// Optional
					if (*p == ' ')
					{
						p++;

						// Get language - ignore for now
						IMAPParseBodyLanguage(&p, multipart->GetContent());

						// Optional
						if (*p == ' ')
						{
							p++;

							// Look for body extension
							IMAPParseBodyExtension(&p, multipart->GetContent());
						}
					}
				}
				break;
			}
		}

		// Set to next bit
		*txt = p;

		return multipart;
	}

	// Parse non-multipart item
	else
	{
		// Got a single attachment - create new and add to parent
		CAttachment* attach = IMAPParseBodyContent(txt);
		if (parent)
			parent->AddPart(attach);
		return attach;
	}
}

// Parse IMAP BODY/BODYSTRUCTURE content
CAttachment* CIMAPClient::IMAPParseBodyContent(char** txt)
{
	cdstring parsed;
	char* p = *txt;
	CMIMEContent temp_content;

	// Get type
	parsed.steal(INETParseString(&p));
	temp_content.SetContentType(parsed);

	// Get subtype
	parsed.steal(INETParseString(&p));
	temp_content.SetContentSubtype(parsed);

	// Get parameter
	IMAPParseBodyParameter(&p, temp_content, true, false);

	// Have enough info to create the attachment now
	CAttachment* attach = CAttachment::CreateAttachment(temp_content);
	CMIMEContent& content = attach->GetContent();

	// Get id
	parsed.steal(INETParseString(&p));
	content.SetContentId(parsed);

	// Get description (can be 1522 encoded)
	parsed.steal(INETParseString(&p));
	CRFC822::TextFrom1522(parsed);
	content.SetContentDescription(parsed);

	// Get encoding
	parsed.steal(INETParseString(&p));
	content.SetTransferEncoding(parsed);
	content.SetTransferMode(eMIMEMode);

	// Get size
	parsed.steal(INETParseString(&p));
	content.SetContentSize(parsed);

	// Look for extra bits
	switch(content.GetContentType())
	{
		case eContentMessage:
			// Only for RFC822's
			if (content.GetContentSubtype() == eContentSubRFC822)
			{
				// Create a new sub-message for this type
				// Sub-message will be created with its cache
				CMessage* sub_message = (mCurrent_msg && mCurrent_msg->IsCached()) ? new CMessage(mCurrent_msg) : NULL;

				// Save current message, and set to new one
				CMessage* save = mCurrent_msg;
				mCurrent_msg = sub_message;

				// Set size based on body octets
				if (sub_message)
					sub_message->SetSize(content.GetContentSize());

#define ASIP_FIX
#ifdef ASIP_FIX
				// Look for terminating bracket (missing ENVELOPE & BODY)
				while(*p == ' ') p++;
				if (*p != ')')
				{
#endif

				// Read in envelope and body into sub-message
				IMAPParseEnvelope(&p);
				IMAPParseBody(&p);
				delete INETParseString(&p);	// Forget body line size
#ifdef ASIP_FIX
				}
				else
				{
					// Set blank envelope and body for corrupt message/rfc822 part
					SetEnvelope(new CEnvelope);
					SetBody(new CAttachment);
				}
#endif

				// Restore current message
				mCurrent_msg = save;

				// Give sub-message to attachment
				if (sub_message)
					attach->SetMessage(sub_message);
			}
			break;

		case eContentText:
			// Discard line size
			delete INETParseString(&p);
			break;

		default:
			;
	}

	// Look for extended items
	if (*p == ' ')
	{
		p++;

		// Get md5 - ignore
		delete INETParseString(&p);

		switch(mVersion)
		{
		case eIMAP2bis:
		case eIMAP4:
		default:
			// Look for body extension
			IMAPParseBodyExtension(&p, content);
			break;

		case eIMAP4rev1:
			// Optional
			if (*p == ' ')
			{
				p++;

				// Get disposition
				IMAPParseBodyDisposition(&p, content);

				// Optional
				if (*p == ' ')
				{
					p++;

					// Get language - ignore for now
					IMAPParseBodyLanguage(&p, content);

					// Optional
					if (*p == ' ')
					{
						p++;

						// Look for body extension
						IMAPParseBodyExtension(&p, content);
					}
				}
			}
			break;
		}
	}

	// Set to next bit
	*txt = p;

	return attach;
}

// Parse IMAP body_fld_param
void CIMAPClient::IMAPParseBodyParameter(char** txt, CMIMEContent& content, bool is_content, bool is_disposition)
{
	char* p = *txt;

	// Step over white space
	while(*p == ' ') p++;

	// Look for bracket => parameter values
	if (*p == '(')
	{
		p++;	// Start (

		// Wait for termination
		while(*p && (*p != ')'))
		{
			char* p_orig = p;

			// Parse out two strings
			char* parsed1 = INETParseString(&p);
			char* parsed2 = INETParseString(&p);

			// These must be present
			if (parsed1 && parsed2)
			{
				if (is_content)
					content.SetContentParameter(parsed1, parsed2);
				else if (is_disposition)
					content.SetContentDispositionParameter(parsed1, parsed2);
				
				// If we got something then the pointer did move, however if there was a literal
				// the new pointer may be the same as the old due to buffer refresh after the literal,
				// so we need to force the pointer change test to show a change.
				p_orig = NULL;
			}
			
			delete parsed1;	// Not owned by content
			delete parsed2;	// Not owned by content

			// Step over space
			while(*p == ' ') p++;
			
			// If p has not moved we are in trouble and must break out of the loop with an exception
			if (p == p_orig)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
		}

		p++;	// End )
	}
	else

		// Must have NIL string
		delete INETParseString(&p);

	// Set to next bit
	*txt = p;
}

// Parse IMAP body_fld_dsp
void CIMAPClient::IMAPParseBodyDisposition(char** txt, CMIMEContent& content)
{
	char* p = *txt;

	// Punt space, look for sexpression list
	while(*p == ' ') p++;
	if (*p == '(')
	{
		p++;	// Start (

		// Get disposition string
		char* parsed = INETParseString(&p);
		content.SetContentDisposition(parsed);
		delete parsed;	// Not owned by content

		// Get parameter - will ignore for now
		IMAPParseBodyParameter(&p, content, false, true);

		p++;	// End )
	}
	else
		// Must have NIL string
		delete INETParseString(&p);

	// Set to next bit
	*txt = p;
}

// Parse IMAP body_fld_lang
void CIMAPClient::IMAPParseBodyLanguage(char** txt, CMIMEContent& content)
{
	char* p = *txt;

	// Punt space, look for sexpression list
	while(*p == ' ') p++;
	if (*p == '(')
	{
		p++;	// Start (
		while(*p && (*p != ')'))
		{
			char* p_orig = p;

			// Must have string
			delete INETParseString(&p);

			// Step over space
			while(*p == ' ') p++;
			
			// If p has not moved we are in trouble and must break out of the loop with an exception
			if (p == p_orig)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
		}
		p++;	// End )
	}
	else
		// Must have nstring
		delete INETParseString(&p);

	// Set to next bit
	*txt = p;
}

// Parse IMAP body_fld_extension
void CIMAPClient::IMAPParseBodyExtension(char** txt, CMIMEContent& content)
{
	char* p = *txt;

	// Punt space, look for end
	if (*p == '(')
	{
		p++;	// Start (
		while(*p && (*p != ')'))
		{
			char* p_orig = p;

			IMAPParseBodyExtension(&p, content);
			while(*p == ' ') p++;
			
			// If p has not moved we are in trouble and must break out of the loop with an exception
			if (p == p_orig)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
		}
		p++;	// End )
	}
	else
		// Must have nstring / number
		delete INETParseString(&p);

	// Set to next bit
	*txt = p;
}

// Parse IMAP BODY[#] reply
void CIMAPClient::IMAPParseBodySection(char** txt)
{
	char* p = *txt;

	// Bump the counter
	BumpItemCtr("Status::IMAP::MessageFetch");

	// Look for HEADER
	if (::CheckStrAdv(&p, cBODYHEADER) ||
		::CheckStrAdv(&p, cBODYHEADERFIELDS) ||
		::CheckStrAdv(&p, cBODYHEADERFIELDSNOT))
	{
		IMAPParseBodySectionHeader(&p);
	}
	else
	{
		// Look for end bracket, error if not there
		char* q = ::strchr(p, ']');
		if (!q)
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}

		// Terminate and advance
		*q++ = '\0';

		// Do not parse section spec as we assume we're getting the section we requested

		// Look for possible partial fetch
		if (*q == '<')
		{
			// Punt over partial fetch item
			while(*q && (*q != '>')) q++;
			if (*q == '>') q++;
		}

		// Advance to actual text and read it in
		*txt = q;
		INETParseStringStream(txt);
	}
}

// Parse IMAP BODY[HEADER.XXX] reply
void CIMAPClient::IMAPParseBodySectionHeader(char** txt)
{
	char* p = *txt;

	// Look for end bracket, error if not there
	char* q = ::strchr(p, ']');
	if (!q)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Terminate and advance
	*q++ = '\0';

	// Do not parse section spec as we assume we're getting the section we requested

	// Look for possible partial fetch
	if (*q == '<')
	{
		// Punt over partial fetch item
		while(*q && (*q != '>')) q++;
		if (*q == '>') q++;
	}

	// Advance to actual text and read it in
	*txt = q;
	p = INETParseString(txt);

	// Filter out <LF>s
	if (p)
	{
		::FilterEndls(p);

		// Add text to current message
		if (mCurrent_msg && mCurrent_msg->IsCached())
			mCurrent_msg->SetHeader(p);
	}
}

// Parse IMAP FLAGS reply
void CIMAPClient::IMAPParseFlags(char** txt)
{

	// Look for bracket
	char* p = ::strmatchbra(txt);
	if (!p)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	SBitFlags new_flags(NMessage::eNone);

	while(p && *p)
	{

		// Look for all posibilities
		if (CheckStrAdv(&p,cFLAGRECENT))
			new_flags.Set(NMessage::eRecent);

		else if (CheckStrAdv(&p,cFLAGANSWERED))
			new_flags.Set(NMessage::eAnswered);

		else if (CheckStrAdv(&p,cFLAGFLAGGED))
			new_flags.Set(NMessage::eFlagged);

		else if (CheckStrAdv(&p,cFLAGDELETED))
			new_flags.Set(NMessage::eDeleted);

		else if (CheckStrAdv(&p,cFLAGSEEN))
			new_flags.Set(NMessage::eSeen);

		else if (CheckStrAdv(&p,cFLAGDRAFT))
			new_flags.Set(NMessage::eDraft);

		else if (CheckStrAdv(&p,cFLAGMDNSENT))
			new_flags.Set(NMessage::eMDNSent);

		else
		{
			// Check for labels
			bool got_label = false;
			for(int i = 0; i < NMessage::eMaxLabels; i++)
			{
				if (CheckStrAdv(&p, CPreferences::sPrefs->mIMAPLabels.GetValue()[i]))
				{
					new_flags.Set(static_cast<NMessage::EFlags>(NMessage::eLabel1 << i));
					got_label = true;
					break;
				}
			}

			// Unknown flag - ignore
			if (!got_label)
			{
				while(*p == ' ') p++;
				p = ::strpbrk(p,SPACE);
			}
		}
	}

	// Change flags on cached message - only update if changed
	if (mCurrent_msg && mCurrent_msg->SetFlags(new_flags))
	{
		// Must prevent visual hierarchy throwing an exception
		try
		{
			CMailControl::MessageChanged(mCurrent_msg);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		mMboxUpdate = true;
	}

} // CIMAPClient::IMAPParseFlags

// Parse IMAP INTERNALDATE reply
void CIMAPClient::IMAPParseInternalDate(char** txt)
{
	// Ignore this
	char* p = ::strgetquotestr(txt);

	if (!p)
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Set messages size
	if (mCurrent_msg && mCurrent_msg->IsCached())
		mCurrent_msg->SetInternalDate(p);

} // CIMAPClient::IMAPParseInternalDate

// Parse IMAP UID reply
void CIMAPClient::IMAPParseUID(char** txt)
{
	// Get a number
	unsigned long uid = ::strtoul(*txt, txt, 10);

	// Set messages size
	if (mCurrent_msg)
		mCurrent_msg->SetUID(uid);

} // CIMAPClient::IMAPParseRFC822Size

// Parse IMAP RFC822 reply
void CIMAPClient::IMAPParseRFC822(char** txt)
{
	// Parse to stream
	INETParseStringStream(txt);

} // CIMAPClient::IMAPParseRFC822

// Parse IMAP RFC822.HEADER reply
void CIMAPClient::IMAPParseRFC822Header(char** txt)
{
	// Get a string
	char* p = INETParseString(txt);

	// Filter out <LF>s
	if (p)
	{
		::FilterEndls(p);

		// Add text to current message
		if (mCurrent_msg && mCurrent_msg->IsCached())
			mCurrent_msg->SetHeader(p);
	}

} // CIMAPClient::IMAPParseRFC822Header

// Parse IMAP RFC822.SIZE reply
void CIMAPClient::IMAPParseRFC822Size(char** txt)
{
	// Get a number
	long msg_size = ::strtol(*txt, txt, 10);

	// Set messages size
	if (mCurrent_msg && mCurrent_msg->IsCached())
		mCurrent_msg->SetSize(msg_size);

} // CIMAPClient::IMAPParseRFC822Size

// Parse IMAP RFC822.TEXT reply
void CIMAPClient::IMAPParseRFC822Text(char** txt)
{
	// Parse to stream
	INETParseStringStream(txt);

} // CIMAPClient::IMAPParseRFC822Text

#pragma mark ____________________________SORT/THREAD Responses

// Parse SORT list
void CIMAPClient::IMAPParseSort(char** txt)
{
	// Add search results to mailbox
	unsigned long msg_num = ::strtoul(*txt, txt, 10);
	while(msg_num && *txt && **txt && (**txt != '\r'))	// NB msgnum == 0 => error
	{
		if (mCurrentResults)
			mCurrentResults->push_back(msg_num);
		msg_num = ::strtoul(*txt, txt, 10);
	}

	// Check for trailing number
	if (msg_num && mCurrentResults)
		mCurrentResults->push_back(msg_num);
}

// Parse THREAD list
void CIMAPClient::IMAPParseThread(char** txt)
{
	char* p = *txt;
	if (!*p)
		return;
	while(isspace(*p)) p++;
	if (*p != '(')
	{
		CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
		throw CINETException(CINETException::err_BadParse);
	}

	// Parse it into results (starts at depth 0)
	IMAPParseThreadItem(&p, 0);
}

// Parse IMAP THREAD reply
void CIMAPClient::IMAPParseThreadItem(char** txt, unsigned long depth)
{
	// Punt over paren
	char* p = *txt;

	while(*p == '(')
	{
		// Punt over paren
		p++;
		while(isspace(*p)) p++;

		// Look for nest
		if (*p == '(')
		{
			// Must add a fake item here
			if (mThreadResults)
				mThreadResults->push_back(threadvector::value_type(0, depth));

			// Now parse child items
			IMAPParseThreadItem(&p, depth + 1);

			// Punt over trailing ')'
			if (*p == ')') p++;
		}
		else if (isdigit(*p))
		{
			// Get first number
			unsigned long msg_num = ::strtoul(p, &p, 10);
			if (!msg_num)
			{
				CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
				throw CINETException(CINETException::err_BadParse);
			}
			
			// Add this as the root item for sub-thread
			if (mThreadResults)
				mThreadResults->push_back(threadvector::value_type(msg_num, depth));

			unsigned long working_depth = depth;
			while(isspace(*p)) p++;
			while(*p && (*p != ')'))
			{
				// Check for nested item
				if (isdigit(*p))
				{
					working_depth++;

					msg_num = ::strtoul(p, &p, 10);
					if (!msg_num)
					{
						CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
						throw CINETException(CINETException::err_BadParse);
					}
					
					// Add this as the root item for sub-thread
					if (mThreadResults)
						mThreadResults->push_back(threadvector::value_type(msg_num, working_depth));
				}
				else if (*p == '(')
				{
					// Parse it into results
					IMAPParseThreadItem(&p, working_depth + 1);
				}
				else
				{
					CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
					throw CINETException(CINETException::err_BadParse);
				}
				while(isspace(*p)) p++;
			}
			if (*p == ')') p++;
		}
		else
		{
			CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
			throw CINETException(CINETException::err_BadParse);
		}
		while(isspace(*p)) p++;
	}
	
	*txt = p;
}

#pragma mark ____________________________ACL Responses

// Parse ACL list
void CIMAPClient::IMAPParseACL(char** txt)
{
	// Get mbox name (not used)
	delete INETParseString(txt);

	// Add each uid/rights pair to ACL list in mailbox
	char* uid = NULL;
	while(*txt && ((uid = INETParseString(txt)) != NULL))
	{
		// Get rights
		char* rights = INETParseString(txt);

		// Create ACL
		CMboxACL acl;
		acl.SetUID(uid);
		acl.SetRights(rights);

		// Give it to mailbox
		mTargetMbox->AddACL(&acl);

		delete uid;
		delete rights;
	}

} // CIMAPClient::IMAPParseQuota

// Parse ACL list rights
void CIMAPClient::IMAPParseListRights(char** txt)
{
	// Get mbox name (not used)
	delete INETParseString(txt);

	// Get uid (not used)
	char* uid = INETParseString(txt);
	delete uid;

	// Parse remainder of line
	char* allowed = *txt;
	mACL->SetAllowedRights(allowed);

	// Bump to end of line
	while(**txt) txt++;

} // CIMAPClient::IMAPParseQuota

// Parse ACL user rights
void CIMAPClient::IMAPParseMyRights(char** txt)
{
	// Get mbox name (not used)
	delete INETParseString(txt);

	// Get rights
	char* rights = INETParseString(txt);

	// Create temp ACL
	CMboxACL temp;
	temp.SetRights(rights);
	delete rights;

	// Copy parsed rights into mailbox
	mTargetMbox->SetMyRights(temp.GetRights());

} // CIMAPClient::IMAPParseQuota

#pragma mark ____________________________QUOTA Responses

// Parse quota message
void CIMAPClient::IMAPParseQuota(char** txt)
{
	// Get quotaroot name and items
	char* qroot = INETParseString(txt);
	char* quota = ::strgetbrastr(txt);

	// Create quotaroot in mbox and reparse
	CQuotaRoot root(qroot ? qroot : "<<QUOTA>>");
	delete qroot;
	root.ParseList(quota);

	// Add to list
	GetMboxOwner()->AddQuotaRoot(&root);

} // CIMAPClient::IMAPParseQuota

// Parse quotaroot message
void CIMAPClient::IMAPParseQuotaRoot(char** txt)
{
	// Get mailbox name
	char* mbox_name = INETParseString(txt);
	if (mVersion == eIMAP4rev1)
	{
		// Do mUTF7 decode
		char* decoded = cdstring::FromModifiedUTF7(mbox_name, true);
		if (decoded != NULL)
		{
			// Reset mailbox name to the decoded value if it was actually decoded
			delete mbox_name;
			mbox_name = decoded;
		}
	}

	// Must match mailbox names to be safe
	bool matched = (mTargetMbox->GetName() == mbox_name);
	delete mbox_name;

	// Add each root to target mailbox if matched
	char* root = NULL;
	while(*txt && **txt)
	{
		// NB Root may be empty string
		root = INETParseString(txt);
		if (matched)
			mTargetMbox->AddQuotaRoot(root ? root : "<<QUOTA>>");
		delete root;
	}

} // CIMAPClient::IMAPParseQuotaRoot

#pragma mark ____________________________NAMEPSPACE Responses

// Parse IMAP NAMESPACE reply
void CIMAPClient::IMAPParseNamespace(char** txt)
{
	// Ignore if no data stotre set
	if (!mNamespace)
		return;

	char* p = *txt;

	// Do each section
	for(int i = CMboxProtocol::ePersonal; i <= CMboxProtocol::ePublic; i++)
		IMAPParseNamespaceItem(&mNamespace->mItems[i], &p);
}

// Parse IMAP NAMESPACE item
void CIMAPClient::IMAPParseNamespaceItem(cdstrpairvect* names, char** txt)
{
	char* p = *txt;

	// Punt spaces
	while(*p && (*p == ' ')) p++;

	// Check for NULL - if found we have an empty address list so just return
	if (::CheckStrAdv(&p, cNIL))
	{
		*txt = p;
		return;
	}

	// Must be bracket
	p++;

	// Look for multiple items (...)(...)...
	while(*p == '(')
	{
		p++;

		// Get name and hierarchy character
		char* q = INETParseString(&p);

		// Punt spaces
		while(*p == ' ') p++;

		// Check for NULL
		char* r = NULL;
		if (!::CheckStrAdv(&p, cNIL))
			r = ::strgetquotestr(&p);

		// Add to list
		names->push_back(cdstrpairvect::value_type((q ? q : cdstring::null_str.c_str()), (r ? r : cdstring::null_str.c_str())));
		delete q;

		// Step over remaining white space
		while(*p == ' ') p++;

		// Look for extension
		if (*p != ')')
		{
			// atom SP (string *SP string)

			// Read atom to nowhere
			q = INETParseString(&p);
			delete q;

			// Punt to bracket
			while(*p == ' ') p++;

			if (*p == '(')
			{
				p++;
				while(*p && (*p != ')'))
				{
					char* p_orig = p;

					q = INETParseString(&p);
					delete q;
					while(*p == ' ') p++;
					
					// If p has not moved we are in trouble and must break out of the loop with an exception
					if (p == p_orig)
					{
						CLOG_LOGTHROW(CINETException, CINETException::err_BadParse);
						throw CINETException(CINETException::err_BadParse);
					}
				}
				p++;
			}
		}

		// Punt past item ')'
		p++;
	}

	// Punt past end ')'
	if (*p == ')')
		p++;

	*txt = p;
}
