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


// Code for SMTP sender class

#include "CSMTPSender.h"

#include "CAddress.h"
#include "CAddressList.h"
#include "CAuthPlugin.h"
#include "CCertificateManager.h"
#include "CINETAccount.h"
#include "CINETClient.h"
#include "CINETCommon.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryCommon.h"
#endif
#include "CPasswordManager.h"
#include "CPluginManager.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CStatusWindow.h"
#include "CStreamFilter.h"
#include "CStreamType.h"
#include "CStringUtils.h"
#include "CTaskClasses.h"
#include "CTCPException.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CVisualProgress.h"
#endif
#include "CXStringResources.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#else
#include "StValueChanger.h"
#endif

#include "base64.h"

#include <algorithm>
#include <errno.h>
#include <string.h>
#include <stdio.h>

const int cSMTPBufferLen = 1024;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor

CSMTPSender::CSMTPSender(CINETAccount* account)
{
	SetAccount(account);

	mReceiver = 0;
	mMessage = NULL;
	mMailState = cSMTPNotOpen;
	mLineData = new char[cSMTPBufferLen];
	mToCtr = 0;
	mCcCtr = 0;
	mBccCtr = 0;
	mAllowLog = true;
	mESMTP = false;
	mSize = false;
	mSizeLimit = -1;
	mSTARTTLS = false;
	mAUTH = false;
	mDSN = false;

	mUseQueue = false;
	mQueueMbox = NULL;
}

CSMTPSender::~CSMTPSender()
{
	mAccount = NULL;
	mMessage = NULL;
	delete mLineData;
	mLineData = NULL;
	mQueueMbox = NULL;
}


// O T H E R  M E T H O D S _________________________________________________________________________

void CSMTPSender::SetAccount(CINETAccount* account)
{
	mAccount = account;
	mAccountName = account->GetName();
}

bool CSMTPSender::IsSecure() const
{
	// Check for TLS
	return (mAccount->GetTLSType() != CINETAccount::eNoTLS);
}

bool CSMTPSender::SMTPStartAsync()
{
	// Can only do if mailbox exists
	if (!mQueueMbox)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	// Mailbox must be open
	mQueueMbox->Open();
	if (!mQueueMbox->IsFullOpen())
		return false;
	mQueueMbox->CacheAllMessages();

	// Look for suitable messages in async mailbox
	if (!SMTPAsyncMessage())
	{
		mQueueMbox->Close();
		return false;
	}

	// Always close the mailbox to remove thread lock
	mQueueMbox->Close();

	// Begin SMTP by logging in
	SMTPBegin();

	return true;
}

bool CSMTPSender::SMTPNextAsync(bool reset)
{
	bool result = false;
	bool opened = false;

	try
	{
		// Send RSET if required
		if (reset)
		{
			SMTPSendRset();
			SMTPReceiveData();
		}
		
		// Mailbox must be open
		mQueueMbox->Open();
		opened = true;
		if (!mQueueMbox->IsFullOpen())
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		mQueueMbox->CacheAllMessages();

		// Look for available message
		CMessage* found = SMTPAsyncMessage();
		if (found)
		{
			bool sent = false;
			try
			{
				found->ChangeFlags(NMessage::eSendingNow, true);

				// Must recreate send header - don't use the one in the message
				CRFC822::SendHeader(found, mMsgDSN, true);

				// Send it
				SMTPSendMessage(found);
				sent = true;

				// UID expunge it
				ulvector nums;
				nums.push_back(found->GetMessageNumber());
				mQueueMbox->ExpungeMessage(nums, false);

				// Safety check - if we get the same message back
				// something went very wrong. We MUST make sure it doesn't
				// get sent out again and again
				CMessage* next_found = SMTPAsyncMessage();
				if (next_found == found)
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Always put into paused state and mark with error
				found->ChangeFlags(NMessage::eHold, true);
				found->ChangeFlags(NMessage::eSendError, true);
				found->ChangeFlags(NMessage::eSendingNow, false);
			}

			// Check if more to come
			result = (SMTPAsyncMessage() != NULL);
		}

		// Always close the mailbox to remove thread lock
		mQueueMbox->Close();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (opened)
			mQueueMbox->Close();

		CLOG_LOGRETHROW;
		throw;
	}

	return result;
}

void CSMTPSender::SMTPStopAsync()
{
	// Just close the connection
	SMTPEnd();
}

// Get first message available for sending
CMessage* CSMTPSender::SMTPAsyncMessage()
{
	// Policy:
	// deleted messages  : permanently removed from queue - ignore
	// hold/ mdnsent messages  : temporarily paused - ignore
	// priority/flagged messages  : high priority - send before anything else
	// send now/answered messages : set once processing starts

	// Look for first undeleted/draft message (try important ones first then the rest)
	CMessage* found = mQueueMbox->GetNextFlagMessage(NULL, NMessage::ePriority, static_cast<NMessage::EFlags>(NMessage::eDeleted | NMessage::eHold));
	if (!found)
		found = mQueueMbox->GetNextFlagMessage(NULL, NMessage::eNone, static_cast<NMessage::EFlags>(NMessage::eDeleted | NMessage::eHold));
	
	return found;
}

// Send mail with specified information
void CSMTPSender::SMTPSend(CMessage* theMsg, bool async)
{
	// Must recreate send header - don't use the one in the message
	CRFC822::SendHeader(theMsg, mMsgDSN, false);

	SMTPBegin();
	SMTPSendMessage(theMsg);
	SMTPEnd();
}

