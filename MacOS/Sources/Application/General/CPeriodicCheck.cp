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
#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMailControl.h"
#include "CMulberryApp.h"
#include "CNetworkResetError.h"
#include "CPreferences.h"
#include "CVisualProgress.h"
#if PP_Target_Carbon
#include "CSoundManager.h"
#endif
#include "CSpeechSynthesis.h"
#include "CTaskQueue.h"
#include "CTCPSocket.h"

// __________________________________________________________________________________________________
// C L A S S __ C P E R I O D I C C H E C K
// __________________________________________________________________________________________________

// Called during idle
void CPeriodicCheck::SpendTime(const EventRecord &inMacEvent)
{
	StValueChanger<unsigned long> _depth(mCallDepth, mCallDepth + 1);

	// Check for power wakeup signal
	if (mPwrSignal && !mPwrSignalProcess)
	{
		// Set lock to prevent multiple wakeup alerts
		StValueChanger<bool> _lock(mPwrSignalProcess, true);

		// Do wakeup action before turning off flag so that flag is set
		// during wakeup actions in case sleep occurs during wakeup dialog
		CMulberryApp::sApp->WakeFromSleepAction();
		mPwrSignal = false;
	}

	// Prevent any periodic tasks whilst busy dialog is on screen
	if (CMailControl::BusyDialog() || mBlock)
		return;

	// Do not allow exceptions to propogate out of here
	try
	{
		// Check for app idled out (due to OS sleep)
		CheckAppIdleTooLong();

		// Do idle time tasks
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

		// Do any speech
		if (CSpeechSynthesis::Available())
			CSpeechSynthesis::SpendTime();

		// Do asyn sounds
#if PP_Target_Carbon
		CSoundManager::sSoundManager.SoundIdle();
#endif

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

void CPeriodicCheck::PwrSignal()
{
	mPwrSignal = true;
}

// Computer about to wake
void CPeriodicCheck::Resume()
{
	// If any connections are present do warning
	if (CTCPSocket::CountConnected())
	{
		// Determine whether we can allow disconnected ops
		bool allow_disconnect = !CAdminLock::sAdminLock.mNoDisconnect && CConnectionManager::sConnectionManager.CanDisconnect();

		// Display alert to user warning them of invalid conections and allow choice of disconnect if appropriate
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
			CBalloonDialog* dlog = CProgressDialog::StartDialog("Alerts::General::ConnectionsRecover");
			
			try
			{
				// Force connections to recover by tickling each one forcibly
				CMailControl::SpendTime(true, true);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

				CProgressDialog::EndDialog(dlog);
				
				CLOG_LOGRETHROW;
				throw;
			}

			CProgressDialog::EndDialog(dlog);
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

// __________________________________________________________________________________________________
// C L A S S __ C P E R I O D I C R E S E T
// __________________________________________________________________________________________________

// Wait for user to pause
bool CPeriodicReset::UserPaused(void)
{
	// Check whether 2 seconds have passed since last user action
	return ::difftime(::time(NULL), mLastTime) >= 2;
}

// Reset timer if key or mouse event
void CPeriodicReset::ExecuteSelf(
	MessageT		inMessage,
	void*			ioParam)
{
	// Force reset of timer if key or mouse event
	switch(((EventRecord*) ioParam)->what)
	{
	case keyUp:
	case keyDown:
	case mouseUp:
	case mouseDown:
	case autoKey:
		mLastTime = ::time(NULL);
		break;
	default: ;
	}
}
