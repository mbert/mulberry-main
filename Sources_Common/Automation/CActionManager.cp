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


// Source for CActionManager class

#include "CActionManager.h"

#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CAdbkManagerView.h"
#include "CAliasAttachment.h"
#include "CBrowseMailboxDialog.h"
#include "CCaptionParser.h"
#include "CDataAttachment.h"
#include "CDSN.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CForwardOptionsDialog.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailboxWindow.h"
#include "CMailboxView.h"
#include "CMailboxInfoWindow.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMDNPromptDialog.h"
#include "CMessage.h"
#include "CMessageAttachment.h"
#include "CMulberryApp.h"
#include "CParserEnriched.h"
#include "CParserHTML.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSecurityPlugin.h"
#include "CServerView.h"
#include "CSMTPAccountManager.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif
#include "CStringUtils.h"
#include "CTextEngine.h"
#include "C3PaneWindow.h"

#pragma mark ____________________________Mailbox UI processing

// Open a mailbox
bool CActionManager::OpenMailbox(CMbox* mbox, bool use3pane, bool is_search)
{
	// Check for 3pane open
	if (use3pane && C3PaneWindow::s3PaneWindow)
	{
		C3PaneWindow::s3PaneWindow->GetServerView()->DoPreview(mbox, true, is_search);
		FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
	}
	else
		// Open the mailbox in its own window
		CMailboxInfoWindow::OpenWindow(mbox, is_search);

	return true;
}

// Close a mailbox
bool CActionManager::CloseMailbox(CMbox* mbox)
{
	return true;
}

#pragma mark ____________________________Address Book UI processing

CAddressBook* CActionManager::sOpenAdbk = NULL;

// Open an address book
bool CActionManager::OpenAddressBook(CAddressBook* adbk, bool use3pane)
{
	// Check for 3pane open
	if (use3pane && (C3PaneWindow::s3PaneWindow || CPreferences::sPrefs->mUse3Pane.GetValue()))
	{
		// Do immediate open into 3-pane if it exists
		if (C3PaneWindow::s3PaneWindow)
		{
			C3PaneWindow::s3PaneWindow->GetContactsView()->DoPreview(adbk);
			FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
		}
		else
		{
			// Cache address book for later open
			if (!sOpenAdbk)
				sOpenAdbk = adbk;
		}
	}
	else
		// Open the address book in its own window
		CAddressBookWindow::OpenWindow(adbk);

	return true;
}

// Open a mailbox
void CActionManager::Open3PaneAddressBook()
{
	// Check for 3pane open
	if (sOpenAdbk && C3PaneWindow::s3PaneWindow)
	{
		// Do immediate open into 3-pane if it exists
		C3PaneWindow::s3PaneWindow->GetContactsView()->DoPreview(sOpenAdbk);
	}

	sOpenAdbk = NULL;
}

#pragma mark ____________________________Message UI processing

// User 'saw' the message for the first time - do appropriate actions
void CActionManager::MessageSeenChange(CMessage* msg)
{
	// Look for MDN processing
	if (CPreferences::sPrefs->mMDNOptions.GetValue().GetValue() != eMDNNeverSend)
	{
		// Determine whether MDN is required and do that action
		if (!msg->IsMDNSent() && !msg->IsDraft() && msg->MDNRequested())
			MDNMessage(msg);
	}

	// Do address capture if needed
	if (msg && CPreferences::sPrefs->mCaptureRead.GetValue())
		CAddressBookManager::sAddressBookManager->CaptureAddress(*msg);
}

