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


// Source for CServerBrowse class

#include "CServerBrowse.h"

#include "CCreateMailboxDialog.h"
#include "CCopyToMenu.h"
#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CIMAPCommon.h"
#include "CINETCommon.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CNamespaceDialog.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#endif
#include "CStringUtils.h"
#include "CTCPException.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CWaitCursor.h"

#include "CStringResources.h"

#elif __dest_os == __linux_os
#include "CWaitCursor.h"
#include <JTableSelection.h>
#endif

#include <typeinfo>

#if __dest_os == __win32_os
// BUG in Metrowerks Compiler - need this specialization
inline void allocator<CServerBrowse::SServerBrowseData>::deallocate(CServerBrowse::SServerBrowseData* p, size_t)
{
	delete p;
}
#endif

#pragma mark ____________________________Message Handling

void CServerBrowse::ListenTo_Message(long msg, void* param)
{
	switch (msg)
	{
	case CMailAccountManager::eBroadcast_RefreshAll:
		FRAMEWORK_REFRESH_WINDOW(this)
		break;
	case CMailAccountManager::eBroadcast_BeginMailAccountChange:
		//mListChanging = true;
		break;
	case CMailAccountManager::eBroadcast_EndMailAccountChange:
		//mListChanging = false;
		//Refresh();
		break;
	case CMailAccountManager::eBroadcast_NewMailAccount:
		// Only if manager
		if (mManager)
			AddServer(static_cast<CMboxProtocol*>(param));
		break;
	case CMailAccountManager::eBroadcast_RemoveMailAccount:
		// Only if manager
		if (mManager)
			RemoveServer(static_cast<CMboxProtocol*>(param));
		break;
	case CMailAccountManager::eBroadcast_ChangedMbox:
		ChangedMbox(static_cast<CMbox*>(param));
		break;

	case CMboxProtocol::eBroadcast_MailLogon:
		ChangedServer(static_cast<CMboxProtocol*>(param));
		break;
	case CMboxProtocol::eBroadcast_MailLogoff:
		ChangedServer(static_cast<CMboxProtocol*>(param));
		break;
	case CMboxProtocol::eBroadcast_NewList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxList))
			AddWD(static_cast<CMboxList*>(param));
		else if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
		{
			// If list is visible then must add it to the server table now as it was
			// not previousoly added
			if (static_cast<CMboxRefList*>(param)->IsVisible())
				AddFavourite(static_cast<CMboxRefList*>(param));
			else
			{
				// Make sure listening to the list - but do not add it (visible flag toggle will add)
				static_cast<CMboxRefList*>(param)->Add_Listener(this);
				static_cast<CMboxRefList*>(param)->Add_Listener(CCopyToMenu::sMailboxMainMenu);
			}
		}
		break;
	case CMboxProtocol::eBroadcast_RemoveList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxList))
			RemoveWD(static_cast<CMboxList*>(param));
		else if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
			RemoveFavourite(static_cast<CMboxRefList*>(param));
		break;

	case CTreeNodeList::eBroadcast_ResetList:
		if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxList))
			RefreshWD(static_cast<CMboxList*>(param));
		else if (typeid(*static_cast<CTreeNodeList*>(param)) == typeid(CMboxRefList))
			RefreshFavourite(static_cast<CMboxRefList*>(param));
		break;
	case CTreeNodeList::eBroadcast_ShowList:
		AddFavourite(static_cast<CMboxRefList*>(param));
		break;
	case CTreeNodeList::eBroadcast_HideList:
		RemoveFavourite(static_cast<CMboxRefList*>(param));
		break;

	case CTreeNodeList::eBroadcast_AddNode:
		{
			CTreeNodeList::SBroadcastAddNode* info = (CTreeNodeList::SBroadcastAddNode*) param;
			AddedNode(info->mList, info->mIndex, info->mNumber);
			break;
		}
	case CTreeNodeList::eBroadcast_ChangeNode:
		{
			CTreeNodeList::SBroadcastChangeNode* info = (CTreeNodeList::SBroadcastChangeNode*) param;
			ChangedNode(static_cast<CTreeNodeList*>(info->mList), info->mIndex);
			break;
		}
	case CTreeNodeList::eBroadcast_DeleteNode:
		{
			CTreeNodeList::SBroadcastDeleteNode* info = (CTreeNodeList::SBroadcastDeleteNode*) param;
			DeletedNode(static_cast<CTreeNodeList*>(info->mList), info->mIndex);
			break;
		}
	}
}

#pragma mark ____________________________Management

// Set the mboxList
void CServerBrowse::SetServer(CMboxProtocol* server)
{
	// Save server
	mManager = false;
	mSingle = true;
	mShowFavourites = false;
	mRecordExpansion = false;
	mServer = server;

	// Calculate number of rows for first time
	ResetTable();
}

// Set the mboxList
CMboxProtocol* CServerBrowse::GetSingleServer(void) const
{
	// Return single server if not manager
	if (!mManager)
		return mServer;

	// Only if one or no manager
	if (!mSingle || !CMailAccountManager::sMailAccountManager)
		return NULL;

	// Retrurn first (and only) server
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	return protos.at(0);
}

// Set view state
void CServerBrowse::SetView(long view)
{
	// Change visibility state
	CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(view);
	CMailAccountManager::sMailAccountManager->SetFlag(list, CTreeNodeList::eVisible, !list->IsVisible());
}

#pragma mark ____________________________Expand/collapse

void CServerBrowse::CollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::CollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CServerBrowse::DeepCollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::DeepCollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CServerBrowse::ExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery
	if (GetCellDataType(inWideOpenRow) == eServerBrowseMbox)
	{
		// Set expansion flag
		CMbox* mbox = GetCellMbox(inWideOpenRow);
		if (mbox != NULL)
		{
			if (!mbox->HasExpanded() && mbox->GetProtocol()->IsLoggedOn())
				mbox->GetProtocol()->LoadSubList(mbox, false);
			mbox->SetFlags(NMbox::eHasExpanded);
		}
	}

	CHierarchyTableDrag::ExpandRow(inWideOpenRow);
}

void CServerBrowse::DeepExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery (deep)
	if (GetCellDataType(inWideOpenRow) == eServerBrowseMbox)
	{
		// Set expansion flag
		CMbox* mbox = GetCellMbox(inWideOpenRow);

		// Must be logged in to do listing
		if ((mbox != NULL) && mbox->GetProtocol()->IsLoggedOn())
		{
			// Check current expansion mode
			if (!mbox->HasExpanded())
			{
				// Do deep list off this root mailbox
				mbox->GetProtocol()->LoadSubList(mbox, true);
				mbox->SetFlags(NMbox::eHasExpanded);
				CHierarchyTableDrag::DeepExpandRow(inWideOpenRow);
			}
			else
			{
				// Make sure all children are deep expanded
				// NB This action will cause change in number of children
				UInt32 i = inWideOpenRow;
				UInt32	oneLevelDown = GetNestingLevel(inWideOpenRow) + 1;
				while(GetNestingLevel(++i) >= oneLevelDown)
				{
					// Deep expand immediate children only - others will be hit by recursion
					if ((GetNestingLevel(i) == oneLevelDown) && IsCollapsable(i))
						DeepExpandRow(i);
				}

				// Just expand as normal
				ExpandRow(inWideOpenRow);
			}
		}
		else
			// Just expand whatever we have in logged off state
			CHierarchyTableDrag::DeepExpandRow(inWideOpenRow);
	}
	else if ((GetCellDataType(inWideOpenRow) == eServerBrowseWD) ||
				(GetCellDataType(inWideOpenRow) == eServerBrowseServer))
	{
		// Make sure all children are deep expanded
		// NB This action will cause change in number of children
		UInt32 i = inWideOpenRow;
		UInt32	oneLevelDown = GetNestingLevel(inWideOpenRow) + 1;
		while(GetNestingLevel(++i) >= oneLevelDown)
		{
			// Deep expand immediate children only - others will be hit by recursion
			if ((GetNestingLevel(i) == oneLevelDown) && IsCollapsable(i))
				DeepExpandRow(i);
		}

		// Just expand as normal
		ExpandRow(inWideOpenRow);
	}
	else
		// Just expand whatever we have in logged off state
		CHierarchyTableDrag::DeepExpandRow(inWideOpenRow);
}

