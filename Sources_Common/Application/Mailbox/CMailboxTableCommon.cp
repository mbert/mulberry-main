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


// Source for CMailboxTable class

#include "CMailboxTable.h"

#include "CAddressBookManager.h"
#include "CActionManager.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CMailControl.h"
#include "CMailboxView.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif
#include "CTaskClasses.h"
#include "CUserAction.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#elif __dest_os == __win32_os
#include "StValueChanger.h"
#elif __dest_os == __linux_os
#include "StValueChanger.h"
#include <JTableSelection.h>
#endif

#pragma mark ____________________________StMailboxTableSelection

StMailboxTableSelection::StMailboxTableSelection(CMailboxTable* aTable, bool scroll)
	: StTableAction(aTable), mUpdateDefer(aTable)
{
	// Cache table and its mailbox
	mTemp = aTable->GetMbox();
	mScroll = scroll;

	// Preserve current selection
	mPreserved = aTable->PreserveSelection();
}

StMailboxTableSelection::~StMailboxTableSelection()
{
	// Check that temp mailbox object is still open
	if (mTemp && mTemp->IsOpen())
	{
		// Reset the selection
		if ((mTable != NULL) && mPreserved)
		{
			static_cast<CMailboxTable*>(mTable)->ResetSelection(mScroll);
		}
	}
}

#pragma mark ____________________________Selection tests

// Test for selected message deleted
std::pair<bool, bool> CMailboxTable::TestSelectionFlag(TableIndexT row, NMessage::EFlags param)
{
	// Get the message
	CMessage* theMsg = NULL;
	try
	{
		if (GetMbox())
			theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		theMsg = NULL;
	}

	// This is deleted
	return std::pair<bool, bool>(theMsg && theMsg->HasFlag(param), theMsg && !theMsg->IsFake());
}

// Test for an outgoing message
std::pair<bool, bool> CMailboxTable::TestSelectionSmart(TableIndexT row)
{
	// Get the message
	CMessage* theMsg = NULL;
	try
	{
		if (GetMbox())
			theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		theMsg = NULL;
	}

	return std::pair<bool, bool>(theMsg && theMsg->IsSmartFrom(), theMsg && !theMsg->IsFake());
}

// Test for a fake message
bool CMailboxTable::TestSelectionFake(TableIndexT row)
{
	// Get the message
	CMessage* theMsg = NULL;
	try
	{
		if (GetMbox())
			theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		theMsg = NULL;
	}

	return theMsg && theMsg->IsFake();
}

#pragma mark ____________________________Init

// Set the mbox
void CMailboxTable::SetMbox(CMbox* anMbox)
{
	mMbox = anMbox;
	mMboxError = false;

	// mbox maybe NULL after closing
	if (GetMbox())
	{
		// No do explicit ItemIsAcceptable test as we always accept mailbox drops
		// but don't accept message drops if mailbox is read-only
#if 0
		// Acceptance of drags depends on mailbox read-only state
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		SetDDReadOnly(GetMbox()->IsReadOnly());
#elif __dest_os == __win32_os || __dest_os == __linux_os
		SetReadOnly(GetMbox()->IsReadOnly());
#else
#error Must set drag and drop read-only status here so that d&d to this window is rejected if mailbox is read-only
#endif
#endif

		// Do not allow drawing to force messages into cache until after initial pos is set
		mListChanging = true;
		
		// Must not preserve selection because of mbox change
		UnselectAllCells();
		ResetTable();
	}
	else
	{
		// Wipe all data from the table
		ClearTable();
	}
}

// Set the mbox
void CMailboxTable::ForceClose()
{
	mMbox = NULL;
	mMboxError = true;
}

// Set initial position
void CMailboxTable::InitPos(void)
{
	// Allow drawing to bring items into cache
	mListChanging = false;
	FRAMEWORK_REFRESH_WINDOW(this)
}