// Forward a message from a message window
bool CActionManager::ForwardMessage(CMessage* msg, bool msg_wnd, const cdstring& quote, EContentSubType subtype, bool use_dialog)
{
	// Generate forward options
	EForwardOptions forward = static_cast<EForwardOptions>(0);

	// Setup the quote value
	EReplyQuote quote_option;
	if (!quote.empty() && CPreferences::sPrefs->mQuoteSelection.GetValue())
		quote_option = eQuoteSelection;
	else
		quote_option = CPreferences::sPrefs->mForwardQuoteOriginal.GetValue() ? eQuoteAll : eQuoteNone;

	if (CPreferences::sPrefs->mForwardChoice.GetValue() || use_dialog)
	{
		if (!CForwardOptionsDialog::PoseDialog(forward, quote_option, !quote.empty()))
			return false;
	}
	else
	{
		if (CPreferences::sPrefs->mForwardQuoteOriginal.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardQuote);
		if (CPreferences::sPrefs->mForwardHeaders.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardHeaders);
		if (CPreferences::sPrefs->mForwardAttachment.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardAttachment);
		if (CPreferences::sPrefs->mForwardRFC822.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardRFC822);
	}

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetForwardMessage(msg, forward);
		
		// Now look for possible quote
		if (forward & eForwardQuote)
		{
			if (msg_wnd && (quote_option == eQuoteSelection))
			{
				const char* hdr = NULL;
				if (forward & eForwardHeaders)
					hdr = msg->GetHeader();
				newWindow->IncludeMessageTxt(msg, hdr, quote, true, subtype);
			}
			else if (quote_option == eQuoteAll)
				newWindow->IncludeFromReply(true, forward & eForwardHeaders);
		}

		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

// Forward messages
bool CActionManager::ForwardMessages(CMessageList* msgs, bool use_dialog)
{
	// Generate forward options
	EForwardOptions forward = static_cast<EForwardOptions>(0);

	// Setup the quote value
	EReplyQuote quote_option = CPreferences::sPrefs->mForwardQuoteOriginal.GetValue() ? eQuoteAll : eQuoteNone;

	if (CPreferences::sPrefs->mForwardChoice.GetValue() || use_dialog)
	{
		if (!CForwardOptionsDialog::PoseDialog(forward, quote_option, false))
			return false;
	}
	else
	{
		if (CPreferences::sPrefs->mForwardQuoteOriginal.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardQuote);
		if (CPreferences::sPrefs->mForwardHeaders.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardHeaders);
		if (CPreferences::sPrefs->mForwardAttachment.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardAttachment);
		if (CPreferences::sPrefs->mForwardRFC822.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardRFC822);
	}

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetForwardMessages(msgs, forward);

		// Now look for possible quote
		if ((forward & eForwardQuote) && (quote_option == eQuoteAll))
			// Quote whole message
			newWindow->IncludeFromReply(true, forward & eForwardHeaders);

		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

// Bounce a message from a message window
bool CActionManager::BounceMessage(CMessage* msg)
{
	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetBounceMessage(msg);
		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

// Reject a message from a message window
bool CActionManager::RejectMessage(CMessage* msg)
{
	// Check the validity of the message being rejected
	if (!msg->CanReject())
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::CannotReject");
		return false;
	}

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetRejectMessage(msg, false);
		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

// Send Again a message from a message window
bool CActionManager::SendAgainMessage(CMessage* msg)
{
	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetSendAgainMessage(msg);
		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

// Create digest of messages
bool CActionManager::CreateDigest(CMessageList* msgs)
{
	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		newWindow->SetDigestMessages(msgs);
		newWindow->PostSetMessage();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

bool CActionManager::NewDraft(const CMbox* mbox)
{
	cdstring subject;
	cdstring body;
	cdstrvect files;
	const CIdentity* id = NULL;

	if (mbox)
	{
		// Check for mbox specific identity
		id = mbox->GetTiedIdentity();

		// Get account specific id if not tied to mailbox
		if (!id)
			id = &mbox->GetProtocol()->GetMailAccount()->GetAccountIdentity();
	}

	return NewDraft(NULL, NULL, NULL, subject, body, files, id);
}

bool CActionManager::NewDraft(const CMboxProtocol* acct)
{
	cdstring subject;
	cdstring body;
	cdstrvect files;
	const CIdentity* id = NULL;

	if (acct)
		// Get account specific id
		id = &acct->GetMailAccount()->GetAccountIdentity();

	return NewDraft(NULL, NULL, NULL, subject, body, files, id);
}

bool CActionManager::NewDraft(CAddressList* to, CAddressList* cc, CAddressList* bcc)
{
	cdstring subject;
	cdstring body;
	cdstrvect files;
	return NewDraft(to, cc, bcc, subject, body, files);
}

bool CActionManager::NewDraft(CAddressList* to, CAddressList* cc, CAddressList* bcc,
								const cdstring& subject, const cdstring& body, const cdstrvect& files, const CIdentity* id)
{
	CLetterWindow* newWindow = NULL;
	try
	{
		newWindow = CLetterWindow::ManualCreate();

		if (newWindow)
		{
			if ((to && to->size()) || (cc && cc->size()) || (bcc && bcc->size()))
				newWindow->AddAddressLists(to, cc, bcc);
			if (!subject.empty())
				newWindow->SetSubject(subject.c_str());
			if (!body.empty())
				newWindow->SetText(body.c_str(), true);
			if (files.size())
				newWindow->SetFiles(files);

			newWindow->PostSetMessage(id);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)
		newWindow = NULL;
	}
	
	return newWindow;
}

// Copy messages betweenn mailboxes with possible delete
bool CActionManager::CopyMessage(CMbox* from, CMbox* to, ulvector* nums, bool delete_after)
{
	// Need to preserve message selection as Copy operation may result in table reset
	CMessageList preserve;
	preserve.SetOwnership(false);
	preserve.reserve(nums->size());
	for(ulvector::const_iterator iter = nums->begin(); iter != nums->end(); iter++)
		preserve.push_back(from->GetMessage(*iter));

	// Do copy (NB this may change selection if a new message arrives)
	ulmap temp;
	from->CopyMessage(*nums, false, to, temp, false);

	// Reset any open copied to window
	CMailboxView* aView = CMailboxView::FindView(to);
	if (aView)
		aView->ResetTable();

	// If copy OK then delete selection if required and not all already deleted
	if (delete_after && from->HasAllowedFlag(NMessage::eDeleted))
	{
		// Redo message numbers for delete operation
		nums->clear();
		for(CMessageList::const_iterator iter = preserve.begin(); iter != preserve.end(); iter++)
		{
			// See if message still exists and if so where
			unsigned long index = from->GetMessageIndex(*iter);
			if (index)
				nums->push_back(index);	
		}
		
		// Set deleted flag on chosen messages
		from->SetFlagMessage(*nums, false, NMessage::eDeleted, true, false);
	}

	return true;
}

bool CActionManager::MDNMessage(CMessage* msg)
{
	// Must have a message in a mailbox that allows setting MDNSent flag
	// If we can't setthe flag we have no way of determining later whether an MDN has been sent or not
	// so we must fail the MDN
	if (!msg->GetMbox() || !msg->GetMbox()->HasAllowedFlag(NMessage::eMDNSent))
	{
		// Put up MDN sent failure
		CErrorHandler::PutStopAlertRsrcStr("Alerts::Message::CannotMDNSent", msg->GetMbox() ? msg->GetMbox()->GetName() : cdstring::null_str);
		return false;
	}

	// Read in the required DSN params
	cdstring mdn_to;
	cdstring orcpt;
	msg->GetMDNData(mdn_to, orcpt);
	
	// Look for user prompt
	bool automatic = true;
	if (CPreferences::sPrefs->mMDNOptions.GetValue().GetValue() == eMDNPromptSend)
	{
		// Do dialog
		if (!CMDNPromptDialog::PoseDialog(mdn_to))
			return false;
		
		// Flag as manually approved
		automatic = false;
	}
	
	// Do the MDN - note this is sent out automatically without a draft UI

	// Figure out the correct identity for this message
	const CIdentity* id = NULL;

	// Look for tied identities
	if (msg->GetMbox())
	{
		// Check for mbox specific identity
		if (!id)
			id = msg->GetMbox()->GetTiedIdentity();

		// Get account specific id if not tied to mailbox
		if (!id)
			id = &msg->GetMbox()->GetProtocol()->GetMailAccount()->GetAccountIdentity();
	}
	
	// Use default if nothing else
	if (!id)
		id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList);
	std::auto_ptr<CAddressList> cc_list(new CAddressList);
	std::auto_ptr<CAddressList> bcc_list(new CAddressList);

	// Set the To address based on MDN header
	to_list->push_back(new CAddress(mdn_to));

	// Set Subject: text
	cdstring subject("Disposition notification: message displayed");

	// Create body
	std::auto_ptr<CAttachment> body(msg->CreateMDNSeenBody(id, automatic));

	// Change the MDNsent flag BEFORE sending it - that way if MDNSent cannot be set for some reason
	// we ensure that multiple MDNs won't be sent
	msg->ChangeFlags(NMessage::eMDNSent, true);

	{
		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> msg2(CreateMessage(to_list.release(), cc_list.release(), bcc_list.release(), subject, body.get(), id, NULL, NULL, NULL));
		
		// Now send it
		SendMessage(*msg2.get(), id, NULL, NMessage::eDraftMDN, NULL);
	}

	return true;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
bool CActionManager::PrintMessage(CMessage* msg, LPrintSpec* printSpec)
{
	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(msg);
	if (theWindow)
	{
		// Found existing window so print
		theWindow->SetPrintSpec(printSpec);
		try
		{
			theWindow->DoPrint();
			theWindow->SetPrintSpec(NULL);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			theWindow->SetPrintSpec(NULL);
			CLOG_LOGRETHROW;
			throw;
		}
		return true;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(msg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Create the message window
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessage(msg);
		newWindow->SetPrintSpec(printSpec);
		newWindow->DoPrint();
		newWindow->SetPrintSpec(NULL);
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (CMessageWindow::FindWindow(msg))
		{
			newWindow->SetPrintSpec(NULL);
			FRAMEWORK_DELETE_WINDOW(newWindow)
		}

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}
#elif __dest_os == __win32_os
bool CActionManager::PrintMessage(CMessage* msg)
{
	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(msg);

	if (theWindow)
	{
		// Found existing window so print and return
		theWindow->SendMessage(WM_COMMAND, ID_FILE_PRINT);
		return true;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(msg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Read the message
		//theMsg->ReadPart();

		// Create the message window and send save command to it
		newWindow = CMessageWindow::ManualCreate();
		newWindow->SetMessage(msg);
		newWindow->SendMessage(WM_COMMAND, ID_FILE_PRINT);
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Window failed to be created
		if (CMessageWindow::FindWindow(msg))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}
#endif

bool CActionManager::SaveMessage(CMessage* msg)
{
	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(msg);
	if (theWindow)
	{
		// Found existing window so send save command to it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		theWindow->ObeyCommand(cmd_Save, NULL);
#elif __dest_os == __win32_os
		theWindow->SendMessage(WM_COMMAND, ID_FILE_SAVE_AS);
#elif __dest_os == __linux_os
		theWindow->DoSaveAs();
#else
#error __dest_os
#endif
		return true;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(msg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Create the message window and send save command to it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessage(msg);
		newWindow->ObeyCommand(cmd_Save, NULL);
#elif __dest_os == __win32_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessage(msg);
		newWindow->GetDocument()->DoFileSave();
#elif __dest_os == __linux_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessage(msg);
		newWindow->DoSaveAs();
#else
#error __dest_os
#endif
		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (CMessageWindow::FindWindow(msg))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}

bool CActionManager::SaveMessages(CMessageList*& msgs)
{
	CMessageWindow* newWindow = NULL;
	try
	{
		// Check message size first
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			if (!CMailControl::CheckSizeWarning(*iter))
			{
				delete msgs;
				msgs = NULL;
				return false;
			}
		}

		// Create the message window and send save command to it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessageList(msgs);
		msgs = NULL;
		newWindow->ObeyCommand(cmd_Save, NULL);
#elif __dest_os == __win32_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessageList(msgs);
		msgs = NULL;
		newWindow->GetDocument()->DoFileSave();
#elif __dest_os == __linux_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessageList(msgs);
		msgs = NULL;
		newWindow->DoSaveAs();
#else
#error __dest_os
#endif

		FRAMEWORK_DELETE_WINDOW(newWindow)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete it if it still exists
		if (CMessageWindow::WindowExists(newWindow))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		delete msgs;
		msgs = NULL;

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}

#pragma mark ____________________________iMIP processing

bool CActionManager::NewDraftiTIP(const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id, CMessage* msg)
{
	return NewDraftiTIP(NULL, NULL, NULL, subject, description, attach, id, msg);
}

bool CActionManager::NewDraftiTIP(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id, CMessage* msg)
{
	// Check for auto-send
	if (CPreferences::sPrefs->mAutomaticIMIP.GetValue() && (to != NULL) && (to->size() > 0))
	{
		return NewDraftiTIPAutomatic(to, cc, bcc, subject, description, attach, id, msg);
	}
	else
	{
		return NewDraftiTIPManual(to, cc, bcc, subject, description, attach, id, msg);
	}
}

bool CActionManager::NewDraftiTIPManual(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id, CMessage* msg)
{
	CLetterWindow* newWindow = NULL;
	try
	{
		newWindow = CLetterWindow::ManualCreate();

		if (newWindow)
		{
			if ((to && to->size()) || (cc && cc->size()) || (bcc && bcc->size()))
				newWindow->AddAddressLists(to, cc, bcc);
			
			newWindow->SetiTIP(subject, description, attach, msg);

			newWindow->PostSetMessage(id);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)
		newWindow = NULL;
	}
	
	return newWindow;
}

bool CActionManager::NewDraftiTIPAutomatic(const CAddressList* to, const CAddressList* cc, const CAddressList* bcc, const cdstring& subject, const cdstring& description, CDataAttachment* attach, const CIdentity* id, CMessage* msg)
{
	// Create body text
	cdstring body_txt(description);

	// Do signature insert
	IncludeSignature(body_txt, id);

	// Create multipart/mixed body
	std::auto_ptr<CAttachment> body;
	body.reset(new CDataAttachment);
	body->GetContent().SetContent(eContentMultipart, eContentSubMixed);

	// Add text/plain then text/calendar
	body->AddPart(new CDataAttachment(body_txt.grab_c_str()));
	body->AddPart(attach);
	
	{
		CMessageList msgs;
		msgs.SetOwnership(false);
		if (msg)
			msgs.push_back(msg);

		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> rmsg(CreateMessage((to != NULL) ? new CAddressList(*to) : NULL, 
												(cc != NULL) ? new CAddressList(*cc) : NULL,
												(bcc != NULL) ? new CAddressList(*bcc) : NULL,
												subject, body.get(), id, &id->GetDSN(true), msg != NULL ? &msgs : NULL, NULL));
		
		// Now send it
		if (SendMessage(*rmsg.get(), id, &id->GetDSN(true), NMessage::eDraftReply, &msgs))
			CErrorHandler::PutNoteAlertRsrc("CITIPProcessor::IMIPDone");
	}
	
	return true;
}

#pragma mark ____________________________Direct draft processing

bool CActionManager::ReplyMessages(CMessageList& msgs, bool quote, NMessage::EReplyType reply,
									const cdstring& text, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList);
	std::auto_ptr<CAddressList> cc_list(new CAddressList);
	std::auto_ptr<CAddressList> bcc_list(new CAddressList);

	for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		CEnvelope*	theEnv = (*iter)->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Replying to all recipients, sender & Ccs
		switch (reply)
		{
		// Only replying to Reply-To
		case NMessage::eReplyTo:
			// Set To: text
			
			// No Reply-to => use From
			if (!theEnv->GetReplyTo()->empty())
				to_list->MergeList(theEnv->GetReplyTo());
			else
				to_list->MergeList(theEnv->GetFrom());
			break;

		// Only replying to Sender
		case NMessage::eReplySender:
			// Set To: text
			to_list->MergeList(theEnv->GetSender());
			break;

		// Only replying to From
		case NMessage::eReplyFrom:
			// Set To: text
			to_list->MergeList(theEnv->GetFrom());
			break;

		// Replying to all
		case NMessage::eReplyAll:
			// Add Reply-To first (or From if no Reply-to)
			if (!theEnv->GetReplyTo()->empty())
				to_list->AddUnique(theEnv->GetReplyTo()->front());
			else
				to_list->AddUnique(theEnv->GetFrom()->front());

			// Add all recipients except me
			for(CAddressList::iterator iter2 = theEnv->GetTo()->begin(); iter2 != theEnv->GetTo()->end(); iter2++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter2))
					to_list->AddUnique(*iter2);
			}

			// Add all CCs except me
			for(CAddressList::iterator iter2 = theEnv->GetCC()->begin(); iter2 != theEnv->GetCC()->end(); iter2++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter2))
					cc_list->AddUnique(*iter2);
			}
			break;
		}
	}

	// Set Subject: text
	cdstring subject("Re: ");
	const char* osubject = msgs.front()->GetEnvelope()->GetSubject();
	if (osubject)
	{
		if (((osubject[0]!='R') && (osubject[0]!='r')) ||
			((osubject[1]!='E') && (osubject[1]!='e')) ||
			(osubject[2]!=':'))
			subject += osubject;
		else
			subject = osubject;
	}

	// Create body
	cdstring body_txt;

	// Add reply text at top if needed
	if (CPreferences::sPrefs->mReplyCursorTop.GetValue() && text.length())
	{
		body_txt += text;
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
	}

	// Do quote if requested
	if (quote)
	{
		// See whether cursor at top or bottom
		if (CPreferences::sPrefs->mReplyCursorTop.GetValue())
		{
			// Add all to message (cursor at top => in reverse)
			for(CMessageList::reverse_iterator riter = msgs.rbegin(); riter != msgs.rend(); riter++)
				IncludeMessage(**riter, false, body_txt);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			for(CMessageList::iterator iter = msgs.begin(); iter != msgs.end(); iter++)
				IncludeMessage(**iter, false, body_txt);
		}
	}

	// Add reply text at bottom if needed
	if (!CPreferences::sPrefs->mReplyCursorTop.GetValue() && text.length())
	{
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
		body_txt += text;
	}

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);
	SetIdentity(to_list.get(), cc_list.get(), bcc_list.get(), id);

	// Do signature insert
	IncludeSignature(body_txt, id);

	// Create body
	std::auto_ptr<CAttachment> body(new CDataAttachment(body_txt.grab_c_str()));

	{
		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> msg(CreateMessage(to_list.release(), cc_list.release(), bcc_list.release(), subject, body.get(), id, &id->GetDSN(true), &msgs, NULL));
		
		// Now send it
		SendMessage(*msg.get(), id, &id->GetDSN(true), NMessage::eDraftReply, &msgs);
	}

	return true;
}

bool CActionManager::CreateReplyMessages(CMessageList& msgs, bool quote, NMessage::EReplyType reply,
									const cdstring& text, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList);
	std::auto_ptr<CAddressList> cc_list(new CAddressList);
	std::auto_ptr<CAddressList> bcc_list(new CAddressList);

	for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		CEnvelope*	theEnv = (*iter)->GetEnvelope();
		if (!theEnv)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}

		// Replying to all recipients, sender & Ccs
		switch (reply)
		{
		// Only replying to Reply-To
		case NMessage::eReplyTo:
			// Set To: text

			// No Reply-to => use From
			if (!theEnv->GetReplyTo()->empty())
				to_list->MergeList(theEnv->GetReplyTo());
			else
				to_list->MergeList(theEnv->GetFrom());
			break;

		// Only replying to Sender
		case NMessage::eReplySender:
			// Set To: text
			to_list->MergeList(theEnv->GetSender());
			break;

		// Only replying to From
		case NMessage::eReplyFrom:
			// Set To: text
			to_list->MergeList(theEnv->GetFrom());
			break;

		// Replying to all
		case NMessage::eReplyAll:
			// Add Reply-To first (or From if no Reply-to)
			if (!theEnv->GetReplyTo()->empty())
				to_list->AddUnique(theEnv->GetReplyTo()->front());
			else
				to_list->AddUnique(theEnv->GetFrom()->front());

			// Add all recipients except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter))
					to_list->AddUnique(*iter);
			}

			// Add all CCs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				// Do not add me
				if (!CPreferences::TestSmartAddress(**iter))
					cc_list->AddUnique(*iter);
			}
			break;
		}
	}

	// Set Subject: text
	cdstring subject("Re: ");
	const char* osubject = msgs.front()->GetEnvelope()->GetSubject();
	if (osubject)
	{
		if (((osubject[0]!='R') && (osubject[0]!='r')) ||
			((osubject[1]!='E') && (osubject[1]!='e')) ||
			(osubject[2]!=':'))
			subject += osubject;
		else
			subject = osubject;
	}

	// Create body
	cdstring body_txt;

	// Add reply text at top if needed
	if (CPreferences::sPrefs->mReplyCursorTop.GetValue() && text.length())
	{
		body_txt += text;
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
	}

	// Do quote if requested
	if (quote)
	{
		// See whether cursor at top or bottom
		if (CPreferences::sPrefs->mReplyCursorTop.GetValue())
		{
			// Add all to message (cursor at top => in reverse)
			for(CMessageList::reverse_iterator riter = msgs.rbegin(); riter != msgs.rend(); riter++)
				IncludeMessage(**riter, false, body_txt);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			for(CMessageList::iterator iter = msgs.begin(); iter != msgs.end(); iter++)
				IncludeMessage(**iter, false, body_txt);
		}
	}

	// Add reply text at bottom if needed
	if (!CPreferences::sPrefs->mReplyCursorTop.GetValue() && text.length())
	{
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
		body_txt += text;
	}

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		CMessageList* pmsgs = new CMessageList(msgs);
		pmsgs->SetOwnership(false);
		newWindow->SetReplyMessages(pmsgs, to_list.get(), cc_list.get(), bcc_list.get());

		// Quote whole message
		newWindow->SetText(body_txt);

		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

