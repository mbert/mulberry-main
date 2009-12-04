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


// Source for CLetterWindow class

#include "CLetterWindow.h"

#include "CAddress.h"
#include "CAddressDisplay.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCopyToMenu.h"
#include "CDataAttachment.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CEditFormattedTextDisplay.h"
#else
#include "CLetterHeaderView.h"
#include "CLetterTextEditView.h"
#endif
#include "CEnvelope.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFileTable.h"
#include "CIdentityPopup.h"
#if __dest_os != __linux_os
#include "CLetterDoc.h"
#endif
#include "CMailboxPopup.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CTextEngine.h"
#if __dest_os == __linux_os
#include "CTwister.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#include <LDisclosureTriangle.h>
#endif

#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C L E T T E R W I N D O W
// __________________________________________________________________________________________________

// Respond to list changes
void CLetterWindow::ListenTo_Message(long msg, void* param)
{
	switch (msg)
	{
	case CMboxProtocol::eBroadcast_MailLogon:
		ServerState((CMboxProtocol*) param, true);
		break;
	case CMboxProtocol::eBroadcast_MailLogoff:
		ServerState((CMboxProtocol*) param, false);
		break;
	case CMboxProtocol::eBroadcast_MailMboxLogoff:
		MailboxState((CMbox*) param);
		break;
	}
}

// Check for window
bool CLetterWindow::WindowExists(const CLetterWindow* wnd)
{
	cdmutexprotect<CLetterWindowList>::lock _lock(sLetterWindows);
	CLetterWindowList::iterator found = std::find(sLetterWindows->begin(), sLetterWindows->end(), wnd);
	return found != sLetterWindows->end();
}

// Server logs on/off
void CLetterWindow::ServerState(CMboxProtocol* proto, bool logon)
{
	// Only bother if window is actually open
	if (!IsOpen())
		return;

	if (logon)
	{
		// Nothing special for now
	}
	else
	{

		if (mMsgs)
		{
	 		// Must remove all references to messages from this server
			for(CMessageList::iterator iter = mMsgs->begin(); iter != mMsgs->end(); )
			{
				if (((*iter)->GetMbox()->GetProtocol() == proto) ||
					((*iter)->GetMbox()->GetMsgProtocol() == proto))
				{
					mMsgs->erase(iter);
					continue;
				}
				
				iter++;
			}

			if (!mMsgs->size())
			{
				delete mMsgs;
				mMsgs = NULL;
			}
		}

		// Now make sure message is not included in parts table
		GetPartsTable()->ServerReset(proto);
	}
}

// Mailbox closed
void CLetterWindow::MailboxState(const CMbox* mbox)
{
	// Only bother if window is actually open
	if (!IsOpen())
		return;

	if (mMsgs)
	{
		// Must remove all references to messages from this mailbox
		for(CMessageList::iterator iter = mMsgs->begin(); iter != mMsgs->end(); )
		{
			if ((*iter)->GetMbox() == mbox)
			{
				mMsgs->erase(iter);
				continue;
			}
			
			iter++;
		}

		if (!mMsgs->size())
		{
			delete mMsgs;
			mMsgs = NULL;
		}
	}

	// Now make sure message is not included in parts table
	GetPartsTable()->MailboxReset(mbox);
}

// Check whether mbox in use in draft
bool CLetterWindow::TestOwnership(const CMbox* mbox, bool replies) const
{
	// Only bother if window is actually open
	if (!IsOpen())
		return false;

	// Test messages being replied to
	if (mMsgs && replies)
	{
		for(CMessageList::const_iterator iter = mMsgs->begin(); iter != mMsgs->end(); iter++)
			if ((*iter)->GetMbox() == mbox)
				return true;
	}

	// Test attachments
	if (mBody)
		return mBody->TestOwnership(mbox);

	return false;
}

// Make sure references to this message are eliminated
void CLetterWindow::MessageRemoved(const CMessage* msg)
{
	// Only bother if window is actually open
	if (!IsOpen())
		return;

	// Check reply/forward message list
	if (mMsgs && mMsgs->size())
	{
		// Find matching message and erase
		CMessageList::iterator found = std::find(mMsgs->begin(), mMsgs->end(), msg);
		if (found != mMsgs->end())
			mMsgs->erase(found);
	}

	// Now make sure message is not included in parts table
	GetPartsTable()->MessageRemoved(msg);
}

#pragma mark ____________________________Initialise

// Add address lists
void CLetterWindow::AddAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc)
{
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;

	try
	{
		// Get existing lists
		GetAddressLists(to_list, cc_list, bcc_list);

		// Unique add of new lists
		to_list->MergeList(to);
		cc_list->MergeList(cc);
		bcc_list->MergeList(bcc);

		// Set them back in fields
		SetAddressLists(to_list, cc_list, bcc_list);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Clean up
	delete to_list;
	delete cc_list;
	delete bcc_list;
}

// Remove address lists
void CLetterWindow::RemoveAddressLists(const CAddressList* to,
									const CAddressList* cc,
									const CAddressList* bcc)
{
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;

	try
	{
		// Get existing lists
		GetAddressLists(to_list, cc_list, bcc_list);

		// Unique add of new lists
		to_list->SubtractList(to);
		cc_list->SubtractList(cc);
		bcc_list->SubtractList(bcc);

		// Set them back in fields
		SetAddressLists(to_list, cc_list, bcc_list);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Clean up
	delete to_list;
	delete cc_list;
	delete bcc_list;
}

// Set reply message and init letter
void CLetterWindow::SetReplyMessage(CMessage* msg,
									EReplyTo reply_to)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetReplyMessages(msgs, reply_to);
}

// Set reply message and init letter
void CLetterWindow::SetReplyMessages(CMessageList* msgs,
									EReplyTo reply_to)
{
	if (msgs->size())
	{
		CEnvelope*	theEnv = msgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Replying to all recipients, sender & Ccs
		switch (reply_to)
		{
		// Only replying to Reply-To
		case replyReplyTo:
			// Set To: text
			// Use From if Reply-to is empty
			if (!theEnv->GetReplyTo()->empty())
				SetReplyMessages(msgs, theEnv->GetReplyTo(), NULL, NULL);
			else
				SetReplyMessages(msgs, theEnv->GetFrom(), NULL, NULL);
			break;

		// Only replying to Sender
		case replySender:
			// Set To: text
			SetReplyMessages(msgs, theEnv->GetSender(), NULL, NULL);
			break;

		// Only replying to From
		case replyFrom:
			// Set To: text
			SetReplyMessages(msgs, theEnv->GetFrom(), NULL, NULL);
			break;

		// Replying to all
		case replyAll:
		{
			CAddressList to_addrs;
			CAddressList cc_addrs;

			// Add Reply-To first
			if (!theEnv->GetReplyTo()->empty())
				to_addrs.push_back(theEnv->GetReplyTo()->front());
			else
				to_addrs.push_back(theEnv->GetFrom()->front());

			// Add all recipients except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter))
					to_addrs.push_back(*iter);
			}

			// Add all CCs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter))
					cc_addrs.push_back(*iter);
			}

			SetReplyMessages(msgs, &to_addrs, &cc_addrs, NULL);

			// Do not delete addresses
			to_addrs.clear_without_delete();
			cc_addrs.clear_without_delete();
			break;
		}

		default:;
		}
	}
}

