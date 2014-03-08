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


// Source for CMailboxInfoTable class

#include "CMailboxInfoTable.h"

#include "CActionManager.h"
#include "CAddress.h"
#include "CAddressList.h"
#include "CAdminLock.h"
#include "CCacheMessageDialog.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CDragIt.h"
#endif
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CIMAPCommon.h"
#if __dest_os != __linux_os
#include "CLetterDoc.h"
#endif
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailControl.h"
#include "CMailboxInfoView.h"
#include "CMailboxInfoWindow.h"
#include "CMailboxPropDialog.h"
#include "CMatchOptionsDialog.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CRulesWindow.h"
#if __dest_os == __win32_os
#include "CSDIFrame.h"
#endif
#include "CSearchEngine.h"
#include "CSearchWindow.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CServerBrowse.h"
#endif
#include "CSynchroniseDialog.h"
#include "CStreamFilter.h"
#include "CStringUtils.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#include <LBevelButton.h>
#elif __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#if __dest_os == __linux_os
#include <JTableSelection.h>
#include "CMulberryCommon.h" //For now for MessageBeep
#endif

// Consts

#pragma mark ____________________________Command Handlers

// Single click on a message
void CMailboxInfoTable::DoSingleClick(unsigned long row, unsigned long col, const CKeyModifiers& mods)
{
	if (!GetMbox())
		return;

	// Only for option clicks
	if (mods.Get(CKeyModifiers::eAlt))
	{
		// Determine which heading it is
		SColumnInfo col_info = mTableView->GetColumnInfo()[col - TABLE_START_INDEX];

		// Determine which heading it is
		CMatchItem::EMatchItem match_item = CMatchItem::eNone;
		switch (col_info.column_type)
		{
		case eMboxColumnTo:
			match_item = CMatchItem::eSelectedTo;
			break;
		
		case eMboxColumnFrom:
			match_item = CMatchItem::eSelectedFrom;
			break;
		
		case eMboxColumnSmart:
		{
			CMessage* theMsg = NULL;
			try
			{
				theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);
			}
			catch(...)
			{
			}

			// Check to see whether current item is smart or not
			if (theMsg && theMsg->IsSmartFrom())
			{
				// Use To when its smart from
				match_item = CMatchItem::eSelectedTo;
			}
			else
			{
				// Use From in all other cases
				match_item = CMatchItem::eSelectedFrom;
			}
			break;
		}
		
		case eMboxColumnReplyTo:
		case eMboxColumnSender:
		case eMboxColumnCc:
			break;
		
		case eMboxColumnSubject:
		case eMboxColumnThread:
			match_item = CMatchItem::eSelectedSubject;
			break;
		
		case eMboxColumnDateSent:
		case eMboxColumnDateReceived:
			match_item = CMatchItem::eSelectedDate;
			break;
		
		case eMboxColumnSize:
		case eMboxColumnFlags:
		case eMboxColumnNumber:
		case eMboxColumnAttachments:
		case eMboxColumnParts:
		case eMboxColumnMatch:
		case eMboxColumnDisconnected:
		default:
			break;
		
		}
		
		// Do match if item selected
		if (match_item != CMatchItem::eNone)
		{
			// Set the match value in the cached match state
			mMatch.SetSingleMatch(match_item);

			// Look to see if match wants selection
			std::auto_ptr<CMessageList> msgs;
			if (mMatch.NeedsSelection() && IsSelectionValid())
			{
				msgs.reset(new CMessageList);
				msgs->SetOwnership(false);

				// Add selection to list
				DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get());
			}

			// Reset the match criteria
			SetMatch(NMbox::eViewMode_ShowMatch, mMatch.ConstructSearch(msgs.get()), false, true);
		}
	}

	// Now do inherited
	CMailboxTable::DoSingleClick(row, col, mods);
}

// Create a new draft
void CMailboxInfoTable::DoNewLetter(bool option_key)
{
	// Determine context if required
	bool is_context = CPreferences::sPrefs->mContextTied.GetValue() ^ option_key;
	
	if (GetMbox() && is_context)
		// New draft with identity tied to this mailbox
		CActionManager::NewDraft(GetMbox());
	else
		// New draft with default identity
		CActionManager::NewDraft();
}

// Close via file menu
void CMailboxInfoTable::OnFileClose(void)
{
	if (!GetMbox())
		return;

	// Close tab/view if 3-pane
	if (mInfoTableView->Is3Pane())
	{
		// If close allowed do it
		if (mInfoTableView->IsOpen() && mInfoTableView->TestClose(GetMbox()) || mInfoTableView->IsClosing())
			// Close the view  - this will close the actual window at idle time
			mInfoTableView->DoClose(GetMbox());
	}
	else
	{
		// If close allowed do it
		if (mInfoTableView->IsOpen() && mInfoTableView->TestClose() || mInfoTableView->IsClosing())
			// Close the view  - this will close the actual window at idle time
			mInfoTableView->DoClose();
	}
}

// Should File:Close be active
bool CMailboxInfoTable::CanFileClose() const
{
	if (!GetMbox())
		return false;
	
	// If using tabs, only close if not lcoked
	if (mInfoTableView->GetUseSubstitute())
		return !mInfoTableView->IsSubstituteLocked(mInfoTableView->GetSubstituteIndex());
	else
		return true;
}

// Selected mail messages to be cleared (seen but not deleted)
void CMailboxInfoTable::SelectClearMessages(void)
{
	if (!GetMbox())
		return;

	{
		// Prevent button flashing
		StDeferSelectionChanged noSelChange(this);

		// Clear selection first
		UnselectAllCells();
	}

	NMessage::EFlags set_flag = NMessage::eSeen;
	NMessage::EFlags unset_flag = NMessage::eDeleted;
	ulvector rows;

	// Now do match (may force serach of mailbox if only some messages are cached)
	GetMbox()->MatchMessageFlags(set_flag, unset_flag, rows, false, true);

	// Now select them
	SelectByRow(rows);
}