// Send mail with specified information
void CSMTPSender::SMTPBegin()
{
	// Clear errors on stream from last time
	mStream.clear();

	// Clear any previous response
	*mLineData = 0;
	bool auth_ok = true;

	try
	{
		// Set Status
		SMTPSetStatus("Status::SMTP::Opening");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Start spinning
		//if (CTaskQueue::sTaskQueue.InMainThread())
		//	::BeginResSpinning(crsr_StdSpin);
#endif
		// Open SMTP sender
		SMTPOpen();

		// Update state
		mMailState = cSMTPOpen;

// -- Connect
		// Set Status
		SMTPSetStatus("Status::SMTP::Connecting");

		// Look for SSL and turn on here
		if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) ||
			(GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
		{
			mStream.TLSSetTLSOn(true, GetAccount()->GetTLSType());
			
			// Check for client cert
			if (GetAccount()->GetUseTLSClientCert())
			{
				// Try to load client certificate
				if (!SMTPTLSClientCertificate())
				{
					CLOG_LOGTHROW(unsigned long, 1UL);
					throw 1UL;
				}
			}
		}
		else
			mStream.TLSSetTLSOn(false);

		// Start connection
		mMailState = cSMTPOpeningReceiver;
		mStream.TCPStartConnection();

		// Get first info from server
		mMailState = cSMTPWaitingReceiverResponse;
		SMTPReceiveData();

// -- EHLO
		// Set status
		SMTPSetStatus("Status::SMTP::Begin");

		// Send data
		mMailState = cSMTPSendingEHello;
		SMTPSendEHello();

		// Get response - may fail if EHLO not supported
		mMailState = cSMTPWaitingEHelloResponse;
		try
		{
			// Process the EHLO response as capability
			SMTPReceiveCapability();
			mESMTP = true;
		}
		catch (CSMTPException&)
		{
			CLOG_LOGCATCH(CSMTPException&);

			// Allow to fail silently if ESMTP not supported
		}

		if (!mESMTP)
		{
			// Look for TLS and fail
			if ((GetAccount()->GetTLSType() == CINETAccount::eTLS) ||
				(GetAccount()->GetTLSType() == CINETAccount::eTLSBroken))
			{
				mMailState = cSMTPErrorNoTLS;
				CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
				throw CSMTPException(FAIL_RESPONSE);
			}

			// Check whether user wants AUTH if so fail
			if (GetAccount()->GetAuthenticatorType() != CAuthenticator::eNone)
			{
				mMailState = cSMTPErrorNoAUTH;
				CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
				throw CSMTPException(FAIL_RESPONSE);
			}

// -- HELO
			// Set status
			SMTPSetStatus("Status::SMTP::Begin");

			// Send data
			mMailState = cSMTPSendingHello;
			SMTPSendHello();

			// Get response
			mMailState = cSMTPWaitingHelloResponse;
			SMTPReceiveData();
		}
		else
		{
			// Look for TLS and do STARTTLS
			if ((GetAccount()->GetTLSType() == CINETAccount::eTLS) ||
				(GetAccount()->GetTLSType() == CINETAccount::eTLSBroken))
			{
				if (mSTARTTLS)
				{
					// Check for client cert
					if (GetAccount()->GetUseTLSClientCert())
					{
						// Try to load client certificate
						if (!SMTPTLSClientCertificate())
						{
							CLOG_LOGTHROW(unsigned long, 1UL);
							throw 1UL;
						}
					}
					
					SMTPStartTLS();
				}
				else
				{
					mMailState = cSMTPErrorNoTLS;
					CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
					throw CSMTPException(FAIL_RESPONSE);
				}
			}

			// Must check that AUTH is available
			if (!mAUTH && (GetAccount()->GetAuthenticatorType() != CAuthenticator::eNone))
			{
				mMailState = cSMTPErrorNoAUTH;
				CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
				throw CSMTPException(FAIL_RESPONSE);
			}

			// Must check that requested AUTH method is available
			if (mAUTH && (GetAccount()->GetAuthenticatorType() != CAuthenticator::eNone))
			{
				cdstring desc = GetAccount()->GetAuthenticator().GetSASLID();
				::strupper(desc.c_str_mod());
				cdstrvect::const_iterator found = std::find(mAUTHTypes.begin(), mAUTHTypes.end(), desc);

				// Special case - try LOGIN if PLAIN was specified and failed
				if ((found == mAUTHTypes.end()) && (desc == cPLAIN))
					found = std::find(mAUTHTypes.begin(), mAUTHTypes.end(), cLOGIN);

				if (found == mAUTHTypes.end())
				{
					mMailState = cSMTPErrorNoAUTHType;
					CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
					throw CSMTPException(FAIL_RESPONSE);
				}
			}

// -- AUTH
			// Set status
			SMTPSetStatus("Status::SMTP::Authenticate");

			// Do authentication
			mMailState = cSMTPSendingAuth;
			auth_ok = SMTPDoAuthentication();

			// Reset status
			SMTPSetStatus("Status::SMTP::Begin");
		}

	}
	catch (unsigned long)
	{
		CLOG_LOGCATCH(unsigned long);

		// Fall through and treat like a failed auth
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Handle it
		SMTPHandleGeneralException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CSMTPException& ex)
	{
		CLOG_LOGCATCH(CSMTPException&);

		// Handle it
		SMTPHandleSMTPException(ex, true);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CTCPException& ex)
	{
		CLOG_LOGCATCH(CTCPException&);

		// Handle it
		SMTPHandleTCPException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}

#if __dest_os == __win32_os
	// Might catch exception when opening file attachment
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Handle it
		SMTPHandleFileException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
#endif

	// Any other exception must be caught and handled
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handle it
		SMTPHandleUnknownException();

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}

	// Now, if authentication failed (maybe cancelled) then we must throw up
	// to prevent further processing
	if (!auth_ok)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Make sure cursor has stopped spinning
		//if (CTaskQueue::sTaskQueue.InMainThread())
		//	::StopSpinning();
#endif
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}
}