void CServerBrowse::ProcessExpansion(UInt32 inWideOpenRow, bool expand)
{
	// Only process in prefs if this is the main server window
	if (mRecordExpansion)
	{
		// Set expansion flags in prefs items
		switch(GetCellDataType(inWideOpenRow))
		{
		case eServerBrowseServer:
		{
			bool was_expanded = ResolveCellServer(inWideOpenRow)->GetMailAccount()->GetExpanded();
			if (was_expanded ^ expand)
			{
				ResolveCellServer(inWideOpenRow)->GetMailAccount()->SetExpanded(expand);

				// Mark prefs as dirty
				CPreferences::sPrefs->mMailAccounts.SetDirty();
			}
			break;
		}
		case eServerBrowseWD:
		case eServerBrowseSubs:
			ResolveCellServer(inWideOpenRow)->SetFlagWD(GetCellMboxList(inWideOpenRow), CTreeNodeList::eExpanded, expand);
			break;
		case eServerBrowseMboxRefList:
			CMailAccountManager::sMailAccountManager->SetFlag(GetCellMboxRefList(inWideOpenRow), CTreeNodeList::eExpanded, expand);
			break;
		case eServerBrowseMbox:
			if (GetCellMbox(inWideOpenRow) != NULL)
				GetCellMbox(inWideOpenRow)->SetIsExpanded(expand);
			break;
		case eServerBrowseMboxRef:
			if (GetCellMboxRef(inWideOpenRow) != NULL)
				GetCellMboxRef(inWideOpenRow)->SetIsExpanded(expand);
			break;
		default:
			break;
		}
	}
}

#pragma mark ____________________________Retrieve data

CServerBrowse::EServerBrowseDataType CServerBrowse::GetCellDataType(TableIndexT woRow) const	// Check data type
{
	EServerBrowseDataType type = mData.at(woRow - 1).GetType();

	// Fake INBOX as Mbox
	if (type == eServerBrowseINBOX)
		return eServerBrowseMbox;
	else
		return type;
}

// Get mbox
CMbox* CServerBrowse::GetCellMbox(TableIndexT woRow) const
{
	long index = mData.at(woRow - 1).mType;
	if ((index & eServerBrowseMask) == eServerBrowseINBOX)
		return ((CMboxProtocol*) mData.at(woRow - 1).mData)->GetINBOX();
	else
	{
		index &= eServerBrowseIndex;
		CMboxList* list = (CMboxList*) mData.at(woRow - 1).mData;
		return (index < list->size() ? dynamic_cast<CMbox*>(list->at(index)) : NULL);
	}
}

// Get mbox ref
CMboxRef* CServerBrowse::GetCellMboxRef(TableIndexT woRow) const
{
	unsigned long index = mData.at(woRow - 1).GetIndex();
	CMboxRefList* list = (CMboxRefList*) mData.at(woRow - 1).mData;
	return (index < list->size() ? dynamic_cast<CMboxRef*>(list->at(index)) : NULL);
}

// Get WD
const char* CServerBrowse::GetCellWD(TableIndexT woRow) const
{
	CMboxList* list = (CMboxList*) mData.at(woRow - 1).mData;
	return list->GetName().c_str();
}

// Get mbox list for this wd or mbox
CMboxList* CServerBrowse::GetCellMboxList(TableIndexT woRow) const
{
	return (CMboxList*) mData.at(woRow - 1).mData;
}

// Get mbox list for this wd or mbox
CMboxRefList* CServerBrowse::GetCellMboxRefList(TableIndexT woRow) const
{
	return (CMboxRefList*) mData.at(woRow - 1).mData;
}

// Get server
CMboxProtocol* CServerBrowse::GetCellServer(TableIndexT woRow) const
{
	return (CMboxProtocol*) mData.at(woRow - 1).mData;
}

// Get server
CMboxProtocol* CServerBrowse::ResolveCellServer(TableIndexT woRow, bool mboxrefs) const
{
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseServer:
		return GetCellServer(woRow);
	case eServerBrowseWD:
	case eServerBrowseSubs:
		return GetCellMboxList(woRow)->GetProtocol();
	case eServerBrowseMbox:
		return (IsCellINBOX(woRow) ? GetCellServer(woRow) : GetCellMboxList(woRow)->GetProtocol());
	case eServerBrowseMboxRef:
		if (mboxrefs)
		{
			CMbox* mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
			return mbox ? mbox->GetProtocol() : NULL;
		}

		// Fall through
	case eServerBrowseMboxRefList:
	default:
		return NULL;
	}
}

bool CServerBrowse::IsCellINBOX(TableIndexT woRow) const
{
	return ((EServerBrowseDataType) (mData.at(woRow - 1).mType & eServerBrowseMask) == eServerBrowseINBOX);
}

// Get text for row
const char* CServerBrowse::GetRowText(UInt32 woRow)
{
	EServerBrowseDataType type = GetCellDataType(woRow);

	// Determine name to use
	switch(type)
	{
	case eServerBrowseMbox:
		{
			// Always do INBOX
			CMbox* mbox = GetCellMbox(woRow);
			if (!IsCellINBOX(woRow) && GetCellMboxList(woRow)->IsHierarchic())
				return mbox->GetShortName();
			else
				return mbox->GetName();
		}
	case eServerBrowseMboxRef:
		if (GetCellMboxRef(woRow) != NULL)
		{
			if (GetCellMboxRefList(woRow)->IsHierarchic() && !GetCellMboxRef(woRow)->IsWildcard())
				return GetCellMboxRef(woRow)->GetShortName();
			else
				return GetCellMboxRef(woRow)->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
		}
		else
			return cdstring::null_str;
	case eServerBrowseWD:
		return GetCellWD(woRow);
	case eServerBrowseSubs:
		return sSubscribedName;
	case eServerBrowseMboxRefList:
		return GetCellMboxRefList(woRow)->GetName();
	case eServerBrowseServer:
		return GetCellServer(woRow)->GetAccountName();
	default:
		return cdstring::null_str;
	}
}