// Selected mail messages with a specific flag
void CMailboxInfoTable::SelectByRow(const ulvector& rows)
{
	// Prevent button flashing
	StDeferSelectionChanged noSelChange(this);

	// Clear selection first
	UnselectAllCells();

	// Iterate over rows and select them
	for(ulvector::const_iterator iter = rows.begin(); iter != rows.end(); iter++)
	{
		SelectRow(*iter - TABLE_ROW_ADJUST);
	}
}

// Select next new message
void CMailboxInfoTable::SelectNextNew(bool shift)
{
	if (!GetMbox())
		return;

	// Prevent display flashing
	StDeferSelectionChanged noSelChange(this);

	// Always moving in ascending sequence order unless shift is down
	bool ascending_seq = !shift;

	// Get list of selected rows
	ulvector rows;
	GetSelectedRows(rows);
	unsigned long last_row = 0;
	if (rows.size())
		last_row = (ascending_seq ? rows.back() : rows.front());

	// Clear selection first
	UnselectAllCells();

	// Get last selected message and then find next one with flag set
	CMessage* msg = last_row ? GetMbox()->GetMessage(last_row, true) : NULL;
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
	unset_flag = static_cast<NMessage::EFlags>(unset_flag | NMessage::eDeleted);
	msg = GetMbox()->GetNextFlagMessage(msg, set_flag, unset_flag, true, ascending_seq);

	// If we didn't find one, and we're not doing rollover, cycle back to the first if available
	if (!msg && !CPreferences::sPrefs->mDoRollover.GetValue())
		msg = GetMbox()->GetNextFlagMessage(NULL, set_flag, unset_flag, true, ascending_seq);

	// If found select its row
	if (msg)
	{
		unsigned long row = GetMbox()->GetMessageIndex(msg, true);
		ScrollToRow(row - TABLE_ROW_ADJUST, true, true,
					GetMbox()->ValidSort(GetMbox()->GetSortBy()) ? eScroll_Center : eScroll_Bottom);
		UpdateState();
	}
	else if (CPreferences::sPrefs->mDoRollover.GetValue())
		// Try roll over if not found
		DoRollover();
}

// Cycle to next/previous tab
void CMailboxInfoTable::CycleTabs(bool forward)
{
	mTableView->CycleSubstitute(forward);
}

// Rollover to next unseen
void CMailboxInfoTable::DoRollover(bool ask)
{
	// Look for possible rollover
	CMbox* next_mbox = GetMbox() ? CMailAccountManager::sMailAccountManager->GetNewMailbox(GetMbox()) : NULL;

	// Ask user to roll-over?
	if (next_mbox)
	{
		if (ask && CPreferences::sPrefs->mRolloverWarn.GetValue())
		{
			bool dontshow = false;
			short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Mailbox::MailboxRollover", 0, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->mRolloverWarn.SetValue(false);
			if (answer == CErrorHandler::Cancel)
				return;
		}

		// Does window already exist?
		CMailboxWindow* theWindow = CMailboxWindow::FindWindow(next_mbox);

		if (theWindow)
		{
			// Try to close this one
			if (mTableView->TestClose())
				// Close it
				mTableView->DoClose();

			// Select next one
			FRAMEWORK_WINDOW_TO_TOP(theWindow)
		}
		else
			// Recycle this one
			mTableView->Recycle(next_mbox);
	}
	else
		CErrorHandler::PutStopAlertRsrc("Alerts::Mailbox::NoMailboxRollover");
}

// Get properties of mailbox
void CMailboxInfoTable::OnMailboxProperties(void)
{
	// Only if mailbox exists
	if (!GetMbox())
		return;

	try
	{
		CMboxList selected;
		selected.push_back(GetMbox());

		// Must not allow one failure to stop others
		try
		{
			GetMbox()->CheckMyRights();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}

		try
		{
			GetMbox()->CheckACLs();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}

		try
		{
			GetMbox()->CheckQuotas();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}

		// Create the dialog
		CMailboxPropDialog::PoseDialog(&selected);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Catch errors to prevent further processing
	}
}

// Punt unseen in the mailbox
void CMailboxInfoTable::OnMailboxPunt(void)
{
	if (!GetMbox())
		return;

	// Do warning if required
	if (CPreferences::sPrefs->mWarnPuntUnseen.GetValue())
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::PuntUnseen", 0, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mWarnPuntUnseen.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return;
	}

	BeginMboxUpdate();
	
	try
	{
		// Punt unseen the mailbox
		GetMbox()->PuntUnseen();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		EndMboxUpdate(false);
	}

	EndMboxUpdate();
}

// Full synchonise of mailbox
void CMailboxInfoTable::OnMailboxSynchronise()
{
	if (!GetMbox())
		return;

	// Get list of selected rows
	ulvector nums;
	GetSelectedRows(nums);
	CSynchroniseDialog::PoseDialog(GetMbox(), nums);
	FRAMEWORK_REFRESH_WINDOW(this)
}

// Full synchonise of mailbox
void CMailboxInfoTable::OnMailboxClearDisconnect()
{
	if (!GetMbox())
		return;

	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::ReallyClearDisconnect") == CErrorHandler::Ok)
	{
		// Get list of selected rows
		ulvector nums;
		GetSelectedRows(nums);

		GetMbox()->ClearDisconnectMessage(nums, false, true);

		FRAMEWORK_REFRESH_WINDOW(this)
	}
}