// Send mail with specified information
void CSMTPSender::SMTPSendMessage(CMessage* theMsg)
{
	// Cache message
	mMessage = theMsg;
	mToCtr = 0;
	mCcCtr = 0;
	mBccCtr = 0;

	// Clear any previous response
	try
	{
		// Check size limit first
		if (mSize && mMessage->GetSize() && (mMessage->GetSize() >= mSizeLimit))
		{
			// Create fake error message
			cdstring error = rsrc::GetString("Error::SMTP::OversizeMessage");
			error += cdstring(mMessage->GetSize());
			
			// Copy into line buffer as error handling code will read error message from there
			::strcpy(mLineData, error.c_str());

			if (mAllowLog && mLog.DoLog())
				*mLog.GetLog() << error << os_endl << std::flush;

			// Force failure before even attempting SMTP commands as we know it will fail
			mMailState = cSMTPSendingMail;
			CLOG_LOGTHROW(CSMTPException, FAIL_RESPONSE);
			throw CSMTPException(FAIL_RESPONSE);
		}

// -- MAIL
		// Send data
		mMailState = cSMTPSendingMail;
		SMTPSendMail();

		// Get response
		mMailState = cSMTPWaitingMailResponse;
		SMTPReceiveData();

// --RCPTs
		for(mToCtr = 0; mToCtr < mMessage->GetEnvelope()->GetTo()->size(); mToCtr++)
		{
			// Send data
			mMailState = cSMTPSendingToRcpt;
			SMTPSendToRcpt();

			// Get response
			mMailState = cSMTPWaitingRcptResponse;
			SMTPReceiveData();
		}

		for(mCcCtr = 0; mCcCtr < mMessage->GetEnvelope()->GetCC()->size(); mCcCtr++)
		{
			// Send data
			mMailState = cSMTPSendingCCRcpt;
			SMTPSendCCRcpt();

			// Get response
			mMailState = cSMTPWaitingRcptResponse;
			SMTPReceiveData();
		}

		for(mBccCtr = 0; mBccCtr < mMessage->GetEnvelope()->GetBcc()->size(); mBccCtr++)
		{
			// Send data
			mMailState = cSMTPSendingBCCRcpt;
			SMTPSendBCCRcpt();

			// Get response
			mMailState = cSMTPWaitingRcptResponse;
			SMTPReceiveData();
		}

// -- DATA
		// Send data
		mMailState = cSMTPSendingDataCmd;
		SMTPSendDataCmd();

		// Get response
		mMailState = cSMTPWaitingDataCmdResponse;
		SMTPReceiveData(DATA_RESPONSE);

// -- data
		// Set Status
		SMTPSetStatus("Status::SMTP::Sending");

		// Send data
		mMailState = cSMTPSendingData;
		SMTPSendData();

		// Get response
		mMailState = cSMTPWaitingDataResponse;
		SMTPReceiveData();
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Handle it
		SMTPHandleGeneralException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CSMTPException& ex)
	{
		CLOG_LOGCATCH(CSMTPException&);

		// Handle it
		SMTPHandleSMTPException(ex, true);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CTCPException& ex)
	{
		CLOG_LOGCATCH(CTCPException&);

		// Handle it
		SMTPHandleTCPException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}

#if __dest_os == __win32_os
	// Might catch exception when opening file attachment
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Handle it
		SMTPHandleFileException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
#endif

	// Any other exception must be caught and handled
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handle it
		SMTPHandleUnknownException();

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
}

// Send mail with specified information
void CSMTPSender::SMTPEnd()
{
	// Clear errors on stream from last time
	mStream.clear();

	// May already be closed
	if (mMailState == cSMTPNotOpen)
		return;

	try
	{
// -- QUIT
		// Set Status
		SMTPSetStatus("Status::SMTP::Closing");

		// Send data
		mMailState = cSMTPSendingQuit;
		SMTPSendQuit();

		// Get response
		mMailState = cSMTPWaitingQuitResponse;
		SMTPReceiveData();

// -- close
		// Do action
		mMailState = cSMTPClosing;
		SMTPClose();
		mMailState = cSMTPNotOpen;

		// Set Status
		SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Stop spinning
		//if (CTaskQueue::sTaskQueue.InMainThread())
		//	::StopSpinning();
#endif
	}
	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException&);

		// Handle it
		SMTPHandleGeneralException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CSMTPException& ex)
	{
		CLOG_LOGCATCH(CSMTPException&);

		// Handle it
		SMTPHandleSMTPException(ex, false);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
	catch (CTCPException& ex)
	{
		CLOG_LOGCATCH(CTCPException&);

		// Handle it
		SMTPHandleTCPException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}

#if __dest_os == __win32_os
	// Might catch exception when opening file attachment
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Handle it
		SMTPHandleFileException(ex);

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
#endif

	// Any other exception must be caught and handled
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handle it
		SMTPHandleUnknownException();

		// Quit async operation with error
		CLOG_LOGRETHROW;
		throw;
	}
}

void CSMTPSender::SMTPSetStatus(const char* rsrcid)
{
	// Set status
	CStatusWindow::SetSMTPStatus(rsrcid);

	// Set busy status info
	{
		// Status string
		cdstring status = rsrc::GetString(rsrcid);
	
		// Add account descriptor
		status += os_endl2;
		status += "Account: ";
		status += GetAccountName();
		
		// Set this as the busy indicator
		mStream.SetBusyDescriptor(status);
	}
}

void CSMTPSender::SMTPHandleGeneralException(CGeneralException& ex)
{
	// Reset Status
	SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure cursor has stopped spinning
	//if (CTaskQueue::sTaskQueue.InMainThread())
	//	::StopSpinning();
#endif

	// Get Error context
	cdstring err_context;
	SMTPGetErrorContext(err_context);

	// Map state to error string id
	const char* err_id;
	const char* nobad_id;
	SMTPMapErrorStr(err_id, nobad_id);

	// Handle error with alert
	if (ex.GetErrorCode() < 0)
	{
		// Handle SysErr
		COSErrAlertRsrcTxtTask* task = new COSErrAlertRsrcTxtTask(err_id, ex.GetErrorCode(), err_context);
		task->Go();

		// Do recovery here
	}
	else
	{
		// Handle error
		cdstring errtxt = mLineData;
		errtxt += err_context;
		CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(nobad_id, errtxt);
		task->Go();
	}

	// Close SMTP
	SMTPClose();
	mMailState = cSMTPNotOpen;
}