// Preview the selected message or clear preview
void CMailboxTable::PreviewMessage(bool clear)
{
	// Ignore if no preview pane or not in use
	if (!mTableView->GetPreview() || !mTableView->GetUsePreview())
		return;

	if (!GetMbox())
	{
		// do immediate update of preview
		mTableView->GetPreview()->SetMessage(NULL);
		return;
	}

	CMessage* msg = NULL;
	bool changed = clear;
	ulvector sels;
	if (!clear)
	{
		// Valid preview message if only one selected
		GetSelectedRows(sels);
		msg = ((sels.size() == 1) ? GetMbox()->GetMessage(sels.front(), true) : NULL);
		
		// Check whether already cached
		if (msg && !msg->IsFullyCached())
		{
			// Do not allow caching while processing another network command
			if (GetMbox()->GetMsgProtocol() && !GetMbox()->GetMsgProtocol()->_get_mutex().is_locked())
			{
				// This will cause caching

				StValueChanger<bool> _change(mListChanging, true);

				{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					StValueChanger<ETriState> _change(mVisible, triState_Off);
#endif

					{
						// Preserve selection without scroll
						StMailboxTableSelection preserve_selection(this, false);

						msg = GetMbox()->GetCacheMessage(sels.front(), true);
					}

					UpdateItems();
				}

				// If sorting by anything other than by message number, must do entire refresh
				if (GetMbox()->GetSortBy() != cSortMessageNumber)
					FRAMEWORK_REFRESH_WINDOW(this);
			}

		}

		// Get UID for new message
		unsigned long new_uid = (msg ? msg->GetUID() : 0);

		// Don't display if its deleted
		if (msg && msg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		{
			msg = NULL;
			new_uid = 0;
		}

		// Check message size first
		if (msg && (new_uid != mPreviewUID) && !CMailControl::CheckSizeWarning(msg))
		{
			// If user does not want to diplay it, empty out the preview
			msg = NULL;
			new_uid = 0;
		}

		// Check whether different (also check to see whether preview currently empty) and indicate a change
		if ((new_uid != mPreviewUID) || msg && !mTableView->GetPreview()->GetMessage())
		{
			// Remember the one being previewed
			mPreviewUID = new_uid;
			
			// Set flag to force update
			changed = true;
		}
	}

	// Only do preview if there has been a change
	if (changed)
	{
		// Always clear out message preview immediately, otherwise
		// do preview at idle time to prevent re-entrant network calls
		if (msg)
		{
			// Give it to preview (if its NULL the preview will be cleared)
			CMessagePreviewTask* task = new CMessagePreviewTask(mTableView->GetPreview(), msg);
			task->Go();
		}
		else
			// do immediate update of preview
			mTableView->GetPreview()->SetMessage(msg);
	}
}

// Preview the message or clear preview
void CMailboxTable::PreviewMessageUID(unsigned long uid, const CMessageView::SMessageViewState* state)
{
	// Ignore if no preview pane or not in use
	if (!mTableView->GetPreview() || !mTableView->GetUsePreview())
		return;

	if (!GetMbox())
	{
		// do immediate update of preview
		mTableView->GetPreview()->SetMessage(NULL);
		return;
	}

	// Get the message with this uid
	CMessage* msg = GetMbox()->GetMessageUID(uid);

	// Always clear out message preview immediately, otherwise
	// do preview at idle time to prevent re-entrant network calls
	if (msg)
	{
		// Reset current preview UID to one about t be previewed
		mPreviewUID = uid;

		// Check for state restore
		if (state)
		{
			// Give it to preview with state restore
			CMessagePreviewRestoreTask* task = new CMessagePreviewRestoreTask(mTableView->GetPreview(), *state, msg);
			task->Go();
		}
		else
		{
			// Give it to preview
			CMessagePreviewTask* task = new CMessagePreviewTask(mTableView->GetPreview(), msg);
			task->Go();
		}
	}
	else
	{
		// Make sure this is zeroed out
		mPreviewUID = 0;

		// Do immediate update of preview
		mTableView->GetPreview()->SetMessage(msg);
	}
}

#pragma mark ____________________________Commands

// Single click on a message
void CMailboxTable::DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSingleClick() &&
		(preview.GetSingleClickModifiers() == mods))
		PreviewMessage();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSingleClick() &&
		(fullview.GetSingleClickModifiers() == mods))
		DoFileOpenMessage(mods.Get(CKeyModifiers::eAlt));
}