// Get text for current tooltip cell
void CServerBrowse::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(cell.row + TABLE_ROW_ADJUST);

	EServerBrowseDataType type = GetCellDataType(woRow);

	// Determine name to use
	switch(type)
	{
	case eServerBrowseMbox:
	{
		CMbox* mbox = GetCellMbox(woRow);
		if (mbox != NULL)
		{
			if (mbox->IsDirectory())
				txt = "Mailbox Container: ";
			else
				txt = "Mailbox: ";
			txt += mbox->GetName();
			if (!mbox->IsDirectory())
			{
				txt += os_endl;
				txt += "Total Messages: ";
				if (mbox->HasStatus())
					txt += cdstring(mbox->GetNumberFound());
				else
					txt += "Unknown";
				txt += os_endl;
				txt += "Unseen Messages: ";
				if (mbox->HasStatus())
					txt += cdstring(mbox->GetNumberUnseen());
				else
					txt += "Unknown";
				txt += os_endl;
				txt += "Status: ";
				if (mbox->IsOpenSomewhere())
					txt += "Open";
				else
					txt += "Closed";
				if (mbox->IsSubscribed())
				{
					txt += ", ";
					txt += "Subscribed";
				}
			}
		}
		break;
	}
	case eServerBrowseMboxRef:
		if (GetCellMboxRef(woRow) != NULL)
		{
			if (GetCellMboxRefList(woRow)->IsHierarchic() && !GetCellMboxRef(woRow)->IsWildcard() && GetCellMboxRef(woRow)->GetWDLevel())
				txt = GetCellMboxRef(woRow)->GetShortName();
			else
				txt = GetCellMboxRef(woRow)->GetAccountName(CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
		}
		break;
	case eServerBrowseWD:
		{
			cdstring temp = GetCellWD(woRow);
			if ((temp == cWILDCARD_NODIR) || (temp == cWILDCARD))
				temp.FromResource("UI::Namespace::Entire");
			txt = temp;
		}
		break;
	case eServerBrowseSubs:
		txt = sSubscribedName;
		break;
	case eServerBrowseMboxRefList:
		txt = GetCellMboxRefList(woRow)->GetName();
		break;
	case eServerBrowseServer:
		txt = "Account: ";
		txt += GetCellServer(woRow)->GetAccountName();
		txt += os_endl;
		txt += "Type: ";
		switch(GetCellServer(woRow)->GetAccountType())
		{
		case CINETAccount::eIMAP:
			txt += "IMAP";
			break;
		case CINETAccount::ePOP3:
			txt += "POP3";
			break;
		case CINETAccount::eLocal:
			txt += "Local";
			break;
		default:;
		}
		switch(GetCellServer(woRow)->GetAccountType())
		{
		case CINETAccount::eIMAP:
		case CINETAccount::ePOP3:
			txt += os_endl;
			txt += "Address: ";
			txt += GetCellServer(woRow)->GetAccount()->GetServerIP();
			if (GetCellServer(woRow)->GetAccount()->GetTLSType() != CINETAccount::eNoTLS)
			{
				txt += os_endl;
				txt += "Using SSL/TLS encryption";
			}
			break;
		default:
			break;
		}
		
		break;
	default:
		txt = cdstring::null_str;
	}
}

#pragma mark ____________________________Server Related

// Test for selected message deleted
bool CServerBrowse::TestSelectionServer(TableIndexT row)
{
	// This is deleted
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseServer);
}

#pragma mark ____________________________Favourite Related

// Test for selected favourites
bool CServerBrowse::TestSelectionFavourite(TableIndexT row)
{
	// Must be a CMboxRefList
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseMboxRefList);
}

// Test for selected favourite items
bool CServerBrowse::TestSelectionFavouriteItems(TableIndexT row)
{
	// Must be a CMboxRef
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseMboxRef);
}

// Test for selected removeable favourites
bool CServerBrowse::TestSelectionFavouriteRemove(TableIndexT row)
{
	// Must be a CMboxRefList in a hierarchy that is not the Recent favourite
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return ((GetCellDataType(woRow) == eServerBrowseMboxRefList) && CMailAccountManager::sMailAccountManager &&
			(CMailAccountManager::sMailAccountManager->IsRemoveableFavouriteType(GetCellMboxRefList(woRow))));
}

// Test for selected removeable favourites
bool CServerBrowse::TestSelectionFavouriteRemoveItems(TableIndexT row)
{
	// Must be a CMboxRef in a hierarchy that is not the Recent favourite
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return ((GetCellDataType(woRow) == eServerBrowseMboxRef) && CMailAccountManager::sMailAccountManager &&
			(CMailAccountManager::sMailAccountManager->IsRemoveableItemFavouriteType(GetCellMboxRefList(woRow))));
}

// Test for selected wildcard favourites
bool CServerBrowse::TestSelectionFavouriteWildcard(TableIndexT row)
{
	// Must be a CMboxRef in a hierarchy that is not the Recent favourite
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return ((GetCellDataType(woRow) == eServerBrowseMboxRefList) && CMailAccountManager::sMailAccountManager &&
			(CMailAccountManager::sMailAccountManager->IsWildcardFavouriteType(GetCellMboxRefList(woRow))));
}

// Test for selected wildcard favourite items
bool CServerBrowse::TestSelectionFavouriteWildcardItems(TableIndexT row)
{
	// Must be a wildcard CMboxRef
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseMboxRef) && (GetCellMboxRef(woRow) != NULL) && (GetCellMboxRef(woRow)->IsWildcard());
}

#pragma mark ____________________________Hierarchy Related

// Test for selected hierarchies only
bool CServerBrowse::TestSelectionHierarchy(TableIndexT row)
{
	// This is deleted
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseWD);
}

// Test for selected hierarchies/directories only
bool CServerBrowse::TestSelectionResetRefresh(TableIndexT row)
{
	// Servers, WDs, Subs and mailboxes allowed
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseServer:
	case eServerBrowseWD:
	case eServerBrowseSubs:
	case eServerBrowseMbox:
		return true;
	default:
		return false;
	}
}

// Refresh hierarchy
void CServerBrowse::DoRefreshHierarchy(void)
{
	if (IsSelectionValid())
		DoToSelection((DoToSelectionPP) &CServerBrowse::RefreshHierarchy);
	else if (mSingle && GetSingleServer()->FlatWD())
	{
		GetSingleServer()->GetHierarchies().at(1)->Refresh();
		GetSingleServer()->LoadList();
	}

#if __dest_os == __win32_os
	// Set focus back to table after button push
	SetFocus();
#endif
}

// Open specific server window
bool CServerBrowse::RefreshHierarchy(TableIndexT row)
{
	// Determine what to do
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Only if actually logged in
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseServer:
		// Always refresh server
		if (GetCellServer(woRow)->IsLoggedOn())
		{
			GetCellServer(woRow)->Refresh();
			return true;
		}
		else
			return false;
	case eServerBrowseWD:
	case eServerBrowseSubs:
	case eServerBrowseMbox:
		// Force it to refresh
		{
			// Get base protocol and appropriate list
			CMboxProtocol* proto = ResolveCellServer(woRow);
			CMboxList* list = (IsCellINBOX(woRow) ? NULL : GetCellMboxList(woRow));
			if (proto->IsLoggedOn())
			{
				if (list)
				{
					list->Refresh();
					proto->LoadList();
				}
				else
					proto->Refresh();
				
				return true;
			}
			else
				return false;
		}
	default:
		return false;
	}
}

#pragma mark ____________________________Mailbox Related

// Test for selected hierarchies only
bool CServerBrowse::TestSelectionInferiors(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseServer:
		// Always create in server
		return true;
	case eServerBrowseWD:
	case eServerBrowseSubs:
		// Check for valid hierarchy
		return GetCellMboxList(woRow)->CanCreateInferiors();
	case eServerBrowseMbox:
		// Must allow inferiors
		return (GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->NoInferiors();
	default:
		return false;
	}
}

// Test for selected mailboxes only
bool CServerBrowse::TestSelectionMbox(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	if (GetCellDataType(woRow) == eServerBrowseMbox)
		return (GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->NoSelect();
	else
		return false;
}

// Test for selected disconnectable mailboxes only
bool CServerBrowse::TestSelectionMboxDisconnected(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	CMbox* mbox = NULL;
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseMbox:
		mbox = GetCellMbox(woRow);
		break;
	case eServerBrowseMboxRef:
		mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
		break;
	default:;
	}

	return mbox && mbox->GetProtocol()->CanDisconnect() && !mbox->GetProtocol()->IsDisconnected();

}

// Test for selected disconnect mailboxes only
bool CServerBrowse::TestSelectionMboxClearDisconnected(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	CMbox* mbox = NULL;
	switch(GetCellDataType(woRow))
	{
	case eServerBrowseMbox:
		mbox = GetCellMbox(woRow);
		break;
	case eServerBrowseMboxRef:
		mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
		break;
	default:;
	}

	return mbox && mbox->GetProtocol()->CanDisconnect();
}

// Test for selected deleteable mailboxes only
bool CServerBrowse::TestDSelectionMbox(TableIndexT row)
{
	// Check availability first
	if (!TestSelectionMboxAvailable(row))
		return false;

	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	if (GetCellDataType(woRow) == eServerBrowseMbox)
		return (GetCellMbox(woRow) != NULL) && (GetCellMbox(woRow)->IsDirectory() || !GetCellMbox(woRow)->NoSelect());
	else if (GetCellDataType(woRow) == eServerBrowseMboxRef)
		return true;
	else
		return false;
}

