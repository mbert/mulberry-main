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


// Source for CSMTPView class

#include "CSMTPView.h"

#include "CMailboxTable.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CSMTPAccountManager.h"
#if __dest_os == __win32_os
#include "CSMTPFrame.h"
#endif
#include "CSMTPSender.h"
#include "CSMTPTable.h"
#include "CSMTPToolbar.h"
#include "CSMTPWindow.h"
#include "CTaskClasses.h"

#if __dest_os == __linux_os
#include <JXImageWidget.h>
#endif

// Close window objects
void CSMTPView::DoClose()
{
	// Close the mailbox
	DoCloseMbox();

	// Now close its parent 1-pane window at idle time
	if (!Is3Pane())
	{
		CCloseMailboxWindowTask* task = new CCloseMailboxWindowTask(GetMailboxWindow());
		task->Go();
	}
}

// Close window objects
void CSMTPView::DoCloseMbox()
{
	// Only meaningful if not already closed
	if (IsClosed())
		return;

	// Save state
	SaveDefaultState();

	// Set status
	SetClosing();

	// Get its mailbox
	CMbox* mbox = GetMbox();
	if (mbox)
	{
		// Close its mailbox (may delete if delete on close!)
		//if (mForceClose)
		//	mbox->CloseSilent(true);
		//else
			mbox->Close();

		// Must set mbox to nil in case of delete on close
		GetTable()->SetMbox(NULL);
	}

	// Set status
	SetClosed();
}

// Set the mbox
void CSMTPView::SetMbox(CMbox* anMbox, bool is_search)
{
	bool reset_state = !GetMbox();

	GetTable()->SetMbox(anMbox);

	// Enable disable it based on mbox
	if (anMbox)
	{
		// Enable it
		FRAMEWORK_ENABLE_WINDOW(this);
	}
	else
	{
		// Disable it
		FRAMEWORK_DISABLE_WINDOW(this);
	}

#if __dest_os == __win32_os
	// Force focus on table
	if (CSDIFrame::IsTopWindow(GetParentFrame()))
		GetTable()->SetFocus();
#endif

	// Listen in to changes to mailbox protocol
	anMbox->GetProtocol()->Add_Listener(this);

	// Reset window state only for first time
	if (reset_state)
		ResetState();

	// Now do init pos
	GetTable()->InitPos();

}

// Set the sender
void CSMTPView::SetSMTPSender(CSMTPSender* sender)
{
	// Cache sender
	mSender = sender;
	
	// Get offline mbox
	CMbox* mbox = CSMTPAccountManager::sSMTPAccountManager->GetOfflineMbox(mSender);

	// Close any existing mailbox
	if (GetMbox() && (GetMbox() != mbox))
		DoCloseMbox();

	bool window_closed = false;
	if (GetMbox() != mbox)
	{
		try
		{
			// Must open first
			mbox->Open();

			// Reset status changed by DoCloseMbox
			// Must do this after opening mailbox so redisplay is blocked while messages are being recached
			SetOpen();

			// Set new mbox
			SetMbox(mbox);

			// Show/hide secure connection icon by forcing
			Broadcast_Message(CSMTPToolbar::eBroadcast_SecureState, mSender->IsSecure() ? (void*) 1 : NULL);

		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must close window
			FRAMEWORK_DELETE_WINDOW(GetMailboxWindow())
			
			CLOG_LOGRETHROW;
			throw;
		}
	}
	
	static_cast<CSMTPWindow*>(GetMailboxWindow())->UpdateTitle();
}

// Mailbox was renamed
void CSMTPView::RenamedMailbox(const CMbox* mbox)
{
	// Does nothing as SMTP mailbox should not change
}