void CSMTPSender::SMTPHandleSMTPException(CSMTPException& ex, bool do_quit)
{
	// Reset Status
	SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure cursor has stopped spinning
	//if (CTaskQueue::sTaskQueue.InMainThread())
	//	::StopSpinning();
#endif

	// Get Error context
	cdstring err_context;
	SMTPGetErrorContext(err_context);

	// Map state to error string id
	const char* err_id;
	const char* nobad_id;
	SMTPMapErrorStr(err_id, nobad_id);

	// Handle error
	cdstring errtxt = mLineData;
	errtxt += err_context;
	CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(nobad_id, errtxt);
	task->Go();

	// Close SMTP
	if (do_quit)
		SMTPQuitClose();
	else
		SMTPClose();
	mMailState = cSMTPNotOpen;
}

void CSMTPSender::SMTPHandleTCPException(CTCPException& ex)
{
	// Reset Status
	SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure cursor has stopped spinning
	//if (CTaskQueue::sTaskQueue.InMainThread())
	//	::StopSpinning();
#endif

	// Get Error context
	cdstring err_context;
	SMTPGetErrorContext(err_context);

	// Map state to error string id
	const char* err_id;
	const char* nobad_id;
	SMTPMapErrorStr(err_id, nobad_id);

	// Handle network errors
	const char* mapped_errid = err_id;
	switch(ex.error())
	{
	case CTCPException::err_TCPAbort:
	case CTCPException::err_TCPFailed:
		mapped_errid = "Error::INET::ConnectionAborted";
		break;
	case CTCPException::err_TCPNoSSLPlugin:
		mapped_errid = "Error::INET::NoSSLPlugin";
		break;
	case CTCPException::err_TCPSSLError:
		mapped_errid = "Error::INET::NoSSLError";
		break;
	case CTCPException::err_TCPSSLCertError:
		mapped_errid = "Error::INET::NoSSLCertError";
		break;
	case CTCPException::err_TCPSSLCertNoAccept:
		mapped_errid = "Error::INET::NoSSLCertNoAccept";
		break;
	default:
		{
			COSErrAlertRsrcTxtTask* task = new COSErrAlertRsrcTxtTask(err_id, ex.error(), err_context);
			task->Go();
		}
		return;
	}

	CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(mapped_errid, err_context);
	task->Go();

	// Close SMTP
	SMTPClose();
	mMailState = cSMTPNotOpen;
}

#if __dest_os == __win32_os
void CSMTPSender::SMTPHandleFileException(CFileException* ex)
{
	// Reset Status
	SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure cursor has stopped spinning
	if (CTaskQueue::sTaskQueue.InMainThread())
		::StopSpinning();
#endif

	// Inform user
	//CErrorHandler::PutFileErrAlertRsrc("Alerts::Adbk::RevertError", *ex);
	CFileException fe(ex->m_cause, ex->m_lOsError, ex->m_strFileName);
	fe.ReportError();

	// Close SMTP
	SMTPClose();
	mMailState = cSMTPNotOpen;
}
#endif

void CSMTPSender::SMTPHandleUnknownException()
{
	// Reset Status
	SMTPSetStatus("Status::IDLE");

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Make sure cursor has stopped spinning
	//if (CTaskQueue::sTaskQueue.InMainThread())
	//	::StopSpinning();
#endif

	// Close SMTP
	SMTPClose();
	mMailState = cSMTPNotOpen;
}

// Open SMTP sender
void CSMTPSender::SMTPOpen()
{
	// Init TCP if not already
	mStream.TCPOpen();

	// Do DNS lookup
	SMTPLookup();

	// Create log entry
	mLog.StartLog(CLog::eLogSMTP, mAccount->GetServerIP());
}

// Get error context string
void CSMTPSender::SMTPGetErrorContext(cdstring& error) const
{
	// Add account descriptor
	error += os_endl;
	error += os_endl;
	error += "Account: ";
	error += GetAccountName();
}

