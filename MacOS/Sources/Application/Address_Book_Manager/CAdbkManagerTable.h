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


// Header for CAdbkManagerTable class

#ifndef __CADBKMANAGERTABLE__MULBERRY__
#define __CADBKMANAGERTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"
#include "CListener.h"

#include "CAddressBook.h"
#include "cdstring.h"

// Consts

// Panes

// Messages

// Resources
const	ResIDT		Txtr_DefaultAdbkList = 1000;

// Classes
class CAdbkProtocol;
class CAdbkManagerView;
class CAdbkManagerWindow;
class CAddressBook;
class CAddressBookManager;
class CAddressBookWindow;
class CKeyModifiers;

class CAdbkManagerTable : public CHierarchyTableDrag, public CListener
{
	friend class CAdbkManagerWindow;
	friend class CAdbkManagerView;

private:
	CAddressBookManager*	mManager;				// Manager controlling window
	CAdbkManagerView*		mTableView;				// Owner view
	bool					mListChanging;			// In the process of changing the list
	std::vector<CAddressBook*>	mData;					// data
	short					mHierarchyCol;			// Column containing names
	bool					mDropFirst;				// Indicates first item in drop
	bool					mDropSort;				// Drop with sort
	CAddressBook*			mDropAdbk;				// Address book to drop into
	CAddressBookWindow*		mDropAdbkWnd;			// Address book window dropped onto
	bool					mAddressAdded;			// Indicates address added
	bool					mGroupAdded;			// Indicates group added

public:
	enum { class_ID = 'AmLi' };

					CAdbkManagerTable();
					CAdbkManagerTable(LStream *inStream);
	virtual 		~CAdbkManagerTable();

private:
			void	InitAdbkManagerTable();

protected:
	virtual void		FinishCreateSelf();			// Get details of sub-panes
public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

private:
	bool TestSelectionServer(TableIndexT row);						// Test for selected servers only
	bool TestSelectionAdbk(TableIndexT row);						// Test for selected adbk
	bool TestSelectionAdbkDisconnected(TableIndexT row);			// Test for selected adbk
	bool TestSelectionAdbkClearDisconnected(TableIndexT row);		// Test for selected adbk
	
	// Hierarchy related
	bool TestSelectionHierarchy(TableIndexT row);					// Test for selected hierarchies only
	
protected:

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);			// Draw the message info

	virtual void	HiliteCellActively(const STableCell &inCell,
										Boolean inHilite);
	virtual void	HiliteCellInactively(const STableCell &inCell,
										Boolean inHilite);

	virtual void	CalcCellFlagRect(const STableCell &inCell,
										Rect &outRect);						

public:
	virtual void		CollapseRow(TableIndexT inWideOpenRow);
	virtual void		DeepCollapseRow(TableIndexT inWideOpenRow);
	
	virtual void		ExpandRow(TableIndexT inWideOpenRow);
	virtual void		DeepExpandRow(TableIndexT inWideOpenRow);

			void		ProcessExpansion(TableIndexT inWideOpenRow, bool expand);
			void		ExpandRestore(TableIndexT worow);
			void		ExpandAction(TableIndexT worow, bool deep);

	void	SetManager(CAddressBookManager* manager);			// Set the mail server

protected:
	virtual void	DoSelectionChanged(void);

	void		DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
	void		DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
	void		DoPreview();
	void		DoPreview(CAddressBook* adbk);
	void		DoFullView();

	void	PreviewAddressBook(bool clear = false);							// Do preview of address book
	void	PreviewAddressBook(CAddressBook* adbk, bool clear = false);		// Do preview of address book

	void	OnAddressBookProperties();								// Get properties of selected address books

	void	OnNewAddressBook();

	void	OnOpenAddressBook();
	bool	OpenAddressBook(TableIndexT row);						// Display a specified address book

	void	OnRenameAddressBook();
	bool	RenameAddressBook(TableIndexT row);						// Rename a specified address book

	void	OnDeleteAddressBook();

	void	OnSearchAddressBook();									// Search address books

	void	OnLogin();

	void	OnNewHierarchy();							// Create new hierarchy
	void	OnRenameHierarchy();						// Rename hierarchy
	void	OnDeleteHierarchy();						// Delete hierarchy
	
	void	OnRefreshAddressBook();

	void	OnSynchroniseAddressBook();
	bool	SynchroniseAddressBook(TableIndexT row);				// Synchronise address book

	void	OnClearDisconnectAddressBook();
	bool	ClearDisconnectAddressBook(TableIndexT row);			// ClearDisconnected address book

private:
	ResIDT	GetPlotIcon(const CAddressBook* adbk,
									CAdbkProtocol* proto);			// Get appropriate icon id
	void	SetTextStyle(const CAddressBook* adbk,
							CAdbkProtocol* proto, bool& strike);	// Get appropriate text style
	bool 	UsesBackgroundColor(const STableCell &inCell) const;
	const RGBColor& GetBackgroundColor(const STableCell &inCell) const;

	CAddressBook*	GetCellNode(TableIndexT row, bool worow = false) const;			// Get the selected node
	CAdbkProtocol*	GetCellAdbkProtocol(TableIndexT row) const;					// Get the selected adbk protocol

	bool	AddSelectionToList(TableIndexT row,
										CAddressBookList* list);		// Add selected address books to list

public:
	virtual void	ResetTable();								// Reset the table
	virtual void	ClearTable();								// Clear the table
	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync

			void	AddNode(CAddressBook* adbk,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
			void	AddChildren(const CAddressBook* adbk,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
			void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh);

			void	AddProtocol(CAdbkProtocol* proto);
			void	InsertProtocol(CAdbkProtocol* proto);
			void	RemoveProtocol(CAdbkProtocol* proto);
			void	ClearProtocol(CAdbkProtocol* proto);
			void	RefreshProtocol(CAdbkProtocol* proto);
			void	SwitchProtocol(CAdbkProtocol* proto);

			void	InsertNode(CAddressBook* node);					// Insert a node to the list
			void	DeleteNode(CAddressBook* node);					// Delete a node from the list
			void	RefreshNode(CAddressBook* node);				// Refresh a node from the list

			void	ClearSubList(CAddressBook* node);
			void	RefreshSubList(CAddressBook* node);

// Drag methods
protected:
	virtual bool	IsCopyCursor(DragReference inDragRef);			// Use copy cursor?
	virtual bool	IsDropCell(DragReference inDragRef, STableCell row);// Can cell be dropped into
	virtual void	DoDragReceive(DragReference	inDragRef);			// Get multiple text items
	virtual void	DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);		// Drop data into cell
};

#endif
