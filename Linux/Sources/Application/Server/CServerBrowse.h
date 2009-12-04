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


// Header for CServerBrowse class

#ifndef __CSERVERBROWSE__MULBERRY__
#define __CSERVERBROWSE__MULBERRY__

#include "CHierarchyTableDrag.h"
#include "CListener.h"

#include "CMailAccountManager.h"
#include "CWindowStatesFwd.h"

#include "cdstring.h"
#include "templs.h"
#include <JBroadcaster.h>
#include <jColor.h>

// Classes
class JXKeyModifiers;
class CMboxProtocol;
class CMbox;
class CMboxList;
class CMboxRef;
class CMboxRefList;
class JPainter;
struct SCreateMailbox;

class CServerBrowse : public CHierarchyTableDrag,
						public CListener
{
public:

	enum IconResIDT
	{
		eServerIconDirectory = 0,
		eServerIconDirectoryRef,
		eServerIconSubscribed,
		eServerIconMailbox,
		eServerIconMailboxRef,
		eServerIconMailboxCached,
		eServerIconMailboxUncached,
		eServerIconMailboxNoSelect,
		eServerIconMailboxRefNoSelect,
		eServerIconMailboxUnmarked,
		eServerIconMailboxRefUnmarked,
		eServerIconMailboxMarked,
		eServerIconMailboxRefMarked,
		eServerIconSearchHierarchy,
		eServerIconSearchRefHierarchy,
		eServerIconDisconnectedHierarchy,
		eServerIconRemoteHierarchy,
		eServerIconLocalHierarchy,
		eServerIconPOP3Hierarchy,
		eServerIconFavouriteHierarchy,
		eServerIconMailboxOpen,
		eServerIconMailboxCloseNew,
		eServerIconMailboxCloseCheck,
		eServerIconCalculateSize,
		eServerIconMailboxAutoCheck,		// These last two are special and must always be at the end
		eServerIconMailboxNoAutoCheck
	};

	// Data type
	enum EServerBrowseDataType
	{
		eServerBrowseIndex = 0x00FFFFFF,
		eServerBrowseMask = 0xFF000000,
		eServerBrowseNone = 0x01000000,
		eServerBrowseServer = 0x02000000,
		eServerBrowseWD = 0x03000000,
		eServerBrowseSubs = 0x04000000,
		eServerBrowseINBOX = 0x05000000,
		eServerBrowseMbox = 0x06000000,
		eServerBrowseMboxRefList = 0x07000000,
		eServerBrowseMboxRef = 0x08000000
	};

protected:
	// Data held in flat list
	struct SServerBrowseData
	{
		long mType;
		void* mData;
		
		SServerBrowseData()
			{ mType = eServerBrowseNone; mData = nil; }
		SServerBrowseData(CTreeNodeList* list, long index, bool mbox_list)
			{ mType = (mbox_list ? eServerBrowseMbox : eServerBrowseMboxRef) | (index & 0x00FFFFFF); mData = list; }
		SServerBrowseData(CMboxList* list, EServerBrowseDataType type)
			{ mType = type; mData = list; }
		SServerBrowseData(CMboxRefList* list)
			{ mType = eServerBrowseMboxRefList; mData = list; }
		SServerBrowseData(CMboxProtocol* proto)
			{ mType = eServerBrowseServer; mData = proto; }
		~SServerBrowseData() {}
		
		EServerBrowseDataType GetType(void) const
			{ return (EServerBrowseDataType) (mType & eServerBrowseMask); }
		unsigned long GetIndex(void) const
			{ return mType & eServerBrowseIndex; }
	};

	typedef std::vector<SServerBrowseData> CServerNodeArray;

	CMboxProtocol*		mServer;					// Mail server associated with this window
	CServerNodeArray	mData;						// data
	bool				mManager;					// Set if table is mail account manager
	bool				mSingle;					// Set if table has only a single server
	bool				mShowFavourites;			// Set if showing favourites
	bool				mRecordExpansion;			// Record expansion in prefs
	bool				mListChanging;				// In the process of changing the mbox list
	static cdstring     sSubscribedName;			// Text for subscribed row

public:
  CServerBrowse(JXScrollbarSet* scrollbarSet, 
					JXContainer* enclosure,
					const HSizingOption hSizing, 
					const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
  virtual 		~CServerBrowse();

private:
			void	InitServerBrowse(void);

public:
	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes
	virtual	void	GetTooltipText(cdstring& txt, const STableCell& cell);				// Get text for current tooltip cell

protected:

	// Key/Mouse related
	virtual void	LClickCell(const STableCell& inCell, const JXKeyModifiers& mods); // Clicked item
	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);				// Draw the items
	virtual SColumnInfo	GetColumnInfo(TableIndexT col);

public:
	virtual void		CollapseRow(
								UInt32		inWideOpenRow);
	virtual void		DeepCollapseRow(
								UInt32		inWideOpenRow);
	
	virtual void		ExpandRow(
								UInt32		inWideOpenRow);
	virtual void		DeepExpandRow(
								UInt32		inWideOpenRow);
	virtual void		ProcessExpansion(UInt32 inWideOpenRow, bool expand);

	virtual void			SetManager(void)						// Make this table the manager
								{ mManager = true; }
	virtual bool			IsManager(void)							// Is this table the manager
								{ return mManager; }
	virtual bool			IsSingle(void)							// Is this table showing a single server
								{ return mSingle; }

	virtual void			SetServer(CMboxProtocol* server);		// Set the mail server
	virtual CMboxProtocol*	GetServer(void) const
								{ return mServer; }					// Get the server
	virtual CMboxProtocol*	GetSingleServer(void) const;			// Get the server

	virtual void			SetTitle() {};							// Set the title of the window

	virtual void			SetView(long view);						// Set view state

protected:
	EServerBrowseDataType	GetCellDataType(TableIndexT woRow) const;	// Check data type
	CMbox*			GetCellMbox(TableIndexT woRow) const;			// Get mbox
	CMboxRef*		GetCellMboxRef(TableIndexT woRow) const;		// Get mbox ref
	const char*		GetCellWD(TableIndexT woRow) const;				// Get WD
	CMboxList*		GetCellMboxList(TableIndexT woRow) const;		// Get mbox list for this wd or mbox
	CMboxRefList*	GetCellMboxRefList(TableIndexT woRow) const;	// Get mbox list for this ref or mbox ref
	CMboxProtocol*	GetCellServer(TableIndexT woRow) const;			// Get server
	CMboxProtocol*	ResolveCellServer(TableIndexT woRow,  // Get server from cell of any type
																		bool mboxrefs = false) const;
	bool			IsCellINBOX(TableIndexT woRow) const;			// Is it INBOX
	virtual const char* GetRowText(TableIndexT inWideOpenIndex); // Get text for row


	IconResIDT	GetPlotIcon(EServerBrowseDataType type, void* data);		// Get appropriate icon id
	void	SetTextStyle(JPainter* pDC, CMboxProtocol* proto, bool directory, CMbox* mbox, bool& strike);		// Get appropriate text style
	bool 	UsesBackgroundColor(EServerBrowseDataType type) const;
	JColorIndex GetBackgroundColor(EServerBrowseDataType type) const;

	// Server related
	bool TestSelectionServer(TableIndexT row);				// Test for selected servers only

	// Favourite related
	bool TestSelectionFavourite(TableIndexT row);			// Test for selected favourite hierarchies only
	bool TestSelectionFavouriteItems(TableIndexT row);		// Test for selected favourite items only
	bool TestSelectionFavouriteRemove(TableIndexT row);		// Test for selected removeable favourites only
	bool TestSelectionFavouriteRemoveItems(TableIndexT row);	// Test for selected removeable favourite items only
	bool TestSelectionFavouriteWildcard(TableIndexT row);	// Test for selected wildcard favourites only
	bool TestSelectionFavouriteWildcardItems(TableIndexT row);	// Test for selected wildcard favourite items only

	// Hierarchy related
	bool TestSelectionHierarchy(TableIndexT row);		// Test for selected hierarchies only
	bool TestSelectionResetRefresh(TableIndexT row);		// Test for selected hierarchies/directories only
	void DoRefreshHierarchy(void);					// Refresh hierarchy
	bool RefreshHierarchy(TableIndexT row);				// Refresh hierarchy for server

	// Mailbox related
	bool TestSelectionInferiors(TableIndexT row);					// Test for selected inferiors only
	bool TestSelectionMbox(TableIndexT row);							// Test for selected mailboxes only
	bool TestSelectionMboxDisconnected(TableIndexT row);		// Test for selected disconnectable mailboxes only
	bool TestSelectionMboxClearDisconnected(TableIndexT row);	// Test for selected disconnect mailboxes only
	bool TestDSelectionMbox(TableIndexT row);						// Test for selected mailboxes or directories
	bool TestSelectionMboxAll(TableIndexT row);						// Test for selected mailboxes or mailbox refs only
	bool TestSelectionMboxAvailable(TableIndexT row);               

public:
	// Test for selected mailboxes on logged in servers
	bool AddSelectionToList(TableIndexT row, CMboxList* list);				// Add selected mboxes to list
	bool AddDSelectionToList(TableIndexT row, CMboxList* list);				// Add selected mboxes & directories to list
	bool AddSelectedNodesToList(TableIndexT row, CServerNodeArray* list);	// Add selected items to list

	virtual void	DoCreateMailbox(bool selection = true);			// Create & open a mailbox
protected:
	virtual bool	CreateMailboxName(SCreateMailbox& create);		// Create mailbox name
	virtual void	NewMailbox(CMboxProtocol* proto,
								CMboxList* list,
								cdstring& mbox_name,
								bool subscribe);					// Create and open named mailbox
	virtual void	PostCreateAction(CMbox* mbox);					// Process mailbox after creation

public:
	virtual void	ResetTable();									// Reset the table from the mboxList
	virtual void	ClearTable();									// Reset the table from the mboxList

protected:
	// Add items to table
	void	AddServerHierarchy(CMboxProtocol* proto, // Add server below this parent or as sibling
				   unsigned long parent,
				   bool use_sibling);

	void	AddHierarchy(CMboxList* list,							// Add hierarchy below this parent
										unsigned long parent,
										bool use_sibling);

	void	AddChildHierarchy(CTreeNodeList* list,
										unsigned long parent,
										unsigned long start = 0,
										unsigned long number = 0,
										bool top = false);				// Add hierarchy below this parent
	void	AddSiblingHierarchy(CTreeNodeList* list,				// Add hierarchy after sibling
										unsigned long sibling,
										unsigned long start = 0,
										unsigned long number = 0,
										bool as_sibling = false);

	void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh);
	void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list

	unsigned long FetchIndexOf(CTreeNodeList* list) const;		// Get woRow for list

	// Handle changes to table
	virtual void	UpdateState();							// Update status items like toolbar

	// Servers
	void	ChangedServer(const CMboxProtocol* proto);		// Server status changed
	void	ChangedMbox(const CMbox* aMbox);				// Mailbox status changed

	void	AddServer(CMboxProtocol* proto);				// Add server hierarchy
	void	RemoveServer(CMboxProtocol* proto);				// Remove server hierarchy

	// WDs
	void	AddWD(CMboxList* list);							// Remove single hierarchy
	void	RemoveWD(CMboxList* list);						// Remove single hierarchy
	void	RefreshWD(CMboxList* list);						// Refresh single hierarchy

	// Favourites
	void	AddFavourite(CMboxRefList* list);				// Add copy to favourite hierarchy
	void	RemoveFavourite(CMboxRefList* list);			// Remove favourite hierarchy
	void	RefreshFavourite(CMboxRefList* list,			// Refresh favourite hierarchy
										bool reset = true);

	// Nodes
	void	AddedNode(CTreeNodeList* list, unsigned long index, unsigned long number);		// Nodes added
	void	ChangedNode(CTreeNodeList* list, unsigned long index);							// Nodes changed
	void	DeletedNode(CTreeNodeList* list, unsigned long index);							// Nodes removed

	void	ResetIndex(CTreeNodeList* list, unsigned long from);		// Reset all starting at from

protected:

	// message handlers
	virtual void OnCreate();

};

#endif
