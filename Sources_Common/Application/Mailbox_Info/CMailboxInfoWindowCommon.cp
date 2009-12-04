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


// Source for CMailboxInfoWindow class

#include "CMailboxInfoWindow.h"

#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CIMAPCommon.h"
#include "CMailAccountManager.h"
#if __dest_os == __win32_os
#include "CMailboxInfoFrame.h"
#endif
#include "CMailboxInfoTable.h"
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
#include "CSearchWindow.h"
#include "CStringUtils.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CWaitCursor.h"
#include "CWindowsMenu.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#include <LIconControl.h>
#endif

#if __dest_os == __linux_os
#include <JXImageWidget.h>
#include "CWaitCursor.h"
#endif

//#include <UProfiler.h>

//#include <string.h>

#include <numeric>

// Open a window for this mailbox
CMailboxWindow* CMailboxInfoWindow::OpenWindow(CMbox* mbox, bool is_search)
{
	CMailboxWindow* new_window = NULL;

	// Protect
	if (CMulberryApp::sApp->IsDemo() && ::strcmpnocase(mbox->GetName(), cINBOX))
		CErrorHandler::PutNoteAlertRsrc("Alerts::General::DemoWarn");

	try
	{
		// Resolve to open mailbox
		CMbox* open_mbox = mbox->GetProtocol()->FindOpenMbox(mbox->GetName());
		if (open_mbox)
			mbox = open_mbox;

		// Does window already exist?
		new_window = CMailboxWindow::FindWindow(mbox);

		if (new_window)
		{
			// Select it if it exists
			FRAMEWORK_WINDOW_TO_TOP(new_window)
		}
		// Not found so create new mailbox window
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
				// Now open the mailbox
				mbox->Open();

				// Now force transfer of search results into view if anything found
				if (is_search)
					mbox->OpenSearch();

				{
					// May take some time so spin cursor
					CWaitCursor wait;

					// Create the window
#if __dest_os == __mac_os || __dest_os == __mac_os_x
					new_window = (CMailboxWindow*) LWindow::CreateWindow(paneid_MailboxWindow, CMulberryApp::sApp);
#elif __dest_os == __win32_os || __dest_os == __linux_os
					new_window = CMailboxInfoWindow::ManualCreate();
#else
#error __dest_os
#endif

					// Set info table's details (and window title)
					new_window->GetMailboxView()->SetMbox(mbox, is_search);
#if __dest_os != __linux_os
					FRAMEWORK_WINDOW_TO_TOP(new_window)
#endif
					// Do filter action
					CPreferences::sPrefs->GetFilterManager()->OpenMailbox(mbox);
				}
			}
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete the window if its not already been deleted
		if (new_window && mbox->IsOpen())
			new_window->GetMailboxView()->DoClose();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	return new_window;
}