// Set reply message and init letter
void CLetterWindow::SetReplyMessage(CMessage* msg,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetReplyMessages(msgs, reply_to, reply_cc, reply_bcc);
}

// Set reply message and init letter
void CLetterWindow::SetReplyMessages(CMessageList* msgs,
									const CAddressList* reply_to,
									const CAddressList* reply_cc,
									const CAddressList* reply_bcc)
{
	mMsgs = msgs;

	if (mMsgs->size())
	{
		CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Replying to all recipients, sender & Ccs
		AddAddressLists(reply_to, reply_cc, reply_bcc);

		// Set Subject: text
		cdstring theTxt;
		const char* subject = theEnv->GetSubject();
		if (subject)
		{
			if (((subject[0]!='R') && (subject[0]!='r')) ||
				((subject[1]!='E') && (subject[1]!='e')) ||
				(subject[2]!=':'))
			{
				theTxt = "Re: ";
				theTxt += subject;
			}
			else
				theTxt = subject;
		}
		SetSubject(theTxt);
		mReplySubject = theTxt;
	}

	// Set reply flag and give quotation to text display
	mReply = true;
	GetTextDisplay()->SetQuotation(CPreferences::sPrefs->mReplyQuote.GetValue());
	
	// Check for originally encrypted content
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		const CMessageCryptoInfo* crypto = (*iter)->GetCryptoInfo();
		if (crypto && crypto->GetDidDecrypt())
			mOriginalEncrypted = true;
	}
}

// Set forward message and init letter
void CLetterWindow::SetForwardMessage(CMessage* msg, EForwardOptions forward)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetForwardMessages(msgs, forward);
}

// Set forward message and init letter
void CLetterWindow::SetForwardMessages(CMessageList* msgs, EForwardOptions forward)
{
	mMsgs = msgs;

	if (mMsgs->size())
	{
		CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Set Subject: text - same as forwarded message's
		cdstring theTxt = theEnv->GetSubject();
		CPreferences::sPrefs->ForwardSubject(theTxt);
		SetSubject(theTxt);
	}

	// No - the following is now down outside of this call to allow for
	// quoting of a selection in a message window
	// Auto add included text to letter
	//IncludeFromReply(true);

	// Set forward flag and give quotation to text display
	mForward = true;
	GetTextDisplay()->SetQuotation(CPreferences::sPrefs->mForwardQuote.GetValue());

	// Add whole message
	GetPartsTable()->ForwardMessages(mMsgs, forward);
	UpdatePartsCaption();
	
	// Check for originally encrypted content
	for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
	{
		const CMessageCryptoInfo* crypto = (*iter)->GetCryptoInfo();
		if (crypto && crypto->GetDidDecrypt())
			mOriginalEncrypted = true;
	}
}

// Set bounce message and init letter
void CLetterWindow::SetBounceMessage(CMessage* msg)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetBounceMessages(msgs);
}

// Set bounce message and init letter
void CLetterWindow::SetBounceMessages(CMessageList* msgs)
{
	mMsgs = msgs;

	if (mMsgs->size())
	{
		CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Set Subject: text - same as forwarded message's
		cdstring theTxt = theEnv->GetSubject();
		SetSubject(theTxt);
	}

	// Set bounce flag
	mBounce = true;

	// Add whole message
	GetPartsTable()->BounceMessages(mMsgs);
	UpdatePartsCaption();

	if (mMsgs->size())
		// Copy bounce message header
		mBounceHeader = ::strdup(mMsgs->front()->GetHeader());

	// Force parts to display
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mPartsTwister->SetValue(1);
#elif __dest_os == __win32_os
	OnLetterPartsTwister();
#elif __dest_os == __linux_os
	mHeader->mPartsTwister->SetState(kTrue);
#else
#error __dest_os
#endif

	// Get bounce info string
	cdstring txt;
	txt.FromResource("Alerts::Letter::BounceWarning");

	// Focus before fiddling with TEHandle
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	StFocusTE clip_hide(mText);
	mText->SetText(txt);
	mText->SetReadOnly(true);
#elif __dest_os == __win32_os
	mText->SetText(txt);
	mText->GetRichEditCtrl().SetReadOnly(true);
#elif __dest_os == __linux_os
	mText->SetText(txt);
	mText->SetReadOnly(true);
#else
#error __dest_os
#endif

	// Remove signature length as its been replaced with bounce text
	mSignatureLength = 0;
}

// Set reject message and init letter
void CLetterWindow::SetRejectMessage(CMessage* msg, bool return_msg)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetRejectMessages(msgs, return_msg);
}

