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


// Source for CMailboxView class

#include "CMailboxView.h"

#include "CErrorDialog.h"
#include "CFilterManager.h"
#include "CGetStringDialog.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoTable.h"
#include "CMailboxTitleTable.h"
#include "CMailboxWindow.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessageWindow.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CResources.h"
#include "CStringResources.h"
#endif
#include "CStringUtils.h"
#include "CTaskClasses.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CWindowsMenu.h"
#endif
#include "C3PaneWindow.h"

#if __dest_os == __linux_os
#include <JTableSelection.h>
#endif

#include <algorithm>
#include <numeric>
#include <typeinfo>

extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X V I E W
// __________________________________________________________________________________________________

// Find the corresponding window using URL
CMailboxView* CMailboxView::FindView(const CMbox* mbox)
{
	if (!mbox)
		return NULL;

	// Does window already exist?
	cdmutexprotect<CMailboxViewList>::lock _lock(sMailboxViews);
	cdstring compare_acct = mbox->GetAccountName();
	for(CMailboxViewList::iterator iter = sMailboxViews->begin(); iter != sMailboxViews->end(); iter++)
	{
		// Compare with the one we want
		if ((*iter)->GetMbox() &&
			((*iter)->GetMbox()->GetAccountName() == compare_acct))
		{
			// Set return items if found
			return *iter;
		}
	}

	return NULL;
}

// Count how many views have the mailbox open
unsigned long CMailboxView::CountViews(const CMbox* mbox)
{
	if (!mbox)
		return 0;

	unsigned long ctr = 0;

	// Does window already exist?
	cdmutexprotect<CMailboxViewList>::lock _lock(sMailboxViews);
	cdstring compare_acct = mbox->GetAccountName();
	for(CMailboxViewList::iterator iter = sMailboxViews->begin(); iter != sMailboxViews->end(); iter++)
	{
		// Look for substitiutes
		if ((*iter)->mUseSubstitute)
		{
			if ((*iter)->IsSubstitute(compare_acct))
				ctr++;
		}
		
		// Compare with the default one
		else if ((*iter)->GetMbox() && ((*iter)->GetMbox()->GetAccountName() == compare_acct))
			ctr++;
	}

	return ctr;
}

// Respond to list changes
void CMailboxView::ListenTo_Message(long msg, void* param)
{
	switch (msg)
	{
	case CMboxProtocol::eBroadcast_MailLogoff:
		// Handle close of protocol
		// For some reason we don't do this with 3-pane items
		if (!Is3Pane())
			DoProtocolLogoff(reinterpret_cast<CMboxProtocol*>(param));
		break;
	case CMboxProtocol::eBroadcast_RemoveList:
		// Handle removal of WD
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxList))
			DoRemoveWD(reinterpret_cast<CMboxList*>(param));
		break;
	default:
		CTableView::ListenTo_Message(msg, param);
		break;
	}
}

// Check for window
bool CMailboxView::ViewExists(const CMailboxView* wnd)
{
	cdmutexprotect<CMailboxViewList>::lock _lock(sMailboxViews);
	CMailboxViewList::iterator found = std::find(sMailboxViews->begin(), sMailboxViews->end(), wnd);
	return found != sMailboxViews->end();
}

CMailboxWindow* CMailboxView::GetMailboxWindow() const
{
	return !Is3Pane() ? static_cast<CMailboxWindow*>(GetOwningWindow()) : NULL;
}

void CMailboxView::ForceClose(CMbox* mbox)
{
	// Have table do force close if this is the active mailbox
	if (GetMbox() && (GetMbox() == mbox) && GetTable())
		GetTable()->ForceClose();
}

void CMailboxView::ForceClose(CMboxProtocol* proto)
{
	// Have table do force close if this is the active mailbox
	if (GetMbox() && GetMbox()->TestMailer(proto) && GetTable())
		GetTable()->ForceClose();
}

// Is mailbox one of the substitutes
bool CMailboxView::ContainsMailbox(const CMbox* mbox)
{
	if (mUseSubstitute)
		return IsSubstitute(mbox);
	else
		return GetMbox() && (GetMbox() == mbox);
}

