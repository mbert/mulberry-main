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


// Source for CMessageView class

#include "CMessageView.h"

#include "CActionManager.h"
#include "CAddressList.h"
#include "CBinHexFilter.h"
#include "CBodyTable.h"
#include "CCharsetManager.h"
#include "CErrorHandler.h"
#include "CFormattedTextDisplay.h"
#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CMailboxWindow.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CNetworkException.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CReplyChooseDialog.h"
#include "CSecurityPlugin.h"
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif
#include "CSpellPlugin.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif
#include "CTaskClasses.h"
#include "CUUFilter.h"

#if __framework == __powerplant
#include "CBalloonDialog.h"
#include "CVisualProgress.h"
#include "CWaitCursor.h"
#elif __framework == __mfc
#include <WIN_LDataStream.h>
#elif __framework == __jx
#include "CWaitCursor.h"
#include "UNX_LDataStream.h"
#include <JXDisplay.h>
#endif

#include <algorithm>
#include <strstream>

#if __framework != __powerplant
#include "StValueChanger.h"
#endif

// __________________________________________________________________________________________________
// C L A S S __ C M E S S A G E W I N D O W
// __________________________________________________________________________________________________

// Server logs on/off
void CMessageView::ServerState(bool logon)
{
#if 0
	if (logon)
	{
		// Last last append to to value before logoff
		sLastCopyTo = CServerWindow::sMainServer->GetServer()->GetMboxList()->FindMbox(sSavedCopyToMbox);

	}
	else
	{
		// Get name of current append to selection
		if ((sLastCopyTo != (CMbox*) -1) && sLastCopyTo)
			sSavedCopyToMbox = sLastCopyTo->GetFullName();
		else
			sSavedCopyToMbox = cdstring::null_str;
	}
#endif
}

// Convert text to utf16
void CMessageView::Text2UTF16(i18n::ECharsetCode charset)
{
	// Do charset conversion to utf16
	mUTF16Text.reset();
	std::ostrstream sout;
	if ((mShowText != NULL) && (charset != i18n::eUTF16) && i18n::CCharsetManager::sCharsetManager.ToUTF16(charset, mShowText, ::strlen(mShowText), sout))
	{
		sout << std::ends << std::ends;
		mUTF16Text.reset((unichar_t*)sout.str());
	}
}

// Reset message text
void CMessageView::ShowPart(CAttachment* attach, bool restore)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	try
	{
		if (!attach)
			attach = mItsMsg->FirstDisplayPart();

		mShowText = attach ? attach->ReadPart(mItsMsg, restore || (GetViewOptions().GetPreviewFlagging() != CMailViewOptions::eMarkSeen)) : NULL;

		// Do charset conversion to utf8
		Text2UTF16(attach ? attach->GetContent().GetCharset() : i18n::eUSASCII);

		// Only do if message still exists
		if (!mItsMsg)
			return;

		mCurrentPart = attach;
		
		// Show it only if mode is not raw
		// If it is raw, someone else will take care of showing the raw text
		if (mParsing != eViewAsRaw)
			ResetText();
		
		// Now start the seen timer if requested by user
		if (mItsMsg && mItsMsg->IsUnseen() && (GetViewOptions().GetPreviewFlagging() == CMailViewOptions::eDelayedSeen))
			StartSeenTimer(GetViewOptions().GetPreviewDelay());
	}
	catch(CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Must throw out if disconnected/reconnected and window close
		if (ex.disconnected() || ex.reconnected())
		{
			CLOG_LOGRETHROW;
			throw;
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Extract part to disk
void CMessageView::ExtractPart(CAttachment* part, bool view)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	mItsMsg->ExtractPart(part, view);
}

// Find window belonging to message or its sub-messages
CMessageView* CMessageView::FindView(const CMessage* theMsg, bool owned_by)
{
	// Does window already exist?
		cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
	for(CMessageViewList::iterator iter = sMsgViews->begin(); iter != sMsgViews->end(); iter++)
	{
		// Compare with the one we want
		CMessage* test = (*iter)->GetMessage();
		if (test && owned_by)
		{
			// Iterate to find root owner
			while(test->GetOwner())
				test = test->GetOwner();
		}

		// Do test
		if (test == theMsg)

			// Return item if found
			return *iter;
	}

	return NULL;
}

// Check for window
bool CMessageView::ViewExists(const CMessageView* view)
{
	cdmutexprotect<CMessageViewList>::lock _lock(sMsgViews);
	CMessageViewList::iterator found = std::find(sMsgViews->begin(), sMsgViews->end(), view);
	return found != sMsgViews->end();
}

// Check for dangling messages and do expunge
void CMessageView::DoClose()
{
	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		// Clear out the message info which may be stale
		ClearMessage();
		
		// Now close parent window at idle time
		CCloseMessageWindowTask* task = new CCloseMessageWindowTask(static_cast<CMessageWindow*>(mOwnerWindow));
		task->Go();
	}
	else
	{
		// Reset preview to NULL
		SetMessage(NULL);
		
		// Save current state
		SaveState();
	}
}

