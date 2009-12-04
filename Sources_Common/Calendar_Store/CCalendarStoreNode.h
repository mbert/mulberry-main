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

/* 
	CCalendarStoreNode.h

	Author:			
	Description:	Represents a node in a calendar store calendar hierarchy
*/

#ifndef CCalendarStoreNode_H
#define CCalendarStoreNode_H

#include "CListener.h"

#include "CCalendarACL.h"

#include "CICalendar.h"
#include "CICalendarRef.h"

#include "cdstring.h"
#include "ptrvector.h"
#include "templs.h"

#include "SBitFlags.h"

namespace xmllib 
{
class XMLDocument;
class XMLNode;
};

namespace iCal 
{
};

namespace calstore {

class CCalendarProtocol;
class CCalendarStoreWebcal;

class CCalendarStoreNode;
typedef ptrvector<CCalendarStoreNode> CCalendarStoreNodeList;

class CCalendarStoreNode : public CListener
{
public:
	enum EFlags
	{
		// Mbox status
		eNone = 					0,
		eIsProtocol =				1L << 0,
		eIsDirectory =				1L << 1,
		eHasExpanded =				1L << 2,
		eIsSubscribed =				1L << 3,
		eIsActive =					1L << 4,
		eIsCached =					1L << 5,
		eIsInbox =					1L << 6,
		eIsOutbox =					1L << 7,
		eIsDisplayHierarchy =       1L << 8
	};

	// std::sort methods
	static bool sort_by_name(const CCalendarStoreNode* s1, const CCalendarStoreNode* s2);

	CCalendarStoreNode();
	CCalendarStoreNode(CCalendarProtocol* proto);
	CCalendarStoreNode(CCalendarProtocol* proto, CCalendarStoreNode* parent, bool is_dir = false, bool is_inbox = false, bool is_outbox = false, const cdstring& name = cdstring::null_str);
	virtual ~CCalendarStoreNode();

	virtual void ListenTo_Message(long msg, void* param);

	CCalendarProtocol* GetProtocol() const
	{
		return mProtocol;
	}

	CCalendarStoreNode* GetParent() const
	{
		return mParent;
	}

	void AddChild(CCalendarStoreNode* child, bool sort = false);
	void InsertChild(CCalendarStoreNode* child, uint32_t index, bool sort = false);
	CCalendarStoreNodeList* GetChildren() const
	{
		return mChildren;
	}
	void SortChildren();

	cdstring GetAccountName(bool multi = true) const;
	cdstring GetURL() const;

	void SetName(const cdstring& name)							// Set name
		{ mName = name; SetShortName(); }
	const cdstring&	GetName() const								// Get full name
		{ return mName; }
	const cdstring&	GetShortName() const							// Get short name
		{ return mShortName; }
	const cdstring&	GetDisplayShortName() const						// Get display short name
		{ return mDisplayName.empty() ? mShortName : mDisplayName; }
	void	NewName(const cdstring& name);						// Tell this and children to adjust names

	void SetDisplayName(const cdstring& name)					// Set name
		{ mDisplayName = name; }
	const cdstring&	GetDisplayName() const						// Get full name
		{ return mDisplayName; }
	
	void	SetFlags(EFlags new_flags, bool add = true)			// Set flags
		{ mFlags.Set(new_flags, add); }
	EFlags	GetFlags() const									// Get flags
		{ return (EFlags) mFlags.Get(); }

	bool IsProtocol() const
	{
		return mFlags.IsSet(eIsProtocol);
	}
	
	bool IsDisplayHierarchy() const
	{
		return mFlags.IsSet(eIsDisplayHierarchy);
	}
	
	bool IsDirectory() const
	{
		return mFlags.IsSet(eIsDirectory);
	}

	bool IsInbox() const
	{
		return mFlags.IsSet(eIsInbox);
	}

	bool IsOutbox() const
	{
		return mFlags.IsSet(eIsOutbox);
	}
	
	bool CanSchedule() const;

	bool IsViewableCalendar() const
	{
		return !IsProtocol() && !IsDirectory() && !IsOutbox();
	}
	
	bool IsStandardCalendar() const
	{
		return !IsProtocol() && !IsDirectory() && !IsOutbox() && !IsInbox();
	}

	bool HasInferiors() const
	{
		return IsDirectory() && (mChildren != NULL) && (mChildren->size() != 0);
	}

