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


// Header for CAdbkSearchTable class

#ifndef __CADBKSEARCHTABLE__MULBERRY__
#define __CADBKSEARCHTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "cdstring.h"


// Consts

// Resources

// Classes
class CAdbkSearchWindow;
class CAdbkServerPopup;
class LView;
class CTextFieldX;
class LPopupButton;

class CAdbkSearchTable : public CHierarchyTableDrag,
						public LListener {

	friend class CAdbkManagerWindow;
	friend class CAdbkSearchWindow;

private:
	CAdbkSearchWindow*					mWindow;				// Owner window
	LView*								mAdbkView;				// AddressBook View
	LView*								mLDAPView;				// LDAP Server View
	LPopupButton*						mMethod;				// Method choice
	CAdbkServerPopup*					mServer;				// Server choice
	CTextFieldX*						mLookup;				// Item to lookup
	LPopupButton*						mMatch;					// Match choice
	LPopupButton*						mCriteria;				// Criteria choice
	std::vector<void*>						mData;					// data
	TableIndexT							mLastParent;			// last parent row

public:
	enum { class_ID = 'AsTb' };

					CAdbkSearchTable();
					CAdbkSearchTable(LStream *inStream);
	virtual 		~CAdbkSearchTable();

private:
			void	InitAdbkSearchTable(void);

protected:
	virtual void		FinishCreateSelf(void);			// Get details of sub-panes
public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);	// Respond to clicks in the icon buttons

private:
	bool TestSelectionAddr(TableIndexT row);						// Test for selected addr

protected:

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);			// Draw the message info

	virtual void	CalcCellFlagRect(const STableCell &inCell,
										Rect &outRect);						

protected:
	void	CreateNewLetter(bool option_key);				// Create letter from selection
	void	DoSearch(void);
	void	DoClear(void);

	void	DoEditEntry(void);							// Edit selected entries
	bool	EditEntry(TableIndexT row);					// Edit specified address

	void	DoDeleteAddress(void);
	bool	DeleteAddress(TableIndexT row);				// Delete specified entry

private:
	virtual void*	GetCellData(TableIndexT row);					// Get the selected adbk
	virtual void	AddSelectionToList(CAddressList* list);			// Add selected address books to list
	virtual bool	CopyEntryToList(TableIndexT row,
										CAddressList* list);		// Copy cell to list

public:
	virtual void	ResetTable(void);								// Reset the table from the mboxList
	virtual void	AppendItem(const CAddressSearchResult* item);	// Add an item to end of list
	virtual void	AddList(const CAddressList* list, TableIndexT& row);	// Add a node to the list

	virtual void	ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync

// Drag methods
	virtual void		AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect);			// Add address to drag
	virtual void		DoDragSendData(FlavorType inFlavor,
										ItemReference inItemRef,
										DragReference inDragRef);	// Other flavor requested by receiver
};

#endif