#pragma mark ____________________________Commands

// Create a new draft
void CMessageView::DoNewLetter(bool option_key)
{
	// Determine context if required
	bool is_context = CPreferences::sPrefs->mContextTied.GetValue() ^ option_key;
	
	if (is_context && mItsMsg)
		// New draft with identity tied to this mailbox
		CActionManager::NewDraft(mItsMsg->GetMbox());
	else
		// New draft with default identity
		CActionManager::NewDraft();
}

// Spell check text - use hilite only mode
void CMessageView::OnEditSpellCheck()
{
	CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
	if (speller)
	{
#if __dest_os == __win32_os
		mText->GetRichEditCtrl().SetReadOnly(false);
#endif
		speller->SpellCheck(NULL, mText, true, true);
#if __dest_os == __win32_os
		mText->GetRichEditCtrl().SetReadOnly(true);
#endif
	}
}

// Speak message
#ifdef __use_speech
void CMessageView::SpeakMessage()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Add spoken message to queue
	CSpeechSynthesis::SpeakMessage(mItsMsg, const_cast<CAttachment*>(GetCurrentPart()), false);
}
#endif

// Read previous message
void CMessageView::OnMessageReadPrev()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Check for top level message
	if (!mItsMsg->IsSubMessage())
	{
		// Get previous message to this
		CMessage* prev = mItsMsg->GetMbox()->GetPrevMessage(mItsMsg, true, true);	// <- remember to get it cached!
		bool done = false;

		// Only do if message still exists
		if (!mItsMsg)
			return;

		while(!done)
		{
			// Loop over deleted messages
			while (prev && prev->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
			{
				prev = prev->GetMbox()->GetPrevMessage(prev, true, true);		// <- remember to get it cached!

				// Only do if message still exists
				if (!mItsMsg)
					return;
			}

			if (prev)
			{
				if (RedisplayMessage(prev))
					done = true;
				else
					prev = prev->GetMbox()->GetPrevMessage(prev, true, true);	// <- remember to get it cached!
			}
			else
				done = true;

			// Only do if message still exists
			if (!mItsMsg)
				return;
		}
	}
	else
	{
		// Get previous digest message to this
		CMessage* prev = mItsMsg->GetPrevDigest();
		if (prev)
		{
			RedisplayMessage(prev);
			prev->GetBody()->GetParent()->SetSeen(true);
		}
	}
}

// Read next message
void CMessageView::OnMessageReadNext()
{
	ReadNextMessage(false, false, false);
}

// Read next new message
void CMessageView::OnMessageReadNextNew()
{
	// Set focus to the mailbox pane, then get the mailbox pane to select the next new
	if (GetOwnerView())
	{
		GetOwnerView()->Focus();
		GetOwnerView()->SelectNextNew();
	}
}