bool CActionManager::ForwardMessages(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
										const cdstring& text, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList(addresses.mTo, addresses.mTo.length()));
	std::auto_ptr<CAddressList> cc_list(new CAddressList(addresses.mCC, addresses.mCC.length()));
	std::auto_ptr<CAddressList> bcc_list(new CAddressList(addresses.mBcc, addresses.mBcc.length()));

	// Set Subject: text
	cdstring subject = msgs.front()->GetEnvelope()->GetSubject();
	CPreferences::sPrefs->ForwardSubject(subject);

	// Create body
	cdstring body_txt;

	// Add reply text at top if needed
	if (CPreferences::sPrefs->mForwardCursorTop.GetValue() && text.length())
	{
		body_txt += text;
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
	}

	// Do quote if requested
	if (quote)
	{
		// See whether cursor at top or bottom
		if (CPreferences::sPrefs->mForwardCursorTop.GetValue())
		{
			// Add all to message (cursor at top => in reverse)
			for(CMessageList::reverse_iterator riter = msgs.rbegin(); riter != msgs.rend(); riter++)
				IncludeMessage(**riter, true, body_txt);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			for(CMessageList::iterator iter = msgs.begin(); iter != msgs.end(); iter++)
				IncludeMessage(**iter, true, body_txt);
		}
	}

	// Add reply text at bottom if needed
	if (!CPreferences::sPrefs->mForwardCursorTop.GetValue() && text.length())
	{
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
		body_txt += text;
	}

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);
	SetIdentity(to_list.get(), cc_list.get(), bcc_list.get(), id);

	// Do signature insert
	IncludeSignature(body_txt, id);

	// Create body
	std::auto_ptr<CAttachment> body;
	CAttachment* attach_txt = new CDataAttachment(body_txt.grab_c_str());
	
	// Look for attachments
	if (attach)
	{
		// Create multipart/mixed
		body.reset(new CDataAttachment);
		body->GetContent().SetContent(eContentMultipart, eContentSubMixed);

		// Add text part first
		body->AddPart(attach_txt);
		
		// Add each message
		for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
			body->AddPart(new CMessageAttachment(*iter, *iter));
	}
	else
		body.reset(attach_txt);

	{
		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> msg(CreateMessage(to_list.release(), cc_list.release(), bcc_list.release(), subject, body.get(), id, &id->GetDSN(true), &msgs, NULL));
		
		// Now send it
		SendMessage(*msg.get(), id, &id->GetDSN(true), NMessage::eDraftForward, &msgs);
	}

	return true;
}