// Double click on a message
void CMailboxTable::DoDoubleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetDoubleClick() &&
		(preview.GetDoubleClickModifiers() == mods))
		PreviewMessage();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetDoubleClick() &&
		(fullview.GetDoubleClickModifiers() == mods))
		DoFileOpenMessage(mods.Get(CKeyModifiers::eAlt));
}

// Spacebar scroll
void CMailboxTable::HandleSpacebar(bool shift_key)
{
	// Scroll preview if available
	if (!mTableView->GetUsePreview() || !mTableView->GetPreview()->SpacebarScroll(shift_key))
	{
		// Preview at bottom so go to next/previous message
		SelectionNudge(shift_key);
	}
}

// Display a mail message
void CMailboxTable::DoFileOpenMessage(bool option_key)
{
	// Display each selected message
	DoToSelection1((DoToSelection1PP) &CMailboxTable::OpenMailMessage, &option_key);
}

// Display a specified mail message
bool CMailboxTable::OpenMailMessage(TableIndexT row, bool* option_key)
{
	if (!GetMbox())
		return false;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't display if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		::SysBeep(1);
#elif __dest_os == __win32_os
		::MessageBeep(-1);
#elif __dest_os == __linux_os
		::MessageBeep(-1);
#else
#error __dest_os
#endif
		return false;
	}

	// If draft do send again
	if (theMsg->IsDraft())
	{
		// Check message size first
		if (!CMailControl::CheckSizeWarning(theMsg))
			return false;

		// Create the letter window and give it the message
		CActionManager::SendAgainMessage(theMsg);
	}
	else
	{
		// Does window already exist?
		CMessageWindow*	theWindow = CMessageWindow::FindWindow(theMsg);

		// Check for window re-use
		{
			cdmutexprotect<CMessageWindow::CMessageWindowList>::lock _lock(CMessageWindow::sMsgWindows);
			if ((*option_key ^ CPreferences::sPrefs->mOpenReuse.GetValue()) && CMessageWindow::sMsgWindows->size())
			{
				// Use top window
				theWindow = CMessageWindow::sMsgWindows->front();
				theWindow->SetMessage(theMsg);
			}
		}

		if (theWindow)
		{
			// Found existing window so select and return - no refesh required
			FRAMEWORK_WINDOW_TO_TOP(theWindow)
			return false;
		}

		// Check message size first
		if (!CMailControl::CheckSizeWarning(theMsg))
			return false;

		CMessageWindow* newWindow = NULL;
		try
		{
			// Create the message window
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
			newWindow->SetMessage(theMsg);
			newWindow->Show();
			newWindow->PostSetMessage();		// Make sure visible processing occurs
#elif __dest_os == __win32_os
			newWindow = CMessageWindow::ManualCreate();
			newWindow->SetMessage(theMsg);
			newWindow->GetParentFrame()->ShowWindow(SW_SHOW);
			//newWindow->GetText()->UpdateMargins();
			//newWindow->PostSetMessage();
#elif __dest_os == __linux_os
			newWindow = CMessageWindow::ManualCreate();
			newWindow->SetMessage(theMsg);
			newWindow->GetWindow()->Show();
#else
#error __dest_os
#endif
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Only delete if it still exists
			if (CMessageWindow::FindWindow(theMsg))
				FRAMEWORK_DELETE_WINDOW(newWindow)

			// Should throw out of here in case abort and mbox now destroyed
			CLOG_LOGRETHROW;
			throw;
		}

		// Force refresh of row only
		RefreshRow(row);
	}

	return true;
}