// Set reject message and init letter
void CLetterWindow::SetRejectMessages(CMessageList* msgs, bool return_msg)
{
	mMsgs = msgs;

	// Set reject flag
	mReject = true;

	// Set To:
	// Read in the required DSN params
	cdstring return_path;
	cdstring received_from;
	cdstring received_by;
	cdstring received_for;
	cdstring received_date;
	
	mMsgs->front()->GetDeliveryData(return_path, received_from, received_by, received_for, received_date);
	CAddressList temp;
	temp.push_back(new CAddress(return_path));
	SetAddressLists(&temp, NULL, NULL);

	// Set Subject: text - default for DSN
	cdstring theTxt("Returned mail: User unknown");
	SetSubject(theTxt);

	// Add whole message
	GetPartsTable()->RejectMessages(mMsgs, return_msg);
	UpdatePartsCaption();

	// Remove signature length as its been replaced with reject text
	mSignatureLength = 0;
}

// Set send again message and init letter
void CLetterWindow::SetSendAgainMessage(CMessage* msg)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetSendAgainMessages(msgs);
}

// Set send again message and init letter
void CLetterWindow::SetSendAgainMessages(CMessageList* msgs)
{
	mMsgs = msgs;

	if (mMsgs->size())
	{
		CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Replying to all recipients, sender & Ccs
		AddAddressLists(theEnv->GetTo(), theEnv->GetCC(), theEnv->GetBcc());

		// Set Subject: text - same as original message's
		cdstring theTxt = theEnv->GetSubject();
		SetSubject(theTxt);
	}

	// Set send again flag
	mSendAgain = true;

	// Add whole message
	GetPartsTable()->SendAgainMessages(mMsgs);
	UpdatePartsCaption();

	// Remove signature length as its been replaced with send again text
	mSignatureLength = 0;
}

// Set digest messages and init letter
void CLetterWindow::SetDigestMessage(CMessage* msg)
{
	CMessageList* msgs = new CMessageList;
	msgs->SetOwnership(false);
	msgs->push_back(msg);
	SetDigestMessages(msgs);
}

// Set digest messages and init letter
void CLetterWindow::SetDigestMessages(CMessageList* msgs)
{
	mMsgs = msgs;

	if (mMsgs->size())
	{
		CEnvelope*	theEnv = mMsgs->front()->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Set Subject: text - same as forwarded message's
		cdstring theTxt = theEnv->GetSubject();
		CPreferences::sPrefs->ForwardSubject(theTxt);
		SetSubject(theTxt);
	}

	// Set forward flag and give quotation to text display
	mForward = true;
	GetTextDisplay()->SetQuotation(CPreferences::sPrefs->mForwardQuote.GetValue());

	// Add whole message
	GetPartsTable()->DigestMessages(mMsgs);
	UpdatePartsCaption();
}

void CLetterWindow::SetFiles(const cdstrvect& files)
{
	for(cdstrvect::const_iterator iter = files.begin(); iter != files.end(); iter++)
		GetPartsTable()->AddFile(*iter);
	UpdatePartsCaption();
}

void CLetterWindow::SetiTIP(const cdstring& subject, const cdstring& description, CDataAttachment* attach, CMessage* msg)
{
	if (msg != NULL)
	{
		mMsgs = new CMessageList;
		mMsgs->SetOwnership(false);
		mMsgs->push_back(msg);
	}

	SetSubject(subject);

	// Give attachment to parts list
	GetPartsTable()->AddAttachment(attach);
	UpdatePartsCaption();
	
	// Now insert the description text at the start of the main message text
	if (!description.empty())
	{
		CTextDisplay::StPreserveSelection _preserve(mText);
		mText->SetSelectionRange(0, 0);
#if __dest_os == __linux_os
		mText->SetText(description.c_str());
#else
		mText->InsertUTF8(description);
#endif
	}
	
	// Check for originally encrypted content
	if (mMsgs != NULL)
	{
		for(CMessageList::const_iterator iter = mMsgs->begin(); iter != mMsgs->end(); iter++)
		{
			const CMessageCryptoInfo* crypto = (*iter)->GetCryptoInfo();
			if (crypto && crypto->GetDidDecrypt())
				mOriginalEncrypted = true;
		}
	}
}

#pragma mark ____________________________Sending