bool CActionManager::CreateForwardMessages(CMessageList& msgs, bool quote, bool attach, NMessage::SAddressing addresses,
										const cdstring& text, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList(addresses.mTo, addresses.mTo.length()));
	std::auto_ptr<CAddressList> cc_list(new CAddressList(addresses.mCC, addresses.mCC.length()));
	std::auto_ptr<CAddressList> bcc_list(new CAddressList(addresses.mBcc, addresses.mBcc.length()));

	// Create body
	cdstring body_txt;

	// Add reply text at top if needed
	if (CPreferences::sPrefs->mForwardCursorTop.GetValue() && text.length())
	{
		body_txt += text;
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
	}

	// Do quote if requested
	if (quote)
	{
		// See whether cursor at top or bottom
		if (CPreferences::sPrefs->mForwardCursorTop.GetValue())
		{
			// Add all to message (cursor at top => in reverse)
			for(CMessageList::reverse_iterator riter = msgs.rbegin(); riter != msgs.rend(); riter++)
				IncludeMessage(**riter, true, body_txt);
		}
		else
		{
			// Add all to message (cursor at bottom => normal)
			for(CMessageList::iterator iter = msgs.begin(); iter != msgs.end(); iter++)
				IncludeMessage(**iter, true, body_txt);
		}
	}

	// Add reply text at bottom if needed
	if (!CPreferences::sPrefs->mForwardCursorTop.GetValue() && text.length())
	{
		if (quote)
		{
			body_txt += os_endl;
			body_txt += os_endl;
		}
		body_txt += text;
	}

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	// Set up forwarding options
	EForwardOptions forward = static_cast<EForwardOptions>(0);
	if (quote)
		forward = static_cast<EForwardOptions>(forward | eForwardQuote);
	if (attach)
	{
		// Use attachment
		forward = static_cast<EForwardOptions>(forward | eForwardAttachment);
		
		// Get preference for message/rfc822 behaviour as this is not controlled via the action itself
		if (CPreferences::sPrefs->mForwardRFC822.GetValue())
			forward = static_cast<EForwardOptions>(forward | eForwardRFC822);
	}

	try
	{
		// Quote whole message before adding forward items
		newWindow->SetText(body_txt);

		// Give it the message
		CMessageList* pmsgs = new CMessageList(msgs);
		pmsgs->SetOwnership(false);
		newWindow->SetForwardMessages(pmsgs, forward);
		newWindow->AddAddressLists(to_list.get(), cc_list.get(), bcc_list.get());

		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

bool CActionManager::BounceMessages(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList(addresses.mTo, addresses.mTo.length()));
	std::auto_ptr<CAddressList> cc_list(new CAddressList(addresses.mCC, addresses.mCC.length()));
	std::auto_ptr<CAddressList> bcc_list(new CAddressList(addresses.mBcc, addresses.mBcc.length()));

	// Set Subject: text
	cdstring subject = msgs.front()->GetEnvelope()->GetSubject();

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);
	SetIdentity(to_list.get(), cc_list.get(), bcc_list.get(), id);

	// Create body
	std::auto_ptr<CAttachment> body;
	
	// If more than one use multipart/mixed
	if (msgs.size())
	{
		// Create multipart/mixed
		body.reset(new CDataAttachment);
		body->GetContent().SetContent(eContentMultipart, eContentSubMixed);

	}

	for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		CAttachment* new_attach;

		// Make aliases of original body
		if ((*iter)->GetBody()->IsMessage())
			new_attach = new CMessageAttachment(*iter, (*iter)->GetBody()->GetMessage());
		else
			new_attach = new CAliasAttachment(*iter, (*iter)->GetBody());

		if (msgs.size())
			body->AddPart(new_attach);
		else
			body.reset(new_attach);
	}

	// Copy bounce message header
	std::auto_ptr<char> bounce(::strdup(msgs.front()->GetHeader()));

	{
		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> msg(CreateMessage(to_list.release(), cc_list.release(), bcc_list.release(), subject, body.get(), id, &id->GetDSN(true), &msgs, bounce.get()));
		
		// Now send it
		SendMessage(*msg.get(), id, &id->GetDSN(true), NMessage::eDraftForward, &msgs, bounce.get());
	}

	return true;
}
	