// Subscribe to mailbox
void CMailboxInfoTable::OnMailboxSubscribe(void)
{
	if (!GetMbox())
		return;

	try
	{
		// Subscribe to the mailbox
		GetMbox()->Subscribe();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

// Unsubscribe from mailbox
void CMailboxInfoTable::OnMailboxUnsubscribe(void)
{
	if (!GetMbox())
		return;

	try
	{
		// Unsubscribe from the mailbox
		GetMbox()->Unsubscribe();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

// Check for new mail in this mailbox
void CMailboxInfoTable::OnMailboxCheck(void)
{
	if (!GetMbox())
		return;

	long new_mail = GetMbox()->Check();

	if (new_mail)
	{
		CMailControl::NewMailUpdate(GetMbox(), new_mail, CPreferences::sPrefs->mMailNotification.GetValue().front());
		CMailControl::NewMailAlert(CPreferences::sPrefs->mMailNotification.GetValue().front());
	}
}

// Goto a message
void CMailboxInfoTable::OnMailboxGotoMessage()
{
	if (!GetMbox())
		return;

	unsigned long goto_num = 0;
	bool result = CCacheMessageDialog::PoseDialog(goto_num);

	if (result)
	{
		if ((goto_num > 0) && (goto_num <= GetMbox()->GetNumberFound()))
		{
			// See if message is in view
			CMessage* theMsg = GetMbox()->GetMessage(goto_num);

			if (!theMsg || !GetMbox()->GetMessageIndex(theMsg, true))
				CErrorHandler::PutStopAlertRsrcStr("Alerts::Mailbox::NotViewedMessageNum", cdstring(goto_num));
			else
			{
				// See if message needs to be cached
				if (!theMsg->IsFullyCached())
				{
					GetMbox()->CacheMessage(goto_num);

					// If sorting by anything other than by message number, must do entire refresh
					if (GetMbox()->GetSortBy() != cSortMessageNumber)
						FRAMEWORK_REFRESH_WINDOW(this)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					else
						// Always update port after dialog in case of scroll
						UpdatePort();
#endif
				}
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				else
					// Always update port after dialog in case of scroll
					UpdatePort();
#endif

				unsigned long row = GetMbox()->GetMessageIndex(theMsg, true);
				if (row)
				{
					ScrollToRow(row - TABLE_ROW_ADJUST, true, true,
								GetMbox()->ValidSort(GetMbox()->GetSortBy()) ? eScroll_Center : eScroll_Bottom);
					UpdateItems();
				}
			}
		}
		else
			CErrorHandler::PutStopAlertRsrcStr("Alerts::Mailbox::IllegalMessageNum", cdstring(goto_num));
	}
}

// Do cache all
void CMailboxInfoTable::OnMailboxCacheAll()
{
	if (!GetMbox())
		return;

	// Preserve selection
	StMailboxTableSelection preserve_selection(this, false);

	BeginMboxUpdate();
	CMbox* temp = GetMbox();			// Allow use of mbox after this is deleted

	try
	{
		GetMbox()->CacheAllMessages();
		FRAMEWORK_REFRESH_WINDOW(this)
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only do if not already deleted
		if (temp->IsOpen())
			EndMboxUpdate(false);

		CLOG_LOGRETHROW;
		throw;
	}

	EndMboxUpdate();
}

// Do mailbox search
void CMailboxInfoTable::OnSearchMailbox()
{
	// Just create the search window
	CSearchWindow::CreateSearchWindow();
	if (GetMbox())
		CSearchWindow::AddMbox(GetMbox());
}

// Do mailbox search again
void CMailboxInfoTable::OnSearchAgainMailbox()
{
	if (!GetMbox())
		return;

	// Redo last search with this mbox (might have been done on different mbox)
	CSearchWindow::SearchAgain(GetMbox());
}

// Recycle to next search mbox
void CMailboxInfoTable::OnNextSearchMailbox()
{
	// Determine next search target
	long next = CSearchEngine::sSearchEngine.NextTarget();
	if (next >= 0)
	{
		// Try to recycle
		CSearchEngine::sSearchEngine.OpenTarget(next, mTableView);
	}
	else
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		::SysBeep(1);
#elif __dest_os == __win32_os
		::MessageBeep(-1);
#elif __dest_os == __linux_os
		::MessageBeep(1);
#else
#error __dest_os
#endif
	}
}

// Apply rules toolbar item
void CMailboxInfoTable::OnApplyRuleMailboxToolbar(const char* name)
{
	// The name is actual the uid encoded as a string
	unsigned long uid = ::strtoul(name, NULL, 10);
	
	// Now get the filter with the matching UID
	if (uid == 0)
	{
		DoApplyRuleMailbox(-1);
	}
	else
	{
		unsigned long index = CPreferences::sPrefs->GetFilterManager()->GetManualIndex(uid);
		
		if (index != -1)
			DoApplyRuleMailbox(index);
	}
}

// Apply rules
void CMailboxInfoTable::DoApplyRuleMailbox(long index)
{
	if (!GetMbox())
		return;

	// Only if not locked out
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Always preserve current selection as filter actions may add/remove messages
	StMailboxTableSelection preserve_selection(this, false);

	ulvector uids;
	ulvector* use_uids = NULL;
	
	if (IsSelectionValid())
	{
		// get list of selected messages
		std::auto_ptr<CMessageList> msgs(new CMessageList);
		msgs->SetOwnership(false);

		// Add selection to list
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddFullSelectionToList, msgs.get());

		// Look at each message and make sure uids exist
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			if (!(*iter)->GetUID())
				uids.push_back((*iter)->GetMessageNumber());
		}

		// Load missing UIDs from server
		if (uids.size())
			GetMbox()->CacheUIDs(uids);

		// Now add all UIDs to list
		uids.clear();
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			uids.push_back((*iter)->GetUID());
		
		use_uids = &uids;
	}

	// Apply each manual rule
	CPreferences::sPrefs->GetFilterManager()->ExecuteManual(GetMbox(), use_uids, index);
}

// Make rule
void CMailboxInfoTable::OnMakeRuleMailbox()
{
	// Only if not locked out
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// get list of selected messages
	std::auto_ptr<CMessageList> msgs(new CMessageList);
	msgs->SetOwnership(false);

	// Add selection to list
	DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get());

	// Limit selection to a maximum of 30 messages
	if (msgs->size() > 30)
		msgs->erase(msgs->begin() + 30, msgs->end());

	// Do example rule
	CRulesWindow::MakeRule(*msgs.get());
}