// Send the mail
void CLetterWindow::OnDraftSendMail()
{
	bool sent_now = false;
	bool smtp_hold = false;

	// Look for possible spell check first - must have editable part to check and not reject
	if (CPluginManager::sPluginManager.HasSpelling() && mCurrentPart && !mReject)
	{
		CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
		if (speller && speller->SpellOnSend() && !DoSpellCheck(true))
		{
			return;
		}
	}

	// Look for reply with changed subject
	if (mReply && CPreferences::sPrefs->mWarnReplySubject.GetValue())
	{
		cdstring subject;
		GetSubject(subject);
		if (mReplySubject != subject)
		{
			bool dontshow = false;
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::LetterSendAsReply",
																			"ErrorDialog::Btn::Cancel",
																			"ErrorDialog::Btn::LetterSendAsOriginal",
																			NULL,
																			"ErrorDialog::Text::WarnReplySubject", 2, NULL, NULL, NULL, &dontshow);

			// Turn off reply if this is the start of a new thread
			if (result == CErrorDialog::eBtn3)
				mReply = false;
			if (dontshow)
				CPreferences::sPrefs->mWarnReplySubject.SetValue(false);
			if (result == CErrorDialog::eBtn2)
				return;
		}
	}

	// Look for possible attachments
	if (mCurrentPart && !mReject && CPreferences::sPrefs->mWarnMissingAttachments.GetValue() && MissingAttachments())
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::MissingAttachments", "Alerts::Letter::MissingAttachmentsDontShow", &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mWarnMissingAttachments.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return;
	}

	// Warn about unencrypted send of encrypted data
	if (CPreferences::sPrefs->mWarnUnencryptedSend.GetValue() && UnencryptedSend())
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::UnencryptedSend", "Alerts::Letter::UnencryptedSendDontShow", &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mWarnUnencryptedSend.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return;
	}

	CMessage* mail_msg = NULL;

	// Protect
	if (CMulberryApp::sApp->IsDemo())
		CErrorHandler::PutNoteAlertRsrc("Alerts::General::DemoWarn");

	// Authenticate
	if (CAdminLock::sAdminLock.mLockSMTPSend && CAdminLock::sAdminLock.GetServerID().empty())
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::NoAuthenticate");
		return;
	}

	// Check append to same as embedded message/rfc822
	CMbox* append_mbox = NULL;
	if (!mReject)
	{
		// Get mailbox to append to (allow choice of No Copy to in Choose dialog)
		bool set_as_default = false;
		if (GetAppendPopup()->GetSelectedMboxSend(append_mbox, set_as_default))
		{
			if (append_mbox && (append_mbox != (CMbox*) -1))
			{
				// Check if user wants this set as the default.
				// This will only be the case if the Choose dialog was used and the
				// checkbox there was turned on.
				// Don't make changes if the identity is custom (temporary)
				if (set_as_default && !mCustomIdentity)
				{
					// Tell current identity to use the chosen mailbox as the default from now on
					CIdentity* id = const_cast<CIdentity*>(GetIdentity());
					
					id->SetCopyTo(append_mbox->GetAccountName(), true);
					id->SetCopyToNone(false);
					id->SetCopyToChoose(false);
					
					// Mark preference as dirty due to change
					CPreferences::sPrefs->mIdentities.SetDirty();
				}
			}

			// Failed to find copy to - alert user
			else if (!append_mbox)
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Letter::MissingAppendToSend");
				return;
			}
		}
		else
			// Return silently since user cancelled Choose Now...
			return;
	}

	try
	{
		// Save temporary file while maintaining dirty state
		{
			bool dirty = mDirty;
			static_cast<CLetterDoc*>(GetDocument())->SaveTemporary();
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			SetDirty(dirty);
#elif __dest_os == __win32_os
			static_cast<CLetterDoc*>(GetDocument())->SetModifiedFlag(dirty);
#elif __dest_os == __linux_os
			DataModified();
#else
#error __dest_os
#endif
		}

		mail_msg = CreateMessage(true);

		// Check whether it is sendable
		if (!CAdminLock::sAdminLock.CanSend(mail_msg))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Identity to use
		const CIdentity* id = GetIdentity();

		// Look for separate BCC and strip header for first send (non-bcc)
		if (CPreferences::sPrefs->mSeparateBCC.GetValue() && mail_msg->GetEnvelope()->GetBcc()->size())
		{
			// Only if some To or CC addresses exist
			if (mail_msg->GetEnvelope()->GetTo()->size() || mail_msg->GetEnvelope()->GetCC()->size())
			{
				// Create header for non-bcc send
				CRFC822::ECreateHeaderFlags flags = static_cast<CRFC822::ECreateHeaderFlags>(CRFC822::eAddXMulberry | CRFC822::eNonBccSend);
				if (mReject)
					flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eRejectDSN);
				CRFC822::CreateHeader(mail_msg, flags, id, &mDSN, mBounceHeader);

				// Send it
				smtp_hold = CSMTPAccountManager::sSMTPAccountManager->SendMessage(mail_msg, *id, mBounceHeader != NULL);
			}

			// Create special bcc message
			std::auto_ptr<CMessage> mail_msg_bcc(CreateMessage(true, true));

			// Create header for bcc send
			CRFC822::ECreateHeaderFlags flags = static_cast<CRFC822::ECreateHeaderFlags>(CRFC822::eAddBcc | CRFC822::eAddXMulberry | CRFC822::eBccSend);
			if (mReject)
				flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eRejectDSN);
			CRFC822::CreateHeader(mail_msg_bcc.get(), flags, id, &mDSN, mBounceHeader);

			// Send it
			smtp_hold = CSMTPAccountManager::sSMTPAccountManager->SendMessage(mail_msg_bcc.get(), *id, mBounceHeader != NULL);
		}
		else
			// Send it
			smtp_hold = CSMTPAccountManager::sSMTPAccountManager->SendMessage(mail_msg, *id, mBounceHeader != NULL);

		// Delete temporary
		static_cast<CLetterDoc*>(GetDocument())->DeleteTemporary();

		// Flag as successful send
		sent_now = true;
		mSentOnce = true;
		ShowSentIcon();

		// Check to see whether user might have wanted to copy this message to IMAP server
		// and make sure server is logged on

		if (append_mbox && (append_mbox != (CMbox*) -1) && !append_mbox->GetProtocol()->IsLoggedOn())
		{
			// Ask user what to do: logon, close letter, cancel (leaving letter open)
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::LetterServerLogon",
																			"ErrorDialog::Btn::LetterClose",
																			"ErrorDialog::Btn::LetterLeaveOpen",
																			NULL,
																			"ErrorDialog::Text::LetterServerLogon", 3);

			if (result == CErrorDialog::eBtn1)
			{
				CMboxProtocol* proto = append_mbox->GetProtocol();

				// Begin IMAP
				if (CMulberryApp::sApp->BeginINET(proto))
				{

					// Now find the mailboxes
					proto->SetSorted(true);
					proto->LoadList();

					// Get mailbox again as list will change after logon
					GetAppendPopup()->GetSelectedMbox(append_mbox);
				}
				else
				{
					CLOG_LOGTHROW(CGeneralException, -1L);
					throw CGeneralException(-1L);
				}
			}
			else if (result == CErrorDialog::eBtn2)
			{
				// Just fall through as this is what would have happened anyway
			}
			else
			{
				// Make sure its not closed, but otherwise exit gracefully
				mSentOnce = false;
			}
		}

		try
		{
			// Mark any reply message as answered (these will have a logged on protocol)
			if (mMsgs && mMsgs->size() && mReply)
			{
				// Protect against mMsgs changing whilst doing ops
				size_t old_size = mMsgs->size();
				for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
				{
					if ((*iter)->GetMbox()->HasAllowedFlag(NMessage::eAnswered))
						(*iter)->ChangeFlags(NMessage::eAnswered, true);
				}
			}

			// Mark any rejects message as deleted (these will have a logged on protocol)
			if (mMsgs && mMsgs->size() && mReject)
			{
				// Protect against mMsgs changing whilst doing ops
				size_t old_size = mMsgs->size();
				for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
				{
					if ((*iter)->GetMbox()->HasAllowedFlag(NMessage::eAnswered))
						(*iter)->ChangeFlags(NMessage::eDeleted, true);
				}
			}
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Do not allow failure to set flags to cause the append operation to fail
		}

		// Only do if not 'None'
		if (append_mbox && (append_mbox != (CMbox*) -1) && append_mbox->GetProtocol()->IsLoggedOn())
		{

			try
			{
				// Copy replied message if required
				if (mMsgs && mMsgs->size() && DoAppendReplied())
				{
					// Protect against mMsgs changing whilst doing ops
					size_t old_size = mMsgs->size();
					for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
					{
						CMessage* msg = *iter;

						// Do copy
						msg->GetMbox()->CopyMessage(msg->GetMessageNumber(), false, append_mbox);

						// Reset any open copied to window
						CMailboxView* aView = CMailboxView::FindView(append_mbox);
						if (aView)
							aView->ResetTable();

						// If copy OK then delete from original mbox if required
						if (CPreferences::sPrefs->deleteAfterCopy.GetValue() &&
							msg->GetMbox()->HasAllowedFlag(NMessage::eDeleted))
						{

							// Delete message
							msg->ChangeFlags(NMessage::eDeleted, true);

							// Always force message window (and its sub-message windows) to close
							// even if allow deleted to be open is on
							CMessageWindow* aMsgWindow;
							while((aMsgWindow = CMessageWindow::FindWindow(msg, true)) != NULL)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
								aMsgWindow->DoClose();
#else
								FRAMEWORK_DELETE_WINDOW(aMsgWindow)
#endif
						}
					}
				}
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

				// Do not allow failure to copy replied to cause the append operation to fail
			}

			// Reset message header
			// Create header for appending (i.e. bcc, no identity, no x-mulberries)
			CRFC822::ECreateHeaderFlags flags = CRFC822::eAddBcc;
			if (mReject)
				flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eRejectDSN);
			CRFC822::CreateHeader(mail_msg, flags, id, &mDSN, mBounceHeader);

			// Do append (does not acquire message)
			unsigned long new_uid = 0;
			append_mbox->AppendMessage(mail_msg, new_uid);

			// Update corresponding window if it exists
			CMailboxView* aView = CMailboxView::FindView(append_mbox);
			if (aView)
				aView->ResetTable();
		}

		// Delete message
		delete mail_msg;

		// Close window if sent
		if (mSentOnce)
		{
			// Mark as saved
			DraftSaved();

			// Undirty it so that close happens without save dialog
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			SetDirty(false);
			AttemptClose();
#elif __dest_os == __win32_os
			GetDocument()->SetModifiedFlag(false);
			GetDocument()->OnCloseDocument();
#elif __dest_os == __linux_os
			// Undirty it so that close happens without save dialog
			DataReverted();
			Close();
#else
#error __dest_os
#endif
		}
	}