// Test for selected mailboxes only
bool CServerBrowse::TestSelectionMboxAll(TableIndexT row)
{
	// This is a mailbox
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetCellDataType(woRow) == eServerBrowseMbox) || (GetCellDataType(woRow) == eServerBrowseMboxRef);
}

// Test for selected mailboxes only
bool CServerBrowse::TestSelectionMboxAvailable(TableIndexT row)
{
	// Check for mailbox
	if (TestSelectionMboxAll(row))
	{
		// Check its server is logged in to
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
		CMboxProtocol* server = ResolveCellServer(woRow, true);
		return server && server->IsLoggedOn();
	}
	else
		return false;
}

// Add selected mboxes to list
bool CServerBrowse::AddSelectionToList(TableIndexT row, CMboxList* list)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	switch(GetCellDataType(woRow))
	{
	case eServerBrowseMbox:
		if ((GetCellMbox(woRow) == NULL) || GetCellMbox(woRow)->IsDirectory())
			return false;
		else
		{
			// Determine delete mailbox
			list->push_back(GetCellMbox(woRow));
			return true;
		}
	case eServerBrowseMboxRef:
		if ((GetCellMboxRef(woRow) == NULL) || GetCellMboxRef(woRow)->IsDirectory())
			return false;
		else
		{
			CMbox* mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
			if (mbox)
			{
				list->push_back(mbox);
				return true;
			}
			else
				return false;
		}
	default:
		return false;
	}
}

// Add selected mboxes and directories to list
bool CServerBrowse::AddDSelectionToList(TableIndexT row, CMboxList* list)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	switch(GetCellDataType(woRow))
	{
	case eServerBrowseMbox:
		// Determine delete mailbox
		if (GetCellMbox(woRow) != NULL)
		{
			list->push_back(GetCellMbox(woRow));
			return true;
		}
		else
			return false;
	case eServerBrowseMboxRef:
		{
			CMbox* mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
			if (mbox)
				{
					list->push_back(mbox);
					return true;
				}
			else
				return false;
		}
	default:
		return false;
	}
}

// Add selected items to list
bool CServerBrowse::AddSelectedNodesToList(TableIndexT row, CServerNodeArray* list)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Add items
	list->push_back(mData.at(woRow - 1));

	return true;
}