// Save a mail message
void CMailboxTable::OnFileSave(void)
{
	if (!GetMbox())
		return;

	bool multi_msgs = true;

	// Handle multiple selection
	if (mIsSelectionValid && !IsSingleSelection())
	{
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::MessageMulti",
																		"ErrorDialog::Btn::MessageSingle",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::MultiSave", 3);
		if (result == CErrorDialog::eBtn1)
			multi_msgs = true;
		if (result == CErrorDialog::eBtn2)
			multi_msgs = false;
		if (result == CErrorDialog::eBtn3)
			return;
	}

	BeginMboxUpdate();

	if (multi_msgs)
		// Save each selected message
		DoToSelection((DoToSelectionPP) &CMailboxTable::SaveMailMessage);
	else
	{
		CMessageList* msgs = NULL;
		CMessageWindow* newWindow = NULL;
		CMbox* temp = GetMbox();					// Allow access to mbox if this is deleted

		try
		{
			msgs = new CMessageList;
			msgs->SetOwnership(false);

			// Add selection to list
			DoToSelection1((DoToSelection1PP) &CMailboxTable::AddSelectionToList, msgs);

			// Check message size first
			for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			{
				if (!CMailControl::CheckSizeWarning(*iter))
				{
					delete msgs;
					EndMboxUpdate(false);
					return;
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
			if (CMessageWindow::FindWindow(NULL))
				FRAMEWORK_DELETE_WINDOW(newWindow)
			delete msgs;

			// Only do if this is not deleted
			if (temp->IsOpen())
				EndMboxUpdate(false);

			// Should throw out of here in case abort and mbox now destroyed
			CLOG_LOGRETHROW;
			throw;
		}
	}

	EndMboxUpdate();
}

// Save a specified mail message
bool CMailboxTable::SaveMailMessage(TableIndexT row)
{
	if (!GetMbox())
		return false;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't save if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		::SysBeep(1);
#elif __dest_os == __win32_os || __dest_os == __linux_os
		::MessageBeep(-1);
#else
#error __dest_os
#endif
		return false;
	}

	// Does window already exist?
	CMessageWindow*	theWindow = CMessageWindow::FindWindow(theMsg);
	if (theWindow)
	{
		// Found existing window so send save command to it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		theWindow->ObeyCommand(cmd_Save, NULL);
#elif __dest_os == __win32_os
		theWindow->GetDocument()->DoFileSave();
#elif __dest_os == __linux_os
		theWindow->DoSaveAs();
#else
#error __dest_os
#endif
		return false;
	}

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CMessageWindow* newWindow = NULL;
	try
	{
		// Create the message window and send save command to it
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		newWindow = (CMessageWindow*) CMessageWindow::CreateWindow(paneid_MessageWindow, CMulberryApp::sApp);
		newWindow->SetMessage(theMsg);
		newWindow->ObeyCommand(cmd_Save, NULL);
#elif __dest_os == __win32_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessage(theMsg);
		newWindow->GetDocument()->DoFileSave();
#elif __dest_os == __linux_os
		newWindow = CMessageWindow::ManualCreate(true);
		newWindow->SetMessage(theMsg);
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
		if (CMessageWindow::FindWindow(theMsg))
			FRAMEWORK_DELETE_WINDOW(newWindow)

		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}

	return false;
}

// Delete or undelete selected mail messages
void CMailboxTable::DoFlagMailMessage(NMessage::EFlags flags, bool set)
{
	if (!GetMbox())
		return;

	// Do flag test
	bool test = set ? !TestSelectionIgnore1And(&CMailboxTable::TestSelectionFlag, flags) :
						!TestSelectionIgnore1Or(&CMailboxTable::TestSelectionFlag, flags);

	// Make into std::vector<unsigned long>
	ulvector nums;
	GetSelectedRows(nums);

	{
		// Preserve selection
		StMailboxTableSelection preserve_selection(this, false);

		BeginMboxUpdate();
		CMbox* temp = GetMbox();			// Allow use of mbox after this is deleted
		try
		{
			// Change flag
			GetMbox()->SetFlagMessage(nums, false, flags, test, true);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Only do if not already deleted
			if (temp->IsOpen())
				EndMboxUpdate(false);

			// Should throw out of here in case abort and mbox now destroyed
			CLOG_LOGRETHROW;
			throw;
		}

		EndMboxUpdate();
	}

	// Refresh (remember to redisplay if sorting by flags)
	if (GetMbox()->GetSortBy() != cSortMessageFlags)
	{
		// Check for single selection delete
		if (test && (flags & NMessage::eDeleted))
			SelectionNudge();
	}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	else
		Refresh();
#endif

	// For undelete, signal selection change to force preview on if it was off
	if (!test && (flags & NMessage::eDeleted))
		SelectionChanged();
}

// Expunge the mailbox
bool CMailboxTable::DoMailboxExpunge(bool closing, CMbox* substitute)
{
	bool expunge_done = false;

	// Mailbox is either the default or the substitute
	CMbox* mbox = substitute ? substitute : GetMbox();

	// If nothing to expunge return
	if (!mbox || !mbox->AnyDeleted() || mbox->IsReadOnly())
		return true;

	try
	{
		bool do_expunge = true;

		// If warning required display dialog
		if (CPreferences::sPrefs->warnOnExpunge.GetValue())
		{
			cdstring number = mbox->GetNumberDeleted();

			if (!closing)
			{
				bool dontshow = false;
				CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																				"ErrorDialog::Btn::Expunge",
																				"ErrorDialog::Btn::Cancel",
																				NULL,
																				NULL,
																				"ErrorDialog::Text::ExpungeManual", 2,
																				number, mbox->GetAccountName(), NULL, &dontshow);
				if (dontshow)
					CPreferences::sPrefs->warnOnExpunge.SetValue(false);
				if (result == CErrorDialog::eBtn1)
					do_expunge = true;
				if (result == CErrorDialog::eBtn2)
					do_expunge = false;
			}
			else
			{
				bool dontshow = false;
				CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																				"ErrorDialog::Btn::Expunge",
																				"ErrorDialog::Btn::Cancel",
																				"ErrorDialog::Btn::CloseWithoutExpunge",
																				NULL,
																				"ErrorDialog::Text::ExpungeOnClose", 2,
																				number, mbox->GetAccountName(), NULL, &dontshow);
				if (dontshow)
					CPreferences::sPrefs->warnOnExpunge.SetValue(false);
				if (result == CErrorDialog::eBtn1)
					do_expunge = true;
				if (result == CErrorDialog::eBtn2)
					do_expunge = false;
				if (result == CErrorDialog::eBtn3)
				{
					// Pretend that expunge occured without actually doing it
					do_expunge = false;
					expunge_done = true;
				}
			}
		}

		if (do_expunge)
		{
			// Remove any message windows belonging to deleted messages in this mbox

			if (closing || (mbox != GetMbox()))
				// Expunge mailbox
				mbox->Expunge(closing);
			else
			{
				// Preserve selection while expunging
				StMailboxTableSelection preserve_selection(this, false);
				StMailboxTablePostponeUpdate postpone(this);

				mbox->Expunge(closing);
			}

			expunge_done = true;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Handled by CMboxProtocol

		// Need to throw out of here in case window is deleted
		CLOG_LOGRETHROW;
		throw;
	}

	return expunge_done;
}