#if __dest_os == __win32_os
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Must report file exceptions here
		CFileException fe(ex->m_cause, ex->m_lOsError, ex->m_strFileName);
		fe.ReportError();

		// Remove any messages
		delete mail_msg;
		
		// Flag error after sending
		if (sent_now)
			CErrorHandler::PutStopAlertRsrc("Alerts::Letter::PostSendFailure");
	}
#endif
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove any message
		delete mail_msg;

		// Alert user to failure if send completed
		if (sent_now)
			CErrorHandler::PutStopAlertRsrc("Alerts::Letter::PostSendFailure");
	}
	
	// Alert if message held in a queue
	if (smtp_hold)
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::SendHeld");
}


// Create message from contents
CMessage* CLetterWindow::CreateMessage(bool send, bool bcc_only)
{
	CAddressList* to_list = NULL;
	CAddressList* cc_list = NULL;
	CAddressList* bcc_list = NULL;
	cdstring subjectText;
	CMessage* mail_msg = NULL;

	try
	{
		// Get address lists
		GetAddressLists(to_list, cc_list, bcc_list);

		// Check for unqualified To address
		to_list->QualifyAddresses(CPreferences::sPrefs->mMailDomain.GetValue());
		if (!ValidAddressList(to_list, "Alerts::Letter::UnqualifiedToAddr", "Alerts::Letter::IllegalToAddr") && send && !bcc_only)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Check for unqualified CC address
		cc_list->QualifyAddresses(CPreferences::sPrefs->mMailDomain.GetValue());
		if (!ValidAddressList(cc_list, "Alerts::Letter::UnqualifiedCcAddr", "Alerts::Letter::IllegalCcAddr") && send && !bcc_only)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Check for unqualified Bcc address
		bcc_list->QualifyAddresses(CPreferences::sPrefs->mMailDomain.GetValue());
		if (!ValidAddressList(bcc_list, "Alerts::Letter::UnqualifiedBccAddr", "Alerts::Letter::IllegalBccAddr") && send && !bcc_only)
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Check for empty addresses
		if (!to_list->size() && !cc_list->size() && !bcc_list->size() && send && !bcc_only)
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Letter::EmptyAddr");
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Get subject
		GetSubject(subjectText);

		// Warn of empty subject
		if (subjectText.empty() && send && !bcc_only && CPreferences::sPrefs->mNoSubjectWarn.GetValue())
		{
			bool dontshow = false;
			short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::EmptySubject", 0, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->mNoSubjectWarn.SetValue(false);
			if (answer == CErrorHandler::Cancel)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

		// Sync text on display with current part
		SyncPart();

		// Warn of empty mail message
		if ((!mBody || !mBody->GetTotalSize()) && send && !bcc_only)
		{
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Letter::EmptyText") == CErrorHandler::Cancel)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

		// If bcc only message add the bcc caption to the first text part
		std::auto_ptr<CAttachment> fake_body;
		CAttachment* msg_body = mBody;
		if (bcc_only)
		{
			// Find the first appropriate attachment in the draft
			CAttachment* first = NULL;
			if (mBody && mBody->IsMixed())
			{
				first = mBody->GetParts()->front();
			}
			else
				first = mBody;
			
			// If the first one can be edited by us, then use it for th bcc caption
			if (first && first->CanEdit())
			{
				// Insert bcc caption in first part
				cdstring data = CPreferences::sPrefs->mBCCCaption.GetValue();
				data += first->GetData();
				first->SetData(data.grab_c_str());
			}
			else
			{
				// We need to create a special part to use
				first = new CDataAttachment(::strdup(CPreferences::sPrefs->mBCCCaption.GetValue()));
				
				// Now add to body
				if (!mBody)
					fake_body.reset(first);
				else if (mBody->IsMixed())
				{
					// Create a fake body
					fake_body.reset(CAttachment::CopyAttachment(*mBody));
					
					// Add new attachment to start of fake multipart
					fake_body->AddPart(first, 0);
				}
				else
				{
					// Create a fake multipart
					fake_body.reset(new CDataAttachment);
					fake_body->GetContent().SetContent(eContentMultipart, eContentSubMixed);
					
					// Add new attachment to start of fake multipart
					fake_body->AddPart(first);
					
					// Add copy of previous body as next part
					fake_body->AddPart(CAttachment::CopyAttachment(*mBody));
				}
				
				// Make sure the fake body is used to create the message
				msg_body = fake_body.get();
			}
		}
			
		// Create the message to send
		mail_msg = new CMessage(GetIdentity(),
								&mDSN,
								to_list,
								cc_list,
								bcc_list,
								subjectText.c_str_mod(),
								msg_body,
								mReply ? mMsgs : NULL,
								mBounceHeader);
		
		// Must protect against failure to fully create message
		if (!mail_msg->IsFullyCached())
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Now set flags
		SBitFlags new_flags(NMessage::eSeen);
		mail_msg->SetFlags(new_flags);

		// Check whether security required - not for rejects
		bool sign = IsSigned() && !mReject;
		bool encrypt = IsEncrypted() && !mReject;
		if (CPluginManager::sPluginManager.HasSecurity() && (sign || encrypt))
		{
			CSecurityPlugin* plugin = CSecurityPlugin::GetDefaultPlugin();

			// Determine mode and any key
			CSecurityPlugin::ESecureMessage mode = CSecurityPlugin::eNone;
			if (sign && !encrypt)
				mode = CSecurityPlugin::eSign;
			else if (!sign && encrypt)
				mode = CSecurityPlugin::eEncrypt;
			else if (sign && encrypt)
				mode = CSecurityPlugin::eEncryptSign;
			cdstring key = GetIdentity()->GetSigningID();

			// Get signing key
			if (!plugin->ProcessMessage(mail_msg, mode, key))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// If message exists then address lists are owned by message
		if (mail_msg)
		{
			delete mail_msg;
			mail_msg = NULL;
		}
		else
		{
			// No owner of address lists so delete them if they exist
			delete to_list;
			delete cc_list;
			delete bcc_list;
		}

		// throw up
		CLOG_LOGRETHROW;
		throw;
	}

	return mail_msg;
}

// Check that all addresses are valid format
bool CLetterWindow::ValidAddressList(const CAddressList* addrs, const char* str_id_unqualified, const char* str_id_illegal) const
{
	const CAddress* addr;
	if (addrs->GetUnqualifiedAddress(addr))
	{
		CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Stop,
										"ErrorDialog::Btn::OK",
										NULL,
										NULL,
										NULL,
										str_id_unqualified, 2, addr->GetMailbox());
		return false;
	}
	else if (addrs->GetIllegalAddress(addr))
	{
		CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Stop,
										"ErrorDialog::Btn::OK",
										NULL,
										NULL,
										NULL,
										str_id_illegal, 2, addr->GetFullAddress());
		return false;
	}
	
	return true;
}

// Copy to mailbox now
void CLetterWindow::CopyNow(bool option_key)
{
	// Get mailbox to append to
	CMbox* append_mbox = NULL;
	if (GetAppendPopup()->GetSelectedMbox(append_mbox) && append_mbox && (append_mbox != (CMbox*) -1))
		CopyNow(append_mbox, option_key);
	else
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::MissingAppendTo");
}

// Copy to mailbox now
void CLetterWindow::CopyNow(CMbox* mbox, bool option_key)
{
	if (!mbox)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::MissingAppendTo");
		return;
	}

	CMessage* mail_msg = NULL;

	// Protect
	if (CMulberryApp::sApp->IsDemo())
		CErrorHandler::PutNoteAlertRsrc("Alerts::General::DemoWarn");

	try
	{
		mail_msg = CreateMessage(false);

		// Now set draft flag if required
		bool as_draft = CPreferences::sPrefs->mAppendDraft.GetValue() ^ option_key;
		if (as_draft)
		{
			SBitFlags new_flags(NMessage::eSeen | NMessage::eDraft);
			mail_msg->SetFlags(new_flags);
		}

		// Record outgoing message if logged on to server
		// Only do if not 'None'
		if (mbox && (mbox != (CMbox*) -1))
		{
			// Reset message header
			// Create header for appending (i.e. bcc, identity (if draft), no x-mulberries)
			CRFC822::ECreateHeaderFlags flags = CRFC822::eAddBcc;
			if (as_draft)
				flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eAddXIdentity);
			else
				flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eNoFlags);
			if (mReject)
				flags = static_cast<CRFC822::ECreateHeaderFlags>(flags | CRFC822::eRejectDSN);
			CRFC822::CreateHeader(mail_msg, flags, GetIdentity(), &mDSN, mBounceHeader);

			// Do append (does not acquire message)
			unsigned long new_uid = 0;
			mbox->AppendMessage(mail_msg, new_uid);

			// Update corresponding window if it exists
			CMailboxView* aView = CMailboxView::FindView(mbox);
			if (aView)
				aView->ResetTable();

			// Mark as unmodified
			SetDirty(false);

			// Mark as saved
			DraftSaved();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	// Always remove any message
	delete mail_msg;
}

