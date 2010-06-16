/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

/* 
	CCalendarStoreNode.cpp

	Author:			
	Description:	<describe the CCalendarStoreNode class here>
*/

#include "CCalendarStoreNode.h"

#include "CLog.h"
#include "CMailAccount.h"
#include "CStringUtils.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreWebcal.h"
#include "CCalendarStoreXML.h"
#include "CICalendar.h"

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLObject.h"

#include <algorithm>
#include <memory>
#include <stdint.h>

using namespace calstore; 

// This constructs the calendar manager root
CCalendarStoreNode::CCalendarStoreNode()
{
	mProtocol = NULL;
	mParent = NULL;
	mChildren = new CCalendarStoreNodeList;		// Root must always have list
	mChildren->set_delete_data(false);
	mWebcal = NULL;
	mActiveCount = 0;
	SetFlags(eIsProtocol, true);
	SetFlags(eIsDirectory, true);
	SetFlags(eHasExpanded, true);
	mCalendarRef = iCal::cCalendarRef_Invalid;
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;
}

// This constructs the root of a protocol
CCalendarStoreNode::CCalendarStoreNode(CCalendarProtocol* proto)
{
	mProtocol = proto;
	mParent = &CCalendarStoreManager::sCalendarStoreManager->GetRoot();
	mChildren = NULL;
	mWebcal = NULL;
	mActiveCount = 0;
	SetFlags(eIsProtocol, true);
	SetFlags(eIsDirectory, true);
	SetFlags(eHasExpanded, true);
	mCalendarRef = iCal::cCalendarRef_Invalid;
	mName = proto->GetCalendarAccount()->GetBaseRURL();
	mShortName = mName;
	mDisplayName = proto->GetAccountName();
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;
}

// This constructs an actual node
CCalendarStoreNode::CCalendarStoreNode(CCalendarProtocol* proto, CCalendarStoreNode* parent, bool is_dir, bool is_inbox, bool is_outbox, const cdstring& name)
{
	mProtocol = proto;
	mParent = parent;
	mChildren = NULL;
	mWebcal = NULL;
	mActiveCount = 0;
	SetFlags(eIsDirectory, is_dir);
	SetFlags(eIsInbox, is_inbox);
	SetFlags(eIsOutbox, is_outbox);
	mCalendarRef = iCal::cCalendarRef_Invalid;
	mName = name;
	mSize = ULONG_MAX;
	mLastSync = 0;
	mACLs = NULL;

	SetShortName();
}

CCalendarStoreNode::~CCalendarStoreNode()
{
	// Always deactivate when deleting
	while (IsActive())
		CCalendarStoreManager::sCalendarStoreManager->DeactivateNode(this);

	// Deactivate/delete all children
	Clear();
	
	delete mWebcal;
	mWebcal = NULL;
	
	delete mACLs;
	mACLs = NULL;
}

void CCalendarStoreNode::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case iCal::CICalendar::eBroadcast_Edit:
		CalendarChanged(static_cast<iCal::CICalendar*>(param));
		break;
	case iCal::CICalendar::eBroadcast_AddedComponent:
	case iCal::CICalendar::eBroadcast_ChangedComponent:
	case iCal::CICalendar::eBroadcast_RemovedComponent:
		CalendarComponentChanged(reinterpret_cast<const iCal::CICalendar::CComponentAction*>(param));
		break;
	default:;
	}
}

cdstring CCalendarStoreNode::GetAccountName(bool multi) const
{
	cdstring name;
	if (multi)
	{
		name = mProtocol->GetAccountName();
		name += cMailAccountSeparator;
	}
	name += GetName();
	return name;
}

cdstring CCalendarStoreNode::GetAccountDisplayName(bool multi) const
{
	cdstring name;
	if (multi)
	{
		name = mProtocol->GetAccountName();
		name += cMailAccountSeparator;
	}
	name += GetDisplayShortName();
	return name;
}

void CCalendarStoreNode::CheckSize()
{
	GetProtocol()->SizeCalendar(*this);
}

bool CCalendarStoreNode::CanSchedule() const
{
	return mProtocol ? mProtocol->GetHasScheduling() : false;
}

void CCalendarStoreNode::SyncNow() const
{
	time_t now = ::time(NULL);
	mLastSync = ::mktime(::gmtime(&now));
}

// return: 	true if not previously active
//			false if previosuly active
bool CCalendarStoreNode::Activate()
{
	mActiveCount++;
	return !IsActive();
}