// Copy the message
void CMailboxTable::DoMessageCopy(CMbox* copy_mbox, bool option_key, bool force_delete)
{
	if (!GetMbox())
		return;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	// Only if mbox to copy to
	if (!copy_mbox)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::Message::MissingCopyTo");
		return;
	}
#endif

	try
	{
		// Get delete state for deletion if required
		bool delete_test = !mTestSelectionAndDeleted;

		// Get list of selected rows
		ulvector nums;
		GetSelectedRows(nums);

		// Must use unsorted numbers between two IMAP commands
		ulvector actual_nums;
		GetMbox()->MapSorted(actual_nums, nums, true);

		// Need to preserve message selection as Copy operation may result in table reset
		ulvector preserve;	// Preallocate in case of large selection
		preserve.reserve(actual_nums.size());
		for(ulvector::const_iterator iter = actual_nums.begin(); iter != actual_nums.end(); iter++)
			preserve.push_back(reinterpret_cast<unsigned long>(GetMbox()->GetMessage(*iter)));

		// Reset table reset flag
		mResetTable = false;

		// Do copy (NB this may change selection if a new message arrives)
		ulmap temp;
		GetMbox()->CopyMessage(actual_nums, false, copy_mbox, temp, false);

		// Reset any open copied to window
		CMailboxView* aView = CMailboxView::FindView(copy_mbox);
		if (aView)
			aView->ResetTable();

		// If copy OK then delete selection if required and not all already deleted
		if (((CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key) && delete_test || force_delete) &&
			GetMbox()->HasAllowedFlag(NMessage::eDeleted))
		{
			// Look for possible table reset and redo message number array
			if (mResetTable)
			{
				// Redo message numbers for delete operation
				actual_nums.clear();
				for(ulvector::const_iterator iter = preserve.begin(); iter != preserve.end(); iter++)
				{
					// See if message still exists and if so where
					unsigned long index = GetMbox()->GetMessageIndex(reinterpret_cast<CMessage*>(*iter));
					if (index)
						actual_nums.push_back(index);	
				}
			}
			
			// Set deleted flag on chosen messages
			GetMbox()->SetFlagMessage(actual_nums, false, NMessage::eDeleted, true, false);
			
			// Move selection to next item
			SelectionNudge();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);


		// Should throw out of here in case abort and mbox now destroyed
		CLOG_LOGRETHROW;
		throw;
	}
}