// Draft was saved in some fashion
void CLetterWindow::DraftSaved()
{
	// For send again messages that are drafts - delete them if requested
	if (!mMarkSaved && mSendAgain && mMsgs &&
		CPreferences::sPrefs->mDeleteOriginalDraft.GetValue())
	{
		// Protect against mMsgs changing whilst doing ops
		size_t old_size = mMsgs->size();
		for(CMessageList::iterator iter = mMsgs->begin(); mMsgs && (mMsgs->size() == old_size) && (iter != mMsgs->end()); iter++)
		{
			// Must be a draft
			if ((*iter)->IsDraft())
			{
				// Don't delete Important if requested
				if (CPreferences::sPrefs->mTemplateDrafts.GetValue() && (*iter)->IsFlagged())
					continue;
				
				// Delete message if allowed
				if ((*iter)->GetMbox()->HasAllowedFlag(NMessage::eDeleted))
					(*iter)->ChangeFlags(NMessage::eDeleted, true);
			}
		}
		
		// Set flag
		mMarkSaved = true;
	}
}

#pragma mark ____________________________Message actions

// Spell check text
bool CLetterWindow::DoSpellCheck(bool sending)
{
	CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
	if (speller)
		return speller->SpellCheck(this, mText, sending);

	return true;
}

// Check for missing attachments
bool CLetterWindow::MissingAttachments()
{
	// Get text currently on display
	cdstring text;
	mText->GetText(text);

	// Remove quoted lines from text so test is only against unquoted part
	text.steal(const_cast<char*>(CTextEngine::StripQuotedLines(text.c_str(), CPreferences::sPrefs->mRecognizeQuotes.GetValue())));

	// Check for subject test
	cdstring subject;
	if (!mReply && !mForward && !mBounce && !mReject && !mSendAgain && CPreferences::sPrefs->mMissingAttachmentSubject.GetValue())
		GetSubject(subject);

	// Now look for words in the text
	bool has_attachment_word = false;
	for(cdstrvect::const_iterator iter = CPreferences::sPrefs->mMissingAttachmentWords.GetValue().begin();
									iter != CPreferences::sPrefs->mMissingAttachmentWords.GetValue().end(); iter++)
	{
		cdstring pattern("*");
		pattern += *iter;
		pattern += "*";
		if (::strpmatch(text.c_str(), pattern.c_str()))
		{
			has_attachment_word = true;
			break;
		}
		// Do subject test
		if (!subject.empty())
		{
			if (::strpmatch(subject.c_str(), pattern.c_str()))
			{
				has_attachment_word = true;
				break;
			}
		}
	}
	
	// See if attachment are present
	if (has_attachment_word)
		return !mBody->HasAttachments();
	else
		return false;
}