// return: 	true if deactivation occurs
//			false if previosuly inactive or refcount still above zero
bool CCalendarStoreNode::Deactivate()
{
	if (!IsActive() || (mActiveCount == 0))
		return false;
	return --mActiveCount == 0;
}

bool CCalendarStoreNode::IsCached() const
{
	// Cached if not disconnected or known to have 
	return !GetProtocol()->IsDisconnected() || mFlags.IsSet(eIsCached);
}

cdstring CCalendarStoreNode::GetURL() const
{
	cdstring ruri = GetName();
	ruri.EncodeURL(GetProtocol()->GetDirDelim());

	cdstring result;
	if (!IsDisplayHierarchy())
	{
		result = mProtocol->GetURL(true);
		result += "/";
	}
	result += ruri;
	return result;
}

void CCalendarStoreNode::AddChild(CCalendarStoreNode* child, bool sort)
{
	// Create if required
	if (mChildren == NULL)
		mChildren = new CCalendarStoreNodeList;
	mChildren->push_back(child);
	child->mParent = this;
	
	// Setting a child means it has been expanded
	SetFlags(eHasExpanded, true);
	
	// Do sort if requested
	if (sort)
		SortChildren();
}

void CCalendarStoreNode::InsertChild(CCalendarStoreNode* child, uint32_t index, bool sort)
{
	// Do ordinary add if no children or insert at end
	if ((mChildren == NULL) || (index >= mChildren->size()))
		AddChild(child, sort);
	else
	{
		mChildren->insert(mChildren->begin() + index, child);
		child->mParent = this;
		
		// Do sort if requested
		if (sort)
			SortChildren();
	}
}

void CCalendarStoreNode::SortChildren()
{
	if (mChildren == NULL)
		return;
	
	std::sort(mChildren->begin(), mChildren->end(), sort_by_name);
}

bool CCalendarStoreNode::sort_by_name(const CCalendarStoreNode* s1, const CCalendarStoreNode* s2)
{
	// Put display hierarchies at the end
	if (s1->IsDisplayHierarchy() ^ s2->IsDisplayHierarchy())
	{
		return s2->IsDisplayHierarchy();
	}
	else
	{
		return ::strcmpnocase(s1->GetDisplayShortName(), s2->GetDisplayShortName()) < 0;
	}
}

void CCalendarStoreNode::SetCalendar(const iCal::CICalendar* cal)
{
	if (cal != NULL)
	{
		SetFlags(eIsActive, true);
		mCalendarRef = cal->GetRef();
		
		const_cast<iCal::CICalendar*>(cal)->Add_Listener(this);
	}
	else
	{
		SetFlags(eIsActive, false);
		mCalendarRef = iCal::cCalendarRef_Invalid;
	}
}

iCal::CICalendar* CCalendarStoreNode::GetCalendar() const
{
	return iCal::CICalendar::GetICalendar(mCalendarRef);
}

CCalendarStoreNode* CCalendarStoreNode::FindNode(const iCal::CICalendar* cal) const
{
	if (!IsDirectory())
	{
		return (mCalendarRef == cal->GetRef()) ? const_cast<CCalendarStoreNode*>(this) : NULL;
	}
	else if (mChildren)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			CCalendarStoreNode* result = (*iter)->FindNode(cal);
			if (result != NULL)
				return result;
		}
	}
	
	return NULL;
}

CCalendarStoreNode* CCalendarStoreNode::FindNode(const cdstring& path, bool discover) const
{
	// Find top-level item matching last item in hierarchy
	if (mChildren)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			if (path == (*iter)->GetName())
				return *iter;
			if (path.compare_start((*iter)->GetName()))
			{
				// May need discovery
				if (discover && (*iter)->IsDirectory() && !(*iter)->HasExpanded())
					GetProtocol()->LoadSubList(*iter, false);					
				return (*iter)->FindNode(path, discover);
			}
		}
	}
	
	return NULL;
}

// Remove node from parent withtout deleting the node
void CCalendarStoreNode::RemoveFromParent()
{
	if (mParent)
	{
		CCalendarStoreNodeList* list = mParent->GetChildren();
		CCalendarStoreNodeList::iterator found = std::find(list->begin(), list->end(), this);
		if (found != list->end())
		{
			// NULL it out so that the node is not deleted, then erase
			*found = NULL;
			list->erase(found);
		}
		
		mParent = NULL;
	}
}

