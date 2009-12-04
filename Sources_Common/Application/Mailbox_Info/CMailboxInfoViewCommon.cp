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


// Source for CMailboxInfoView class

#include "CMailboxInfoView.h"

#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CIMAPCommon.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#if __dest_os == __win32_os
#include "CMailboxInfoFrame.h"
#endif
#include "CMailboxInfoTable.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxInfoWindow.h"
#include "CMailboxTitleTable.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessageWindow.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CResources.h"
#endif
#include "CStringUtils.h"
#include "CTaskClasses.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CWaitCursor.h"
#endif
#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __linux_os
#include "CWindowsMenu.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#include <LIconControl.h>
#include "MyCFString.h"
#else
#include "StValueChanger.h"
#endif

#if __dest_os == __linux_os
#include <JXImageWidget.h>
#include "CWaitCursor.h"
#endif

//#include <UProfiler.h>

//#include <string.h>

#include <numeric>

// Handle close of protocol
void CMailboxInfoView::DoProtocolLogoff(const CMboxProtocol* proto)
{
	// Special case 3-pane/1-pane
	if (Is3Pane())
	{
		// Look for subtitutes
		if (GetUseSubstitute())
		{
			// Find the appropriate mailbox
			unsigned long index = GetSubstituteIndex(proto);
			if (index < mSubsList.size())
			{
				// Close this one
				DoCloseOne(index);
			}
		}
		else
			ViewMbox(NULL);
	}
	else
		// Just close this view to force window to close
		DoClose();
}

// Handle removal of WD
void CMailboxInfoView::DoRemoveWD(const CMboxList* wd)
{
	// Check for any mailbox in this hierarchy and close it

	// Special case 3-pane/1-pane
	if (Is3Pane())
	{
		// Look for subtitutes
		if (GetUseSubstitute())
		{
			// Find the appropriate mailbox
			unsigned long index = GetSubstituteIndex(wd);
			if (index < mSubsList.size())
			{
				// Close this one
				DoCloseOne(index);
			}
		}
		else if (GetMbox()->GetMboxList() == wd)
			ViewMbox(NULL);
	}
	else if (GetMbox()->GetMboxList() == wd)
		// Just close this view to force window to close
		DoClose();
}

// Check for dangling messages and do expunge
bool CMailboxInfoView::TestClose()
{
	// Must be open for this to make sense
	if (!IsOpen())
		return true;

	// Set status
	SetClosing();

	// Try to close all of them
	if (!TestCloseAll())
	{
		SetOpen();
		return false;
	}
	else
		return true;
}

// Check for dangling messages and do expunge
bool CMailboxInfoView::TestCloseOne(unsigned long index)
{
	// Must be open for this to make sense
	if (!IsOpen())
		return true;

	// Determine which mailbox is being closed
	CMbox* mbox = NULL;
	if (mUseSubstitute)
		// Get mailbox from tab list
		mbox = mSubsList.at(index).mMailbox;
	else
		// Just use standard mailbox
		mbox = GetMbox();

	// Set status if this is the actve one
	if (mbox == GetMbox())
		SetClosing();

	if (mbox && !TestClose(mbox))
	{
		SetOpen();
		return false;
	}
	else
		return true;
}

// Check whether close allowed
bool CMailboxInfoView::TestCloseAll(bool not_locked)
{
	// Now look for other mailbox in a tabbed view and close all those
	if (mUseSubstitute)
	{
		// Do in reverse to prevent tab display updating problems
		for(SMailboxSubstituteList::reverse_iterator riter = mSubsList.rbegin(); riter != mSubsList.rend(); riter++)
		{
			// Ignore locked
			if (not_locked && (*riter).mLocked)
				continue;

			// Do test
			if (!TestClose((*riter).mMailbox))
				return false;
		}
		
		return true;
	}
	else
	{
		// Only if a mailbox exists
		return GetMbox() ? TestClose(GetMbox()) : true;
	}
}