// Create & open a new mailbox
void CServerBrowse::DoCreateMailbox(bool selection)
{
	SCreateMailbox	create;
	CMboxProtocol* proto = NULL;
	CMboxList* list = NULL;
	bool insert_delim = false;

	// Determine initial creation type
	if (IsSelectionValid() && selection)
	{
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

		// First check logged in state of server - may come back as NULL if cabinet selected
		CMboxProtocol* server = ResolveCellServer(woRow, true);

		// If logged in then use current selection as reference point
		if (server && server->IsLoggedOn())
		{
			switch (GetCellDataType(woRow))
			{
			case eServerBrowseServer:
				proto = GetCellServer(woRow);
				create.account = proto->GetAccountName();
				create.account_type = proto->GetAccountType();
				create.use_wd = false;
				break;
			case eServerBrowseSubs:
				list = GetCellMboxList(woRow);
				proto = list->GetProtocol();
				create.account = proto->GetAccountName();
				create.account_type = proto->GetAccountType();
				create.use_wd = false;
				break;
			case eServerBrowseWD:
				{
					list = GetCellMboxList(woRow);
					proto = list->GetProtocol();
					create.account = proto->GetAccountName();
					create.account_type = proto->GetAccountType();
					create.parent = list->GetName();
					create.use_wd = true;

					// Must adjust parent if ends in wildcard
					char c = create.parent.length() ? create.parent[create.parent.length() - 1] : 0;
					if ((c == *cWILDCARD) || (c == *cWILDCARD_NODIR))
						create.parent[create.parent.length() - 1] = 0;
				}
				break;
			case eServerBrowseMbox:
				if (IsCellINBOX(woRow))
					list = GetCellMbox(woRow)->GetProtocol()->GetINBOXRootWD();
				else
					list = GetCellMboxList(woRow);
				proto = GetCellMbox(woRow)->GetProtocol();
				create.account = proto->GetAccountName();
				create.account_type = proto->GetAccountType();
				if (list)							// In case no WD for INBOX
					create.parent = GetCellMbox(woRow)->GetName();
				create.use_wd = (list != NULL);		// In case no WD for INBOX
				insert_delim = (list != NULL);		// In case no WD for INBOX
				break;
			case eServerBrowseMboxRef:
				{
					CMbox* mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
					if (!::strcmpnocase(mbox->GetName(), cINBOX))
						list = mbox->GetProtocol()->GetINBOXRootWD();
					else
						list = mbox->GetMboxList();
					proto = mbox->GetProtocol();
					create.account = proto->GetAccountName();
					create.account_type = proto->GetAccountType();
					if (list)							// In case no WD for INBOX
						create.parent = mbox->GetName();
					create.use_wd = (list != NULL);		// In case no WD for INBOX
					insert_delim = (list != NULL);		// In case no WD for INBOX
				}
				break;
			default:;
			}
		}
		else
		{
			// Force generic create
			create.use_wd = false;
		}
	}

	// Handle flat WD
	else if (mSingle && GetSingleServer()->FlatWD())
	{
		list = GetSingleServer()->GetHierarchies().at(1);
		proto = GetSingleServer();
		create.account = proto->GetAccountName();
		create.account_type = proto->GetAccountType();
		create.parent = list->GetName();
		create.use_wd = true;

		// Must adjust parent if ends in wildcard
		char c = create.parent.length() ? create.parent[create.parent.length() - 1] : 0;
		if ((c == *cWILDCARD) || (c == *cWILDCARD_NODIR))
			create.parent[create.parent.length() - 1] = 0;
	}

	// Get mailbox name and create & open the mailbox
	try
	{
		if (CreateMailboxName(create))
		{
			cdstring new_name;
			if (create.use_wd)
			{
				// WDs do not need dir delim added, mboxes do
				if (insert_delim)
				{
					new_name = create.parent;
					if (list->GetDirDelim())
						new_name += list->GetDirDelim();
					new_name += create.new_name;
				}
				else
					new_name = create.parent + create.new_name;
			}
			else
				new_name = create.new_name;

			// Check and get proto from dialog
			if (!proto)
				proto = CMailAccountManager::sMailAccountManager->GetProtocol(create.account);
			if (!proto)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Append hierarchy delim for directory
			char dir_delim = (list ? list->GetDirDelim() : proto->GetMailAccount()->GetDirDelim());
			if (create.directory && !new_name.compare_end(cdstring(dir_delim)))
				new_name += dir_delim;
	
			// Check for duplicate
			CMbox* mbox = proto->FindMbox(new_name);

			if (mbox && !mbox->IsDirectory())
				// Error
				CErrorHandler::PutStopAlertRsrc("Alerts::Server::DuplicateCreate");
			else
				// Create and open it
				NewMailbox(proto, create.use_wd ? list : NULL, new_name, create.subscribe);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do nothing if it fails
		// lower handlers display dialog
	}

} // CServerBrowse::CreateMailbox

// Create mailbox name
bool CServerBrowse::CreateMailboxName(SCreateMailbox& create)
{
	bool oked = CCreateMailboxDialog::PoseDialog(create);

	return oked;

} // CServerBrowse::CreateMailboxName

// Create and open named mailbox
void CServerBrowse::NewMailbox(CMboxProtocol* proto, CMboxList* list, cdstring& mbox_name, bool subscribe)
{
	CMbox* mbox = NULL;
	bool added = false;

	// Logon to server - must be successful
	if (!proto->IsLoggedOn())
	{
		CMailAccountManager::sMailAccountManager->StartProtocol(proto);

		if (!proto->IsLoggedOn())
			return;
	}

	try
	{
		// Create the mailbox
		char dir_delim = (list ? list->GetDirDelim() : proto->GetMailAccount()->GetDirDelim());
		mbox = new CMbox(proto, mbox_name, dir_delim, list);
		mbox->Create();

		// Now add to list
		mbox = mbox->AddMbox();
		if (!mbox)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
		added = true;

		// Do not allow subscribe to kill remainder of operation
		try
		{
			// If requested subscribe to it (do not if INBOX)
			if (subscribe && ::strcmpnocase(mbox->GetName(), cINBOX))
				mbox->Subscribe();
		}
		catch (CTCPException& ex)
		{
			CLOG_LOGCATCH(CTCPException&);

			// Allow window kill if aborting
			if (ex.error() == CTCPException::err_TCPAbort)
			{
				CLOG_LOGRETHROW;
				throw;
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}

		// IMPORTANT: If the mailbox was added as a singleton, then it must now be deleted
		// since its not actually being used anywhere. Remember, however, that the mailbox
		// may have been auto-assigned to an appropriate hierarchy
		if (!mbox->GetMboxList())
		{
			// Try to find and select the subscribed one
			if (subscribe)
			{
				CMbox* subs = mbox->GetProtocol()->FindMbox(mbox->GetName());
				if (subs)
					PostCreateAction(subs);
			}
			mbox->GetProtocol()->RemoveSingleton(mbox);
			mbox = NULL;
		}
		else
			// Post process action
			PostCreateAction(mbox);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (mbox)
		{
			if (added)
				mbox->Remove();
			else
				delete mbox;
		}

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

} // CServerBrowse::NewMailbox

// Process mailbox after creation
void CServerBrowse::PostCreateAction(CMbox* mbox)
{
	// Does nothing here
}

#pragma mark ____________________________Table Construction

// Reset the table from the mboxList
void CServerBrowse::ResetTable(void)
{
	// Start cursor for busy operation
	CWaitCursor wait;

	{
		// Prevent window update during changes
		StDeferTableAdjustment changing(this);

		// Delete all existing rows
		Clear();
		mData.clear();

		// Add INBOX (must be present)
		TableIndexT last_wd_row = 0;

		if (mManager)
		{
			// Add each account/server to table under its own hierarchy (if more than one)
			CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();

			bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
			mSingle = !multi;
			//bool multi = true;
			//mSingle = false;
			long index = 0;
			unsigned long hier_parent = 0;
			unsigned long parent = 0;

			// Add favourites
			if (mShowFavourites)
			{
				for(CFavourites::iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
					iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
				{
					// Check view state
					if ((*iter)->IsVisible())
						AddFavourite(*iter);
					else
					{
						// Make sure listening to the list
						(*iter)->Add_Listener(this);
						(*iter)->Add_Listener(CCopyToMenu::sMailboxMainMenu);
					}
				}
			}

			for(CMboxProtocolList::iterator iter = protos.begin(); iter != protos.end(); iter++, index++)
			{
				// Create hierarchy row if more than one
				if (multi)
				{
					parent = AddLastChildRow(hier_parent, NULL, 0, true, false);
					mData.insert(mData.begin() + (parent - 1), SServerBrowseData(*iter));
				}
				else
					parent = mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchSiblingIndexOf(NULL) : 0;

				AddServerHierarchy(*iter, parent, !multi);

				// Expand server hierarchy if required
				if (multi && (*iter)->GetMailAccount()->GetExpanded())
					ExpandRow(parent);
			}

		}
		else
			// Just add single server to table
			AddServerHierarchy(mServer, 0, true);
	}

	// Refresh list
	FRAMEWORK_REFRESH_WINDOW(this)
	UpdateState();

	// Make sure it listens to changes
	Start_Listening();

	// Listen to account manager
	CMailAccountManager::sMailAccountManager->Add_Listener(this);
	CMailAccountManager::sMailAccountManager->Add_Listener(CCopyToMenu::sMailboxMainMenu);

	// Update title in case of server change
	SetTitle();

} // CServerBrowse::ResetTable

// Clear the table from the mboxList
void CServerBrowse::ClearTable(void)
{
	// Delete all existing rows
	Clear();
	mData.clear();
	
	// Stop listening to broadcast changes
	Stop_Listening();
}

// Add server below this parent or as sibling
void CServerBrowse::AddServerHierarchy(CMboxProtocol* proto, unsigned long hier_parent, bool use_sibling)
{
	unsigned long sibling = 0;

	// Add INBOX (if present)
	if (proto->GetINBOX())
	{
		sibling = (!use_sibling ? InsertChildRows(1, hier_parent, NULL, 0, false, false) :
												InsertSiblingRows(1, hier_parent, NULL, 0, false, false));
		mData.insert(mData.begin() + (sibling - 1), SServerBrowseData((CMboxList*) proto, eServerBrowseINBOX));
	}

	CHierarchies& hiers = proto->GetHierarchies();

	bool first = true;
	bool flat_wd = proto->FlatWD() && (use_sibling || proto->AlwaysFlatWD());
	bool as_sibling = false;
	for(CHierarchies::const_iterator iter = hiers.begin(); iter != hiers.end(); iter++)
	{
		// Don't add Subscribed if empty
		if (!first || (*iter)->size())
		{
			// Add hierarchic title
			if (first || !flat_wd)
			{
				if (sibling)
					sibling = InsertSiblingRows(1, sibling, NULL, 0, true, false);
				else if (!use_sibling)
					sibling = InsertChildRows(1, hier_parent, NULL, 0, false, false);
				else
					sibling = InsertSiblingRows(1, hier_parent, NULL, 0, false, false);
				mData.insert(mData.begin() + (sibling - 1), SServerBrowseData(*iter, first ? eServerBrowseSubs : eServerBrowseWD));
				as_sibling = false;
			}
			else if (!first && flat_wd)
			{
				if (!use_sibling)
					sibling = hier_parent;
				else
					as_sibling = true;
			}

			AddHierarchy(*iter, sibling, as_sibling);

			// Check on expansion
			if ((first || !flat_wd) && (*iter)->IsExpanded())
				ExpandRow(sibling);
		}
		else
		{
			// Make sure listening to the list even though its not displayed
			(*iter)->Add_Listener(this);
			(*iter)->Add_Listener(CCopyToMenu::sMailboxMainMenu);
		}

		first = false;
	}

	// Make sure listening to the server
	proto->Add_Listener(this);
	proto->Add_Listener(CCopyToMenu::sMailboxMainMenu);
}

// Add hierarchy below this parent
void CServerBrowse::AddHierarchy(CMboxList* list, unsigned long parent, bool use_sibling)
{
	// Add as child and listen to it
	if (list->size())
	{
		if (!use_sibling)
			AddChildHierarchy(list, parent);
		else
			// Add the hierarchy as a sibling of the 'parent' row, not a child
			AddSiblingHierarchy(list, parent, 0, 0, true);
	}

	// Make sure listening to the list
	list->Add_Listener(this);
	list->Add_Listener(CCopyToMenu::sMailboxMainMenu);
}

// Add hierarchy below this parent
void CServerBrowse::AddChildHierarchy(CTreeNodeList* list, unsigned long parent, unsigned long start, unsigned long number, bool top)
{
	// Determine real type
	bool mbox_list = (typeid(*list) == typeid(CMboxList));

	CMboxList::const_iterator start_iter = list->begin() + start;
	CMboxList::const_iterator end_iter = (number ? start_iter + number : static_cast<const CTreeNodeList*>(list)->end());

	unsigned long level = (*start_iter)->GetWDLevel();
	unsigned long last_wd_row = 0;
	unsigned long index = start;
	ulvector parents;

	// Tweak for better memory usage
	mData.reserve(mData.size() + (end_iter - start_iter));

	// Insert required number of mbox entries
	for(CMboxList::const_iterator iter = start_iter; iter != end_iter; iter++)
	{
		unsigned long new_level = (*iter)->GetWDLevel();

		// Check whether increase in level
		if (new_level > level)
		{
			while(new_level != level)
			{
				// Push current parent
				parents.push_back(parent);

				// Current parent was last item
				parent = last_wd_row;

				// Bump level
				level++;
			}
		}
		// Check whether decrease in level
		else if (new_level < level)
		{
			while(new_level != level)
			{
				// Get previous parent
				parent = parents.back();

				// Pop previous parent
				parents.pop_back();

				// Decrease level
				level--;
			}
		}
		
		// Determine if hierarchic
		bool hierarchic = (*iter)->HasInferiors() || (*iter)->IsDirectory();

		// Add row to parent
		// Special case: first child has to be added as InsertChildRows
		//               subsequent children use AddLastChildRow
		if (top)
		{
			last_wd_row = InsertChildRows(1, parent, NULL, 0, hierarchic, false);
			top = false;
		}
		else
			last_wd_row = AddLastChildRow(parent, NULL, 0, hierarchic, false);
			
		// Add entry into data cache
		mData.insert(mData.begin() + (last_wd_row - 1), SServerBrowseData(list, index++, mbox_list));
		
		// Check expanded state of item and expand if previously set
		// NB Do this AFTER inserting into mData as ExpandRow will try and access mData for this item
		if ((*iter)->IsExpanded())
			ExpandRow(last_wd_row);
	}
}

// Add hierarchy after this sibling
void CServerBrowse::AddSiblingHierarchy(CTreeNodeList* list, unsigned long sibling, unsigned long start, unsigned long number, bool as_sibling)
{
	// Determine real type
	bool mbox_list = (typeid(*list) == typeid(CMboxList));

	CMboxList::const_iterator start_iter = list->begin() + start;
	CMboxList::const_iterator end_iter = (number ? start_iter + number : static_cast<const CTreeNodeList*>(list)->end());

	unsigned long level = (*start_iter)->GetWDLevel();
	unsigned long sibling_level = (*start_iter)->GetWDLevel();
	unsigned long last_wd_row = 0;
	unsigned long index = start;
	ulvector parents;
	bool first_child = (start == 0) && !as_sibling;

	// Tweak for better memory usage
	mData.reserve(mData.size() + (end_iter - start_iter));

	// Insert required number of mbox entries
	for(CMboxList::const_iterator iter = start_iter; iter != end_iter; iter++)
	{
		unsigned long new_level = (*iter)->GetWDLevel();

		// Check whether increase in level
		if (new_level > level)
		{
			while(new_level != level)
			{
				// Push current parent
				parents.push_back(sibling);

				// Current parent was last item
				sibling = last_wd_row;

				// Bump level
				level++;
			}
		}
		// Check whether decrease in level
		else if (new_level < level)
		{
			while(new_level != level)
			{
				// Get previous parent
				sibling = parents.back();

				// Pop previous parent
				parents.pop_back();

				// Decrease level
				level--;
			}
		}
		bool hierarchic = (*iter)->HasInferiors() || (*iter)->IsDirectory();
		if (level == sibling_level)
		{
			if (first_child)
			{
				// Very first sibling => first child of parent
				last_wd_row = sibling = InsertChildRows(1, sibling, NULL, 0, hierarchic, false);
				first_child = false;
			}
			else
				last_wd_row = sibling = InsertSiblingRows(1, sibling, NULL, 0, hierarchic, false);
		}
		else
			last_wd_row = AddLastChildRow(sibling, NULL, 0, hierarchic, false);
		mData.insert(mData.begin() + (last_wd_row - 1), SServerBrowseData(list, index++, mbox_list));
	}
}

// Get woRow for list
unsigned long CServerBrowse::FetchIndexOf(CTreeNodeList* list) const
{
	unsigned long list_index = 0;

	// Determine real type
	if (typeid(*list) == typeid(CMboxList))
	{
		CMboxList* mlist = static_cast<CMboxList*>(list);

		list_index = mlist->GetProtocol()->FetchIndexOf(mlist);

		if (mManager)
			list_index += CMailAccountManager::sMailAccountManager->FetchIndexOf(mlist->GetProtocol(), true) +
									(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);
	}
	else if (typeid(*list) == typeid(CMboxRefList))
	{
		CMboxRefList* mlist = static_cast<CMboxRefList*>(list);

		list_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(mlist);
	}

	return list_index;
}

#pragma mark ____________________________Update display

// Server status changed
void CServerBrowse::ChangedServer(const CMboxProtocol* proto)
{
	// Manager refreshed server
	if (mManager)
	{
		unsigned long server_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
										(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);
		if (server_index)
		{
			TableIndexT exposed = GetExposedIndex(server_index);

			if (exposed)
				RefreshRow(exposed - TABLE_ROW_ADJUST);
		}
	}

	// Refresh toolbar
	UpdateState();

	// Update title in case of server change
	SetTitle();
}

// Mailbox status changed
void CServerBrowse::ChangedMbox(const CMbox* aMbox)
{
	// Get all favourite matches
	ulvector indices;
	CMailAccountManager::sMailAccountManager->FetchFavouriteIndexOf(aMbox, indices);

	// Get all mbox matches
	unsigned long offset = (mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);
	if (mManager)
		CMailAccountManager::sMailAccountManager->FetchIndexOf(aMbox, indices, offset, true);
	else
		GetSingleServer()->FetchIndexOf(aMbox, indices, offset, true);

	// Refresh each row
	for(ulvector::const_iterator iter = indices.begin(); iter != indices.end(); iter++)
	{
		TableIndexT exposed = GetExposedIndex(*iter);

		if (exposed)
			RefreshRow(exposed - TABLE_ROW_ADJUST);
	}
}

#pragma mark ____________________________Servers

// Add server hierarchy
void CServerBrowse::AddServer(CMboxProtocol* proto)
{
	// If its the last one add to end, but only do when more than two
	if ((CMailAccountManager::sMailAccountManager->GetProtocolCount() > 2 ) &&
		(CMailAccountManager::sMailAccountManager->GetProtocolIndex(proto) ==
		 CMailAccountManager::sMailAccountManager->GetProtocolCount() - 1))
	{
		// Start cursor for busy operation
		CWaitCursor wait;

		StDeferTableAdjustment changing(this);

		bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
		unsigned long hier_parent = 0;
		unsigned long parent = 0;

		// Create hierarchy row if more than one
		if (multi)
		{
			parent = AddLastChildRow(hier_parent, NULL, 0, true, false);
			mData.insert(mData.begin() + (parent - 1), SServerBrowseData(proto));
		}
		else
			parent = CMailAccountManager::sMailAccountManager->FetchSiblingIndexOf(NULL);

		AddServerHierarchy(proto, parent, !multi);

		// Expand server hierarchy if required
		if (multi && proto->GetMailAccount()->GetExpanded())
			ExpandRow(parent);
	}
	else
		// For now do complete reset
		ResetTable();
}

// Remove server hierarchy
void CServerBrowse::RemoveServer(CMboxProtocol* proto)
{
	unsigned long index = CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
							(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);

	// Start cursor for busy operation
	CWaitCursor wait;

	{
		StDeferTableAdjustment changing(this);

		// Check single status
		if (mManager && (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1))
		{
			// Remove this and all children
			RemoveRows(1, index, true);

			// If left with single server then force entire reset to flatten server hierarchy
			if (CMailAccountManager::sMailAccountManager->GetProtocolCount() == 2)
			{
				// Find the one that's left
				CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
				CMboxProtocol* single = protos.at(0);
				if (single == proto)
					single = protos.at(1);

				// Adjust index to last remaining server
				index = std::min(index, (mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0) + 1);

				// Remove this and all children
				RemoveRows(1, index, true);

				// Adjust index to last cabinet to allow sibling creation
				index = CMailAccountManager::sMailAccountManager->FetchSiblingIndexOf(NULL);

				mSingle = true;

				// Now add it back in
				AddServerHierarchy(single, index, true);
			}
		}
		else
		{
			if (CMailAccountManager::sMailAccountManager->GetProtocolCount() == 1)
				index++;

			while(mData.size() >= index)
				RemoveRows(1, index, false);

			if (index > 1)
				RefreshRowsDownFrom(index);
		}


	}

	UpdateState();

	// Update title in case of server change
	SetTitle();
}

#pragma mark ____________________________WDs

// Add single hierarchy
void CServerBrowse::AddWD(CMboxList* list)
{
	CMboxProtocol* proto = list->GetProtocol();

	// Check whether currently in flat_wd mode and force reset
	bool flat_wd = (mSingle || proto->AlwaysFlatWD()) && (proto->GetHierarchies().size() <= 3);
	if (flat_wd)
	{
		ResetTable();
		return;
	}

	unsigned long index = FetchIndexOf(list);
	unsigned long proto_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
									(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);

	// Determine whether multi- or single level
	CHierarchies& hiers = proto->GetHierarchies();

	unsigned long pos = proto->GetHierarchyIndex(list);
	unsigned long sibling = 0;
	if (pos)
	{
		// Determine index of previous sibling
		sibling = proto->FetchIndexOf(hiers.at(pos - 1));
	}
	else
		// Sibling must either be INBOX
		sibling = 1;

	// Offset from server
	if (mManager)
		sibling += proto_index;

	// Start cursor for busy operation
	CWaitCursor wait;

	{
		StDeferTableAdjustment changing(this);

		unsigned long parent = InsertSiblingRows(1, sibling, NULL, 0, true, false);
		mData.insert(mData.begin() + (parent - 1), SServerBrowseData(list, list->GetHierarchyIndex() ? eServerBrowseWD : eServerBrowseSubs));
		if (list->size())
			AddChildHierarchy(list, parent);

		// Check on expansion
		if (list->IsExpanded())
			ExpandRow(parent);
	}

	// Refresh list
	RefreshRowsDownFrom(index);
	UpdateState();

	// Make sure listening to the list
	list->Add_Listener(this);
	list->Add_Listener(CCopyToMenu::sMailboxMainMenu);
}

// Remove single hierarchy
void CServerBrowse::RemoveWD(CMboxList* list)
{
	CMboxProtocol* proto = list->GetProtocol();

	// Check whether will be in flat mode after removal
	bool go_flat_wd = (mSingle || proto->AlwaysFlatWD()) &&
						(proto->GetHierarchies().size() == 3) &&
						(list->GetHierarchyIndex() != 0);

	unsigned long index = FetchIndexOf(list);
	bool flat_wd = (mSingle || proto->AlwaysFlatWD()) &&
					proto->FlatWD() &&
					(list->GetHierarchyIndex() == 1);

	// Adjust index for case of removal of Subscribed hierarchy
	if (!list->GetHierarchyIndex())
		index++;

	// Start cursor for busy operation
	CWaitCursor wait;

	{
		StDeferTableAdjustment changing(this);

		// Now do flat wd adjustment
		if (flat_wd)
		{
			// Remove flat items
			for(unsigned long ctr = 0; ctr < list->size();)
			{
				ctr += CountAllDescendents(index) + 1;
				RemoveRows(1, index, false);
			}

			if (index > 1)
				RefreshRowsDownFrom(index);
		}
		else
		{
			// Remove this and all children
			RemoveRows(1, index, true);

			// Check for flatten of remaining wd
			if (go_flat_wd)
			{
				// Find the one that's left
				CHierarchies& hiers = proto->GetHierarchies();
				CMboxList* flatten = hiers.at(1);
				if (flatten == list)
					flatten = hiers.at(2);

				// Get its index (modified after delete) and remove its hierarchy
				index = std::min(index, FetchIndexOf(flatten));
				RemoveRows(1, index, false);

				// Determine parent of flattened list
				unsigned long flat_parent = mSingle ? 0 : 
							CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
							(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);

				// Now add it back
				AddHierarchy(flatten, flat_parent, false);
				FRAMEWORK_REFRESH_WINDOW(this)
			}
		}
	}

	// Refresh list
	//if (GetExposedIndex(index))
	//	RefreshRowsDownFrom(index);
	UpdateState();
}

// Refresh single hierarchy
void CServerBrowse::RefreshWD(CMboxList* list)
{
	CMboxProtocol* proto = list->GetProtocol();

	// Check to see if hidden Subscribed
	if ((list->GetHierarchyIndex() == 0) && !list->size())
		return;

	unsigned long index = FetchIndexOf(list);
	bool flat_wd = (mSingle || proto->AlwaysFlatWD()) &&
					proto->FlatWD() &&
					(list->GetHierarchyIndex() == 1);

	// Start cursor for busy operation
	CWaitCursor wait;

	// Check collapsible state of WD in case need to refersh
	bool had_twister = HasTwister(index);

	{
		StDeferTableAdjustment changing(this);
		unsigned long parent = index;

		if (flat_wd)
		{
			// Get index of last item to delete
			unsigned long last_index = 0;
			if (mSingle)
				last_index = mData.size() + 1;
			else
			{
				unsigned long proto_pos = CMailAccountManager::sMailAccountManager->GetProtocolIndex(proto);
				if (proto_pos == CMailAccountManager::sMailAccountManager->GetProtocolCount() - 1)
					last_index = mData.size() + 1;
				else
				{
					// Get sibling index to protocol based on current table size
					// as the CMailAccountManager data has already changed
					unsigned long current_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
								(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);
					last_index = static_cast<CNodeVectorTree*>(mCollapsableTree)->GetSiblingIndex(current_index);
				}
			}

			// Remove flat items
			for(unsigned long ctr = index; ctr < last_index;)
			{
				ctr += CountAllDescendents(index) + 1;
				RemoveRows(1, index, false);
			}

			if (index > 1)
				RefreshRowsDownFrom(index);

			// Reset index to append children
			parent = mSingle ? 0 : 
							CMailAccountManager::sMailAccountManager->FetchIndexOf(proto, true) +
							(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);
		}
		else
			// Remove children
			RemoveChildren(index, false);

		// Add full hierarchy
		AddHierarchy(list, parent, false);
	}

	// Check for referesh of WD row itself
	if (!flat_wd && (had_twister ^ HasTwister(index)))
		RefreshRow(GetExposedIndex(index) - TABLE_ROW_ADJUST);

	// Refresh list
	if (flat_wd || IsExpanded(index))
		RefreshRowsBelow(index);
	UpdateState();

	// Always refresh favourites as there may have been some sync'ing
	if (mManager && mShowFavourites)
	{
		for(CFavourites::iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
		{
			// Refresh its children
			if ((*iter)->IsVisible())
				RefreshFavourite(*iter, false);
		}
	}
}

#pragma mark ____________________________Favourites

// Add copy to favourite hierarchy
void CServerBrowse::AddFavourite(CMboxRefList* list)
{
	unsigned long sibling = CMailAccountManager::sMailAccountManager->FetchSiblingIndexOf(list);
	unsigned long parent = 0;

	{
		StDeferTableAdjustment changing(this);

		// Add favourite plus hierarchy
		parent = InsertSiblingRows(1, sibling, NULL, 0, list->size(), true);
		mData.insert(mData.begin() + (parent - 1), SServerBrowseData(list));
		if (list->size())
			AddChildHierarchy(list, parent);
		else
			MakeCollapsable(parent, true);

		// Check on expansion
		if (list->IsExpanded())
			ExpandRow(parent);
	}

	// Refresh list
	RefreshRowsDownFrom(parent);
	UpdateState();

	// Make sure listening to the list
	list->Add_Listener(this);
	list->Add_Listener(CCopyToMenu::sMailboxMainMenu);
}

// Remove copy to favourite hierarchy
void CServerBrowse::RemoveFavourite(CMboxRefList* list)
{
	// View must still be as it was when this favourite was added
	unsigned long fav_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(list);

	if (fav_index)
	{
		// Remove from its offset
		RemoveRows(1, fav_index, true);

		UpdateState();
	}
}

// Refresh favourite hierarchy
void CServerBrowse::RefreshFavourite(CMboxRefList* list, bool reset)
{
	// View must still be as it was when this favourite was added
	unsigned long fav_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(list);

	if (fav_index)
	{
		// Only reset favourite if requested
		if (reset)
		{
			// Check collapsible state of WD in case need to refersh
			bool had_twister = HasTwister(fav_index);

			{
				StDeferTableAdjustment changing(this);

				// Remove from its offset
				RemoveChildren(fav_index, false);
				if (list->size())
					AddChildHierarchy(list, fav_index);
			}

			// Check for refresh of WD row itself
			if (had_twister ^ HasTwister(fav_index))
				RefreshRow(GetExposedIndex(fav_index) - TABLE_ROW_ADJUST);

			// Refresh list
			if (IsExpanded(fav_index))
				RefreshRowsBelow(fav_index);
		}

		// Refresh children only
		else if (IsExpanded(fav_index))
		{
			unsigned long num_refresh = CountExposedDescendents(fav_index);
			unsigned long exposed = GetExposedIndex(fav_index) + 1;
			for(; num_refresh; num_refresh--, exposed++)
				RefreshRow(exposed - TABLE_ROW_ADJUST);
		}

		UpdateState();
	}
}

#pragma mark ____________________________Nodes

// Mailboxes added
void CServerBrowse::AddedNode(CTreeNodeList* list, unsigned long first_index, unsigned long number)
{
	// Look for WD
	CMboxList* wd = NULL;
	if (typeid(*list) == typeid(CMboxList))
		wd = static_cast<CMboxList*>(list);
	
	// Check to see if Subscribed needs to be added
	if (wd &&
		(wd->GetHierarchyIndex() == 0) &&
		(wd->size() == number))
	{
		// Add the Subscribed hierarchy
		AddWD(wd);
		return;
	}

	unsigned long list_index = FetchIndexOf(list);
	bool flat_wd = wd &&
					(mSingle || wd->GetProtocol()->AlwaysFlatWD()) &&
					wd->GetProtocol()->FlatWD() &&
					(wd->GetHierarchyIndex() == 1);
	if (flat_wd)
		list_index--;

	// Only do for cabinets that are visible
	if (!wd && (list_index == 0))
		return;

	unsigned long prev_woRow = list_index + first_index;	// woRow of previous cell
	unsigned long woRow = prev_woRow + 1;					// woRow of this

	// Special for very first item in flat wd
	if ((first_index == 0) && flat_wd)
	{
		RefreshWD(wd);
		return;
	}

	// Determine positioning for insert
	if (first_index == 0)
	{
		bool had_twister = HasTwister(prev_woRow);
		if (flat_wd)
			AddChildHierarchy(list, prev_woRow, first_index, number, true);
		else
			AddSiblingHierarchy(list, prev_woRow, first_index, number);

		// Adjust for change in collapsable
		if (had_twister ^ HasTwister(prev_woRow))
			woRow--;
	}
	else
	{
		unsigned long prev_level = list->at(first_index - 1)->GetWDLevel();
		unsigned long level = list->at(first_index)->GetWDLevel();

		// If level > than one above - insert as child
		if (prev_level < level)
		{
			bool had_twister = HasTwister(prev_woRow);
			if (list->size())
				AddChildHierarchy(list, prev_woRow, first_index, number, true);

			// Adjust for change in collapsable
			if (had_twister ^ HasTwister(prev_woRow))
				woRow--;
		}
		// If levels are the same - insert as sibling
		else if (prev_level == level)
			AddSiblingHierarchy(list, prev_woRow, first_index, number);

		// Level is higher - must iterate back to find last item with same level
		else
		{
			unsigned long prev_index = first_index;
			while(prev_level > level)
			{
				prev_woRow--;
				prev_index--;
				prev_level = list->at(prev_index - 1)->GetWDLevel();
			}
			AddSiblingHierarchy(list, prev_woRow, first_index, number);
		}
	}

	// Force reset of indices before refresh
	ResetIndex(list, first_index);

	// Refresh all new rows
	RefreshRowsDownFrom(woRow);
}

// Nodes changed
void CServerBrowse::ChangedNode(CTreeNodeList* list, unsigned long index)
{
	// Look for WD
	CMboxList* wd = NULL;
	if (typeid(*list) == typeid(CMboxList))
		wd = static_cast<CMboxList*>(list);
	
	unsigned long list_index = FetchIndexOf(list);
	bool flat_wd = wd &&
					(mSingle || wd->GetProtocol()->AlwaysFlatWD()) &&
					wd->GetProtocol()->FlatWD() &&
					(wd->GetHierarchyIndex() == 1);
	if (flat_wd)
		list_index--;

	// Only do for cabinets that are visible
	if (!wd && (list_index == 0))
		return;

	TableIndexT exposed = GetExposedIndex(list_index + index + 1);

	if (exposed)
		RefreshRow(exposed - TABLE_ROW_ADJUST);
}

// Mailbox removed
void CServerBrowse::DeletedNode(CTreeNodeList* list, unsigned long mbox_index)
{
	// Look for WD
	CMboxList* wd = NULL;
	if (typeid(*list) == typeid(CMboxList))
		wd = static_cast<CMboxList*>(list);
	
	// Check to see if Subscribed needs to be removed
	if (wd &&
		(wd->GetHierarchyIndex() == 0) &&
		!wd->size())
	{
		// Note at this point the index calculations will think that the subscribed hierarchy is hidden

		// Remove the Subscribed hierarchy
		unsigned long index = FetchIndexOf(list) + 1;
		RemoveRows(1, index, true);
		UpdateState();

		return;
	}

	unsigned long list_index = FetchIndexOf(list);
	bool flat_wd = wd &&
					(mSingle || wd->GetProtocol()->AlwaysFlatWD()) &&
					wd->GetProtocol()->FlatWD() &&
					(wd->GetHierarchyIndex() == 1);
	if (flat_wd)
		list_index--;

	// Only do for cabinets that are visible
	if (!wd && (list_index == 0))
		return;

	// Special to preserve single selection in New Messages cabinet
	bool select_next = false;
	if ((typeid(*list) == typeid(CMboxRefList)) &&
		(CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteNew) &&
		IsSingleSelection())
	{
		// See if row being deleted is selected
		TableIndexT row = GetFirstSelectedRow() + TABLE_ROW_ADJUST;
		if (row == list_index + mbox_index + 1)
		{
			// See if next row is in the same hierarchy
			if (mbox_index < list->size())
				select_next = true;
		}
	}

	// Prevent drawing to stop row delete causing immediate redraw and crash before indices are updated
	StDeferTableAdjustment changing(this);

	// Delete visual entries
	RemoveRows(1, list_index + mbox_index + 1, true);

	// Force reset of indices after refresh
	ResetIndex(list, mbox_index);

	// Reset selection if required
	if (select_next)
		SelectRow(list_index + mbox_index + TABLE_START_INDEX);
}

#pragma mark ____________________________Mboxes

// Reset all starting at from
void CServerBrowse::ResetIndex(CTreeNodeList* list, unsigned long from)
{
	// Look for WD
	CMboxList* wd = NULL;
	if (typeid(*list) == typeid(CMboxList))
		wd = static_cast<CMboxList*>(list);
	
	unsigned long list_index = FetchIndexOf(list);
	bool flat_wd = wd &&
					(mSingle || wd->GetProtocol()->AlwaysFlatWD()) &&
					wd->GetProtocol()->FlatWD() &&
					(wd->GetHierarchyIndex() == 1);
	if (flat_wd)
		list_index--;

	// Scan remainder of list and adjust indices
	for(CTreeNodeList::const_iterator iter = list->begin() + from; iter != list->end(); iter++, from++)
		mData.at(list_index + from).mType = (mData.at(list_index + from).mType & eServerBrowseMask) | (from & eServerBrowseIndex);
}