// Check for unencrypted send of originally encrypted content
bool CLetterWindow::UnencryptedSend()
{
	return mOriginalEncrypted && !IsEncrypted();
}

// Speak message
#ifdef __use_speech
void CLetterWindow::SpeakMessage()
{
	CMessage* mail_msg = NULL;

	try
	{
		mail_msg = CreateMessage(false);

		// Add spoken message to queue
		CSpeechSynthesis::SpeakMessage(mail_msg, GetPartsTable()->GetPartShow(), true);

		delete mail_msg;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		delete mail_msg;
	}
}
#endif

#pragma mark ____________________________Quoting

// Include some text indented
const char* CLetterWindow::QuoteText(const char* theText, bool forward, bool header, bool is_flowed)
{
	// Strip sig dashes if required
	if (!forward && !header && CPreferences::sPrefs->mReplyNoSignature.GetValue())
	{
		cdstring temp(theText);
		CTextEngine::RemoveSigDashes(temp.c_str_mod());

		// Headers must not be wrapped so use very large wrap length
		return CTextEngine::QuoteLines(temp, temp.length(), header ? 990 : CRFC822::GetWrapLength(),
										forward ?
										CPreferences::sPrefs->mForwardQuote.GetValue() :
										CPreferences::sPrefs->mReplyQuote.GetValue(),
										&CPreferences::sPrefs->mRecognizeQuotes.GetValue(),
										is_flowed);
	}
	else
		// Headers must not be wrapped so use very large wrap length
		return CTextEngine::QuoteLines(theText, ::strlen(theText), header ? 990 : CRFC822::GetWrapLength(),
										forward ?
										CPreferences::sPrefs->mForwardQuote.GetValue() :
										CPreferences::sPrefs->mReplyQuote.GetValue(),
										&CPreferences::sPrefs->mRecognizeQuotes.GetValue(),
										is_flowed);
}

#pragma mark ____________________________Parts

#pragma mark ____________________________Identities

// Set intial identity
void CLetterWindow::InitIdentity(const CIdentity* id)
{
	// Determine account of originating messages
	bool use_x = false;
	if (!id && mMsgs && mMsgs->size())
	{
		// Get top message
		const CMessage* msg = mMsgs->front();

		// Look for X-Mulberry-Identity and use that
		cdstring x_identity;
		if (!id && mSendAgain && msg->IsDraft())
			use_x = const_cast<CMessage*>(msg)->GetHeaderField(cHDR_XMULBERRY_IDENTITY, x_identity);

		if (!id && use_x)
		{
			use_x = false;
			for(CIdentityList::const_iterator iter = CPreferences::sPrefs->mIdentities.GetValue().begin();
					iter != CPreferences::sPrefs->mIdentities.GetValue().end(); iter++)
			{
				if ((*iter).GetIdentity() == x_identity)
				{
					id = &(*iter);
					use_x = true;
					break;
				}
			}
		}

		// Get tied identity from messages using prefs
		if (!id)
			id = CPreferences::sPrefs->GetTiedIdentity(mMsgs);
	}

	// Must added empty lines after current point if text already exists as signature insert will eat up blank lines
	// before the cursor pos so the cursor position on replies may not be correct.
	// Only bother doing this when signature auto insert is being used.
	if (mText->GetTextLength() && CPreferences::sPrefs->mAutoInsertSignature.GetValue() && !mReject && !mSendAgain)
	{
		CTextDisplay::StPreserveSelection _selecton(mText);
#if __dest_os == __win32_os
		long text_len = mText->GetTextLengthEx();
#else
		long text_len = mText->GetTextLength();
#endif
		mText->SetSelectionRange(text_len, text_len);
		
		cdstring theTxt(os_endl2);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		mText->InsertUTF8(theTxt);
		mText->SetDirty(true);
#elif __dest_os == __win32_os
		mText->InsertUTF8(theTxt);
		mText->GetRichEditCtrl().SetModify(true);
#elif __dest_os == __linux_os
		mText->InsertUTF8(theTxt);
#endif
	}

	// Now set the identity
	if (id)
	{
		CIdentityList::const_iterator found = std::find(CPreferences::sPrefs->mIdentities.GetValue().begin(),
													CPreferences::sPrefs->mIdentities.GetValue().end(), *id);

		// Don't add signature if already present
		SetIdentity(id, false, !use_x);
		SetIdentityPopup(found - CPreferences::sPrefs->mIdentities.GetValue().begin());
	}
	else
		SetIdentity(NULL);
}