// Is proto used by any substitutes
bool CMailboxView::ContainsProtocol(const CMboxProtocol* proto)
{
	if (mUseSubstitute)
		return IsSubstitute(proto);
	else
		return GetMbox() && GetMbox()->TestMailer(proto);
}

// Existing substitution?
bool CMailboxView::IsSubstitute(const CMbox* mbox) const
{
	// Look for existing substitute item
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
			return true;
	}
	
	return false;
}

// Existing substitution?
bool CMailboxView::IsSubstitute(const CMboxProtocol* proto) const
{
	// Look for existing substitute item
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox->TestMailer(proto))
			return true;
	}
	
	return false;
}

// Existing substitution?
bool CMailboxView::IsSubstitute(const cdstring& acctname) const
{
	// Look for existing substitute item
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox->GetAccountName() == acctname)
			return true;
	}
	
	return false;
}

// Get index of item
unsigned long CMailboxView::GetSubstituteIndex(const CMbox* mbox) const
{
	// Look for existing substitute item
	unsigned long index = 0;
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
			break;
	}
	
	return index;
}

// Get index of item
unsigned long CMailboxView::GetSubstituteIndex(const CMboxProtocol* proto) const
{
	// Look for existing substitute item
	unsigned long index = 0;
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox->GetMsgProtocol() == proto)
			break;
	}
	
	return index;
}

// Get index of item
unsigned long CMailboxView::GetSubstituteIndex(const CMboxList* wd) const
{
	// Look for existing substitute item
	unsigned long index = 0;
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox->GetMboxList() == wd)
			break;
	}
	
	return index;
}

// Get index of dynamic item
unsigned long CMailboxView::GetDynamicIndex() const
{
	// Look for existing substitute item
	unsigned long index = 0;
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mDynamic)
			return index;
	}
	
	// Return one past the end of existing items
	return index;
}

// See if dynamic item exists
bool CMailboxView::HasDynamicItem() const
{
	// Look for existing substitute item
	unsigned long index = 0;
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mDynamic)
			return true;
	}
	
	return false;
}

// New or existing mailbox substitution
void CMailboxView::OpenSubstitute(CMbox* mbox, bool select)
{
	// Look for existing substitute item
	SMailboxSubstitute* subs = NULL;
	unsigned long index = 0;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
		{
			subs = &(*iter);
			break;
		}
	}

	// Use existing info if present - otherwise add new info
	if (subs)
	{
		if (select)
		{
			// Check whether it has previously been viewed
			bool viewed_before = subs->mLastAccess;

			// Broadcast change to select new tab item - do this before the rest
			// as this can result in the view being moved if tabs are shown
			Broadcast_Message(eBroadcast_SelectSubstitute, &index);
			mSubstituteIndex = index;
			subs->mLastAccess = mSubstituteAccess++;

			// Do InitPos if this is the first view
			if (viewed_before)
			{
				{
					// Don't allow a selection change notification while we do this
					// as that may cause the preview to be triggered. Instead we do a manual
					// update of the state after this block is done to make sure
					// toolbar stays in sync with any selection
					CMailboxTable::StNoSelectionChanged noSelChange(GetTable());

					// Recover selection
					GetTable()->SetPreservedMessages(subs->mSelected);
					GetTable()->ResetSelection(false);

					// Recover scroll pos
					GetTable()->SetScrollPos(subs->mScrollHpos, subs->mScrollVpos);
					
					// Do preview of the last one shown, restoring its state
					GetTable()->PreviewMessageUID(subs->mPreviewUID, &subs->mMessageView);
				}
				
				// Now broadcast selection change to update toolbar
				GetTable()->UpdateState();
				
				// Now do auto-scroll for new unseen messages
				GetTable()->ScrollForNewMessages();
			}
			else
				// Do initial positioning of selection
				GetTable()->InitPos();
		}
	}
	else
	{
		// Setup new info and add to list
		SMailboxSubstitute subs;
		subs.mMailbox = mbox;
		
		// Set it as dynamic if dynamic in use and no others are dynamic
		// and its not the first item
		if (mUseDynamicTab && !HasDynamicItem() && mSubsList.size())
			subs.mDynamic = true;

		// Last access set to zero until this item has been viewed once
		// This allows us to trigger InitPos on the mailbox the first time it
		// is viewed
		subs.mLastAccess = select ? mSubstituteAccess++ : 0;
		
		mSubsList.push_back(subs);
		
		// Get aliased name of mailbox
		cdstring name = mbox->GetAccountName(true);
		if (!CPreferences::sPrefs->MapMailboxAlias(name))
			name = mbox->GetShortName();

		// Broadcast change to add new tab item
		SBroadcast_AddSubstitute add;
		add.mName = name.c_str();
		add.mUnseenIcon = mbox->AnyNew();
		Broadcast_Message(eBroadcast_AddSubstitute, (void*) &add);

		// Select it if required
		if (select)
		{
			Broadcast_Message(eBroadcast_SelectSubstitute, &index);
			mSubstituteIndex = index;
		}
	}
}