	void SetHasExpanded(bool has_expanded)
	{
		mFlags.Set(eHasExpanded, has_expanded);
	}
	bool HasExpanded() const									// Has expanded children
	{
		return mFlags.IsSet(eHasExpanded);
	}

	void SetSubscribed(bool is_subscribed)
	{
		mFlags.Set(eIsSubscribed, is_subscribed);
	}
	bool IsSubscribed() const
	{
		return mFlags.IsSet(eIsSubscribed);
	}

	bool IsActive() const
	{
		return mFlags.IsSet(eIsActive);
	}

	void SetSize(uint32_t size)
	{
		mSize = size;
	}
	uint32_t GetSize() const
	{
		return mSize;
	}
	void CheckSize();

	uint32_t GetLastSync() const
	{
		return mLastSync;
	}
	void SyncNow() const;

	bool Activate();
	bool Deactivate();

	bool IsCached() const;
	void TestDisconnectCache();

	void SetCalendar(const iCal::CICalendar* cal);
	iCal::CICalendar* GetCalendar() const;
	iCal::CICalendarRef	GetCalendarRef() const
	{
		return mCalendarRef;
	}
	CCalendarStoreNode* FindNode(const iCal::CICalendar* cal) const;
	CCalendarStoreNode* FindNode(const cdstring& path, bool discover = false) const;
	CCalendarStoreNode* FindNodeOrCreate(const cdstring& path) const
	{
		return FindNode(path, true);
	}

	void RemoveFromParent();
	void Clear();

	uint32_t GetRow() const;
	uint32_t CountDescendants() const;
	uint32_t GetParentOffset() const;
	const CCalendarStoreNode* GetSibling() const;
	void GetInsertRows(uint32_t& parent_row, uint32_t& sibling_row) const;

	// Operations on calendars
	void MoveCalendar(const CCalendarStoreNode* dir, bool sibling);
	void CopyCalendar(CCalendarStoreNode* node);
	void CopyCalendarContents(CCalendarStoreNode* node);

	// Webcal stuff
	CCalendarStoreWebcal* GetWebcal() const
	{
		return mWebcal;
	}
	void SetRemoteURL(const cdstring& url);
	const cdstring& GetRemoteURL() const;
	CCalendarStoreNode* FindNodeByRemoteURL(const cdstring& url) const;

	// Access Control
	void	SetMyRights(SACLRight rights)						// Set user's rights to this mailbox
		{ mMyRights = rights; }
	SACLRight	GetMyRights() const
		{ return mMyRights; }
	void	CheckMyRights();									// Get user's rights from server

	void	AddACL(const CCalendarACL* acl);					// Add ACL to list
	void	SetACL(CCalendarACL* acl, uint32_t index);			// Set ACL on server
	void	DeleteACL(const ulvector& index);					// Delete ACL on server
	void	ResetACLs(const CCalendarACLList* acls);			// Reset all acls
	CCalendarACLList*	GetACLs() const
		{ return mACLs; }
	void	CheckACLs();										// Get ACLs from server

	void WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent, bool is_root = false) const;
	void ReadXML(const xmllib::XMLNode* node, bool is_root = false);

protected:
	CCalendarProtocol*		mProtocol;
	CCalendarStoreNode*		mParent;
	CCalendarStoreNodeList*	mChildren;
	CCalendarStoreWebcal*	mWebcal;					// Webcal info
	uint32_t				mActiveCount;				// ref count active state
	SBitFlags				mFlags;						// State flags
	iCal::CICalendarRef		mCalendarRef;
	cdstring				mName;						// Full path name of item
	cdstring				mShortName;					// Last part of the path name
	cdstring				mDisplayName;				// Display name
	uint32_t				mSize;						// Disk size
	mutable uint32_t		mLastSync;					// Last sync time
	SACLRight				mMyRights;					// User's rights on this mailbox
	CCalendarACLList*		mACLs;						// List of ACLs on this mailbox
	
	void	SetShortName();					// Set short name from full name
	void	ParentRenamed();				// Tell children to adjust names when parent moves

	void	DeactivateAllNodes();			// Deactivate this and all children

	void	CalendarChanged(iCal::CICalendar* cal);	// Calendar was changed in some way by the user
	void	CalendarComponentChanged(const iCal::CICalendar::CComponentAction* action);	// Calendar was changed in some way by the user
};

}	// namespace calstore

#endif	// CCalendarStoreNode_H