#pragma mark ____________________________Speech

#ifdef __use_speech
// Speak selected messages
void CMailboxTable::DoSpeakMessage(void)
{
	bool recent = false;
	DoToSelection1((DoToSelection1PP) &CMailboxTable::SpeakMessage, &recent);
}

// Speak recent messages
void CMailboxTable::DoSpeakRecentMessages(void)
{
	bool recent = true;
	DoToRows1((DoToSelection1PP) &CMailboxTable::SpeakMessage, &recent);
}

// Speak selected message
bool CMailboxTable::SpeakMessage(TableIndexT row, bool* recent)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Must be recent
	if (*recent && !theMsg->IsCheckRecent())
		return false;

	// Don't speak if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	// Speak it (adds to queue for speaking at idle time)
	CSpeechSynthesis::SpeakMessage(theMsg, NULL, false);

	return true;
}
#endif

#pragma mark ____________________________Address Capture

// Capture address from selected messages
void CMailboxTable::DoCaptureAddress(void)
{
	DoToSelection((DoToSelectionPP) &CMailboxTable::CaptureAddress);
}

// Speak selected message
bool CMailboxTable::CaptureAddress(TableIndexT row)
{
	if (!GetMbox() || !CAddressBookManager::sAddressBookManager)
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Get it captured
	CAddressBookManager::sAddressBookManager->CaptureAddress(*theMsg);

	return true;
}

#pragma mark ____________________________Substitution

#pragma mark ____________________________Selection preservation