// Add the subs items to the current list
void CMailboxView::SaveSubstitute(CMbox* mbox)
{
	// Look for existing substitute item
	SMailboxSubstitute* subs = NULL;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
		{
			subs = &(*iter);
			break;
		}
	}

	// Must have item
	if (!subs)
		return;

	// Add mailbox info to the substitute list
	GetTable()->GetScrollPos(subs->mScrollHpos, subs->mScrollVpos);
	
	// Preserve current selection , grab the preserved messages, and then unpreserve
	GetTable()->PreserveSelection();
	subs->mSelected = GetTable()->GetPreservedMessages();
	GetTable()->UnpreserveSelection();

	subs->mPreviewUID = GetTable()->GetPreviewMessageUID();
	
	// Save the message preview state
	GetPreview()->GetViewState(subs->mMessageView);
}

// Change substitution item
void CMailboxView::ChangeSubstitute(CMbox* oldmbox, CMbox* newmbox)
{
	// Look for existing substitute item
	SMailboxSubstitute* subs = NULL;
	unsigned long index = 0;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == oldmbox)
		{
			subs = &(*iter);
			break;
		}
	}

	// Must have item
	if (!subs)
		return;

	// Change the item
	subs->mMailbox = newmbox;
	subs->mScrollHpos = -1;
	subs->mScrollVpos = -1;
	subs->mSelected.clear();
	subs->mPreviewUID = 0;
	subs->mLastAccess = mSubstituteAccess++;

	// Get aliased name of new mailbox
	cdstring name = newmbox->GetAccountName(true);
	if (!CPreferences::sPrefs->MapMailboxAlias(name))
		name = newmbox->GetShortName();

	// Broadcast change to change tab item
	SBroadcast_ChangeSubstitute change;
	change.mIndex = index;
	change.mName = name;
	change.mUnseenIcon = newmbox->AnyNew();
	Broadcast_Message(eBroadcast_ChangeSubstitute, &change);
}

// Change substitution item icon
void CMailboxView::ChangeIconSubstitute(CMbox* mbox, bool icon)
{
	// Look for existing substitute item
	SMailboxSubstitute* subs = NULL;
	unsigned long index = 0;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
		{
			subs = &(*iter);
			break;
		}
	}

	// Must have item
	if (!subs)
		return;

	// Broadcast change to change tab item
	SBroadcast_ChangeIconSubstitute change;
	change.mIndex = index;
	change.mUnseenIcon = mbox->AnyNew();
	Broadcast_Message(eBroadcast_ChangeIconSubstitute, &change);
}

