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

#include "CBrowseMailboxDialog.h"
#include "CDivider.h"
#include "CGeneralException.h"
#include "CIconTextTable.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMboxRef.h"
#include "CMulberryCommon.h"
#include "CTableScrollbarSet.h"
#include "CStatusWindow.h"
#include "CTwister.h"
#include "CXStringResources.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXDownRect.h>
#include <JXProgressIndicator.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXWindow.h>
#include <JXWindowDirector.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxListPanel::CMailboxListPanel(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mTwisted = true;
	mInProgress = false;
	mParent = NULL;
	mContainer = NULL;
	mStopResize = NULL;
}

// Default destructor
CMailboxListPanel::~CMailboxListPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CMailboxListPanel::SetProgress(unsigned long progress)
{
	mProgress->SetValue(progress);
}

void CMailboxListPanel::SetFound(unsigned long found)
{
	mFound->SetText(cdstring(found));
}

void CMailboxListPanel::SetMessages(unsigned long msgs)
{
	mMessages->SetText(cdstring(msgs));
}

void CMailboxListPanel::SetHitState(unsigned long item, bool hit, bool clear)
{
	mMailboxList->SetIcon(item + 1, clear ? 0 : (hit ? IDI_SEARCH_HIT : IDI_SEARCH_MISS));
}

void CMailboxListPanel::SetInProgress(bool in_progress)
{
	mInProgress = in_progress;

	if (mInProgress)
	{
		mCabinets->Deactivate();
		mAddListBtn->Deactivate();
		mClearListBtn->Deactivate();
		mOptionsBtn->Deactivate();
	}
	else
	{
		mCabinets->Activate();
		mAddListBtn->Activate();
		mClearListBtn->Activate();
		mOptionsBtn->Activate();
	}
}

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CMailboxListPanel::OnCreate(JXWindowDirector* parent, JXWidgetSet* container, JXWidgetSet* stop_resize)
{
	mParent = parent;
	mContainer = container;
	mStopResize = stop_resize;

	// Get all controls
// begin JXLayout1

    mTwister =
        new CTwister(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 4,7, 16,16);
    assert( mTwister != NULL );

    mShowCaption =
        new JXStaticText("Show Mailbox Information", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,8, 170,16);
    assert( mShowCaption != NULL );
    const JFontStyle mShowCaption_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mShowCaption->SetFontStyle(mShowCaption_style);

    mHideCaption =
        new JXStaticText("Hide Mailbox Information", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,8, 170,16);
    assert( mHideCaption != NULL );
    const JFontStyle mHideCaption_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mHideCaption->SetFontStyle(mHideCaption_style);

    CDivider* obj1 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 4,27, 492,2);
    assert( obj1 != NULL );

    mMailboxArea =
        new JXWidgetSet(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,35, 500,145);
    assert( mMailboxArea != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Add Cabinet:", mMailboxArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,4, 81,16);
    assert( obj2 != NULL );

    mCabinets =
        new HPopupMenu("",mMailboxArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 95,2, 30,20);
    assert( mCabinets != NULL );

    mAddListBtn =
        new JXTextButton("Add...", mMailboxArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 150,0, 65,25);
    assert( mAddListBtn != NULL );

    mClearListBtn =
        new JXTextButton("Clear", mMailboxArea,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 225,0, 65,25);
    assert( mClearListBtn != NULL );

    mOptionsBtn =
        new JXTextButton("Options...", mMailboxArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 423,0, 70,25);
    assert( mOptionsBtn != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(mMailboxArea,
                    JXWidget::kHElastic, JXWidget::kVElastic, 8,30, 484,115);
    assert( sbs != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Total:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 8,182, 40,16);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);

    mTotal =
        new JXStaticText("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 50,182, 40,16);
    assert( mTotal != NULL );
    mTotal->SetFontSize(10);

    JXStaticText* obj4 =
        new JXStaticText("Found:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 93,182, 40,16);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);

    mFound =
        new JXStaticText("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 135,182, 40,16);
    assert( mFound != NULL );
    mFound->SetFontSize(10);

    JXStaticText* obj5 =
        new JXStaticText("Messages:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 178,182, 55,16);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);

    mMessages =
        new JXStaticText("", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 235,182, 40,16);
    assert( mMessages != NULL );
    mMessages->SetFontSize(10);

    mProgress =
        new JXProgressIndicator(this,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 280,180, 210,16);
    assert( mProgress != NULL );

// end JXLayout1

	// Create the table
	sbs->NoTitles();
	mMailboxList = new CIconTextTable(sbs,sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kVElastic,
										0,0, 10, 10);
	mMailboxList->OnCreate();
	mMailboxList->SetDoubleClickMsg(true);

	// Initialise items
	InitCabinets();

	// Options, progress is hidden by default
	mOptionsBtn->Hide();
	mProgress->Hide();

} // CMailboxListPanel::FinishCreateSelf

#pragma mark ____________________________Commands

void CMailboxListPanel::OnTwist()
{
	int moveby = mMailboxArea->GetFrameHeight();
	if (mTwisted)
	{
		mHideCaption->Hide();
		mShowCaption->Show();
		mMailboxArea->Hide();
		if (mContainer)
		{
			mContainer->AdjustSize(0, -moveby);
			mContainer->Move(0, moveby);
		}
		
		// Must prevent resize of dynamic height item when window size changes
		if (mStopResize)
			mStopResize->AdjustSize(0, moveby);
		mParent->GetWindow()->AdjustSize(0, -moveby);
	}
	else
	{
		mShowCaption->Hide();
		mHideCaption->Show();
		mMailboxArea->Show();
		if (mContainer)
		{
			mContainer->AdjustSize(0, moveby);
			mContainer->Move(0, -moveby);
		}
		
		// Must prevent resize of dynamic height item when window size changes
		mParent->GetWindow()->AdjustSize(0, moveby);
		if (mStopResize)
			mStopResize->AdjustSize(0, -moveby);
	}

	mTwisted = !mTwisted;
}

#pragma mark ____________________________Mailbox List

void CMailboxListPanel::InitCabinets()
{
	// Remove any existing items from main menu
	mCabinets->RemoveAllItems();

	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
		mCabinets->AppendItem((*iter)->GetName(), kFalse, kFalse);
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
			item_states.push_back(IDI_SEARCH_HIT);
			break;
		case eMiss:
			item_states.push_back(IDI_SEARCH_MISS);
			break;
		}
	}

	mMailboxList->SetContents(items, item_states);
	mTotal->SetText(cdstring((unsigned long) items.size()));
	mFound->SetText(cdstring(0UL));
	mMessages->SetText(cdstring(0UL));
}

void CMailboxListPanel::OnClearMailboxList()
{
	CMboxRefList targets;
	ulvector target_hits;
	ResetMailboxList(targets, target_hits);
}

void CMailboxListPanel::OnAddMailboxList()
{
	// Create the dialog
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