// Read next message
void CMessageView::ReadNextMessage(bool delete_it, bool copy_it, bool option_key)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Check for top level message
	if (!mItsMsg->IsSubMessage())
	{
		// Need to cache mailbox for current message as we might need it later after the message
		// has been deleted and removed from the view
		CMbox* current_mbox = mItsMsg->GetMbox();

		// Get next message to this
		CMessage* next = mItsMsg->GetMbox()->GetNextMessage(mItsMsg, true, true);	// <- remember to get it cached!
		bool done = false;

		// Only do if message still exists
		if (!mItsMsg)
			return;

		while(!done)
		{
			// Loop over deleted messages
			while (next && next->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
			{
				next = next->GetMbox()->GetNextMessage(next, true, true);			// <- remember to get it cached!

				// Only do if message still exists
				if (!mItsMsg)
					return;
			}

			// Now do copy action
			if (copy_it)
			{
				// Copy the message (make sure this window is not deleted as we're using it for the next message)
				StValueChanger<bool> change(mRedisplayBlock, true);
				if (!TryCopyMessage(option_key))
					// Stop if copy not done;
					return;
			}
			
			// Now do delete action
			if (delete_it)
			{
				// Delete the message (make sure this window is not deleted as we're using it for the next message)
				StValueChanger<bool> change(mRedisplayBlock, true);
				mItsMsg->ChangeFlags(NMessage::eDeleted, true);

				// Only do if message still exists
				if (!mItsMsg)
					return;
			}

			if (next)
			{
				if (RedisplayMessage(next))
					done = true;
				else
					next = next->GetMbox()->GetNextMessage(next, true, true);		// <- remember to get it cached!
			}
			else
			{
				done = true;

				// Try rolling over to next recent
				if (CPreferences::sPrefs->mDoRollover.GetValue())
					DoRollover(current_mbox);
			}

			// Only do if message still exists
			if (!mItsMsg)
				return;
		}
	}
	else
	{
		// Get next digest message to this
		CMessage* next = mItsMsg->GetNextDigest();

		// Now do copy action
		if (copy_it)
		{
			// Copy the message (make sure this window is not deleted as we're using it for the next message)
			StValueChanger<bool> change(mRedisplayBlock, true);
			if (!TryCopyMessage(option_key))
				// Stop if copy not done;
				return;
		}
			
		// Only do if message still exists
		if (!mItsMsg)
			return;

		if (next)
		{
			RedisplayMessage(next);
			next->GetBody()->GetParent()->SetSeen(true);
		}
	}
}

// Delete this then read next message
void CMessageView::OnMessageDeleteRead()
{
	ReadNextMessage(true, false, false);
}

// Delete this then read next message
void CMessageView::CopyReadNextMessage(bool option_key)
{
	ReadNextMessage(false, true, option_key);
}