void CCalendarStoreNode::Clear()
{
	// Deactivate all children before deleting the list as the list may be accessed during
	// the deactivation process
	DeactivateAllNodes();

	if (mChildren != NULL)
	{
		// Erase children
		delete mChildren;
		mChildren = NULL;
	}
}

// Deactivate this and all children
void CCalendarStoreNode::DeactivateAllNodes()
{
	// Always deactivate when deleting
	if (IsActive())
		CCalendarStoreManager::sCalendarStoreManager->DeactivateNode(this);
	
	if (mChildren != NULL)
	{
		// Deactivate each child recursively
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
			(*iter)->DeactivateAllNodes();
	}

}

uint32_t CCalendarStoreNode::GetRow() const
{
	// Look for sibling
	uint32_t row = GetParentOffset();
	const CCalendarStoreNode* parent = mParent;
	while(parent != NULL)
	{
		row += parent->GetParentOffset();
		parent = parent->mParent;
	}
	
	return row;
}

uint32_t CCalendarStoreNode::CountDescendants() const
{
	uint32_t result = 0;
	if (mChildren != NULL)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
			result += (*iter)->CountDescendants() + 1;
	}
	
	return result;
}

uint32_t CCalendarStoreNode::GetParentOffset() const
{
	uint32_t result = 0;
	if ((mParent != NULL) && mParent->HasInferiors())
	{
		result++;
		for(CCalendarStoreNodeList::iterator iter = mParent->GetChildren()->begin(); iter != mParent->GetChildren()->end(); iter++)
		{
			if (*iter == this)
				break;
			
			result += (*iter)->CountDescendants() + 1;
		}
	}
	
	return result;
}

const CCalendarStoreNode* CCalendarStoreNode::GetSibling() const
{
	const CCalendarStoreNode* result = NULL;
	if ((mParent != NULL) && mParent->HasInferiors())
	{
		for(CCalendarStoreNodeList::iterator iter = mParent->GetChildren()->begin(); iter != mParent->GetChildren()->end(); iter++)
		{
			if (*iter == this)
				break;
			
			result = *iter;
		}
	}
	
	return result;
}

void CCalendarStoreNode::GetInsertRows(uint32_t& parent_row, uint32_t& sibling_row) const
{
	// Get parent's row
	parent_row = (mParent ? mParent->GetRow() : 0);

	// Now get the sibling row
	uint32_t parent_offset = GetParentOffset();
	if ((parent_offset != 0) && (GetSibling() != NULL))
	{
		sibling_row = parent_row + parent_offset - GetSibling()->CountDescendants() - 1;
	}
	else
		sibling_row = 0;
}

// Set pointer to short name
void CCalendarStoreNode::SetShortName()
{
	// Determine last directory break
	if (GetProtocol()->GetDirDelim() != 0)
	{
		cdstrvect splits;
		mName.split(cdstring(GetProtocol()->GetDirDelim()), splits);
		if (splits.size() > 1)
		{
			if (splits.back().empty())
				splits.pop_back();
			mShortName = splits.back();
		}
		else
		{
			mShortName = splits.front();
		}

	}
	else
		mShortName = mName.c_str();
}

// Tell this and children to adjust names
void CCalendarStoreNode::NewName(const cdstring& name)
{
	// Adjust this one
	SetName(name);
	
	// Now iterate over children doing rename
	if (mChildren != NULL)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			(*iter)->ParentRenamed();
		}
	}
}

// Tell children to adjust names when parent moves
void CCalendarStoreNode::ParentRenamed()
{
	// Must have a parent
	if (mParent == NULL)
		return;

	// Adjust this node
	cdstring new_name;
	new_name = mParent->GetName();
	if (GetProtocol()->GetDirDelim())
		new_name += GetProtocol()->GetDirDelim();
	new_name += GetShortName();
	SetName(new_name);
	
	// Now iterate over children doing rename
	if (mChildren != NULL)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			(*iter)->ParentRenamed();
		}
	}
}

