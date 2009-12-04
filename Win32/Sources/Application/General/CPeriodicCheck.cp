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


// Source for CPeriodicCheck class

#include "CPeriodicCheck.h"

#include "CAdminLock.h"
#include "CCalendarNotifier.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMailControl.h"
#include "CNetworkResetError.h"
#include "CPreferences.h"
#include "CSpeechSynthesis.h"
#include "CTaskQueue.h"
#include "CTCPSocket.h"
#include "CVisualProgress.h"

#include "StValueChanger.h"

// __________________________________________________________________________________________________
// C L A S S __ C P E R I O D I C C H E C K
// __________________________________________________________________________________________________

// Called during idle
void CPeriodicCheck::SpendTime(bool do_tasks)
{
	StValueChanger<unsigned long> _depth(mCallDepth, mCallDepth + 1);

	// Prevent any periodic tasks whilst busy dialog is on screen
	if (CMailControl::BusyDialog() || mBlock)
		return;

	// Do not allow exceptions to propogate out of here
	try
	{
		// Check for app idled out (due to OS sleep)
		CheckAppIdleTooLong();

		// Do idle time tasks
		if (do_tasks)
			while(CTaskQueue::sTaskQueue.DoNextWork()) {}

		// Check servers
		CMailControl::SpendTime(false, mCallDepth == 1);

		// Check calendar alerts
		calstore::CCalendarNotifier::sCalendarNotifier.Poll();

		// Check for async alerts
		if (CMailControl::HasAlert())
		{
			cdstring txt;
			CMailControl::PopAlert(txt);
			CErrorHandler::PutNoteAlert(txt);
		}

		// Do speech
		CSpeechSynthesis::SpendTime();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Computer about to sleep
void CPeriodicCheck::Suspend()
{
	// Block any further periodic processing
	mBlock = true;
}

// Computer about to wake
void CPeriodicCheck::Resume(bool silent)
{
	// If any connections are present do wraning
	if (CTCPSocket::CountConnected())
	{
		// Determine whether we can allow disconnected ops
		bool allow_disconnect = !CAdminLock::sAdminLock.mNoDisconnect && CConnectionManager::sConnectionManager.CanDisconnect();

		// Display alert to user warning them of invalid conections
		bool disconnect = false;
		// See if quick reconnect is on and do that test first
		if (!CPreferences::sPrefs->mSleepReconnect.GetValue() || !CTCPSocket::CheckConnectionState())
			disconnect = CNetworkResetError::PoseDialog(allow_disconnect);

		if (disconnect)
			CConnectionManager::sConnectionManager.Disconnect(true, true, NULL);
		else
		{
			// See if any connections are now invalid
			CTCPSocket::CheckConnections(false);
			
			// Put up progress dialog
			CProgressDialog* dlog = CProgressDialog::StartDialog("Alerts::General::ConnectionsRecover");
			
			try
			{
				// Force connections to recover by tickling each one forcibly
				CMailControl::SpendTime(true, true);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

				CProgressDialog::StopDialog(dlog);
				
				CLOG_LOGRETHROW;
				throw;
			}

			CProgressDialog::StopDialog(dlog);
		}
	}

	// Allow periodic processing
	mBlock = false;
}

// Called during idle
void CPeriodicCheck::CheckAppIdleTooLong()
{
	// First call - set time to current time
	if (mLastSpendTime == 0)
	{
		mLastSpendTime = ::time(NULL);
		return;
	}
	
	// See if timer has been idle too long
	if (::difftime(::time(NULL), mLastSpendTime) > CPreferences::sPrefs->mAppIdleTimeout.GetValue())
	{
		StValueChanger<bool> _save(mBlock, true);

		// See if any connections are now invalid
		bool tcp_ok = CTCPSocket::CheckConnections(false);
		
		if (!tcp_ok)
			// Force connections to recover by tickling each one forcibly
			CMailControl::SpendTime(true, true);
	}
	
	// Reset timer to now
	mLastSpendTime = ::time(NULL);
}