// Display message in existing window
bool CMessageView::RedisplayMessage(CMessage* theMsg)
{
	// Do not process redisplay if blocked
	if (mRedisplayBlock)
		return true;

	// Indicate close window if message deleted
	if (theMsg->IsDeleted() &&
		(!CPreferences::sPrefs->mOpenDeleted.GetValue() || CPreferences::sPrefs->mCloseDeleted.GetValue()))
	{
		return false;
	}

	// Select it in its mailbox window and force refresh
	CMailboxView* aView = theMsg->IsSubMessage() ? NULL : CMailboxView::FindView(theMsg->GetMbox());
	if (aView)
		aView->SelectMessage(theMsg);

	// Is message already displayed?
	CMessageView* other = FindView(theMsg);
	if (other)
	{
		// Select the other window and delete this
		//FRAMEWORK_WINDOW_TO_TOP(other)
		//FRAMEWORK_DELETE_WINDOW(this)
	}
	else
	{
		// Check message size first
		if (!CMailControl::CheckSizeWarning(theMsg))
			return false;

		try
		{
			// Give it to this window
			SetMessage(theMsg);
		}
		catch(CNetworkException& ex)
		{
			CLOG_LOGCATCH(CNetworkException&);

			// Must throw out if disconnected and window close
			if (ex.disconnected() || ex.reconnected())
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
	}

	return true;
}

// Attempt to rollover to next unseen
void CMessageView::DoRollover(CMbox* current_mbox)
{
	// Hide this window during rollover
	StNoRedraw _noredraw(this);

	// Look for possible rollover
	CMbox* next_mbox = CMailAccountManager::sMailAccountManager->GetNewMailbox(current_mbox);

	// Ask user to roll-over?
	bool done_rollover = false;
	if (next_mbox)
	{
		bool do_rollover = true;
		if (CPreferences::sPrefs->mRolloverWarn.GetValue())
		{
			bool dontshow = false;
			short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Message::MessageRollover", 0, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->mRolloverWarn.SetValue(false);
			if (answer == CErrorHandler::Cancel)
				do_rollover = false;
		}

		// Do rollover
		if (do_rollover)
		{
			// First block redisplay and detach original message to prevent close based on message ownership
			StValueChanger<bool> change(mRedisplayBlock, true);

			// Null out message and wipe parts list to prevent dereference of deleted message data
			mItsMsg = NULL;
			UpdatePartsList();

			// First find existing mailbox window
			CMailboxView* currentView = CMailboxView::FindView(current_mbox);

			// Look for existing new window
			CMailboxView* nextView = CMailboxView::FindView(next_mbox);

			// If new exists
			bool switched = false;
			if (nextView)
			{
				// Try to close this one
				if (currentView->TestClose())
					currentView->DoClose();

				// Pretend it worked
				currentView = nextView;

				// Indicate message selection required
				switched = true;
			}
			// Try recycle
			else
			{
				try
				{
					if (currentView->Recycle(next_mbox))
						// Indicate message selection required
						switched = true;
				}
				catch(...)
				{
					CLOG_LOGCATCH(...);

					// Failure while trying to open/select new mailbox

					// Just carrying on as if rollover failed
				}
			}

			// If successful switch then switch over this message window too
			if (switched)
			{
				// Get it to select the next new message
				NMessage::EFlags set_flag = NMessage::eNone;
				NMessage::EFlags unset_flag = NMessage::eNone;
				if (CPreferences::sPrefs->mUnseenNew.GetValue() && CPreferences::sPrefs->mRecentNew.GetValue())
				{
					set_flag = NMessage::eRecent;
					unset_flag = NMessage::eSeen;
				}
				else if (CPreferences::sPrefs->mUnseenNew.GetValue())
					unset_flag = NMessage::eSeen;
				else if (CPreferences::sPrefs->mRecentNew.GetValue())
					set_flag = NMessage::eRecent;
				bool ascending_seq = (next_mbox->GetShowBy() == cShowMessageAscending);
				CMessage* aMsg = next_mbox->GetNextFlagMessage(NULL, set_flag, unset_flag, true, ascending_seq, true);	// <-- get it cached

				// Message must exist
				if (aMsg)
				{
					// Select it in window
					currentView->SelectMessage(aMsg);

					// Allow display
					mRedisplayBlock = false;

					// Redisplay it in this window
					done_rollover = RedisplayMessage(aMsg);
				}
			}
		}
	}

	// Empty window if no rollover
	if (!done_rollover)
		SetMessage(NULL);

	// Must redo this as it doesn't get triggered while window is hidden
	PostSetMessage();
}

// Reply to this message
void CMessageView::ReplyToThisMessage(EReplyTo reply_to, bool option_key)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Might quote a selection
	cdstring quote;
	if (CPreferences::sPrefs->mQuoteSelection.GetValue())
		mText->GetSelectedText(quote);

	// Check flowed status
	bool is_flowed = false;
	CReplyChooseDialog::ProcessChoice(mItsMsg, quote, eContentSubPlain, (mCurrentPart != NULL) && mCurrentPart->GetContent().IsTextFlowed(), reply_to,
									!(CPreferences::sPrefs->optionKeyReplyDialog.GetValue() ^ option_key));
}

// Forward this message
void CMessageView::ForwardThisMessage(bool option_key)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// If there's a selection automatically quote it
	cdstring quote;
	mText->GetSelectedText(quote);

	CActionManager::ForwardMessage(mItsMsg, true, quote, eContentSubPlain, option_key);
}

// Bounce this message
void CMessageView::OnMessageBounce()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	CActionManager::BounceMessage(mItsMsg);
}

// Reject this message
void CMessageView::OnMessageReject()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	CActionManager::RejectMessage(mItsMsg);
}

// Send this message again
void CMessageView::OnMessageSendAgain()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	CActionManager::SendAgainMessage(mItsMsg);
}