// Move substitution item
void CMailboxView::MoveSubstitute(unsigned long oldindex, unsigned long newindex)
{
	// Only if different
	if (oldindex == newindex)
		return;

	// Rotate the vector
	std::rotate(mSubsList.begin() + std::min(oldindex, newindex),
				(oldindex < newindex) ? mSubsList.begin() + (oldindex + 1) : mSubsList.begin() + oldindex,
				mSubsList.begin() + (std::max(oldindex, newindex) + 1));

	// Now change the value if it moved
	if ((mSubstituteIndex >= std::min(oldindex, newindex)) && (mSubstituteIndex <std::max(oldindex, newindex)))
	{
		if (mSubstituteIndex == oldindex)
			mSubstituteIndex = newindex;
		else
			mSubstituteIndex += (newindex > oldindex ? -1 : 1);
	}

	// Broadcast change to move tab item
	SBroadcast_MoveSubstitute move;
	move.mOldIndex = oldindex;
	move.mNewIndex = newindex;
	Broadcast_Message(eBroadcast_MoveSubstitute, &move);
}

// Rename substitution item
void CMailboxView::RenameSubstitute(unsigned long index)
{
	// Get current item
	CMbox* mbox = mSubsList.at(index).mMailbox;
	
	// Get aliased name of mailbox
	cdstring acctname = mbox->GetAccountName(true);
	cdstring name = acctname;
	if (!CPreferences::sPrefs->MapMailboxAlias(name))
		name = mbox->GetShortName();

	// Now prompt user for new name
	if (CGetStringDialog::PoseDialog("Alerts::Mailbox::RenameSubstitute", name))
	{
		// Update prefs
		CPreferences::sPrefs->mMailboxAliases.Value()[acctname] = name;
		CPreferences::sPrefs->mMailboxAliases.SetDirty(true);

		// Broadcast change to rename tab item
		SBroadcast_ChangeSubstitute change;
		change.mIndex = index;
		change.mName = name.c_str();
		change.mUnseenIcon = mbox->AnyNew();
		Broadcast_Message(eBroadcast_ChangeSubstitute, &change);
	}
}

// Rename substitution item
void CMailboxView::RenamedSubstitute(unsigned long index)
{
	// Get current item
	CMbox* mbox = mSubsList.at(index).mMailbox;
	
	// Get aliased name of mailbox
	cdstring acctname = mbox->GetAccountName(true);
	cdstring name = acctname;
	if (!CPreferences::sPrefs->MapMailboxAlias(name))
		name = mbox->GetShortName();

	// Broadcast change to rename tab item
	SBroadcast_ChangeSubstitute change;
	change.mIndex = index;
	change.mName = name.c_str();
	change.mUnseenIcon = mbox->AnyNew();
	Broadcast_Message(eBroadcast_ChangeSubstitute, &change);
}

// Is subsitiute locked
bool CMailboxView::IsSubstituteLocked(unsigned long index) const
{
	return mSubsList.at(index).mLocked;
}

// Lock substitution item
void CMailboxView::LockSubstitute(unsigned long index)
{
	// Toggle state
	mSubsList.at(index).mLocked = !mSubsList.at(index).mLocked;
	
	// Don't allow item to be locked and dynamic
	if (IsSubstituteLocked(index) && IsSubstituteDynamic(index))
	{
		// Just turn this one off but leave the use dynamic flag on
		// so the next item opened becomes the dynamic one
		mSubsList.at(index).mDynamic = false;
	}
}

// Lock substitution item
void CMailboxView::LockAllSubstitutes()
{
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		// Toggle state
		(*iter).mLocked = true;
		
		// Don't allow item to be locked and dynamic
		if ((*iter).mDynamic)
		{
			// Just turn this one off but leave the use dynamic flag on
			// so the next item opened becomes the dynamic one
			(*iter).mDynamic = false;
		}
	}
}

// Is subsitiute dynamic
bool CMailboxView::IsSubstituteDynamic(unsigned long index) const
{
	return mSubsList.at(index).mDynamic;
}

// Lock substitution item
void CMailboxView::DynamicSubstitute(unsigned long index)
{
	// Look at all items
	unsigned long ctr_index = 0;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, ctr_index++)
	{
		// Turn off if not the one being changed, toggle if it is the one
		if (ctr_index == index)
		{
			// Toggle state
			mSubsList.at(ctr_index).mDynamic = !mSubsList.at(ctr_index).mDynamic;
			
			// Set tab use based on whether its on or off
			mUseDynamicTab = mSubsList.at(ctr_index).mDynamic;
		}
		else
			// Turn it off
			mSubsList.at(ctr_index).mDynamic = false;
	}
}