// Check for dangling messages and do expunge
bool CMailboxInfoView::TestCloseOthers(unsigned long index)
{
	// Must be open for this to make sense
	if (!IsOpen())
		return true;

	// Now look for other mailboxes in a tabbed view and close all those
	if (mUseSubstitute)
	{
		// Do in reverse to prevent tab display updating problems
		unsigned long test_index = mSubsList.size() - 1;
		for(SMailboxSubstituteList::reverse_iterator riter = mSubsList.rbegin(); riter != mSubsList.rend(); riter++, test_index--)
		{
			// Ignore locked or same index
			if ((test_index == index) || (*riter).mLocked)
				continue;

			// Do test
			if (!TestClose((*riter).mMailbox))
				return false;
		}
		
		return true;
	}
	else
	{
		// Always true if no tabs
		return true;
	}
}

// Check whether close allowed
bool CMailboxInfoView::TestClose(const CMboxProtocol* proto)
{
	// Now look for other mailbox in a tabbed view and close all those
	if (mUseSubstitute)
	{
		// Do in reverse to prevent tab display updating problems
		for(SMailboxSubstituteList::reverse_iterator riter = mSubsList.rbegin(); riter != mSubsList.rend(); riter++)
		{
			// Do test
			if ((*riter).mMailbox->TestMailer(proto) && !TestClose((*riter).mMailbox))
				return false;
		}
		
		return true;
	}
	else
	{
		// Only if a mailbox exists
		return (GetMbox() && GetMbox()->TestMailer(proto)) ? TestClose(GetMbox()) : true;
	}
}

// Check for dangling messages and do expunge
bool CMailboxInfoView::TestClose(CMbox* mbox)
{
	// See if more than one view of this mailbox exists, in which
	// case the close actions are not to be completed
	if (CountViews(mbox) > 1)
		return true;

	// Look for dangling messages in drafts

	// Iterate over all letter windows
	{
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
		{
			if ((*iter)->TestOwnership(mbox))
			{
				// Alert and do not allow close
				CErrorHandler::PutStopAlertRsrc("Alerts::Mailbox::NoCloseMailbox");
				return false;
			}
		}
	}

	// Check for clearing of INBOX:
	// Name == INBOX, mailbox to clear is named and seen messages in INBOX
	if (!::strcmpnocase(mbox->GetName(), cINBOX) &&
			CPreferences::sPrefs->mDoMailboxClear.GetValue() &&
			mbox->AnySeen())
	{

		// If required produce warning
		if (CPreferences::sPrefs->clear_warning.GetValue())
		{
			bool dontshow = false;
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::ClearINBOX",
																			"ErrorDialog::Btn::Cancel",
																			"ErrorDialog::Btn::CloseWithoutClear",
																			NULL,
																			"ErrorDialog::Text::ClearINBOX", 2, NULL, NULL, NULL, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->clear_warning.SetValue(false);
			if (result == CErrorDialog::eBtn2)
				return false;
			if (result == CErrorDialog::eBtn3)
				return true;
		}

		// Otherwise do check for deleted messages
		else if (mbox->AnyDeleted() && !CPreferences::sPrefs->warnOnExpunge.GetValue())
		{
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::ClearINBOX",
																			"ErrorDialog::Btn::Cancel",
																			"ErrorDialog::Btn::CloseWithoutClear",
																			NULL,
																			"ErrorDialog::Text::ClearINBOXDeleted");
			if (result == CErrorDialog::eBtn2)
				return false;
			if (result == CErrorDialog::eBtn3)
				return true;
		}

		// Look for copy to mailbox
		cdstring name = CPreferences::sPrefs->clear_mailbox.GetValue();
		CMbox* mbox_copy = CMailAccountManager::sMailAccountManager->FindMboxAccount(name);

		// If found in list then select seen, copy and expunge
		if (mbox_copy)
		{
			bool do_close = false;
			try
			{
				// Get list of message uids that are seen and not deleted
				NMessage::EFlags set_flag = NMessage::eSeen;
				NMessage::EFlags unset_flag = NMessage::eDeleted;
				ulvector uids;
				mbox->MatchMessageFlags(set_flag, unset_flag, uids, true);

				// Do copy
				ulmap temp;
				mbox->CopyMessage(uids, true, mbox_copy, temp);

				// Set deleted flag on chosen messages
				mbox->SetFlagMessage(uids, true, NMessage::eDeleted, true);

				do_close = GetInfoTable()->DoMailboxExpunge(true, mbox);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Any exception should not prevent close
				do_close = true;
			}

			return do_close;
		}

		// Otherwise flag error and disallow close
		else
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Mailbox::NoClearMailbox");
			return false;
		}
	}
	else if (CPreferences::sPrefs->expungeOnClose.GetValue())
	{
		bool do_close = false;

		try
		{
			do_close = GetInfoTable()->DoMailboxExpunge(true, mbox);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Any exception should not prevent close
			do_close = true;
		}

		return do_close;
	}

	return true;
}