// Reply to the message
void CMailboxInfoTable::DoMessageReply(EReplyTo reply_to, bool option_key)
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
																		"ErrorDialog::Text::MultiReply", 3);
		if (result == CErrorDialog::eBtn1)
			multi_msgs = true;
		if (result == CErrorDialog::eBtn2)
			multi_msgs = false;
		if (result == CErrorDialog::eBtn3)
#if __dest_os == __win32_os
		{
			// Set focus back to table after button push
			if (CSDIFrame::IsTopWindow(GetParentFrame()))
				SetFocus();
			return;
		}
#else
			return;
#endif
	}

	if (multi_msgs)
	{
		// Do in reverse order so that draft windows end up in ascending order
		DoToSelection2((DoToSelection2PP) &CMailboxInfoTable::ReplyMessage, &reply_to, &option_key, GetMbox()->GetShowBy() != cShowMessageAscending);

	}
	else
	{
		// NB msgs will be deleted (or used) by the reply
		CMessageList* msgs = new CMessageList;
		msgs->SetOwnership(false);

		// Add selection to list (always add in ascending order - i.e. reverse selection if reverse sort)
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs, GetMbox()->GetShowBy() == cShowMessageAscending);

		// Do reply action
		CReplyChooseDialog::ProcessChoice(msgs, reply_to, true);
	}
}

// Reply to the message
bool CMailboxInfoTable::ReplyMessage(TableIndexT row, EReplyTo* reply_to, bool* option_key)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't reply if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Do reply action
	cdstring quote;
	CReplyChooseDialog::ProcessChoice(theMsg, quote, eContentSubPlain, false, *reply_to, !(CPreferences::sPrefs->optionKeyReplyDialog.GetValue() ^ *option_key));
	return true;
}

// Forward the message
void CMailboxInfoTable::DoMessageForward(bool option_key)
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
																		"ErrorDialog::Text::MultiForward", 3);
		if (result == CErrorDialog::eBtn1)
			multi_msgs = true;
		if (result == CErrorDialog::eBtn2)
			multi_msgs = false;
		if (result == CErrorDialog::eBtn3)
#if __dest_os == __win32_os
		{
			// Set focus back to table after button push
			if (CSDIFrame::IsTopWindow(GetParentFrame()))
				SetFocus();
			return;
		}
#else
			return;
#endif
	}

	if (multi_msgs)
		// Do in reverse order so that draft windows end up in ascending order
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::ForwardMessage, &option_key, GetMbox()->GetShowBy() != cShowMessageAscending);
	else
	{

		std::auto_ptr<CMessageList> msgs(new CMessageList);
		msgs->SetOwnership(false);

		// Add selection to list (always add in ascending order - i.e. reverse selection if reverse sort)
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get(), GetMbox()->GetShowBy() == cShowMessageAscending);

		// Check message size first
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			if (!CMailControl::CheckSizeWarning(*iter))
			{
#if __dest_os == __win32_os
				// Set focus back to table after button push
				if (CSDIFrame::IsTopWindow(GetParentFrame()))
					SetFocus();
#endif
				return;
			}
		}

		// Create the letter window and give it the message
		if (CActionManager::ForwardMessages(msgs.get(), option_key))
			// Window 'owns' message list
			msgs.release();
	}
}

// Forward the message
bool CMailboxInfoTable::ForwardMessage(TableIndexT row, bool* option_key)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't reply if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CActionManager::ForwardMessage(theMsg, false, cdstring::null_str, eContentSubPlain, *option_key);

	return true;
}

// Bounce the message
void CMailboxInfoTable::OnMessageBounce(void)
{
	if (!GetMbox())
		return;

	// Do in reverse order so that draft windows end up in ascending order
	DoToSelection((DoToSelectionPP) &CMailboxInfoTable::BounceMessage, GetMbox()->GetShowBy() != cShowMessageAscending);
}

// Bounce the message
bool CMailboxInfoTable::BounceMessage(TableIndexT row)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't bounce if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CActionManager::BounceMessage(theMsg);

	return true;
}

// Reject the message
void CMailboxInfoTable::OnMessageReject(void)
{
	if (!GetMbox())
		return;

	// Do in reverse order so that draft windows end up in ascending order
	DoToSelection((DoToSelectionPP) &CMailboxInfoTable::RejectMessage, GetMbox()->GetShowBy() != cShowMessageAscending);
}

// Reject the message
bool CMailboxInfoTable::RejectMessage(TableIndexT row)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't reject if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CActionManager::RejectMessage(theMsg);

	return true;
}

// Send again the message
void CMailboxInfoTable::OnMessageSendAgain(void)
{
	if (!GetMbox())
		return;

	// Do in reverse order so that draft windows end up in ascending order
	DoToSelection((DoToSelectionPP) &CMailboxInfoTable::SendAgainMessage, GetMbox()->GetShowBy() != cShowMessageAscending);
}

// Send again the message
bool CMailboxInfoTable::SendAgainMessage(TableIndexT row)
{
	if (!GetMbox())
		return false;

	// Get the relevant message
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Don't send again if its deleted
	if (theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
		return false;

	// Don't send again if its not smart
	if (!theMsg->IsSmartFrom())
		return false;

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	CActionManager::SendAgainMessage(theMsg);

	return true;
}

// Create a digest
void CMailboxInfoTable::OnMessageCreateDigest(void)
{
	if (!GetMbox())
		return;

	std::auto_ptr<CMessageList> msgs(new CMessageList);
	msgs->SetOwnership(false);

	// Add selection to list (always add in ascending order - i.e. reverse selection if reverse sort)
	DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get(), GetMbox()->GetShowBy() == cShowMessageAscending);

	// Create the letter window and give it the message
	if (CActionManager::CreateDigest(msgs.get()))
		// List 'owned' by letter
		msgs.release();
}