// Save substitution items
void CMailboxView::SaveSubstitutes()
{
	cdstrvect temp;
	GetSubstituteItems(temp);
	CPreferences::sPrefs->m3PaneOpenMailboxes.SetValue(temp);
}

// Restore substitution items
void CMailboxView::RestoreSubstitutes()
{
	SetSubstituteItems(CPreferences::sPrefs->m3PaneOpenMailboxes.GetValue());
}

// Add the subs items to the current list
void CMailboxView::CloseSubstitute(CMbox* mbox)
{
	// Look for existing substitute item
	SMailboxSubstitute* subs = NULL;
	unsigned long index = 0;
	for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
	{
		// Look for matching mailbox
		if ((*iter).mMailbox == mbox)
		{
			mSubsList.erase(iter);
			
			// Broadcast change to remove new tab item
			Broadcast_Message(eBroadcast_RemoveSubstitute, &index);
			
			// If this was the one last selected, switch to the last one viewed
			if (mSubstituteIndex == index)
			{
				// Any left?
				if (mSubsList.size())
				{
					// Find the one from the remainder with the highest access value (i.e. it
					// was the last one selected by the user)
					unsigned long max_index = 0;
					unsigned long max_value = 0;
					index = 0;
					for(SMailboxSubstituteList::iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++, index++)
					{
						if ((*iter).mLastAccess > max_value)
						{
							max_index = index;
							max_value = (*iter).mLastAccess;
						}
					}
					
					// Now forcibly select this one and broadcast change
					mSubstituteIndex = ULONG_MAX;
					Broadcast_Message(eBroadcast_SelectSubstitute, &max_index);
					SetSubstitute(max_index);
				}
				else
				{
					// Empty the display
					mSubstituteIndex = 0;
					SetMbox(NULL);
				}
			}
			
			// Adjust substitute index for removal
			else if (index < mSubstituteIndex)
				mSubstituteIndex--;
			break;
		}
	}
}

// Set this as the current substitute view
void CMailboxView::SetSubstitute(unsigned long index)
{
	// Only if different
	if ((mSubstituteIndex != index) && (index < mSubsList.size()))
	{
		mSubstituteIndex = index;

		// Now get it and change it
		Substitute(mSubsList.at(index).mMailbox);
	}
}

// Cycle to next tab in the list
void CMailboxView::CycleSubstitute(bool forward)
{
	// Only bother if tabs in use
	if (!GetUseSubstitute())
		return;

	unsigned long index = mSubstituteIndex;
	if (forward)
	{
		index++;
		if (index >= mSubsList.size())
			index = 0;
	}
	else
	{
		if (index != 0)
			index--;
		else
			index = mSubsList.size() - 1;
	}
	
	// Now change it
	SetSubstitute(index);
}

// Get list of current items
void CMailboxView::GetSubstituteItems(cdstrvect& items) const
{
	items.clear();
	
	// First write out current dynamic index
	items.push_back(mUseDynamicTab ? cValueBoolTrue : cValueBoolFalse);

	// Add each mailbox account name to list - include lock state
	for(SMailboxSubstituteList::const_iterator iter = mSubsList.begin(); iter != mSubsList.end(); iter++)
	{
		items.push_back((*iter).mMailbox->GetAccountName());
		items.push_back((*iter).mLocked ? cValueBoolTrue : cValueBoolFalse);
		items.push_back((*iter).mDynamic ? cValueBoolTrue : cValueBoolFalse);
	}
}