// Scroll to ensure new messages are in view
void CMailboxTable::ScrollForNewMessages()
{
	// Ignore if disabled by prefs
	if (!CPreferences::sPrefs->mScrollForUnseen.GetValue())
		return;

	// Don't do if scroll not needed
		// Get number of visible rows in table
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	unsigned long num_cells = mFrameSize.height / GetRowHeight(1) + 1;
#elif __dest_os == __win32_os
	CRect frame;
	GetClientRect(frame);
	unsigned long num_cells = frame.Height() / GetRowHeight(1) + 1;
#elif __dest_os == __linux_os
	unsigned long num_cells = GetApertureHeight() / GetRowHeight(1) + 1;
#else
#error __dest_os
#endif

	if (num_cells >= mRows)
		return;

	// Policy only scroll if window already scrolled to minimum/maximum (depending on sort order)
	// Do not scroll if doing so would move unseen message outside of current view
	// Do not scroll if doing so would move selected message outside of current view
	
	// Determine current sort order
	bool ascending = (GetMbox()->GetShowBy() == cShowMessageAscending);
	
	// Find first row above/below current view
	TableIndexT visible_top = GetFirstFullyVisibleCell().row;
	TableIndexT visible_bottom = GetLastFullyVisibleCell().row;
	
	// If already max scrolled, ignore
	if (ascending && (visible_bottom == mRows) ||
		!ascending && (visible_top == 1))
	{
		return;
	}
	
	// Look at each message and determine the selected and unseen message that will 'pin' the scroll
	TableIndexT pin_select = 0;
	TableIndexT pin_unseen = 0;
	
	for(TableIndexT row = (ascending ? visible_top : visible_bottom); row != (ascending ? visible_bottom : visible_top); ascending ? row++ : row--)
	{
		// Check selection
		if ((pin_select == 0) && IsRowSelected(row))
			pin_select = row;
		
		// Check new state
		if (pin_unseen == 0)
		{
			const CMessage* msg = GetMbox()->GetMessage(row, true);
			if ((msg != NULL) && msg->IsUnseen())
			{
				pin_unseen = row;
			}
		}
	}

	// Always pin the last row if none matched
	if (pin_select == 0)
		pin_select = (ascending ? visible_bottom : visible_top);
	if (pin_unseen == 0)
		pin_unseen = (ascending ? visible_bottom : visible_top);

	// Check whether pin item is at limit
	TableIndexT can_move_select = (ascending ? pin_select - visible_top : visible_bottom - pin_select);
	TableIndexT can_move_unseen = (ascending ? pin_unseen - visible_top : visible_bottom - pin_unseen);
	TableIndexT can_move = std::min(can_move_select, can_move_unseen);
	
	// If no move allowed then exit
	if (can_move == 0)
		return;
	
	// Clip movement to max/min rows
	if (ascending && (visible_bottom + can_move > mRows))
		can_move = mRows - visible_bottom;
	else if (!ascending && (visible_top < can_move + 1))
		can_move = visible_top - 1;
	

	// Now see how many unseen there are below current pos up to can move amount
	TableIndexT move_by = 0;
	for(TableIndexT row = (ascending ? visible_bottom : visible_top) + 1; ascending ? (row <= visible_bottom + can_move) : (row >= visible_top - can_move); ascending ? row++ : row--)
	{
		// Check new state
		const CMessage* msg = GetMbox()->GetMessage(row, true);
		if ((msg == NULL) || !msg->IsUnseen())
		{
			break;
		}
		
		move_by++;
	}
	
	// Now do the scroll
	if (move_by != 0)
	{
		ScrollCellIntoFrame(STableCell(ascending ? visible_bottom + move_by : visible_top - move_by, 1));
	}
}

// Preserver current selection
bool CMailboxTable::PreserveSelection(void)
{
	if (!mSelectionPreserved)
	{
		DoToSelection((DoToSelectionPP) &CMailboxTable::PreserveMessage);
		mSelectionPreserved = true;
		return true;
	}
	else
		return false;
}

// Preserve messages
bool CMailboxTable::PreserveMessage(TableIndexT row)
{
	if (!GetMbox())
		return true;

	// Important: message list may already have shrunk - protect against failure

	try
	{
		// Allow get message/envelope to fail during caching process
		const CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
		mPreserveMsgs.insert(reinterpret_cast<unsigned long>(theMsg));
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Fail silently
	}

	return true;
}

// Preserver current selection
void CMailboxTable::UnpreserveSelection(void)
{
	if (mSelectionPreserved)
	{
		mPreserveMsgs.clear();
		mSelectionPreserved = false;
	}
}