// Lookup SMTP sender
void CSMTPSender::SMTPLookup()
{
	// Socket descriptor is server address, authenticator descriptor and TLS type
	cdstring desc = mAccount->GetServerIP();
	desc += GetAccount()->GetAuthenticator().GetDescriptor();
	desc += cdstring((long) GetAccount()->GetTLSType());

	// Set receiver TCP info if different from before
	if (mStream.GetDescriptor() != desc)
	{
		// Copy current receiver for next call
		mStream.SetDescriptor(desc);

		// Set Status
		SMTPSetStatus("Status::SMTP::Lookingup");

		try
		{
			// Get default port based on SSL setting
			tcp_port default_port = 0;
			if ((GetAccount()->GetTLSType() == CINETAccount::eSSL) || (GetAccount()->GetTLSType() == CINETAccount::eSSLv3))
				default_port = kSMTPReceiverPort_SSL;
			else
				default_port = kSMTPReceiverPort;

			// Find out whether reverse lookup is required
			bool need_cname = false;
			switch(GetAccount()->GetAuthenticatorType())
			{
			case CAuthenticator::eNone:			// Actually ANONYMOUS!
			case CAuthenticator::ePlainText:
			case CAuthenticator::eSSL:			// Actually EXTERNAL
			default:;
				break;

			// These ones do AUTHENTICATE processing via plugin
			case CAuthenticator::ePlugin:
				{
					// See if plugin wants cname
					CAuthPlugin* plugin	= GetAccount()->GetAuthenticator().GetPlugin();
					need_cname = plugin ? plugin->NeedCNAME() : false;
				}
				break;
			}

			// Specify remote ip addr (will do reverse lookup if required by auth plugin)
			mStream.TCPSpecifyRemoteName(mAccount->GetServerIP(), default_port, need_cname);
			mReceiver = 0;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Failures must force reset of address to do lookup again
			mStream.SetDescriptor(cdstring::null_str);

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Quit and close SMTP sender
void CSMTPSender::SMTPQuitClose()
{
	// Do safe QUIT
	try
	{
		// Send data
		mMailState = cSMTPSendingQuit;
		SMTPSendQuit();

		// Get response
		mMailState = cSMTPWaitingQuitResponse;
		SMTPReceiveData();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do not report error on closing connection
	}
	
	// Do close
	SMTPClose();
}

// Close SMTP sender
void CSMTPSender::SMTPClose()
{
	mMessage = NULL;

	try
	{
		// Release TCP
		mStream.TCPCloseConnection();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do not report error on closing connection
	}

	// Create log entry
	mLog.StopLog();
}

// R E C E I V E  D A T A ___________________________________________________________________________

// Receive data - handle continuations
void CSMTPSender::SMTPReceiveData(char code)
{
	do
	{
		mStream.qgetline(mLineData, cSMTPBufferLen);
		if (mAllowLog && mLog.DoLog())
			*mLog.GetLog() << mLineData << os_endl << std::flush;
		if (!SMTPCheckResponse(code))
		{
			CLOG_LOGTHROW(CSMTPException, *mLineData);
			throw CSMTPException(*mLineData);
		}

	} while (SMTPContinuation());

}

// Initialise capability flags to empty set
void CSMTPSender::SMTPInitCapability()
{
	mAUTH = false;
	mAUTHTypes.clear();
	mDSN = false;
	mSTARTTLS = false;
	mSize = false;
	mSizeLimit = -1;
}

// Receive capability data - handle continuations
void CSMTPSender::SMTPReceiveCapability(char code)
{
	// Clear out existng capabilities before processing again
	SMTPInitCapability();
	
	do
	{
		mStream.qgetline(mLineData, cSMTPBufferLen);
		if (mAllowLog && mLog.DoLog())
			*mLog.GetLog() << mLineData << os_endl << std::flush;
		if (!SMTPCheckResponse(code))
		{
			CLOG_LOGTHROW(CSMTPException, *mLineData);
			throw CSMTPException(*mLineData);
		}

		// Look for capability
		const char* p = mLineData + 4;

		// Punt to fist tag
		while(*p && (*p == ' ')) p++;
		if (*p)
		{
			if (::strncmp(p, "AUTH", 4) == 0)
			{
				mAUTH = true;
				const char* q = (p + 5);
				char* r = ::strtok(const_cast<char*>(q), " ");
				while(r)
				{
					::strupper(r);
					mAUTHTypes.push_back(r);
					r = ::strtok(NULL, " ");
				}
			}
			else if (::strcmp(p, "DSN") == 0)
				mDSN = true;
			else if (::strcmp(p, STARTTLS) == 0)
				mSTARTTLS = true;
			else if (::strncmp(p, ESMTP_SIZE, 4) == 0)
			{
				mSize = true;
				
				// Look for max. size specifier
				const char* q = p + 4;
				if (*q == ' ')
				{
					while(*q && (*q == ' ')) q++;
					if (*q)
					{
						// Convert to number
						mSizeLimit = ::strtoul(q, NULL, 10);
						
						// SIZE 0 implies no limit so set to max_ulong
						if ((errno == ERANGE) || (mSizeLimit == 0))
							mSizeLimit = -1;
					}
				}
			}
		}

	} while (SMTPContinuation());

}

// Check that received data response is correct
bool CSMTPSender::SMTPCheckResponse(char code)
{
	// Check for positive reply
	return ((*mLineData==code) ? true : false);
}

// Check for continuation of data
bool CSMTPSender::SMTPContinuation()
{
	// Check for positive reply
	return ((mLineData[3] == CONTINUATION) ? true : false);
}

void CSMTPSender::SMTPMapErrorStr(const char*& syserr_id, const char*& protobad_id)
{
	// Handle error or warning condition
	switch (mMailState)
	{

		case cSMTPOpen:
		case cSMTPOpeningReceiver:
		case cSMTPWaitingReceiverResponse:
			syserr_id = "Error::SMTP::OSErrOpen";
			protobad_id = "Error::SMTP::NoBadOpen";
			break;

		case cSMTPSendingEHello:
		case cSMTPWaitingEHelloResponse:
			syserr_id = "Error::SMTP::OSErrEHello";
			protobad_id = "Error::SMTP::NoBadEHello";
			break;

		case cSMTPSendingHello:
		case cSMTPWaitingHelloResponse:
			syserr_id = "Error::SMTP::OSErrHello";
			protobad_id = "Error::SMTP::NoBadHello";
			break;

		case cSMTPSendingAuth:
		case cSMTPWaitingAuthResponse:
			syserr_id = "Error::SMTP::OSErrAuth";
			protobad_id = "Error::SMTP::NoBadAuth";
			break;

		case cSMTPSendingStartTLS:
		case cSMTPWaitingStartTLSResponse:
			syserr_id = "Error::SMTP::OSErrStartTLS";
			protobad_id = "Error::SMTP::NoBadStartTLS";
			break;

		case cSMTPTLSClientCert:
			syserr_id = "Error::SMTP::OSErrTLSClientCert";
			protobad_id = "Error::SMTP::NoBadTLSClientCert";
			break;

		case cSMTPSendingMail:
		case cSMTPWaitingMailResponse:
			syserr_id = "Error::SMTP::OSErrMail";
			protobad_id = "Error::SMTP::NoBadMail";
			break;

		case cSMTPSendingToRcpt:
		case cSMTPSendingCCRcpt:
		case cSMTPWaitingRcptResponse:
			syserr_id = "Error::SMTP::OSErrRcpt";
			protobad_id = "Error::SMTP::NoBadRcpt";
			break;

		case cSMTPSendingDataCmd:
		case cSMTPWaitingDataCmdResponse:
		case cSMTPSendingData:
		case cSMTPWaitingDataResponse:
			syserr_id = "Error::SMTP::OSErrData";
			protobad_id = "Error::SMTP::NoBadData";
			break;

		case cSMTPSendingQuit:
		case cSMTPWaitingQuitResponse:
			syserr_id = "Error::SMTP::OSErrQuit";
			protobad_id = "Error::SMTP::NoBadQuit";
			break;

		case cSMTPClosing:
			syserr_id = "Error::SMTP::OSErrClose";
			protobad_id = "Error::SMTP::NoBadClose";
			break;

		case cSMTPErrorNoAUTH:
			syserr_id = "Error::SMTP::NoBadAUTH";
			protobad_id = "Error::SMTP::NoBadAUTH";
			*mLineData = 0;		// No text to append
			break;

		case cSMTPErrorNoAUTHType:
			syserr_id = "Error::SMTP::NoBadAUTHType";
			protobad_id = "Error::SMTP::NoBadAUTHType";
			*mLineData = 0;		// No text to append
			break;

		case cSMTPErrorNoTLS:
			syserr_id = "Error::SMTP::NoBadTLS";
			protobad_id = "Error::SMTP::NoBadTLS";
			*mLineData = 0;		// No text to append
			break;
		default:
			// Always NULL out if no mapping found
			syserr_id = NULL;
			protobad_id = NULL;
			break;

	}
}

// S E N D  D A T A _________________________________________________________________________________

// Send 'HELO'/'EHLO' to receiver
void CSMTPSender::SMTPSendHello(bool extend)
{
	// Must check for correct format of <domain> in RFC821
	cdstring domain(mStream.TCPGetSocketName());
	if (!CTCPSocket::TCPIsHostName(domain))
	{
		domain = "[";
		domain += mStream.TCPGetSocketName();
		domain += "]";
	}

	// Use host machines canonical name/ip name for domain
	mStream << (extend ? EHLO : HELO) << domain << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << (extend ? EHLO : HELO) << domain << os_endl << std::flush;
}

// Setup TLS certificate
bool CSMTPSender::SMTPTLSClientCertificate()
{
	bool result = false;

	// Get client certificate name from account
	cdstring certfingerprint = GetAccount()->GetTLSClientCert();
	
	// Get the subject of the cert with this fingerprint
	cdstring certname;
	if (!CCertificateManager::sCertificateManager->GetSubject(certfingerprint, certname, CCertificateManager::eByFingerprint))
		return false;
	
	// Get a passphrase for this certificate
	cdstrvect users;
	users.push_back(certname);
	const char** user_list = cdstring::ToArray(users);

	cdstring passphrase;
	passphrase.reserve(512);
	unsigned long chosen;
	while(true)
	{
		if (CSecurityPlugin::GetPassphrase(user_list, passphrase.c_str_mod(), chosen))
		{
			// Try to load private key (this will verify that the password etc is valid)
			if (mStream.TLSSetClientCert(certfingerprint, passphrase))
			{
				result = true;
				break;
			}

			// Display cert error alert
			mMailState = cSMTPTLSClientCert;
			CTCPException ex(CTCPException::err_TCPSSLClientCertLoad);
			SMTPHandleTCPException(ex);
			
			// Clear password cache
			CSecurityPlugin::ClearLastPassphrase();
		}
		else
			break;
	}

	// Clean-up	
	cdstring::FreeArray(user_list);

	return result;
}

// Do TLS
void CSMTPSender::SMTPStartTLS()
{
	// Use host machines canonical name/ip name for domain
	mMailState = cSMTPSendingStartTLS;
	mStream << STARTTLS << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << STARTTLS << os_endl << std::flush;

	// Get response
	mMailState = cSMTPWaitingStartTLSResponse;
	SMTPReceiveData();

	// Now force TLS negotiation
	mStream.TLSSetTLSOn(true, GetAccount()->GetTLSType());
	mStream.TLSStartConnection();
	
	// Now redo EHLO so that we get updated capabilities
	if (mESMTP)
	{
		mMailState = cSMTPSendingEHello;
		SMTPSendEHello();

		// Get response - may fail if EHLO not supported
		mMailState = cSMTPWaitingEHelloResponse;

		// Process the EHLO response as capability
		SMTPReceiveCapability();
	}	
}

// Run authentication loop to try and login
bool CSMTPSender::SMTPDoAuthentication()
{
	bool first = true;
	bool done = false;

	// Loop while trying to authentciate
	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();

	while(CMailControl::PromptUser(acct_auth, GetAccount(), IsSecure(), false, true, false, false, false, false, first))
	{
		first = false;

		// Do authentication, but trap protocol failures
		try
		{
			SMTPAuthenticate();
			done = true;

			// Recache user id & password after successful logon
			if (GetAccount()->GetAuthenticator().RequiresUserPswd())
			{
				CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

				// Only bother if it contains something
				if (!auth->GetPswd().empty())
				{
					CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
				}
			}
		}
		catch (CSMTPException&)
		{
			CLOG_LOGCATCH(CSMTPException&);

			// Do visual alert
			const char* err_id;
			const char* nobad_id;

			SMTPMapErrorStr(err_id, nobad_id);

			// Handle error
			CStopAlertRsrcTxtTask* task = new CStopAlertRsrcTxtTask(nobad_id, mLineData);
			task->Go();

			// Force it to recycle
			done = false;
		}

		if (done)
			break;
	}

	return done;
}

// Send 'AUTH' to receiver
void CSMTPSender::SMTPAuthenticate()
{
	//bool first = true;

	CAuthenticator* acct_auth = GetAccount()->GetAuthenticator().GetAuthenticator();

	switch(GetAccount()->GetAuthenticatorType())
	{
	case CAuthenticator::ePlainText:
		{
			CAuthenticatorUserPswd* auth = static_cast<CAuthenticatorUserPswd*>(acct_auth);

			// Look for AUTH PLAIN
			cdstrvect::const_iterator found1 = std::find(mAUTHTypes.begin(), mAUTHTypes.end(), cPLAIN);
			cdstrvect::const_iterator found2 = std::find(mAUTHTypes.begin(), mAUTHTypes.end(), cLOGIN);
			if (found1 != mAUTHTypes.end())
			{
				// Form buffer of plain text SASL response
				// \0userid\0pswd
				size_t buflen = auth->GetUID().length() + auth->GetPswd().length() + 2;
				char* buffer = new char[buflen];
				char* p = buffer;
				*p++ = 0;
				::memcpy(p, auth->GetUID().c_str(), auth->GetUID().length());
				p += auth->GetUID().length();
				*p++ = 0;
				::memcpy(p, auth->GetPswd().c_str(), auth->GetPswd().length());

				// Base64 encode it
				cdstring b64;
				b64.steal(::base64_encode(reinterpret_cast<unsigned char*>(buffer), buflen));
				delete buffer;

				// Do not allow logging of auth details
				StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

				// Use host machines canonical name/ip name for domain
				mStream << AUTHPLAIN << b64 << CRLF << std::flush;

				// Write to log file
				if (mAllowLog && mLog.DoLog())
					*mLog.GetLog() << AUTHPLAIN << b64 << os_endl << std::flush;

				// Get response
				mMailState = cSMTPWaitingAuthResponse;
				SMTPReceiveData();
			}
			else if (found2 != mAUTHTypes.end())
			{
				// Use host machines canonical name/ip name for domain
				mStream << AUTHLOGIN << CRLF << std::flush;

				// Write to log file
				if (mAllowLog && mLog.DoLog())
					*mLog.GetLog() << AUTHLOGIN << os_endl << std::flush;

				// Wait for data response
				SMTPReceiveData(DATA_RESPONSE);

				// Send base64 encoded user id
				cdstring buffer = auth->GetUID();
				cdstring b64;
				b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length()));
				mStream << b64 << CRLF << std::flush;

				// Write to log file
				if (mAllowLog && mLog.DoLog())
					*mLog.GetLog() << b64 << os_endl << std::flush;
				
				// Wait for data response
				SMTPReceiveData(DATA_RESPONSE);
				
				// Send base64 encoded password
				buffer = auth->GetPswd();
				b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length()));
				mStream << b64 << CRLF << std::flush;

				// Write to log file
				if (mAllowLog && mLog.DoLog())
					*mLog.GetLog() << b64 << os_endl << std::flush;
				
				// Wait for success response
				SMTPReceiveData();
			}
		}
		break;

	case CAuthenticator::eSSL:
		{
			//CAuthenticatorUserPswd* auth = static_cast<CAuthenticatorUserPswd*>(acct_auth);

			// Form buffer of external SASL response (authrization id is empty for now)
			cdstring buffer;

			// Base64 encode it
			cdstring b64;
			b64.steal(::base64_encode(reinterpret_cast<const unsigned char*>(buffer.c_str()), buffer.length()));

			// Do not allow logging of auth details
			StValueChanger<bool> value(mAllowLog, CLog::AllowAuthenticationLog());

			// Use host machines canonical name/ip name for domain
			mStream << AUTHEXTERNAL << b64 << CRLF << std::flush;

			// Write to log file
			if (mAllowLog && mLog.DoLog())
				*mLog.GetLog() << AUTHEXTERNAL << b64 << os_endl << std::flush;

			// Get response
			mMailState = cSMTPWaitingAuthResponse;
			SMTPReceiveData();
		}
		break;

	// These ones do AUTHENTICATE processing via plugin
	case CAuthenticator::ePlugin:
		{
			// Find CRAM-MD5 plugin
			CAuthPlugin* plugin	= GetAccount()->GetAuthenticator().GetPlugin();

			if (plugin)
			{
				cdstring capability;
				if (!plugin->DoAuthentication(&GetAccount()->GetAuthenticator(),
											GetAccount()->GetServerType(),
											GetAccount()->GetServerTypeString(),
											mStream, mLog, mLineData, cSMTPBufferLen,
											capability))
				{
					const char* p = mLineData;

					// Bump past tag & space "a " if there
					if (::strncmp(p, "a ", 2) == 0)
						p += 2;

					// Fake response
					CLOG_LOGTHROW(CSMTPException, *mLineData);
					throw CSMTPException(*mLineData);
				}
			}
			else
			{
				// Fake bad response
				::strcpy(mLineData, "Authentication plugin not found\r");
				CLOG_LOGTHROW(CSMTPException, '5');
				throw CSMTPException('5');
			}

			break;
		}
	default:;
	}
}