// Copy the message to another mailbox
bool CMessageView::CopyThisMessage(CMbox* mbox, bool option_key)
{
	// Only do if message exists
	if (!mItsMsg)
		return false;

	// Check for real mailbox
	if (!mbox)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
		return false;
	}

	// Copy to another mailbox
	if (mItsMsg->GetMbox() != mbox)
	{
		// Check for sub-message
		if (mItsMsg->IsSubMessage())
		{
			unsigned long ignore;
			mbox->AppendMessage(mItsMsg, ignore);
		}
		else
			mItsMsg->GetMbox()->CopyMessage(mItsMsg->GetMessageNumber(), false, mbox);

		// Reset any open copied to window
		CMailboxView* aView = CMailboxView::FindView(mbox);
		if (aView)
			aView->ResetTable();

		// Only do if message still exists
		if (!mItsMsg)
			return false;

		// Delete if required
		if (!mItsMsg->IsSubMessage() &&
			(CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key) &&
			!mItsMsg->IsDeleted() && mItsMsg->GetMbox()->HasAllowedFlag(NMessage::eDeleted))
			OnMessageDelete();

		return true;
	}
	else
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::NoCopyToSame");
		return false;
	}
}

void CMessageView::OnMessageFlags(NMessage::EFlags flag)
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Toggle state
	mItsMsg->ChangeFlags(flag, !mItsMsg->HasFlag(flag));
}

// Verify/decrypt embedded message
void CMessageView::OnMessageViewCurrent()
{
	// Only do if message exists
	if (!mItsMsg)
		return;
		
	// Get parts table to view the currently active one
	GetPartsTable()->DoExtractCurrentPart(true);
}