// Handle change of identities
void CLetterWindow::SyncIdentities(const CIdentityList& ids)
{
	// Try to match existing identity
	const CIdentity* found = &ids.front();
	int pos = 0;
	for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
	{
		if (iter->GetIdentity() == mIdentity)
		{
			found = &(*iter);
			pos = iter - ids.begin();
			break;
		}
	}

	// Reset menu (with custom items)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mIdentities->Reset(ids, true);
#elif __dest_os == __win32_os
	mHeader->mIdentityPopup.Reset(ids, true);
#else
	mHeader->mIdentityPopup->Reset(ids, true);
#endif

	// Do manual replace
	mIdentity = found->GetIdentity();
	SetIdentityPopup(pos);
	
	// Always redo header layout in case From area needs to be shown
	LayoutHeader();
}

// Get current identity
const CIdentity* CLetterWindow::GetIdentity() const
{
	// Get current identity from its title
	const CIdentity* id = NULL;

	// Check for custom
	if (mCustomIdentity)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		id = &mIdentities->GetIdentity(CPreferences::sPrefs);
#elif __dest_os == __win32_os
		id = &mHeader->mIdentityPopup.GetIdentity(CPreferences::sPrefs);
#else
		id = &mHeader->mIdentityPopup->GetIdentity(CPreferences::sPrefs);
#endif
	else		
		id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mIdentity);
	
	// If current one does not exist always return the default identity
	if (!id)
		id = &CPreferences::sPrefs->mIdentities.GetValue().front();
	
	return id;
}

// Replace signature
void CLetterWindow::ReplaceSignature(const cdstring& old_sig, const cdstring& new_sig)
{
	cdstring oldsig = old_sig;
	cdstring newsig = new_sig;
	NormaliseSignature(oldsig);
	NormaliseSignature(newsig);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Must focus
	mText->FocusDraw();
#endif
	StStopRedraw noDraw(mText);

	// Get pos of insert cursor
	CTextDisplay::StPreserveSelection _select(mText);

	// Set selection to old sig
#if __dest_os == __win32_os
	long text_len = mText->GetTextLengthEx();
#else
	long text_len = mText->GetTextLength();
#endif
	long start_sel = std::max(text_len, mSignatureLength) - mSignatureLength;
	long end_sel = text_len;
	mText->SetSelectionRange(start_sel, end_sel);

	// Get previous selection
	cdstring prevsig;
	mText->GetSelectedText(prevsig);
	NormaliseSignature(prevsig);

#if __dest_os == __win32_os
	// RichEdit2.0 does not use \n so we must remove them
	::FilterOutLFs(prevsig.c_str_mod());
#endif
	
	// Check that previous is same as old
	// Don't replace the signature if it was changed
	if (mSignatureLength && (prevsig != oldsig))
		return;

	// If no previous signature check that text at end isn't the same as new sig
	if (!mSignatureLength)
	{
		start_sel = text_len - new_sig.length() - 10;
		if (start_sel < 0)
			start_sel = 0;
		mText->SetSelectionRange(start_sel, end_sel);
		mText->GetSelectedText(prevsig);
		
#if __dest_os == __win32_os
		// RichEdit2.0 does not use \n so we must remove them
		::FilterOutLFs(prevsig.c_str_mod());
#endif
		// Check that its not the same
		const char* p = ::strstr(prevsig.c_str(), newsig.c_str());
		if (p)
		{
			// Reset length to the current one
			mSignatureLength = ::strlen(p);
			return;
		}
		// Set selection to old sig
		start_sel = std::max(text_len, mSignatureLength) - mSignatureLength;
		mText->SetSelectionRange(start_sel, end_sel);
	}
	
	// Do standard insert
	InsertSignature(new_sig);
	mSignature = new_sig;
	mSignatureLength = new_sig.length();
}

// Normalise signature
void CLetterWindow::NormaliseSignature(cdstring& old_sig)
{
	// Remove leading os_endl
	cdstring temp = old_sig;
	char* p = temp.c_str_mod();
	while(*p && isspace(*p)) p++;
	old_sig = p;

	// Remove trailing os_endl
	p = old_sig.c_str_mod() + old_sig.length() - 1;
	while((p > old_sig.c_str_mod()) && isspace(*p))
		*p-- = 0;
}

#pragma mark ____________________________Commands

void CLetterWindow::OnDraftCopyOriginal()
{
	mCopyOriginal = !mCopyOriginal;

	// Force main toolbar to update
	RefreshToolbar();
}

void CLetterWindow::OnDraftSign()
{
	// Set sign buttons
	if (CPluginManager::sPluginManager.HasSecurity())
	{
		// Toggle
		mDoSign = !mDoSign;

		// Force main toolbar to update
		RefreshToolbar();
	}
}

void CLetterWindow::OnDraftEncrypt()
{
	// Set encrypt buttons
	if (CPluginManager::sPluginManager.HasSecurity())
	{
		// Toggle
		mDoEncrypt = !mDoEncrypt;

		// Force main toolbar to update
		RefreshToolbar();
	}
}

void CLetterWindow::OnDraftMDN()
{
	mDSN.SetMDN(!mDSN.GetMDN());

	// Force main toolbar to update
	RefreshToolbar();
}

void CLetterWindow::OnDraftDSNSuccess()
{
	mDSN.SetSuccess(!mDSN.GetSuccess());
	mDSN.SetRequest(true);

	// Force main toolbar to update
	RefreshToolbar();
}

void CLetterWindow::OnDraftDSNFailure()
{
	mDSN.SetFailure(!mDSN.GetFailure());
	mDSN.SetRequest(true);
}

void CLetterWindow::OnDraftDSNDelay()
{
	mDSN.SetDelay(!mDSN.GetDelay());
	mDSN.SetRequest(true);
}

void CLetterWindow::OnDraftDSNFull()
{
	mDSN.SetFull(!mDSN.GetFull());
	mDSN.SetRequest(true);
}