// Send 'RSET' from
void CSMTPSender::SMTPSendRset()
{
	mStream << RSET << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << RSET << os_endl << std::flush;
}

// Send 'MAIL' from
void CSMTPSender::SMTPSendMail()
{
	cdstring theTxt;

	if (mMessage->GetEnvelope()->GetFrom()->size())
		 theTxt = mMessage->GetEnvelope()->GetFrom()->front()->GetMailAddress();

	mStream << MAILFROM << theTxt << '>';

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << MAILFROM << theTxt << '>';

	// Look for size
	if (mSize && mMessage->GetSize())
	{
		mStream << " " << ESMTP_SIZE << "=" << cdstring(mMessage->GetSize());

		// Write to log file
		if (mLog.DoLog())
			*mLog.GetLog() << " " << ESMTP_SIZE << "=" << cdstring(mMessage->GetSize());
	}

	// Look for DSN
	if (mDSN && mMsgDSN.GetRequest())
	{
		mStream << " " << RET << (mMsgDSN.GetFull() ? RET_FULL : RET_HDRS);

		// Write to log file
		if (mLog.DoLog())
			*mLog.GetLog() << " " << RET << (mMsgDSN.GetFull() ? RET_FULL : RET_HDRS);
	}

	// Finished
	mStream << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << os_endl << std::flush;
}

