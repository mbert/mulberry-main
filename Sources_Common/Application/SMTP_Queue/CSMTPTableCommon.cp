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


// Source for CSMTPTable class

#include "CSMTPTable.h"

#include "CActionManager.h"
#include "CErrorHandler.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMessage.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CStringResources.h"
#endif
#include "CPreferences.h"
#if __dest_os == __win32_os
#include "CSDIFrame.h"
#endif
#if __dest_os == __linux_os
#include "CMulberryCommon.h"
#endif
#include "CSMTPAccountManager.h"
#include "CSMTPView.h"

// Consts

#pragma mark ____________________________Commands

// Switch smtp queue modes
void CSMTPTable::OnSMTPHoldMessage()
{
	// See if all currently held
	if (mTestSelectionAndHold)
	{
		// Not MDNsent => processable
		DoFlagMailMessage(NMessage::eHold);
		
		// Must kick sender
		CSMTPAccountManager::sSMTPAccountManager->ResumeSMTPSender(static_cast<CSMTPView*>(mTableView)->GetSMTPSender());
	}
	else
	{
		// Not MDNsent => hold
		DoFlagMailMessage(NMessage::eHold);
	}
#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif
}

// Change message priority
void CSMTPTable::OnSMTPPriority()
{
	// Flagged => high priority
	DoFlagMailMessage(NMessage::ePriority);
#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif
}

// Change message priority
void CSMTPTable::OnSMTPDelete()
{
	// This may change during delete
	bool was_deleted = mTestSelectionAndDeleted;

	DoFlagMailMessage(NMessage::eDeleted);
	
	// Must kick sender if undelete
	if (was_deleted)
		CSMTPAccountManager::sSMTPAccountManager->ResumeSMTPSender(static_cast<CSMTPView*>(mTableView)->GetSMTPSender());

#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif
}

// Switch smtp queue modes
void CSMTPTable::OnSMTPAccountPopup(long index)
{
	CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
	static_cast<CSMTPView*>(mTableView)->SetSMTPSender(senders.at(index));
}

// Switch smtp queue modes
void CSMTPTable::OnSMTPEnableQueue()
{
	CSMTPAccountManager::sSMTPAccountManager->SetConnected(!CSMTPAccountManager::sSMTPAccountManager->GetConnected());
	UpdateState();

#if __dest_os == __win32_os
	// Set focus back to table after button push
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		SetFocus();
#endif
}

// Display a specified mail message
bool CSMTPTable::OpenMailMessage(TableIndexT row, bool* option_key)
{
	if (!GetMbox())
		return false;

	// Get the relevant message and envelope
	CMessage* theMsg = GetMbox()->GetMessage(row + TABLE_ROW_ADJUST, true);

	if (!theMsg || !theMsg->IsFullyCached() || theMsg->IsFake())
		return false;

	// Must try holding it first - cannot open a message that is being sent
	if (!theMsg->IsSendNow())
		theMsg->ChangeFlags(NMessage::eHold, true);

	// Don't display if its deleted or being sent
	if (theMsg->IsSendNow() || theMsg->IsDeleted() && !CPreferences::sPrefs->mOpenDeleted.GetValue())
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

	// Check message size first
	if (!CMailControl::CheckSizeWarning(theMsg))
		return false;

	// Create the letter window and give it the message
	CActionManager::SendAgainMessage(theMsg);

	return true;
}

#pragma mark ____________________________Init

// Set initial position
void CSMTPTable::InitPos(void)
{
	// Allow drawing to bring items into cache
	mListChanging = false;
	FRAMEWORK_REFRESH_WINDOW(this)

	// Make initial selection
	ScrollToRow(1 - TABLE_ROW_ADJUST, false, false, eScroll_Top);
}