// Set list of items
void CMailboxView::SetSubstituteItems(const cdstrvect& items)
{
	// Check validity of input, non-zero and one plus multiple of three
	if (!items.size() || ((items.size() - 1) % 3 != 0))
		return;

	// Add each item to list, resolving name
	try
	{
		// Get dynamic index first
		// Don't use dynamic until after the previous set of tabs has been recovered
		mUseDynamicTab = false;
		bool use_dynamic = !::strcmpnocase(items.front(), cValueBoolTrue);

		for(cdstrvect::const_iterator iter = items.begin() + 1; iter != items.end(); iter++)
		{
			// Create mailbox ref
			CMbox* mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(*iter++);
			
			// Get lock state
			bool locked = !::strcmpnocase(*iter++, cValueBoolTrue);
			bool dynamic = !::strcmpnocase(*iter, cValueBoolTrue);

			// Add as substitute if it exists (do not select it)
			// Only do this if the protocol is already logged in (i.e. login at startup is on)
			if (mbox && mbox->GetProtocol()->IsLoggedOn())
			{
				// Do subsititution (may already be present so ignore duplicate)
				if (GetSubstituteIndex(mbox) == mSubsList.size())
					Substitute(mbox, false);
				
				// Handle lock state (change it on existing item if different)
				if (locked ^ IsSubstituteLocked(GetSubstituteIndex(mbox)))
					LockSubstitute(GetSubstituteIndex(mbox));
				
				// Handle dynamic state (change it on existing item if different)
				if (dynamic ^ IsSubstituteDynamic(GetSubstituteIndex(mbox)))
					DynamicSubstitute(GetSubstituteIndex(mbox));
			}
		}

		// Redo current selection to ensure lock state is up to date
		Broadcast_Message(eBroadcast_SelectSubstitute, &mSubstituteIndex);

		// Now change the dynamic state
		mUseDynamicTab = use_dynamic;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Catch all
	}
}

// Get list of current (non-tab) items
void CMailboxView::GetStaticItems(cdstrvect& items) const
{
	items.clear();
	
	// Add the mailbox account name to list if it exists
	if (GetMbox())
		items.push_back(GetMbox()->GetAccountName());
}

// Set list of (non-tab) items
void CMailboxView::SetStaticItems(const cdstrvect& items)
{
	if (items.size())
	{
		// Create mailbox ref
		CMbox* mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(items.front());

		// Add as view if it exists
		// Only do this if the protocol is already logged in (i.e. login at startup is on)
		if (mbox && mbox->GetProtocol()->IsLoggedOn())
		{
			// Do view
			ViewMbox(mbox);
		}
	}
}

// Get its Mbox
CMbox* CMailboxView::GetMbox() const
{
	// Check that table exists - this method may get called from
	// another thread while a window is being created
	return GetTable() ? GetTable()->GetMbox() : NULL;
}

// Return user action data
const CUserAction& CMailboxView::GetPreviewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsPreview);
}

// Return user action data
const CUserAction& CMailboxView::GetFullViewAction() const
{
	return CPreferences::sPrefs->Get3PaneOptions().GetValue().GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView);
}

void CMailboxView::SetUsePreview(bool use_view)
{
	// Only if different
	if (mUsePreview == use_view)
		return;

	// Clear preview pane if its being hidden
	if (mUsePreview)
		GetTable()->PreviewMessage(true);

	mUsePreview = use_view;
	
	// Update preview pane if its being shown
	if (mUsePreview)
		GetTable()->SelectionChanged();
}

void CMailboxView::SetUseSubstitute(bool use_substitute)
{
	// Only if different
	if (mUseSubstitute == use_substitute)
		return;

	// Can only do substitute if 3-pane
	mUseSubstitute = Is3Pane() ? use_substitute : false;
}

// Reset the table
void CMailboxView::ResetTable()
{
	GetTable()->ResetTable();
}

// Reset the table
void CMailboxView::ResetTableNew()
{
	GetTable()->ResetTable(true);
}

// Message status changed (this means flags)
void CMailboxView::ChangedMessage(const CMessage* aMsg)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	long msg_index = mbox->GetMessageIndex(aMsg, true);

	// NB May have an index > number of rows during filtering as table
	// has not yet been updated to account for new messages being filtered
	if (msg_index && (msg_index <= GetTable()->GetItemCount()))
	{
		if (GetSortBy() == cSortMessageFlags)
			FRAMEWORK_REFRESH_WINDOW(GetTable())
		else
			GetTable()->RefreshRow(msg_index - TABLE_ROW_ADJUST);
	}
}