void CCalendarStoreNode::MoveCalendar(const CCalendarStoreNode* dir, bool sibling)
{
	cdstring new_name;

	// Must have item and it must be a directory or protocol
	if (!dir || !dir->IsProtocol() && !dir->IsDirectory())
		return;

	// Check for sibling
	if (sibling)
	{
		// Rename this mailbox by taking its short name and appending to name of the sibling's parent directory (not protocol)
		if (dir->GetParent() && !dir->GetParent()->IsProtocol())
		{
			new_name = dir->GetName();
			if (dir->GetProtocol()->GetDirDelim() != 0)
				new_name += dir->GetProtocol()->GetDirDelim();
		}
		new_name += GetShortName();
	}
	else
	{
		// Rename this mailbox by taking its short name and appending to name of parent directory (not protocol)
		if (!dir->IsProtocol())
		{
			new_name = dir->GetName();
			if (dir->GetProtocol()->GetDirDelim())
				new_name += dir->GetProtocol()->GetDirDelim();
		}
		new_name += GetShortName();
	}

	// Check for existing duplicate
	if (dir->GetProtocol()->GetNode(new_name) != NULL)
		return;

	// Check for same server
	if (GetProtocol() == dir->GetProtocol())
	{
		// Just rename
		calstore::CCalendarStoreManager::sCalendarStoreManager->RenameCalendar(this, new_name);
	}
	else
	{
		// Logon to server - must be successful
		if (!dir->GetProtocol()->IsLoggedOn())
		{
			calstore::CCalendarStoreManager::sCalendarStoreManager->StartProtocol(dir->GetProtocol());

			if (!dir->GetProtocol()->IsLoggedOn())
				return;
		}

		// Create and open it
		calstore::CCalendarStoreNode* new_node = calstore::CCalendarStoreManager::sCalendarStoreManager->NewCalendar(dir->GetProtocol(), new_name, IsDirectory());

		// Do recursive move if sub-hierarchies
		if (IsDirectory())
		{
			// Must be expanded
			if (!HasExpanded())
			{
				// Do deep expand off this root calendar
				GetProtocol()->LoadSubList(this, true);
				SetHasExpanded(true);
			}

			// Now iterate over children doing move
			if (mChildren != NULL)
			{
				for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
				{
					(*iter)->MoveCalendar(new_node, false);
				}
			}
		}
		else
			CopyCalendar(new_node);
	}
}

// Copy this calendar into another one
void CCalendarStoreNode::CopyCalendar(CCalendarStoreNode* node)
{
	// Determine open state of destination as it must be closed when we operate on it
	bool dst_open = node->IsSubscribed();
	
	// Calendars always up to date	

	// Remove destination from display
	if (dst_open)
	{
		calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, false, false);
	}

	// Create calendar and read in this one's data
	std::auto_ptr<iCal::CICalendar> cal(new iCal::CICalendar);
	
	// Read from source into new calendar object
	GetProtocol()->ReadFullCalendar(*this, *cal);

	// Rename calendar to match destination
	cal->EditName(node->GetShortName());

	// Write new calendar object to destination (no sync - always overwrite)
	cal->SetTotalReplace(true);
	node->GetProtocol()->CopyCalendar(*node, *cal);

	// Restore source and destination to display
	if (dst_open)
	{
		calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, true, false);
	}
}

// Copy this calendar's items into another one
void CCalendarStoreNode::CopyCalendarContents(CCalendarStoreNode* node)
{
	// Determine open state of destination as it must be closed when we operate on it
	bool dst_open = node->IsSubscribed();

	// Calendars always up to date	

	// Remove destination from display
	if (dst_open)
	{
		calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, false, false);
	}

	// Create calendar and read in this one's data
	std::auto_ptr<iCal::CICalendar> cal(new iCal::CICalendar);
	
	// Read from source into new calendar object
	GetProtocol()->ReadFullCalendar(*this, *cal);

	// Rename calendar to match destination
	cal->EditName(node->GetShortName());

	// Write new calendar object to destination (no sync - always overwrite)
	cal->SetTotalReplace(true);
	node->GetProtocol()->CopyCalendarContents(*node, *cal);

	// Restore source and destination to display
	if (dst_open)
	{
		calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, true, false);
	}
}

// Switch into disconnected mode
void CCalendarStoreNode::TestDisconnectCache()
{
	if (mChildren != NULL)
	{
		// Test each child recursively
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
			(*iter)->TestDisconnectCache();
	}

	// See if it exists locally
	if (!IsDirectory())
		SetFlags(eIsCached, mProtocol->TestCalendar(*this));
}

// Calendar was changed in some way by the user
void CCalendarStoreNode::CalendarChanged(iCal::CICalendar* cal)
{
	// Calendar must belong to this node
	if (cal->GetRef() != mCalendarRef)
		return;

	// Always write out the calendar in full
	GetProtocol()->WriteFullCalendar(*this, *cal);
}