#pragma mark ____________________________View Operations

// Reset search UI items from mailbox
void CMailboxInfoTable::ResetSearch()
{
	if (!GetMbox())
		return;

	// Turn on display of search results in match mode
	mSearchResults = true;

	// Now force mailbox into search state
	GetMbox()->OpenSearch();

	// Force selection change notification to update match button state
	UpdateState();
}

void CMailboxInfoTable::OnMatchButton(bool option_key)
{
#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif

	// New mode is opposite ofr existing mode
	NMbox::EViewMode new_mbox_mode;
	bool matched;
	if (GetMbox()->GetViewMode() != NMbox::eViewMode_ShowMatch)
	{
		matched = true;
		new_mbox_mode = NMbox::eViewMode_ShowMatch;
		
		// Always have this off when going in to Match mode
		mSearchResults = false;
	}
	else
	{
		matched = false;
		
		// If option key then user likely wants to remain in matched mode and simply update the criteria
		new_mbox_mode = option_key ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All;
	}

	// Do dialog if not matched and no option key, or matched and option key
	if (matched ^ option_key)
	{
		// Return if dialog cancelled
		if (!CMatchOptionsDialog::PoseDialog(new_mbox_mode, mMatch, mSearchResults))
			return;
	}
	
	// Look to see if match wants selection
	std::auto_ptr<CMessageList> msgs;
	if (!mSearchResults && (new_mbox_mode != NMbox::eViewMode_All) && mMatch.NeedsSelection() && IsSelectionValid())
	{
		msgs.reset(new CMessageList);
		msgs->SetOwnership(false);

		// Add selection to list
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get());
	}

	// Reset the match criteria - do none to clear match hilighting
	SetMatch(new_mbox_mode, !mSearchResults && (new_mbox_mode != NMbox::eViewMode_All) ? mMatch.ConstructSearch(msgs.get()) : NULL, mSearchResults && (new_mbox_mode != NMbox::eViewMode_All));
}

// Set specific search item
void CMailboxInfoTable::SetMatch(NMbox::EViewMode mbox_mode, CSearchItem* spec, bool is_search, bool force_automatch)
{
	if (!GetMbox())
		return;

	// Preserve selection while changing display mode
	// Do this before changing the mode in the mailbox as the list will change
	StMailboxTableSelection preserve_selection(this);

	// See if different
	if (mbox_mode != GetMbox()->GetViewMode())
	{
		// Try auto match first
		switch(mbox_mode)
		{
		case NMbox::eViewMode_All:
		case NMbox::eViewMode_AllMatched:
			// Need to clear any auto match
			if (GetMbox()->GetAutoViewMode())
			{
				// Force preserved Msgs back to what they were before changing criteria (which resets the list)
				GetMbox()->SetAutoViewMode(false);
				mPreserveMsgs = mInfoTableView->mAutoMatchMsgs;
			}
			break;
		case NMbox::eViewMode_ShowMatch:
			// Check for auto match
			if (force_automatch)
			{
				// Set flag and preserve select
				GetMbox()->SetAutoViewMode(true);
				mInfoTableView->mAutoMatchMsgs = mPreserveMsgs;
			}
			break;
		}
	}

	bool reset = is_search ? GetMbox()->TransferSearch() : GetMbox()->SetViewSearch(spec);

	// Do change to match mode if different
	if (mbox_mode != GetMbox()->GetViewMode())
	{
		// Do view mode change
		reset |= GetMbox()->SetViewMode(mbox_mode);
	}

	// Force table reset
	if (reset)
		ResetTable();

	// Otherwise do refresh if required
	else
		FRAMEWORK_REFRESH_WINDOW(this)
		
	// Update quick search items to mimic the search
	mInfoTableView->SyncQuickSearch();
}

void CMailboxInfoTable::OnMailboxUpdateMatch()
{
	// Look to see if current match wants selection
	std::auto_ptr<CMessageList> msgs;
	if (mMatch.NeedsSelection() && IsSelectionValid())
	{
		msgs.reset(new CMessageList);
		msgs->SetOwnership(false);

		// Add selection to list
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get());
	}

	// Reset the match criteria - do none to clear match hilighting
	SetMatch(GetMbox()->GetViewMode(), mMatch.ConstructSearch(msgs.get()), false);
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CMailboxInfoTable::OnSelectPopup(long select)
#elif __dest_os == __win32_os
void CMailboxInfoTable::OnSelectPopup(UINT select)
#elif __dest_os == __linux_os
void CMailboxInfoTable::OnSelectPopup(long select)
#else
#error __dest_os
#endif
{
	if (!GetMbox())
		return;

#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif

	NMessage::EFlags set_flag = NMessage::eNone;
	NMessage::EFlags unset_flag = NMessage::eNone;
	ulvector rows;
	switch(select)
	{
		case eSelectBy_Seen:
			// Select only those seen
			set_flag = NMessage::eSeen;
			break;

		case eSelectBy_Unseen:
			// Select only those unseen
			unset_flag = NMessage::eSeen;
			break;

		case eSelectBy_Important:
			// Select only those flagged
			set_flag = NMessage::eFlagged;
			break;

		case eSelectBy_Unimportant:
			// Select only those not flagged
			unset_flag = NMessage::eFlagged;
			break;

		case eSelectBy_Answered:
			// Select only those answered
			set_flag = NMessage::eAnswered;
			break;

		case eSelectBy_Unanswered:
			// Select only those not answered
			unset_flag = NMessage::eAnswered;
			break;

		case eSelectBy_Deleted:
			// Select only those deleted
			set_flag = NMessage::eDeleted;
			break;

		case eSelectBy_Undeleted:
			// Select only those deleted
			unset_flag = NMessage::eDeleted;
			break;

		case eSelectBy_Draft:
			// Select only those draft
			set_flag = NMessage::eDraft;
			break;

		case eSelectBy_NotDraft:
			// Select only those draft
			unset_flag = NMessage::eDraft;
			break;

		case eSelectBy_Match:
			// Select only those found during search
			set_flag = NMessage::eSearch;
			break;

		case eSelectBy_NonMatch:
			// Select only those not found during search
			unset_flag = NMessage::eSearch;
			break;

		case eSelectBy_Label1:
		case eSelectBy_Label2:
		case eSelectBy_Label3:
		case eSelectBy_Label4:
		case eSelectBy_Label5:
		case eSelectBy_Label6:
		case eSelectBy_Label7:
		case eSelectBy_Label8:
			// Select only those with label
			set_flag = static_cast<NMessage::EFlags>(NMessage::eLabel1 << (select - eSelectBy_Label1));
			break;
	}

	// Now do match (may force serach of mailbox if only some messages are cached)
	GetMbox()->MatchMessageFlags(set_flag, unset_flag, rows, false, true);

	// Now select them
	SelectByRow(rows);
}