// Send 'RCPT'
void CSMTPSender::SMTPSendRcpt(const cdstring& addr)
{
	mStream << RCPTTO << addr << '>';

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << RCPTTO << addr << '>';

	// Look for DSN
	if (mDSN && mMsgDSN.GetRequest())
	{
		mStream << " " << NOTIFY;
		// Write to log file
		if (mLog.DoLog())
			*mLog.GetLog() << " " << NOTIFY;
		if (mMsgDSN.GetSuccess() ||
			mMsgDSN.GetFailure() ||
			mMsgDSN.GetDelay())
		{
			bool done = false;
			if (mMsgDSN.GetSuccess())
			{
				mStream << NOTIFY_SUCCESS;
				if (mLog.DoLog())
					*mLog.GetLog() << NOTIFY_SUCCESS;
				done = true;
			}
			if (mMsgDSN.GetFailure())
			{
				if (done)
					mStream << ',';
				mStream << NOTIFY_FAILURE;
				if (mLog.DoLog())
				{
					if (done)
						*mLog.GetLog() << ",";
					*mLog.GetLog() << NOTIFY_FAILURE;
				}
				done = true;
			}
			if (mMsgDSN.GetDelay())
			{
				if (done)
					mStream << ',';
				mStream << NOTIFY_DELAY;
				if (mLog.DoLog())
				{
					if (done)
						*mLog.GetLog() << ",";
					*mLog.GetLog() <<  NOTIFY_DELAY;
				}
				done = true;
			}
		}
		else
		{
			mStream << NOTIFY_NEVER;
			if (mLog.DoLog())
				*mLog.GetLog() << " " << NOTIFY_NEVER;
		}
	}

	// Finished
	mStream << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << os_endl << std::flush;
}