// Calendar was changed in some way by the user
void CCalendarStoreNode::CalendarComponentChanged(const iCal::CICalendar::CComponentAction* action)
{
	// Calendar must belong to this node
	if (action->GetCalendar().GetRef() != mCalendarRef)
		return;

	// Do change based on operation
	switch (action->GetOperation())
	{
	case iCal::CICalendar::CComponentAction::eAdded:
		GetProtocol()->AddComponent(*this, action->GetCalendar(), action->GetComponent());
		break;
	case iCal::CICalendar::CComponentAction::eChanged:
		GetProtocol()->ChangeComponent(*this, action->GetCalendar(), action->GetComponent());
		break;
	case iCal::CICalendar::CComponentAction::eRemoved:
		GetProtocol()->RemoveComponent(*this, action->GetCalendar(), action->GetComponent());
		break;
	}
}

void CCalendarStoreNode::SetRemoteURL(const cdstring& url)
{
	if (mWebcal == NULL)
		mWebcal = new CCalendarStoreWebcal;
	mWebcal->SetURL(url);
}

const cdstring& CCalendarStoreNode::GetRemoteURL() const
{
	return (mWebcal != NULL) ? mWebcal->GetURL() : cdstring::null_str;
}

CCalendarStoreNode* CCalendarStoreNode::FindNodeByRemoteURL(const cdstring& url) const
{
	if (!IsDirectory())
	{
		return (GetRemoteURL() == url) ? const_cast<CCalendarStoreNode*>(this) : NULL;
	}
	else if (mChildren)
	{
		for(CCalendarStoreNodeList::iterator iter = mChildren->begin(); iter != mChildren->end(); iter++)
		{
			CCalendarStoreNode* result = (*iter)->FindNodeByRemoteURL(url);
			if (result != NULL)
				return result;
		}
	}
	
	return NULL;
}

#pragma mark ____________________________ACLS

// Get user's rights from server
void CCalendarStoreNode::CheckMyRights()
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	mProtocol->MyRights(*this);
}

// Add ACL to list
// NB WebDAV behaviour means we can get multiple ACLs for the same principal,
// but for ease of use for the user we want to merge all those into one.
void CCalendarStoreNode::AddACL(const CCalendarACL* acl)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	// Create list if it does not exist
	if (mACLs == NULL)
		mACLs = new CCalendarACLList;
	
	// Look for existing ACL by principal
	bool merged = false;
	for(CCalendarACLList::iterator iter = mACLs->begin(); iter != mACLs->end(); iter++)
	{
		if ((*iter).SamePrincipal(*acl))
		{
			// Merge rights
			(*iter).MergeRights(acl->GetRights());
			merged = true;
			break;
		}
	}

	// Add if not merged
	if (!merged)
		mACLs->push_back(*acl);
}