void CMailboxInfoView::DoOpenMbox(CMbox* mbox, bool is_search)
{
	// Protect against failure that could leave this window in indeterminate state
	try
	{
		// Must open the new mailbox (NB May already have been opened by search engine)
		// 20-Feb-2000 CD: added check for open mailbox to prevent connection leak with searches
		// 08-Sep-2001 CD: removed check for open mailbox as search engine now handles mailbox
		// 				   connections better. We also need to allow a mailbox window to open a mailbox
		//				   already being viewed in 3-pane window. Also check whether it was open prior
		//				   to opening it again so we can determine whether filter action is required
		//if (!anMbox->IsOpen())
		bool was_open = mbox->OpenIfOpen();
		if (!was_open)
			mbox->Open();

		// Now force transfer of search results into view if anything found
		if (is_search)
			mbox->OpenSearch();

		// Reset status changed by DoCloseMbox
		// Must do this after opening mailbox so redisplay is blocked while messages are being recached
		SetOpen();

		// Now set new mbox
		SetMbox(mbox, is_search);

		// Need to make sure cache increment is reset as window may not resize and force it to be
		GetInfoTable()->UpdateCacheIncrement();

		// Do filter action if not previously open
		if (!was_open)
			CPreferences::sPrefs->GetFilterManager()->OpenMailbox(mbox);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Got here because mbox failed to open
		if (mbox->IsOpen())
		{
			// Forcibly close it
			mbox->CloseSilent(true);
			
			// Change window status to indicate close has been done
			if (!mUseSubstitute)
				SetClosed();
		}

		// Now forcibly close the window (done at idle time)
		DoClose(mbox);

		// Have to throw out of here
		CLOG_LOGRETHROW;
		throw;
	}
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoClose()
{
	// Close all items
	DoCloseAll();
	
	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		CCloseMailboxWindowTask* task = new CCloseMailboxWindowTask(GetMailboxWindow());
		task->Go();
	}
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoClose(CMbox* mbox)
{
	if (mUseSubstitute)
	{
		for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
		{
			// Locate the mailbox to close
			if ((*iter).mMailbox == mbox)
			{
				DoCloseMbox(mbox);
				CloseSubstitute(mbox);
				break;
			}
		}
	}
	else
	{
		// Close it in the normal way - this will clear out the table
		DoCloseMbox();
	}
	
	// Now close its parent 1-pane window at idle time if no left
	if (!GetMbox() && !Is3Pane())
	{
		CCloseMailboxWindowTask* task = new CCloseMailboxWindowTask(GetMailboxWindow());
		task->Go();
	}
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoClose(const CMboxProtocol* proto)
{
	if (mUseSubstitute)
	{
		// Continually loop over substitiute list until all items are remove
		bool removed = true;
		while(removed)
		{
			removed = false;
			for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
			{
				// Locate the mailbox to close
				if ((*iter).mMailbox->TestMailer(proto))
				{
					DoCloseMbox((*iter).mMailbox);
					CloseSubstitute((*iter).mMailbox);
					removed = true;
					break;
				}
			}
		}
	}
	else
	{
		// Close it in the normal way - this will clear out the table
		DoCloseMbox();
	}
	
	// Now close its parent 1-pane window at idle time if no left
	if (!GetMbox() && !Is3Pane())
	{
		CCloseMailboxWindowTask* task = new CCloseMailboxWindowTask(GetMailboxWindow());
		task->Go();
	}
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoCloseOne(unsigned long index)
{
	// Add info to prefs (must do while mbox open)
	SaveState();

	// Determine which mailbox is being closed
	CMbox* mbox = NULL;
	if (mUseSubstitute)
		// Get mailbox from tab list
		mbox = mSubsList.at(index).mMailbox;
	else
		// Just use standard mailbox
		mbox = GetMbox();

	// Now look for other mailbox in a tabbed view and close all those
	if (mbox)
	{
		DoCloseMbox(mbox);
		if (mUseSubstitute)
			CloseSubstitute(mbox);
	}
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoCloseAll(bool not_locked)
{
	// Now look for other mailbox in a tabbed view and close all those
	if (mUseSubstitute)
	{
		// Check whether any locked
		bool not_closing_all = false;
		if (not_locked)
		{
			for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
				not_closing_all |= (*iter).mLocked;
		}

		if (not_closing_all)
		{
			// Do in reverse to prevent tab display updating problems
			unsigned long adjust = 0;
			bool do_not_close = false;
			while(mSubsList.size() > adjust)
			{
				// Get the one at the end adjusting for active mailbox
				SMailboxSubstituteList::iterator iter = mSubsList.end() - (adjust + 1);

				// Do only for mailboxes not shown
				if ((*iter).mMailbox != GetMbox())
				{
					// Ignore if locked
					if ((*iter).mLocked)
						adjust++;
					else
					{
						// Close the tab
						DoCloseMbox((*iter).mMailbox);
						CloseSubstitute((*iter).mMailbox);
					}
				}
				else
				{
					// Check whether the main one should be closed
					do_not_close = (*iter).mLocked;
					adjust++;
				}
			}
			
			// Close remaining one in the normal way - this will clear out the table
			if (!do_not_close)
			{
				CMbox* mbox = GetMbox();
				DoCloseMbox(mbox);
				CloseSubstitute(mbox);
			}
		}
		else
		{
			// Set status - need to do this here to prevent close tasks from being generated
			SetClosing();

			// Do in reverse to prevent tab display updating problems
			unsigned long adjust = 0;
			while(mSubsList.size() > adjust)
			{
				// Get the one at the end adjusting for active mailbox
				SMailboxSubstituteList::iterator iter = mSubsList.end() - (adjust + 1);

				// Do only for mailboxes not shown
				if ((*iter).mMailbox != GetMbox())
				{
					// Close the tab
					DoCloseMbox((*iter).mMailbox);
					CloseSubstitute((*iter).mMailbox);
				}
				else
				{
					// Check whether the main one should be closed
					adjust++;
				}
			}
			
			// Close remaining one in the normal way - this will clear out the table
			DoCloseMbox();
		}
	}
	else
		// Close it in the normal way - this will clear out the table
		DoCloseMbox();
}

// Check for dangling messages and do expunge
void CMailboxInfoView::DoCloseOthers(unsigned long index)
{
	// Now look for other mailbox in a tabbed view and close all those
	if (mUseSubstitute)
	{
		// Get mailbox from tab list
		CMbox* no_close_mbox = mSubsList.at(index).mMailbox;

		// Do in reverse to prevent tab display updating problems
		unsigned long adjust = 0;
		while(mSubsList.size() > adjust)
		{
			// Get the one at the end adjusting for active mailbox
			SMailboxSubstituteList::iterator iter = mSubsList.end() - (adjust + 1);

			// Do only for mailboxes not shown
			if ((*iter).mMailbox != no_close_mbox)
			{
				// Ignore if locked
				if ((*iter).mLocked)
					adjust++;
				else
				{
					// Close the tab
					DoCloseMbox((*iter).mMailbox);
					CloseSubstitute((*iter).mMailbox);
				}
			}
			else
			{
				// Check whether the main one should be closed
				adjust++;
			}
		}
	}
}

// Close window objects
void CMailboxInfoView::DoCloseMbox()
{
	// Only meaningful if not already closed
	if (IsClosed())
		return;

	// Set status
	SetClosing();

	// Get its mailbox
	DoCloseMbox(GetMbox());
	if (mUseSubstitute)
		CloseSubstitute(GetMbox());

	// Must set mbox to NULL in case of delete on close
	SetMbox(NULL);

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);

	// Force any preview to be empty
	GetTable()->PreviewMessage(true);

	// Set status
	SetClosed();
}

// Close window objects
void CMailboxInfoView::DoCloseMbox(CMbox* mbox)
{
	// Save state if this is the active one
	if (mbox == GetMbox())
		SaveState();
		
	// Do close behaviour only if this is the last view on the mailbox
	if (CountViews(mbox) <= 1)
	{
		// Remove any message windows belonging to this mbox
		// Do in reverse as list may change
		cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
		for(CMessageWindow::CMessageWindowList::reverse_iterator riter = CMessageWindow::sMsgWindows->rbegin();
				riter != CMessageWindow::sMsgWindows->rend(); riter++)
		{
			CMessage* aMsg = (*riter)->GetMessage();
			if (aMsg && (aMsg->GetMbox() == mbox))
				// Found existing window so close it
				FRAMEWORK_DELETE_WINDOW(*riter)
		}
	}

	// Close its mailbox
	if (mbox)
	{
		// Do filter action BEFORE its closed
		CPreferences::sPrefs->GetFilterManager()->CloseMailbox(mbox);

		// Now close it
		mbox->Close();
	}
}

// Open and set it
void CMailboxInfoView::ViewMbox(CMbox* mbox, bool use_dynamic, bool is_search)
{
	// Protect
	if (CMulberryApp::sApp->IsDemo() && ::strcmpnocase(mbox->GetName(), cINBOX))
		CErrorHandler::PutNoteAlertRsrc("Alerts::General::DemoWarn");

	try
	{
		// Resolve to open mailbox
		CMbox* open_mbox = mbox ? mbox->GetProtocol()->FindOpenMbox(mbox->GetName()) : NULL;
		if (open_mbox)
			mbox = open_mbox;

		// Is it different from the one already set
		CMbox* mbox_old = GetMbox();
		if (mbox == mbox_old)
			return;

		// No mbox - reset the view to empty - no recyle
		if (!mbox)
		{
			// Close the current one
			DoCloseMbox();
			
			// Clear it out
			SetMbox(mbox);
		}
		else
		{
			// Try to get a global lock on the mailbox
			CMbox::mbox_trylock _trylock(CMbox::_smutex, mbox);

			if (!_trylock.is_locked())
			{
				CErrorHandler::PutStopAlertRsrcStr("Alerts::Mailbox::MailboxBusy", mbox->GetName());
			}
			else
			{
				// May take some time so spin cursor
				CWaitCursor wait;

				// Recycle will take care of closing the old one and opening the new one
				// even if either are NULL
				if (mUseSubstitute)
				{
					if (use_dynamic && mUseDynamicTab)
					{
						// If its already a substitute, then switch to it in the normal way
						if (IsSubstitute(mbox))
							Substitute(mbox, true, is_search);
						
						// If no current dynamic item or there are zero/one - create a new tab
						else if (!HasDynamicItem() || (mSubsList.size() < 2))
							Substitute(mbox, true, is_search);

						else
						{
							// Make sure dynamic item is selected
							SetSubstitute(GetDynamicIndex());
							
							// Now recyle the current (dynamic) tab to new mailbox
							RecycleSubstitute(mbox, is_search);
						}
					}
					else
						Substitute(mbox, true, is_search);
				}
				else
					RecycleClose(mbox, is_search);
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Set the mbox
void CMailboxInfoView::SetMbox(CMbox* anMbox, bool is_search)
{
	// Init the content
	InitMbox(anMbox);
	
	// Force any preview to be empty
	GetTable()->PreviewMessage(true);

#if __dest_os == __win32_os
	// Force focus on table
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		GetTable()->SetFocus();
#endif

	// Listen in to changes to mailbox protocol
	if (anMbox)
		anMbox->GetProtocol()->Add_Listener(this);

	// Reset window state
	mNoSearchReset = is_search;
	ResetState();
	mNoSearchReset = false;

	// Now do init pos
	GetTable()->InitPos();

	// If null, set focus back to server pane in 3-pane
	if (Is3Pane() && (anMbox == NULL))
		FocusOwnerView();
}

// Reset the mbox
void CMailboxInfoView::ResetMbox(CMbox* anMbox)
{
	// Init the content
	InitMbox(anMbox);
	
	// Reset window state with changing the match settings
	{
		StValueChanger<bool> _change(mNoSearchReset, true);
		ResetState();
	}
	
	// Now do init pos
	GetTable()->mListChanging = false;
}

// Init the mbox
void CMailboxInfoView::InitMbox(CMbox* anMbox)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (!Is3Pane() && anMbox)
	{
		cdstring title(anMbox->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1));
		if (anMbox->IsReadOnly())
		{
			cdstring readonly;
			readonly.FromResource("UI::Mailbox::ReadOnlyMailbox");
			title += readonly;
		}
		MyCFString cftext(title.c_str(), kCFStringEncodingUTF8);
		GetMailboxWindow()->SetCFDescriptor(cftext);
	}

	// Do icon state if window exists
	if (!Is3Pane() && anMbox && anMbox->IsOpen())
	{
		ResIDT icon = anMbox->GetMsgProtocol()->IsSecure() ? icnx_MailboxStateRemoteSSL : icnx_MailboxStateRemote;
		if (anMbox->GetMsgProtocol()->CanDisconnect())
		{
			if (anMbox->GetMsgProtocol()->IsDisconnected())
				icon = icnx_MailboxStateDisconnected;
		}
		else if (anMbox->IsLocalMbox())
			icon = icnx_MailboxStateLocal;
		Broadcast_Message(CMailboxInfoToolbar::eBroadcast_IconState, &icon);
	}

#elif __dest_os == __win32_os
	if (!Is3Pane() && anMbox)
	{
		cdstring title(anMbox->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1));
		if (anMbox->IsReadOnly())
		{
			cdstring readonly;
			readonly.FromResource("UI::Mailbox::ReadOnlyMailbox");
			title += readonly;
		}
		GetMailboxWindow()->GetDocument()->SetTitle(title.win_str());
	}

	// Do icon state if window exists
	if (!Is3Pane() && anMbox && anMbox->IsOpen())
	{
		int icon_id = anMbox->GetMsgProtocol()->IsSecure() ? IDI_MAILBOX_REMOTESSL : IDI_MAILBOX_REMOTE;
		if (anMbox->GetMsgProtocol()->CanDisconnect())
		{
			if (anMbox->GetMsgProtocol()->IsDisconnected())
				icon_id = IDI_MAILBOX_DISCONNECTED;
		}
		else if (anMbox->IsLocalMbox())
			icon_id = IDI_MAILBOX_LOCAL;
		Broadcast_Message(CMailboxInfoToolbar::eBroadcast_IconState, &icon_id);
	}
#elif __dest_os == __linux_os
	if (!Is3Pane() && anMbox)
	{
		cdstring title(anMbox->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1));
		if (anMbox->IsReadOnly())
		{
			cdstring readonly;
			readonly.FromResource("UI::Mailbox::ReadOnlyMailbox");
			title += readonly;
		}
		GetWindow()->SetTitle(title.c_str());
	}

	// Do icon state
	if (!Is3Pane() && anMbox && anMbox->IsOpen())
	{
		int icon = anMbox->GetMsgProtocol()->IsSecure() ? IDI_MAILBOX_REMOTESSL : IDI_MAILBOX_REMOTE;
		if (anMbox->GetMsgProtocol()->CanDisconnect())
		{
			if (anMbox->GetMsgProtocol()->IsDisconnected())
				icon = IDI_MAILBOX_DISCONNECTED;
		}
		else if (anMbox->IsLocalMbox())
			icon = IDI_MAILBOX_LOCAL;
		Broadcast_Message(CMailboxInfoToolbar::eBroadcast_IconState, &icon);
	}
#else
#error __dest_os
#endif
	GetTable()->SetMbox(anMbox);

	// Enable disable it based on mbox
	if (anMbox)
	{
		// Enable it
		FRAMEWORK_ENABLE_WINDOW(this);
		SyncQuickSearch();
	}
	else
	{
		// Disable it
		FRAMEWORK_DISABLE_WINDOW(this);
		EnableQuickSearch(false);
	}

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

// Recycle the mbox
bool CMailboxInfoView::Recycle(CMbox* anMbox, bool is_search)
{
	// If substitutes in use and the current one is locked, then
	// treat this as a new subtitution
	if (mUseSubstitute && IsSubstituteLocked(mSubstituteIndex))
	{
		if (mUseDynamicTab)
		{
			// If its already a substitute, then switch to it in the normal way
			if (IsSubstitute(anMbox))
				return Substitute(anMbox, true, is_search);
			
			// If no current dynamic item or there are zero/one - create a new tab
			else if (!HasDynamicItem() || (mSubsList.size() < 2))
				return Substitute(anMbox, true, is_search);

			else
			{
				// Make sure dynamic item is selected
				SetSubstitute(GetDynamicIndex());
				
				// Now recyle the current (dynamic) tab to new mailbox
				return RecycleSubstitute(anMbox, is_search);
			}
		}
		else
			// This creates a new tab
			return Substitute(anMbox, true, is_search);
	}

	// These replace the current tab
	if (mUseSubstitute)
		return RecycleSubstitute(anMbox, is_search);
	else
		return RecycleClose(anMbox, is_search);

}

// Set the mbox
bool CMailboxInfoView::RecycleSubstitute(CMbox* newmbox, bool is_search)
{
	// Try to close existing mailbox on view
	// Look for dangling messages then process close actions
	if (!TestCloseOne(mSubstituteIndex))

		// No recycle
		return false;

	CMbox* oldmbox = GetMbox();

	// Is new one already a substitute
	if (IsSubstitute(newmbox))
	{
		// Make the new one the current one
		SetSubstitute(GetSubstituteIndex(newmbox));
		
		try
		{
			// Close the old one and remove substitute
			DoCloseMbox(oldmbox);
			CloseSubstitute(oldmbox);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Always remove the old tab
			CloseSubstitute(oldmbox);

			// Have to throw out of here
			CLOG_LOGRETHROW;
			throw;
		}
	}
	else
	{
		try
		{
			// Close the old one
			DoCloseMbox(oldmbox);
			
			// Open the new one
			DoOpenMbox(newmbox, is_search);
			
			// Replace the old substitute item
			ChangeSubstitute(oldmbox, newmbox);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

			// Always remove the old tab
			CloseSubstitute(oldmbox);

			// Have to throw out of here
			CLOG_LOGRETHROW;
			throw;
		}
	}

	return true;

}

// Set the mbox
bool CMailboxInfoView::RecycleClose(CMbox* anMbox, bool is_search)
{
	// Try to close existing mailbox on view
	// Look for dangling messages then process close actions
	if (!TestCloseOne(0))

		// No recycle
		return false;

	// In 3-pane, the close operation will set focus to server pane, but we don't want
	// that as we want to keep focus on this pane. So we temporarily set the mServerView
	// value to NULL to prevent that
	StValueChanger<CServerView*> _temp_change(mServerView, NULL);

	CMbox* mbox = GetMbox();

	// Do mailbox close
	DoCloseMbox();

	// Do mailbox open
	DoOpenMbox(anMbox, is_search);
		
	return true;

}

// Set the mbox
bool CMailboxInfoView::Substitute(CMbox* anMbox, bool select, bool is_search)
{
	// Stop drawing until done
	StNoRedraw _nodraw(this);

	CMbox* old_mbox = GetMbox();

	if (old_mbox && select)
	{
		// Force current mailbox's state to be saved
		SaveState();

		// Add the subs items to the current list
		SaveSubstitute(old_mbox);

		// Set status
		SetClosed();
	}

	// Must have at least one selected
	if (!old_mbox)
		select = true;

	// Protect against failure that could leave this window in indeterminate state
	try
	{
		// Must open the new mailbox (NB May already have been opened by search engine)
		// but not if its already a substitute mailbox as we already have it open
		// 20-Feb-2000 CD: added check for open mailbox to prevent connection leak with searches
		// 08-Sep-2001 CD: removed check for open mailbox as search engine now handles mailbox
		// 				   connections better. We also need to allow a mailbox window to open a mailbox
		//				   already being viewed in 3-pane window. Also check whether it was open prior
		//				   to opening it again so we can determine whether filter action is required
		bool was_open = false;
		if (!IsSubstitute(anMbox))
		{
			was_open = anMbox->OpenIfOpen();
			if (!was_open)
				anMbox->Open();

			// Now force mailbox into search state
			if (is_search)
				anMbox->OpenSearch();
		}
		else
			was_open = true;

		// Reset status changed by DoCloseMbox
		// Must do this after opening mailbox so redisplay is blocked while messages are being recached
		SetOpen();

		// Init it if not already a substitute and being selected
		if (select)
		{
			if (IsSubstitute(anMbox))
			{
				// Reset to existing mbox
				ResetMbox(anMbox);

				// Now force transfer of search results into view if anything found
				if (is_search)
					ResetSearch();
			}
			else
				// Now set new mbox
				SetMbox(anMbox, is_search);

			// Need to make sure cache increment is reset as window may not resize and force it to be
			GetInfoTable()->UpdateCacheIncrement();
		}		

		// Do filter action if not previously open
		if (!was_open)
			CPreferences::sPrefs->GetFilterManager()->OpenMailbox(anMbox);
		
		// Now force display update from substitute info
		OpenSubstitute(anMbox, select);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Got here because mbox failed to open
		if (anMbox->IsOpen())
		{
			// Forcibly close it
			anMbox->CloseSilent(true);
			
			// Change window status to indicate close has been done
			SetClosed();
		}

		// Set view back to previous one and set state back to open
		ResetMbox(old_mbox);
		if (old_mbox)
		{
			OpenSubstitute(old_mbox, true);
			SetOpen();
		}

		// Have to throw out of here
		CLOG_LOGRETHROW;
		throw;
	}

	return true;

}

void CMailboxInfoView::SetUseSubstitute(bool subs)
{
	// Must close any existing ones first
	if (!subs && mUseSubstitute)
	{
		if (TestCloseAll())
			DoCloseAll();
		else
			// Do not allow change
			return;
	}
		
	// Do inherited
	CMailboxView::SetUseSubstitute(subs);

	// Check change in state
	if (mUseSubstitute)
	{
		// Must make current items an active tab first
		if (GetMbox())
			OpenSubstitute(GetMbox(), false);
	}
}

// Do search of mailbox
void CMailboxInfoView::SearchMailbox(CSearchItem* spec)
{
	GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec, false);
}

// Mailbox search has changed
void CMailboxInfoView::ResetSearch()
{
	// Must remove any selected cells since mailbox message list
	// has already been changed!
	GetTable()->UnselectAllCells();

	GetInfoTable()->ResetSearch();
	GetInfoTable()->ResetTable();
}

// Mailbox was renamed
void CMailboxInfoView::RenamedMailbox(const CMbox* mbox)
{
	// Only if mailbox is part of this view
	if (!ContainsMailbox(mbox))
		return;

	if (Is3Pane())
	{
		// Rename the tab containg this mailbox
		RenamedSubstitute(GetSubstituteIndex(mbox));

		// Broadcast change to listeners - this will uypdate the mailbox title in the 3-pane banner
		Broadcast_Message(eBroadcast_ViewChanged, this);
	}
	else
	{
		// Change window title
		cdstring title(mbox->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1));
		if (mbox->IsReadOnly())
		{
			cdstring readonly;
			readonly.FromResource("UI::Mailbox::ReadOnlyMailbox");
			title += readonly;
		}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		MyCFString temp(title, kCFStringEncodingUTF8);
		GetMailboxWindow()->SetCFDescriptor(temp);
		CWindowsMenu::RenamedWindow();
#elif __dest_os == __win32_os
		GetMailboxWindow()->GetDocument()->SetTitle(title.win_str());
#elif __dest_os == __linux_os
		GetWindow()->SetTitle(title.c_str());
#else
#error __dest_os
#endif
	}
}

// Select next new message
void CMailboxInfoView::SelectNextNew(bool backwards)
{
	// Pass down to table
	GetInfoTable()->SelectNextNew(backwards);
}