// Reset selection to what it was
void CMailboxTable::ResetSelection(bool scroll)
{
	if (!GetMbox())
		return;

	// Prevent selection change notifications while doing updates
	StDeferSelectionChanged noSelChange(this);
	
	// Iterate over messages and select those that are in the preserved list
	if (!mPreserveMsgs.empty())
	{
		unsigned long first_row = 0;
		for(unsigned long i = 1; i <= GetMbox()->GetNumberMessages(); i++)
		{
			try
			{
				const CMessage* msg = GetMbox()->GetMessage(i, true);
				ulset::const_iterator iter = mPreserveMsgs.find(reinterpret_cast<unsigned long>(msg));
				if (iter != mPreserveMsgs.end())
				{
					// Make sure row is valid as there may be more messages in the mailbox than rows at this time
					if (i <= GetItemCount())
					{
						SelectRow(i - TABLE_ROW_ADJUST);
						if (first_row == 0)
							first_row = i;
					}
				}
				else
					UnselectRow(i - TABLE_ROW_ADJUST);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
			
		}

		mPreserveMsgs.clear();

		// Scroll to first cell found - put halfway up table if no caching will occur
		if (scroll && first_row)
			ScrollToRow(first_row - TABLE_ROW_ADJUST, false, false,
						GetMbox()->ValidSort(GetMbox()->GetSortBy()) ? eScroll_Center : eScroll_Bottom);
	}

	mSelectionPreserved = false;
}

// Move selection on to next message
void CMailboxTable::SelectionNudge(bool previous)
{
	// Check for single selection
	if (IsSingleSelection())
	{
		// Prevent selection change notifications while doing multiple nudges
		StDeferSelectionChanged noSelChange(this);

		// Get selection
		ulvector nums;
		GetSelectedRows(nums);

		while(true)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Force draw update in case table scrolls during nudge
			// and any existing update region ends up wrong
			UpdatePort();
#endif

			// Advance to next cell
			// Take sort order and processing direction into account.
			bool decrease_row = ((GetMbox()->GetShowBy() == cShowMessageAscending) ^ CPreferences::sPrefs->mNextIsNewest.GetValue());

			// Invert behaviour if previous requested
			if (previous)
				decrease_row = !decrease_row;

			if (decrease_row)
			{
				if (nums[0] > 1)
				{
					NudgeSelection(-1, false);
					nums[0]--;
				}
			}
			else
			{
				if (nums[0] < GetItemCount())
				{
					NudgeSelection(1, false);
					nums[0]++;
				}
			}

			// If its reached the top or bottom, stop
			if ((nums[0] == 1) || (nums[0] == GetItemCount()))
				break;
			
			// Skip messages that are fakes or deleted
			const CMessage* theMsg = GetMbox()->GetMessage(nums[0], true);
			if (!theMsg || (!theMsg->IsDeleted() && !theMsg->IsFake()))
				break;
		}
	}
}

#pragma mark ____________________________Selection

// Add selected messages to list
bool CMailboxTable::AddSelectionToList(TableIndexT row, CMessageList* list)
{
	return AddMsgSelectionToList(row, list, true);
}

// Add selected messages to list
bool CMailboxTable::AddFullSelectionToList(TableIndexT row, CMessageList* list)
{
	return AddMsgSelectionToList(row, list, false);
}

// Add selected messages to list
bool CMailboxTable::AddFullSelectionWithFakesToList(TableIndexT row, CMessageList* list)
{
	return AddMsgSelectionToList(row, list, true, true);
}

// Add selected messages to list
bool CMailboxTable::AddMsgSelectionToList(TableIndexT row, CMessageList* list, bool only_cached, bool fakes)
{
	if (!GetMbox())
		return false;

	// Determine message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
	if (theMsg && (!only_cached || theMsg->IsFullyCached()) && (fakes || !theMsg->IsFake()))
	{
		list->push_back(theMsg);
		return true;
	}
	else
		return false;
}