void CMailboxInfoTable::OnThreadSelectAll()
{
	if (IsSelectionValid())
	{
		// Add selection to list
		std::auto_ptr<CMessageList> msgs(new CMessageList);
		msgs->SetOwnership(false);
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddFullSelectionWithFakesToList, msgs.get());
		
		// Now get threads for each selected message
		std::auto_ptr<CMessageList> threads(new CMessageList);
		threads->SetOwnership(false);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			threads->AddThread(*iter);
			
		// Now select all the ones in the list
		UnselectAllCells();
		for(CMessageList::const_iterator iter = threads->begin(); iter != threads->end(); iter++)
		{
			unsigned long index = GetMbox()->GetMessageIndex(*iter, true);

			// Make sure row is valid as there may be more messages in the mailbox than rows at this time
			if (index <= GetItemCount())
				SelectRow(index - TABLE_ROW_ADJUST);
		}
	}
}

void CMailboxInfoTable::OnThreadMarkSeen()
{
	DoThreadFlag(NMessage::eSeen);
}

void CMailboxInfoTable::OnThreadMarkImportant()
{
	DoThreadFlag(NMessage::eFlagged);
}

void CMailboxInfoTable::OnThreadMarkDeleted()
{
	DoThreadFlag(NMessage::eDeleted);
}

void CMailboxInfoTable::DoThreadFlag(NMessage::EFlags flag)
{
	if (IsSelectionValid())
	{
		// Add selection to list
		std::auto_ptr<CMessageList> msgs(new CMessageList);
		msgs->SetOwnership(false);
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddFullSelectionWithFakesToList, msgs.get());
		
		// Now get threads for each selected message
		std::auto_ptr<CMessageList> threads(new CMessageList);
		threads->SetOwnership(false);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
			threads->AddThread(*iter);
			
		// Now get numbers of ones to toggle flag for
		// Also note whether they all currently have the flag set or not
		ulvector nums;
		bool all_set = true;
		for(CMessageList::const_iterator iter = threads->begin(); iter != threads->end(); iter++)
		{
			if (!(*iter)->IsFake())
			{
				nums.push_back((*iter)->GetMessageNumber());
				all_set = all_set && (*iter)->HasFlag(flag);
			}
		}
			
		// Now toggle flag on chosen messages (if they all currently have the specified flag set
		// then turn the flag off rather than on)
		GetMbox()->SetFlagMessage(nums, false, flag, all_set ? false : true, false);
	}
}

void CMailboxInfoTable::OnThreadSkip()
{
	if (IsSelectionValid())
	{
		// Add selection to list
		std::auto_ptr<CMessageList> msgs(new CMessageList);
		msgs->SetOwnership(false);
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddFullSelectionWithFakesToList, msgs.get());
		
		// Do only for the last one in the list
		const CMessage* skiper = msgs->back();
		
		// Find its top-level parent
		const CMessage* parent = skiper;
		while(parent->GetThreadParent() != NULL)
			parent = parent->GetThreadParent();
		
		// Now get the next sibling
		bool move_down = (GetMbox()->GetShowBy() == cShowMessageAscending);
		const CMessage* sibling = move_down ? parent->GetThreadNext() : parent->GetThreadPrevious();
		
		// Select the sibling and scroll to it if it exists
		if (sibling != NULL)
		{
			unsigned long index = GetMbox()->GetMessageIndex(sibling, true);
			ScrollToRow(index - TABLE_ROW_ADJUST, true, true, move_down ? eScroll_Bottom : eScroll_Top);
		}
		else
		{
			ScrollToRow((move_down ? GetItemCount() : 1) - TABLE_ROW_ADJUST, false, true, move_down ? eScroll_Bottom : eScroll_Top);
		}
	}
}

void CMailboxInfoTable::OnMatchUnseen()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eUnseen);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchImportant()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eFlagged);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchDeleted()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eDeleted);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchSentToday()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eSentToday);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchFrom()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eSelectedFrom);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchSubject()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eSelectedSubject);
	DoMatch(match);
}

void CMailboxInfoTable::OnMatchDate()
{
	// Create appropriate match
	CMatchItem match;
	match.SetSingleMatch(CMatchItem::eSelectedDate);
	DoMatch(match);
}

void CMailboxInfoTable::DoMatch(const CMatchItem& match)
{
	SetCurrentMatch(match);

	// Look to see if match wants selection
	std::auto_ptr<CMessageList> msgs;
	if (mMatch.NeedsSelection() && IsSelectionValid())
	{
		msgs.reset(new CMessageList);
		msgs->SetOwnership(false);

		// Add selection to list
		DoToSelection1((DoToSelection1PP) &CMailboxInfoTable::AddSelectionToList, msgs.get());
	}

	// Reset the match criteria
	SetMatch(NMbox::eViewMode_ShowMatch, mMatch.ConstructSearch(msgs.get()), false);
}

#pragma mark ____________________________Init

