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


// Source for CMailboxWindow class

#include "CMailboxWindow.h"

#include "CMailboxView.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessageView.h"
#include "CPreferences.h"
#include "CSplitterView.h"

#include <algorithm>

// Find the corresponding window using URL
CMailboxWindow* CMailboxWindow::FindWindow(const CMbox* mbox)
{
	// Does window already exist?
	cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
	cdstring compare_acct = mbox->GetAccountName();
	for(CMailboxWindowList::iterator iter = sMboxWindows->begin(); iter != sMboxWindows->end(); iter++)
	{
		// Compare with the one we want
		if ((*iter)->GetMailboxView()->GetMbox() &&
			((*iter)->GetMailboxView()->GetMbox()->GetAccountName() == compare_acct))
		{
			// Set return items if found
			return *iter;
		}
	}

	return NULL;
}

// Check for window
bool CMailboxWindow::WindowExists(const CMailboxWindow* wnd)
{
	cdmutexprotect<CMailboxWindowList>::lock _lock(sMboxWindows);
	CMailboxWindowList::iterator found = std::find(sMboxWindows->begin(), sMboxWindows->end(), wnd);
	return found != sMboxWindows->end();
}

void CMailboxWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change preview state
	mSplitter->ShowView(true, show);
	GetMailboxView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetMailViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}
