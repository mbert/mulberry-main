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


// Source for CMailboxListPanel class

#include "CMailboxListPanel.h"

#include "CBalloonDialog.h"
#include "CBrowseMailboxDialog.h"
#include "CIconTextTable.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CVisualProgress.h"
#include "CXStringResources.h"

#include <LDisclosureTriangle.h>
#include <LPopupButton.h>
#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxListPanel::CMailboxListPanel()
{
	mTwisted = true;
	mInProgress = false;
}

// Default destructor
CMailboxListPanel::~CMailboxListPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CMailboxListPanel::SetProgress(unsigned long progress)
{
	mProgress->SetCount(progress);
}

void CMailboxListPanel::SetFound(unsigned long found)
{
	mFound->SetValue(found);
}

void CMailboxListPanel::SetMessages(unsigned long msgs)
{
	mMessages->SetValue(msgs);
}

void CMailboxListPanel::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxList->SetIcon(item + 1, clear ? 0 : (hit ? icnx_SearchHit : icnx_SearchMiss));
}

void CMailboxListPanel::SetInProgress(bool in_progress)
{
	mInProgress = in_progress;

	if (mInProgress)
	{
		mCabinets->Disable();
		mAddListBtn->Disable();
		mClearListBtn->Disable();
	}
	else
	{
		mCabinets->Enable();
		mAddListBtn->Enable();
		mClearListBtn->Enable();
	}
}

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CMailboxListPanel::FinishCreateSelf(LWindow* parent, LPane* container)
{
	mParent = parent;
	mContainer = container;

	// Get all controls
	mTwister = (LDisclosureTriangle*) parent->FindPaneByID(paneid_MailboxListTwist);
	mShowCaption = (CStaticText*) parent->FindPaneByID(paneid_MailboxListShowCaption);
	mHideCaption = (CStaticText*) parent->FindPaneByID(paneid_MailboxListHideCaption);
	mMailboxArea = (LView*) parent->FindPaneByID(paneid_MailboxListMailboxArea);
	mCabinets = (LPopupButton*) parent->FindPaneByID(paneid_MailboxListCabinets);
	mAddListBtn = (LPushButton*) parent->FindPaneByID(paneid_MailboxListMailboxAdd);
	mClearListBtn = (LPushButton*) parent->FindPaneByID(paneid_MailboxListMailboxClear);
	mMailboxList = (CIconTextTable*) parent->FindPaneByID(paneid_MailboxListMailboxList);
	mTotal = (CStaticText*) parent->FindPaneByID(paneid_MailboxListTotal);
	mFound = (CStaticText*) parent->FindPaneByID(paneid_MailboxListFound);
	mMessages = (CStaticText*) parent->FindPaneByID(paneid_MailboxListMessages);
	mProgress = (CBarPane*) parent->FindPaneByID(paneid_MailboxListProgress);

	// Initialise items
	InitCabinets();

} // CMailboxListPanel::FinishCreateSelf

#pragma mark ____________________________Commands

void CMailboxListPanel::OnTwist()
{
	if (mTwisted)
	{
		mHideCaption->Hide();
		mShowCaption->Show();
		mMailboxArea->Hide();
		SDimension16 size;
		mMailboxArea->GetFrameSize(size);
		if (mContainer)
		{
			mContainer->ResizeFrameBy(0, -size.height, false);
			mContainer->MoveBy(0, size.height, false);
		}
		mParent->ResizeWindowBy(0, -size.height);
	}
	else
	{
		mShowCaption->Hide();
		mHideCaption->Show();
		SDimension16 size;
		mMailboxArea->GetFrameSize(size);
		mMailboxArea->Show();
		if (mContainer)
		{
			mContainer->ResizeFrameBy(0, size.height, false);
			mContainer->MoveBy(0, -size.height, false);
		}
		mParent->ResizeWindowBy(0, size.height);
	}

	mTwisted = !mTwisted;
}

#pragma mark ____________________________Mailbox List

void CMailboxListPanel::InitCabinets()
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(mCabinets->GetMacMenuH());
	for(short i = 1; i <= num_menu; i++)
		::DeleteMenuItem(mCabinets->GetMacMenuH(), 1);

	short index = 1;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++, index++)
		::AppendItemToMenu(mCabinets->GetMacMenuH(), index, (*iter)->GetName());

	// Force max/min update
	mCabinets->SetMenuMinMax();
	mCabinets->SetValue(0);
}

void CMailboxListPanel::OnCabinet(long item)
{
	// Get cabinet
	const CMboxRefList* cabinet = CMailAccountManager::sMailAccountManager->GetFavourites().at(item - 1);

	// Must wait for and pause mail checking
	{
		// Begin a busy operation - force it to cancel background threads
		CBusyContext busy;
		busy.SetCancelOthers(true);
		cdstring desc = rsrc::GetString("Status::IMAP::Checking");
		StMailBusy busy_lock(&busy, &desc);

		while(CMailCheckThread::IsRunning())
		{
			if (CMailControl::ProcessBusy(&busy))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
	}
	bool was_paused = CMailCheckThread::Pause(true);

	CMboxList match;
	for(CMboxRefList::const_iterator iter = cabinet->begin(); iter != cabinet->end(); iter++)
	{
		const CMboxRef* mbox_ref = static_cast<CMboxRef*>(*iter);

		// Don't bother with directories
		if (mbox_ref->IsDirectory())
			continue;

		// Resolve to mboxes - maybe wildcard so get list but do not get directories
		if (mbox_ref->IsWildcard())
			mbox_ref->ResolveMbox(match, true, true);
		else
		{
			CMbox* mbox = mbox_ref->ResolveMbox(true);
			if (mbox && !mbox->IsDirectory())
				match.push_back(mbox);
		}
	}

	// Add all to list (will do refresh)
	AddMboxList(&match);

	// Can continue mail checking
	CMailCheckThread::Pause(was_paused);
}

void CMailboxListPanel::ResetMailboxList(const CMboxRefList& targets, const ulvector& target_hits)
{
	cdstrvect items;
	ulvector item_states;

	// Do to each mailbox
	CMboxRefList::const_iterator iter1 = targets.begin();
	ulvector::const_iterator iter2 = target_hits.begin();
	for(; iter1 != targets.end() && iter2 != target_hits.end(); iter1++, iter2++)
	{
		items.push_back((*iter1)->GetName());
		switch(*iter2)
		{
		case eNotHit:
		default:
			item_states.push_back(0);
			break;
		case eHit:
			item_states.push_back(icnx_SearchHit);
			break;
		case eMiss:
			item_states.push_back(icnx_SearchMiss);
			break;
		}
	}

	mMailboxList->SetContents(items, item_states);
	mTotal->SetValue(items.size());
	mFound->SetValue(0);
	mMessages->SetValue(0);
}

void CMailboxListPanel::OnClearMailboxList()
{
	CMboxRefList targets;
	ulvector target_hits;
	ResetMailboxList(targets, target_hits);
}

void CMailboxListPanel::OnAddMailboxList()
{
	// Use browse dialog with multi-selection of mailboxes
	CMboxList found;
	if (CBrowseMailboxDialog::PoseDialog(found))
		AddMboxList(&found);
}

// Add a list of mailboxes
void CMailboxListPanel::AddMboxList(const CMboxList* list)
{
}

void CMailboxListPanel::OnOpenMailboxList()
{
	// This implementation does nothing
}