// Set initial position
void CMailboxInfoTable::InitPos(void)
{
	// Only if mailbox aleady set
	if (!GetMbox())
		return;

	// Allow drawing to bring items into cache
	mListChanging = false;
	FRAMEWORK_REFRESH_WINDOW(this)


#if __framework == __jx
	// Needed because scrollbar visiblilty is done as a task
	// but we need them updated now before the scroll
	UpdateScrollbars();
#endif

	// Determine direction in which to look for first unseen
	bool ascending_seq = (GetMbox()->GetShowBy() == cShowMessageAscending);
	bool scroll_done = false;

	// Opened at first
	if (CPreferences::sPrefs->openAtFirst.GetValue() || !GetMbox()->ValidSort(GetMbox()->GetSortBy()))
	{
		// Scroll to first/last if valid normal/reverse sort
		bool to_top = ascending_seq || !GetMbox()->ValidSort(GetMbox()->GetSortBy());
		ScrollToRow((to_top ? 1 : GetItemCount()) - TABLE_ROW_ADJUST, false, false, to_top ? eScroll_Top : eScroll_Bottom);
		
		// We've done a valid scroll
		scroll_done = true;
	}
	
	// If some unseen open at the unseen one
	else if (CPreferences::sPrefs->openAtFirstNew.GetValue() && (GetMbox()->GetFirstNew() > 0))
	{
		CMessage* theMsg = NULL;
		
		if (GetMbox()->ValidSort(GetMbox()->GetSortBy()))
		{
			// Get the first new message (not deleted) in sorted order
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
			unset_flag = static_cast<NMessage::EFlags>(unset_flag | NMessage::eDeleted);
			theMsg = GetMbox()->GetNextFlagMessage(NULL, set_flag, unset_flag, true, true);
		}
		else
			// Just use first new message in UID order
			theMsg = GetMbox()->GetMessage(GetMbox()->GetFirstNew());

		// If not cached then cache it
		if (theMsg && !theMsg->IsFullyCached())
		{
			// do not allow caching while processing another network command
			// This should not happen on init
			if (GetMbox()->GetMsgProtocol() && !GetMbox()->GetMsgProtocol()->_get_mutex().is_locked())
			{
				// This will cause caching
				StValueChanger<bool> _change(mListChanging, true);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				StValueChanger<ETriState> _change_vis(mVisible, triState_Off);
#endif

				theMsg = GetMbox()->GetCacheMessage(theMsg->GetMessageNumber());
			}
			// Have to redraw
			FRAMEWORK_REFRESH_WINDOW(this)
		}

		// If there is a matching message, scroll to it, otherwise
		// fall through to scroll to end
		if (theMsg)
		{
			bool do_select = !mTableView->GetPreview() || !mTableView->GetUsePreview() ||
								!CPreferences::sPrefs->mNoOpenPreview.GetValue();

			// Now map from unsorted to sorted position
			unsigned long row = GetMbox()->GetMessageIndex(theMsg, true);
			ScrollToRow(row - TABLE_ROW_ADJUST, do_select, true, ascending_seq ? eScroll_Top : eScroll_Bottom);
			UpdateItems();
			
			// We've done a valid scroll
			scroll_done = true;
		}
	}

	// All other options open mailbox at end
	if (!scroll_done)
	{
		// Scroll to last/first if normal/reverse sort
		if (GetItemCount() > 0)
			ScrollToRow((ascending_seq ? GetItemCount() : 1) - TABLE_ROW_ADJUST, false, false, ascending_seq ? eScroll_Bottom : eScroll_Top);
	}

	
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	if (!::strcmpnocase(GetMbox()->GetName(), cINBOX) && CPreferences::sPrefs->mSpeakNewOpen.GetValue())
	{
		// Force immediate update of window to bring messages into cache
		UpdatePort();
		DoSpeakRecentMessages();
	}
#endif
}

#pragma mark ____________________________Display updating

// Update cache increment
void CMailboxInfoTable::UpdateCacheIncrement()
{
	// Only if mailbox aleady set
	if (!GetMbox())
		return;

	// Only bother if cache increment set
	if (GetMbox()->IsLocalMbox() ? CPreferences::sPrefs->mLAutoCacheIncrement.GetValue() : CPreferences::sPrefs->mRAutoCacheIncrement.GetValue())
	{
		// Get number of visible rows in table
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		unsigned long num_cells = mFrameSize.height / GetRowHeight(1) + 1;
#elif __dest_os == __win32_os
		CRect frame;
		GetClientRect(frame);
		unsigned long num_cells = frame.Height() / GetRowHeight(1) + 1;
#elif __dest_os == __linux_os
		unsigned long num_cells= GetApertureHeight() / GetRowHeight(1) + 1;
#else
#error __dest_os
#endif
		GetMbox()->SetCacheIncrement(num_cells);
	}
	else
		// Just set to default
		GetMbox()->SetCacheIncrement(0);
}

#pragma mark ____________________________Tooltips