// Set ACL on server
void CCalendarStoreNode::SetACL(CCalendarACL* acl, uint32_t index)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	// Try to set on server
	try
	{
		// Calendar collections always have inherited turned on until we allow per-component ACLs
		if (!IsProtocol() && !IsDirectory())
			acl->SetInheritable(true);
		
		// Create list if it does not exist
		if (mACLs == NULL)
			mACLs = new CCalendarACLList;

		// Add if no index
		if (index == 0xFFFFFFFF)
			mACLs->push_back(*acl);
		else
		{
			// Replace existing
			mACLs->at(index) = *acl;
		}

		// Now update on server
		mProtocol->SetACL(*this, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Set ACL on server
void CCalendarStoreNode::DeleteACL(const ulvector& indices)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	// Try to delete on server
	try
	{
		if (mACLs != NULL)
		{
			// Make sure it is sorted
			//::sort(indices.begin(), indices.end());
			
			// Remove in reverse
			for(ulvector::const_reverse_iterator riter = indices.rbegin(); riter != indices.rend(); riter++)
			{
				mACLs->erase(mACLs->begin() + *riter);
			}
		}

		// Now update on server
		mProtocol->DeleteACL(*this, NULL);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Reset all acls
void CCalendarStoreNode::ResetACLs(const CCalendarACLList* acls)
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	try
	{
		// Delete all on server
		if (mACLs != NULL)
		{
			mACLs->clear();
		}
		
		// Now add all the new ones
		for(CCalendarACLList::const_iterator iter = acls->begin(); iter != acls->end(); iter++)
			SetACL(const_cast<CCalendarACL*>(static_cast<const CCalendarACL*>(&(*iter))), 0xFFFFFFFF);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Silent failure
	}
}

// Get ACLs from server
void CCalendarStoreNode::CheckACLs()
{
	// Check for valid protocol & state
	if (!mProtocol->IsACLCalendar() || mProtocol->IsDisconnected())
		return;

	// Save existing list in case of failure
	CCalendarACLList* save = (mACLs != NULL ? new CCalendarACLList(*mACLs) : NULL);

	try
	{
		// Delete everything in existing list
		if (mACLs != NULL)
			mACLs->clear();

		mProtocol->GetACL(*this);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Replace failed list with old one
		if (save)
			*mACLs = *save;
		delete save;

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	delete save;
}

#pragma mark ____________________________XML

void CCalendarStoreNode::WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent, bool is_root) const
{
	xmllib::XMLNode* xmlnode = NULL;
	
	// root node just write children
	if (!is_root)
	{
		// Create new node
		xmlnode = new xmllib::XMLNode(doc, parent, cXMLElement_calendarnode);

		// Set directory attribute
		if (IsDirectory())
		{
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_directory, IsDirectory());
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_has_expanded, HasExpanded());
		}
		
		if (IsInbox())
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_inbox, IsInbox());
		if (IsOutbox())
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_outbox, IsOutbox());
		if (IsDisplayHierarchy())		
			xmllib::XMLObject::WriteAttribute(xmlnode, cXMLAttribute_displayhierachy, IsDisplayHierarchy());

		// Set name child node
		xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_name, GetName());
		
		// Set display name child node
		if (!mDisplayName.empty())
			xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_displayname, mDisplayName);
		
		// Set sync child nodes
		if (mLastSync != 0)
			xmllib::XMLObject::WriteValue(doc, xmlnode, cXMLElement_lastsync, mLastSync);
		
		// Do webcal info if present
		if (mWebcal != NULL)
		{
			mWebcal->WriteXML(doc, xmlnode);
		}
	}
	else
		xmlnode = parent;

	// Do children if they exist
	if (GetChildren() != NULL)
	{
		for(CCalendarStoreNodeList::const_iterator iter = GetChildren()->begin(); iter != GetChildren()->end(); iter++)
		{
			(*iter)->WriteXML(doc, xmlnode);
		}
	}
}

void CCalendarStoreNode::ReadXML(const xmllib::XMLNode* xmlnode, bool is_root)
{
	if (is_root)
	{
		// Get has expanded
		bool has_expanded = false;
		if (xmlnode->AttributeValue(cXMLAttribute_has_expanded, has_expanded))
		{
			SetHasExpanded(has_expanded);
		}
	}
	else
	{
		// Must have right type of node
		if (!xmlnode->CompareFullName(cXMLElement_calendarnode))
			return;
		
		// Check attributes
		bool temp = false;
		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_directory, temp))
		{
			SetFlags(eIsDirectory, temp);
			if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_has_expanded, temp))
				SetHasExpanded(temp);
		}

		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_inbox, temp))
			SetFlags(eIsInbox, temp);
		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_outbox, temp))
			SetFlags(eIsOutbox, temp);
		if (xmllib::XMLObject::ReadAttribute(xmlnode, cXMLAttribute_displayhierachy, temp))
			SetFlags(eIsDisplayHierarchy, temp);

		// Must have a name
		cdstring name;
		if (!xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_name, name))
			return;
		SetName(name);

		// Get display name details
		xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_displayname, mDisplayName);
		
		// Get sync details
		xmllib::XMLObject::ReadValue(xmlnode, cXMLElement_lastsync, mLastSync);
		
		// Look for webcal node
		const xmllib::XMLNode* webcalnode = xmlnode->GetChild(cXMLElement_webcal);
		if (webcalnode != NULL)
		{
			delete mWebcal;
			mWebcal = new CCalendarStoreWebcal;
			mWebcal->ReadXML(webcalnode);
		}
	}

	// Scan into directories
	if (IsDirectory())
	{
		for(xmllib::XMLNodeList::const_iterator iter = xmlnode->Children().begin(); iter != xmlnode->Children().end(); iter++)
		{
			// Check child name
			xmllib::XMLNode* child = *iter;
			if (child->CompareFullName(cXMLElement_calendarnode))
			{
				// Create new cal store node
				CCalendarStoreNode* node = new CCalendarStoreNode(GetProtocol(), this);
				
				// Parse it
				node->ReadXML(child);
				
				// Add it to this one
				AddChild(node);
			}
		}
		
		// Always mark node as having been expanded
		SetHasExpanded(true);
		
		// Always sort the children after adding all of them
		SortChildren();
	}
}