bool CActionManager::CreateBounceMessages(CMessageList& msgs, NMessage::SAddressing addresses, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList(addresses.mTo, addresses.mTo.length()));
	std::auto_ptr<CAddressList> cc_list(new CAddressList(addresses.mCC, addresses.mCC.length()));
	std::auto_ptr<CAddressList> bcc_list(new CAddressList(addresses.mBcc, addresses.mBcc.length()));

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		CMessageList* pmsgs = new CMessageList(msgs);
		pmsgs->SetOwnership(false);
		newWindow->SetBounceMessages(pmsgs);
		newWindow->AddAddressLists(to_list.get(), cc_list.get(), bcc_list.get());
		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}

bool CActionManager::RejectMessages(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Get addresses
	std::auto_ptr<CAddressList> to_list(new CAddressList);
	std::auto_ptr<CAddressList> cc_list(new CAddressList);
	std::auto_ptr<CAddressList> bcc_list(new CAddressList);

	// Read in the required DSN params
	cdstring return_path;
	cdstring received_from;
	cdstring received_by;
	cdstring received_for;
	cdstring received_date;
	
	msgs.front()->GetDeliveryData(return_path, received_from, received_by, received_for, received_date);
	to_list->push_back(new CAddress(return_path));

	// Set Subject: text
	cdstring subject("Returned mail: User unknown");

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);
	SetIdentity(to_list.get(), cc_list.get(), bcc_list.get(), id);

	// Create body
	std::auto_ptr<CAttachment> body(msgs.front()->CreateRejectDSNBody(return_msg));

	{
		// Create the message (it takes possesion of address lists)
		std::auto_ptr<CMessage> msg(CreateMessage(to_list.release(), cc_list.release(), bcc_list.release(), subject, body.get(), id, &id->GetDSN(true), &msgs, NULL));
		
		// Now send it
		SendMessage(*msg.get(), id, &id->GetDSN(true), NMessage::eDraftReply, &msgs);
	}

	return true;
}