// Get text for current tooltip cell
void CMailboxInfoTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	if (!GetMbox())
		return;

	// Allow get message/envelope to fail during caching process
	CMessage* theMsg = GetMbox()->GetMessage(cell.row + TABLE_ROW_ADJUST, true);
	if (!theMsg) return;

	const CEnvelope* theEnv = theMsg->GetEnvelope();
	if (!theEnv) return;

	// Look for fake thread message
	if (theMsg->IsFake())
	{
		txt.FromResource("UI::MailboxTips::FAKE");
		return;
	}
	
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[cell.col - TABLE_START_INDEX];

	// Determine which heading it is and draw it
	switch (col_info.column_type)
	{
	case eMboxColumnSmart:
		// Display both to and from
		txt.FromResource("UI::MailboxTips::SENTTO");
		if (theEnv->GetTo()->size())
			txt += theEnv->GetTo()->front()->GetFullAddress();
		txt += os_endl;
		txt.AppendResource("UI::MailboxTips::SENTBY");
		if (theEnv->GetFrom()->size())
			txt += theEnv->GetFrom()->front()->GetFullAddress();
		break;

	case eMboxColumnFrom:
		if (theEnv->GetFrom()->size())
			txt = theEnv->GetFrom()->front()->GetFullAddress();
		break;

	case eMboxColumnTo:
		if (theEnv->GetTo()->size())
			txt = theEnv->GetTo()->front()->GetFullAddress();
		break;

	case eMboxColumnReplyTo:
		if (theEnv->GetReplyTo()->size())
			txt = theEnv->GetReplyTo()->front()->GetFullAddress();
		break;

	case eMboxColumnSender:
		if (theEnv->GetSender()->size())
			txt = theEnv->GetSender()->front()->GetFullAddress();
		break;

	case eMboxColumnCc:
		if (theEnv->GetCC()->size())
			txt = theEnv->GetCC()->front()->GetFullAddress();
		break;

	case eMboxColumnSubject:
	case eMboxColumnThread:
		// Check that fake subject exists
		if (theMsg->IsFake() && !theMsg->GetEnvelope()->HasFakeSubject())
			const_cast<CMessage*>(theMsg)->MakeFakeSubject();

		txt = theEnv->GetSubject();
		break;

	case eMboxColumnDateSent:
	case eMboxColumnDateReceived:
		// Show both sent and received
		txt.FromResource("UI::MailboxTips::DATESENT");
		txt += theEnv->GetTextDate(true).c_str();
		txt += os_endl;
		txt.AppendResource("UI::MailboxTips::DATERECEIVED");
		txt += theMsg->GetTextInternalDate(true).c_str();
		break;

	case eMboxColumnSize:
		// This is always added by default
		break;

	case eMboxColumnFlags:
		// This is always added by default
		break;

	case eMboxColumnNumber:
		txt.FromResource("UI::MailboxTips::NUMBER");
		txt += cdstring(theMsg->GetMessageNumber());
		break;

	case eMboxColumnAttachments:
		if (theMsg->GetBody()->IsVerifiable())
			txt.FromResource("UI::MailboxTips::SIGNED");
		else if (theMsg->GetBody()->IsDecryptable())
			txt.FromResource("UI::MailboxTips::ENCRYPTED");
		else if (theMsg->GetBody()->CountParts() > 1)
		{
			if (theMsg->GetBody()->HasUniqueTextPart())
				txt.FromResource("UI::MailboxTips::ALTERNATIVE");
			else
				txt.FromResource("UI::MailboxTips::ATTACHMENTS");
		}
		else
			txt.FromResource("UI::MailboxTips::NOATTACHMENTS");
		break;

	case eMboxColumnParts:
		// This is always added by default
		break;

	case eMboxColumnMatch:
		if (theMsg->IsSearch())
			txt.FromResource("UI::MailboxTips::MATCHED");
		else
			txt.FromResource("UI::MailboxTips::NOTMATCHED");
		break;

	case eMboxColumnDisconnected:
		if (theMsg->IsFullLocal())
			txt.FromResource("UI::MailboxTips::CACHED");
		else if (theMsg->IsPartialLocal())
			txt.FromResource("UI::MailboxTips::PARTIALCACHED");
		else
			txt.FromResource("UI::MailboxTips::NOTCACHED");
		break;
	}

	// Always add flag state
	if (txt.length())
		txt += os_endl;
	txt.AppendResource("UI::MailboxTips::IS");
	if (theMsg->IsUnseen())
		txt.AppendResource("UI::MailboxTips::UNSEEN");
	else
		txt.AppendResource("UI::MailboxTips::SEEN");

	if (theMsg->IsFlagged())
		txt.AppendResource("UI::MailboxTips::IMPORTANT");

	if (theMsg->IsDeleted())
		txt.AppendResource("UI::MailboxTips::DELETED");

	if (theMsg->IsDraft())
		txt.AppendResource("UI::MailboxTips::DRAFT");

	if (theMsg->IsAnswered())
		txt.AppendResource("UI::MailboxTips::ANSWERED");

	if (theMsg->IsRecent())
		txt.AppendResource("UI::MailboxTips::RECENT");

	if (theMsg->IsMDNSent())
		txt.AppendResource("UI::MailboxTips::MDNSENT");

	// Look for any labels
	bool got_label = false;
	for(unsigned long i = 0; i < NMessage::eMaxLabels; i++)
	{
		if (theMsg->HasLabel(i))
		{
			// Add title if not already done
			if (!got_label)
			{
				if (txt.length())
					txt += os_endl;
				txt.AppendResource("UI::MailboxTips::LABELS");
			}

			// Add label name
			if (got_label)
				txt += ", ";
			txt += CPreferences::sPrefs->mLabels.GetValue()[i]->name;			
			got_label = true;
		}
	}

	// Always add parts count
	if (txt.length())
		txt += os_endl;
	txt.AppendResource("UI::MailboxTips::PARTS");
	txt += cdstring(theMsg->GetBody()->CountParts());

	// Always add size
	if (txt.length())
		txt += os_endl;

	// Get size in bytes
	unsigned long msg_size = theMsg->GetSize();
	cdstring bytes(msg_size);

	// Get normalised numeric format
	cdstring temp = ::GetNumericFormat(msg_size);
	
	// Create string - add actual bytes if K or M used
	txt.AppendResource("UI::MailboxTips::SIZE");
	if (bytes != temp)
	{
		txt += temp;
		txt += " (";
		txt += bytes;
		txt += ")";
	}
	else
		txt += temp;
	
	// Check sync state
	if (GetMbox()->GetProtocol()->CanDisconnect())
	{
		if (txt.length())
			txt += os_endl;
		if (theMsg->HasFlag(NMessage::eFullLocal))
			txt.AppendResource("UI::MailboxTips::FULLSYNC");
		else if (theMsg->HasFlag(NMessage::ePartialLocal))
			txt.AppendResource("UI::MailboxTips::PARTIALSYNC");
		else
			txt.AppendResource("UI::MailboxTips::NOTSYNC");
	}
}