// Decode embedded text as BinHex
void CMessageView::OnMessageDecodeBinHex()
{
	if (mShowText != NULL)
	{
		// Create data stream to hold message text
		long txt_len = ::strlen(mShowText);

		LDataStream msg_txt((char*) mShowText, txt_len);

		// Create progress dialog
#if __framework == __powerplant
		CBalloonDialog	theHandler(paneid_ProgressDialog, this);
		CProgressDialog* progress = (CProgressDialog*) theHandler.GetDialog();

		// Get the progress description
		cdstring status;
		status.FromResource("Alerts::Message::DecodeBinHex");
		progress->SetDescriptor(LStr255(status));
		theHandler.StartDialog();
#else
		CProgress progress1;
		CProgress* progress = &progress1;
#endif

		// Create full file stream
		CFullFileStream* aFile = NULL;

		try
		{
#if __framework == __powerplant
			// Just create new stream
			PPx::FSObject aSpec;
			aFile = new CFullFileStream(aSpec);
#else
			aFile = new CFullFileStream;
#endif

			// Create filter
			CBinHexFilter filter(&msg_txt, progress);
			filter.SetStream(aFile);
			filter.PutBytes(mShowText, txt_len);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		delete aFile;
	}
}

// Decode embedded text as UU
void CMessageView::OnMessageDecodeUU()
{
	if (mShowText)
	{
		// Create data stream to hold message text
		long txt_len = ::strlen(mShowText);

		LDataStream msg_txt((char*) mShowText, txt_len);

		// Create progress dialog
#if __framework == __powerplant
		CBalloonDialog	theHandler(paneid_ProgressDialog, this);
		CProgressDialog* progress = (CProgressDialog*) theHandler.GetDialog();

		// Get the progress description
		cdstring status;
		status.FromResource("Alerts::Message::DecodeUU");
		progress->SetDescriptor(LStr255(status));
		theHandler.StartDialog();
#else
		CProgress progress1;
		CProgress* progress = &progress1;
#endif

		// Create full file stream
		CFullFileStream* aFile = NULL;

		try
		{
#if __framework == __powerplant
			// Just create new stream
			PPx::FSObject aSpec;
			aFile = new CFullFileStream(aSpec);
#else
			aFile = new CFullFileStream;
#endif

			// Create filter
			CUUFilter filter(&msg_txt, progress);
			filter.SetStream(aFile);
			filter.PutBytes(mShowText, txt_len);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		delete aFile;
	}
}

// Verify/decrypt embedded message
void CMessageView::OnMessageVerifyDecrypt()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	try
	{
		// Look for security plugin
		if (CPluginManager::sPluginManager.HasSecurity())
		{
			CAttachment* old_body = mItsMsg->GetBody();
			mItsMsg->MakeCryptoInfo();
			if (CSecurityPlugin::VerifyDecryptPart(mItsMsg, mCurrentPart, *mItsMsg->GetCryptoInfo()))
			{
				// Only do if message still exists
				if (!mItsMsg)
					return;

				// Look for change in message body
				if (mItsMsg->GetBody() != old_body)
				{
					// Reset entire message
					CMessage* msg = mItsMsg;
					mItsMsg = NULL;
					SetMessage(msg);
				}
				else if (mCurrentPart)
				{
					// Reset text pointer
					mShowText = mCurrentPart->GetData();

					// Do charset conversion to utf8
					Text2UTF16(mCurrentPart->GetContent().GetCharset());

					ResetText();
				}

				// Do notification
				if (mItsMsg->GetCryptoInfo()->GetDidSignature() && mItsMsg->GetCryptoInfo()->GetSignatureOK())
					CPreferences::sPrefs->mVerifyOKNotification.GetValue().DoNotification("UI::Message::MessageVerifyOK");
			}
				
			// Show the secure info pane
			SetSecretPane(*mItsMsg->GetCryptoInfo());
			ShowSecretPane(true);
		}
	}
	catch(CNetworkException& ex)
	{
		CLOG_LOGCATCH(CNetworkException&);

		// Must throw out if disconnected/reconnected and window close
		if (ex.disconnected() || ex.reconnected())
		{
			CLOG_LOGRETHROW;
			throw;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Delete the message and then the window
void CMessageView::OnMessageDelete()
{
	// Only do if message exists
	if (!mItsMsg)
		return;

	// Undelete message
	mItsMsg->ChangeFlags(NMessage::eDeleted, !mItsMsg->IsDeleted());

	MessageChanged();
}

void CMessageView::OnViewAs(EView view)
{
	// Look for switch into/out of raw mode
	if ((mParsing == eViewAsRaw) ^ (view == eViewAsRaw))
	{
		if (view == eViewAsRaw)
		{
			// Read in raw text - handle cancel or failure
			if (!ShowRawBody())
			{
				// Set view back to what it was
				view = mParsing;
				
				// Whoever calls this method must reset the popup to its original value
			}
		}
		else
			mRawUTF16Text.reset(NULL);
	}

	mParsing = view;
	ResetText();
}

// Increase displayed font size
void CMessageView::OnIncreaseFont()
{
	// Bump up and reset the text
	mFontScale++;
	ResetFontScale();
	ResetText();
}

// Decrease displayed font size
void CMessageView::OnDecreaseFont()
{
	// Bump down and reset the text
	mFontScale--;
	ResetFontScale();
	ResetText();
}

// Change quotes on display
void CMessageView::OnQuoteDepth(EQuoteDepth quote)
{
	long new_depth = 0;
	
	switch(quote)
	{
	case eQuoteDepth_All:
		new_depth = -1;
		break;
	case eQuoteDepth_No:
		new_depth = 0;
		break;
	case eQuoteDepth_1:
		new_depth = 1;
		break;
	case eQuoteDepth_2:
		new_depth = 2;
		break;
	case eQuoteDepth_3:
		new_depth = 3;
		break;
	case eQuoteDepth_4:
		new_depth = 4;
		break;
	}

	// Look for change in depth
	if (mQuoteDepth != new_depth)
	{
		mQuoteDepth = new_depth;
		ResetText();
	}
}

// Read in raw body
bool CMessageView::ShowRawBody()
{
	// If raw text exists, just use that
	if (!mItsMsg || mRawUTF16Text.get())
		return true;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(mItsMsg, true))
		return false;

	// Read in raw text
	std::ostrstream out;
	mItsMsg->GetRawBody(out, GetViewOptions().GetPreviewFlagging() != CMailViewOptions::eMarkSeen);
	
	// Convert to UTF16 and grab the data
	cdustring uout(out.str());
	out.freeze(false);
	mRawUTF16Text.reset(uout.release());
		
	return true;
}

// Get options for this view
const CMailViewOptions&	CMessageView::GetViewOptions() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions();
}