// Send 'RCPT' for all To's
void CSMTPSender::SMTPSendToRcpt()
{
	cdstring theTxt = mMessage->GetEnvelope()->GetTo()->at(mToCtr)->GetMailAddress();
	SMTPSendRcpt(theTxt);
}

// Send 'RCPT' for all CC's
void CSMTPSender::SMTPSendCCRcpt()
{
	cdstring theTxt = mMessage->GetEnvelope()->GetCC()->at(mCcCtr)->GetMailAddress();
	SMTPSendRcpt(theTxt);
}

// Send 'RCPT' for all BCC's
void CSMTPSender::SMTPSendBCCRcpt()
{
	cdstring theTxt = mMessage->GetEnvelope()->GetBcc()->at(mBccCtr)->GetMailAddress();
	SMTPSendRcpt(theTxt);
}

// Send 'DATA' to receiver
void CSMTPSender::SMTPSendDataCmd()
{
	mStream << DATA << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << DATA << os_endl << std::flush;
}

// Send text to receiver (remember to add header)
void CSMTPSender::SMTPSendData()
{
	//OSErr err = noErr;
	//unsigned long part_count = 1;
	//unsigned long part_offset = 0;

	// Need to dot-stuff
	{
		CStreamFilter dot_stuff(new dotstuff_filterbuf(true), static_cast<std::ostream*>(&mStream));

		// Create stream type for output
		costream stream_out(&dot_stuff, eEndl_CRLF);

		// Send header (with LFs after CRs)
		// Use appropraite filter
		const char* hdr = mMessage->GetHeader();
		if (stream_out.IsLocalType())
		{
			mStream.write(hdr, ::strlen(hdr));
		}
		else
		{
			CStreamFilter filter(new crlf_filterbuf(stream_out.GetEndlType()), static_cast<std::ostream*>(&mStream));
			filter.write(hdr, ::strlen(hdr));
		}

		// Write to log file
		if (mLog.DoLog())
			*mLog.GetLog() << hdr;

		if (mMessage->GetBody())
		{
			CSMTPAttachProgress progress;
			progress.SetTotal(mMessage->GetBody()->CountParts());
			unsigned long level = 0;

			// Write as an unowned draft or a mailbox-based message
			if (mMessage->GetMbox() && (mMessage->GetMbox() == mQueueMbox))
			{
				// Always add CRLF since async message is not reconstructed
				mStream << CRLF;
				if (mLog.DoLog())
					*mLog.GetLog() << os_endl;

				// Copy message body direct to stream
				mQueueMbox->CopyAttachment(mMessage, mMessage->GetBody(), &stream_out);

				// Write to log file
				if (mLog.DoLog())
				{
					// Create stream type for output
					costream log_out(mLog.GetLog(), lendl);
					mQueueMbox->CopyAttachment(mMessage, mMessage->GetBody(), &log_out);
				}
			}
			else
			{
				mMessage->GetBody()->WriteToStream(stream_out, level, false, &progress);
				// Write to log file
				if (mLog.DoLog())
				{
					// Create stream type for output
					costream log_out(mLog.GetLog(), lendl);
					mMessage->GetBody()->WriteToStream(log_out, level, false, NULL);
				}
			}
		}
	}

	// Send mail terminator
	mStream << CRLF_DOT_CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << os_endl << "." << os_endl << std::flush;
}

// Send 'QUIT' to receiver
void CSMTPSender::SMTPSendQuit()
{
	mStream << QUIT << CRLF << std::flush;

	// Write to log file
	if (mLog.DoLog())
		*mLog.GetLog() << QUIT << os_endl << std::flush;
}