// Select msg cell in table
void CMailboxView::SelectMessage(const CMessage* aMsg)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Do not do selection change notification when programmatically setting the selection
	CMailboxTable::StNoSelectionChanged _defer(GetTable());

	long msg_index = mbox->GetMessageIndex(aMsg, true);

	if (msg_index)
	{
		STableCell theCell(msg_index, 1);
		GetTable()->UnselectAllCells();
		GetTable()->FakeClickSelect(theCell, false);
		GetTable()->ScrollCellIntoFrame(theCell);
		
		// Trigger refresh of UI items
		GetTable()->UpdateItems();
	}
}

// Select next new message
void CMailboxView::SelectNextNew(bool backwards)
{
	// Does nothing here.
}

#ifdef __use_speech
// Speak recent messages in mailbox
void CMailboxView::SpeakRecentMessages()
{
	GetTable()->DoSpeakRecentMessages();
}
#endif

// Init columns and text
void CMailboxView::InitColumns()
{
	// Remove any previous columns
	mColumnInfo.clear();

	// Create initial columns

	// Flags, width = 16
	AppendColumn(eMboxColumnFlags, 16);

	// Number, width = 38
	AppendColumn(eMboxColumnNumber, 38);

	// Smart, width = 104
	AppendColumn(eMboxColumnSmart, 104);

	// Date, width = 104
	AppendColumn(eMboxColumnDateSent, 104);

	// Subject, width = 204
	AppendColumn(eMboxColumnSubject, 204);

}

// Get sort type
int CMailboxView::GetSortBy()
{
	// Get sort method
	CMbox* mbox = GetMbox();
	if (!mbox)
		return cSortMessageNumber;

	return mbox->GetSortBy();
}

// Force change of sort
void CMailboxView::SetSortBy(int sort)
{
	bool scroll_to_top = false;
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Check that new sort method is valid
	if (!mbox->ValidSort((ESortMessageBy) sort))
	{

		// Put up alert
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::FullSort",
																		"ErrorDialog::Btn::CurrentSort",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::CachedSort", 3);

		if (result == CErrorDialog::eBtn1)
			// Cache entire contents then allow sort
			mbox->CacheAllMessages();
		else if (result == CErrorDialog::eBtn2)
		{
			// Scroll window to top after sort
			scroll_to_top = true;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Must update port here to prevent excess port update causes more caching than required
			if (IsVisible())
				UpdatePort();
#endif
		}
		else if (result == CErrorDialog::eBtn3)
			// Do nothing!
			return;
	}

	// Preserve selection
	StMailboxTableSelection preserve_selection(GetTable());

	//StProfileSection profile("\pMulberry Profile", 200, 20);
	// Change sort method by sending message to table

	// Look for change in list size due to sorting (threading may insert holes)
	if (mbox->SortBy((ESortMessageBy) sort))
		GetTable()->ResetTable();
	else
		FRAMEWORK_REFRESH_WINDOW(GetTable())

	// Awlays refresh title
	FRAMEWORK_REFRESH_WINDOW(GetTitles());

	if (scroll_to_top)
		// Don't force update of existing update region before scroll
		GetTable()->ScrollCellIntoFrame(STableCell(1, 1), false);
		//GetTitles()->Refresh();
}

// Get sort direction
int CMailboxView::GetShowBy()
{
	// Get sort method
	CMbox* mbox = GetMbox();
	if (!mbox)
		return cShowMessageAscending;

	return mbox->GetShowBy();
}

// Force change of sort
void CMailboxView::SetShowBy(int show)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Preserve selection
	StMailboxTableSelection preserve_selection(GetTable());

	// Change sort method by sending message to table
	mbox->ShowBy(static_cast<EShowMessageBy>(show));
	FRAMEWORK_REFRESH_WINDOW(GetTable())
}