bool CActionManager::CreateRejectMessages(CMessageList& msgs, bool return_msg, const cdstring& identity, bool tied)
{
	// Only if some messages
	if (!msgs.size())
		return false;

	// Now get chosen identity
	const CIdentity* id = GetIdentity(msgs, identity, tied);

	// Create the letter window
	CLetterWindow* newWindow = CLetterWindow::ManualCreate();

	if (!newWindow)
		return false;
	
	try
	{
		// Give it the message
		CMessageList* pmsgs = new CMessageList(msgs);
		pmsgs->SetOwnership(false);
		newWindow->SetRejectMessages(pmsgs, return_msg);

		newWindow->PostSetMessage(id);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (newWindow)
			FRAMEWORK_DELETE_DOC_WINDOW(newWindow)

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
	
	return true;
}

bool CActionManager::SendMessage(CMessage& msg, const CIdentity* id, const CDSN* dsn, NMessage::EDraftType type, CMessageList* msgs, const char* bounce)
{
	CMessage* mail_msg = &msg;

	bool smtp_hold = false;
	bool sent_now = false;

	// Protect
	if (CMulberryApp::sApp->IsDemo())
		CErrorHandler::PutNoteAlertRsrc("Alerts::General::SendDemoWarn");

	// Authenticate
	if (CAdminLock::sAdminLock.mLockSMTPSend && CAdminLock::sAdminLock.GetServerID().empty())
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::NoAuthenticate");
		return false;
	}

	// Get append mailbox
	CMbox* append_mbox = NULL;
	if (id->UseCopyTo() && !id->GetCopyToNone(true) && !id->GetCopyToChoose())
	{
		if (!id->GetCopyToNone(true) && !id->GetCopyToChoose())
		{
			append_mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(id->GetCopyTo(true));
		}
		else if (id->GetCopyToChoose())
		{
			// Do choice
			bool set_as_default = false;
			if (!CBrowseMailboxDialog::PoseDialog(false, true, append_mbox, set_as_default))
				return false;
			
			if (append_mbox && (append_mbox != (CMbox*) -1))
			{
				// Check if user wants this set as the default.
				// This will only be the case if the Choose dialog was used and the
				// checkbox there was turned on.
				if (set_as_default)
				{
					// Tell current identity to use the chosen mailbox as the default from now on
					const_cast<CIdentity*>(id)->SetCopyTo(append_mbox->GetAccountName(), true);
					const_cast<CIdentity*>(id)->SetCopyToNone(false);
					const_cast<CIdentity*>(id)->SetCopyToChoose(false);
					
					// Mark preference as dirty due to change
					CPreferences::sPrefs->mIdentities.SetDirty();
				}
			}

			// Failed to find copy to - alert user
			else if (!append_mbox)
			{
				return false;
			}
		}
	}

	try
	{
		// Check whether it is sendable
		if (!CAdminLock::sAdminLock.CanSend(mail_msg))
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Send it
		smtp_hold = CSMTPAccountManager::sSMTPAccountManager->SendMessage(mail_msg, *id, bounce != NULL);

		// Flag as successful send
		sent_now = true;

		// Check to see whether user might have wanted to copy this message to IMAP server
		// and make sure server is logged on

		if ((type != NMessage::eDraftMDN) && append_mbox && (append_mbox != (CMbox*) -1) && !append_mbox->GetProtocol()->IsLoggedOn())
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
		}

		// Mark any reply message as answered (these will have a logged on protocol)
		if ((type == NMessage::eDraftReply) && msgs && msgs->size())
		{
			for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			{
				if ((*iter)->GetMbox()->HasAllowedFlag(NMessage::eAnswered))
					(*iter)->ChangeFlags(NMessage::eAnswered, true);
			}
		}

		// Only do if not 'None'
		if ((type != NMessage::eDraftMDN) && append_mbox && (append_mbox != (CMbox*) -1) && append_mbox->GetProtocol()->IsLoggedOn())
		{

			// Copy replied message if required
			if (msgs && msgs->size() && id->GetCopyReplied(true))
			{
				// Do to all messages in list
				for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
				{
					// Do copy
					(*iter)->GetMbox()->CopyMessage((*iter)->GetMessageNumber(), false, append_mbox);

					// Reset any open copied to window
					CMailboxView* aView = CMailboxView::FindView(append_mbox);
					if (aView)
						aView->ResetTable();

					// If copy OK then delete from original mbox if required
					if (CPreferences::sPrefs->deleteAfterCopy.GetValue() &&
						(*iter)->GetMbox()->HasAllowedFlag(NMessage::eDeleted))
					{

						// Delete message
						(*iter)->ChangeFlags(NMessage::eDeleted, true);

						// Always force message window (and its sub-message windows) to close
						// even if allow deleted to be open is on
						CMessageWindow* aMsgWindow;
						while((aMsgWindow = CMessageWindow::FindWindow(*iter, true)) != NULL)
							FRAMEWORK_DELETE_WINDOW(aMsgWindow)
					}
				}
			}

			// Reset message header
			// Create header for appending (i.e. bcc, no identity, no x-mulberries)
			CRFC822::CreateHeader(mail_msg, CRFC822::eAddBcc, id, dsn, bounce);

			// Do append (does not acquire message)
			unsigned long new_uid = 0;
			append_mbox->AppendMessage(mail_msg, new_uid);

			// Update corresponding window if it exists
			CMailboxView* aView = CMailboxView::FindView(append_mbox);
			if (aView)
				aView->ResetTable();
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
	
	// Alert if message held in a queue
	if (smtp_hold)
		CErrorHandler::PutStopAlertRsrc("Alerts::Letter::SendHeld");

	return true;
}

CMessage* CActionManager::CreateMessage(CAddressList* to_list, CAddressList* cc_list, CAddressList* bcc_list,
										const char* subject, CAttachment* body, const CIdentity* identity, const CDSN* dsn,
										CMessageList* msgs, const char* bounce)
{
	CMessage* mail_msg = NULL;

	try
	{
		// Create the message to send
		mail_msg = new CMessage(identity,
								dsn,
								to_list,
								cc_list,
								bcc_list,
								subject,
								body,
								msgs,
								bounce);
		
		// Must protect against failure to fully create message
		if (!mail_msg->IsFullyCached())
		{
			CLOG_LOGTHROW(CGeneralException, -1L);
			throw CGeneralException(-1L);
		}

		// Now set flags
		SBitFlags new_flags(NMessage::eSeen);
		mail_msg->SetFlags(new_flags);

		// Check whether security required
		bool sign = identity->GetSign(true);
		bool encrypt = identity->GetEncrypt(true);
		if (CPluginManager::sPluginManager.HasSecurity() && (sign || encrypt))
		{
			CSecurityPlugin* plugin = CSecurityPlugin::GetDefaultPlugin();

			// Determine mode and any key
			CSecurityPlugin::ESecureMessage mode = CSecurityPlugin::eNone;
			cdstring key;
			if (sign && !encrypt)
			{
				mode = CSecurityPlugin::eSign;
				key = identity->GetSigningID();
			}
			else if (!sign && encrypt)
				mode = CSecurityPlugin::eEncrypt;
			else if (sign && encrypt)
			{
				mode = CSecurityPlugin::eEncryptSign;
				key = identity->GetSigningID();
			}

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
			delete to_list;
			delete cc_list;
			delete bcc_list;
		}
		
		CLOG_LOGRETHROW;
		throw;
	}

	return mail_msg;
}

const CIdentity* CActionManager::GetIdentity(CMessageList& msgs, const cdstring& identity, bool tied)
{
	// Default to first
	const CIdentity* id = NULL;

	if (tied && msgs.size())
	{
		// Get top message
		const CMessage* msg = msgs.front();

		// Find its mailbox
		const CMbox* mbox = msg->GetMbox();

		// Check for mbox specific identity
		if (!id)
			id = mbox->GetTiedIdentity();

		// Get account specific id if not tied to mailbox
		if (!id)
			id = &mbox->GetProtocol()->GetMailAccount()->GetAccountIdentity();
	}
	else
	{
		// Now get chosen identity
		if (!identity.empty())
			id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(identity);
	}

	// Default to first
	if (!id)
		id = &CPreferences::sPrefs->mIdentities.GetValue().front();

	return id;
}

void CActionManager::SetIdentity(CAddressList* to_list, CAddressList* cc_list, CAddressList* bcc_list, const CIdentity* identity)
{
	// Add identity default addresses
	cdstring temp;
	temp = identity->GetAddTo(true);
	CAddressList to_addrs(temp, temp.length());
	to_list->MergeList(&to_addrs);

	temp = identity->GetAddCC(true);
	CAddressList cc_addrs(temp, temp.length());
	cc_list->MergeList(&cc_addrs);

	temp = identity->GetAddBcc(true);
	CAddressList bcc_addrs(temp, temp.length());
	bcc_list->MergeList(&bcc_addrs);
}

void CActionManager::IncludeMessage(CMessage& theMsg, bool forward, cdstring& text)
{
	// Only do if text available
	if (!theMsg.HasText())
		return;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(&theMsg))
		return;

	// Read it in
	const char* msg_txt = NULL;
	try
	{
		// Get first suitable quotable part
		CAttachment* attach = theMsg.FirstDisplayPart();

		if (attach)
		{
			// Look for alternative to styled text
			if (attach->GetContent().GetContentSubtype() != eContentSubPlain)
			{
				// Look for plain alternative
				CAttachment* alternative = attach->GetAlternative(eContentText, eContentSubPlain);
				if (alternative)
					attach = alternative;
			}
			
			// Get data for this part
			msg_txt = attach->ReadPart(&theMsg);
			
			if (msg_txt)
				IncludeMessageText(theMsg, msg_txt, forward, attach->GetContent().GetContentSubtype(), attach->GetContent().IsFlowed(), text);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CActionManager::IncludeMessageText(CMessage& theMsg,
										const char* msg_txt,
										bool forward,
										EContentSubType type,
										bool is_flowed,
										cdstring& text)
{
	// Get start caption
	{
		cdstring startTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardStart.GetValue() :
														CPreferences::sPrefs->mReplyStart.GetValue(), &theMsg);;
		if (startTxt.length())
		{
			text += startTxt;
			text += os_endl;
		}
	}

	{
		// Convert styled text to plain
		cdstring plain_text;
		switch(type)
		{
		case eContentSubPlain:
		default:
			// Nothing to do
			plain_text = msg_txt;
			break;
		case eContentSubEnriched:
			{
				// Convert utf8 -> utf16 for Enriched processing
				cdustring utf16(msg_txt);
				
				// Do HTML processing
				CParserEnriched parser(utf16, NULL);
				utf16.steal(const_cast<unichar_t*>(parser.Parse(0, false)));
				
				// Do utf16 -> utf8 conversion for result
				plain_text = utf16.ToUTF8();
				
				// Enriched text is always flowed
				is_flowed = true;
			}
			break;
		case eContentSubHTML:
			{
				// Convert utf8 -> utf16 for HTML processing
				cdustring utf16(msg_txt);
				
				// Do HTML processing
				CParserHTML parser(utf16, NULL, NULL, NULL);
				utf16.steal(const_cast<unichar_t*>(parser.Parse(0, false)));

				// Do utf16 -> utf8 conversion for result
				plain_text = utf16.ToUTF8();
				
				// HTML text is always flowed
				is_flowed = true;
			}
			break;
		}

		// Strip sig dashes if required
		if (!forward && CPreferences::sPrefs->mReplyNoSignature.GetValue())
			CTextEngine::RemoveSigDashes(plain_text.c_str_mod());

		// Include the message's text as addition
		{
			std::auto_ptr<const char> quoted(CTextEngine::QuoteLines(plain_text, plain_text.length(),
											CRFC822::GetWrapLength(),
											forward ?
											CPreferences::sPrefs->mForwardQuote.GetValue() :
											CPreferences::sPrefs->mReplyQuote.GetValue(),
											&CPreferences::sPrefs->mRecognizeQuotes.GetValue(), is_flowed));
			text += quoted.get();
		}
	}

	// Finish with CRs
	if (!text.compare_end(os_endl))
		text += os_endl;
	text += os_endl;

	// Get end caption
	{
		cdstring endTxt = CCaptionParser::ParseCaption(forward ?
														CPreferences::sPrefs->mForwardEnd.GetValue() :
														CPreferences::sPrefs->mReplyEnd.GetValue(), &theMsg);;
		if (endTxt.length())
		{
			text += endTxt;
			text += os_endl;
		}
	}
}

void CActionManager::IncludeSignature(cdstring& text, const CIdentity* identity)
{
	// Do signature insert if required
	if (CPreferences::sPrefs->mAutoInsertSignature.GetValue())
	{
		// Get signature
		cdstring signature = identity->GetSignature(true);

		// Always line break before signature
		if (!text.compare_end(os_endl))
			text += os_endl;
		
		// May need empty line before signature
		if (CPreferences::sPrefs->mSignatureEmptyLine.GetValue() && !text.compare_end(os_endl2))
			text += os_endl;
		
		// May need sigdashes before signature
		CPreferences::sPrefs->SignatureDashes(signature);
		
		// Add signature
		text += signature;
	}
}
