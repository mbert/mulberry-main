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


// Header for CTableDragAndDrop class

#ifndef __CTABLEDRAGANDDROP__MULBERRY__
#define __CTABLEDRAGANDDROP__MULBERRY__

#include "CTable.h"
#include <set>
#include <JXSelectionManager.h>

class CMulSelectionData;

class CTableDragAndDrop : public CTable
{
	friend class CMulSelectionData;
public:
	CTableDragAndDrop(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual 		~CTableDragAndDrop();

	virtual void	SetReadOnly(bool readOnly)					// Set read only
						{ mReadOnly = readOnly; }
	virtual void	SetDropCell(bool dropCell)					// Set drop into cell
						{ mDropCell = dropCell; }
	virtual void	SetDropCursor(bool dropCursor)				// Set drop at cell
						{ mDropCursor = dropCursor; }
	virtual void	SetAllowDrag(bool allowDrag)					// Set allow drag
						{ mAllowDrag = allowDrag; }
	virtual void	SetSelfDrag(bool selfDrag)					// Set self drag
						{ mSelfDrag = selfDrag; }
	virtual void	SetExternalDrag(bool externalDrag)					// Set self drag
						{ mExternalDrag = externalDrag; }
	virtual void	SetAllowMove(bool allowMove)					// Set allow move
						{ mAllowMove = allowMove; }
	virtual void	SetHandleMove(bool handleMove)					// Set allow move
						{ mHandleMove = handleMove; }

	virtual STableCell&		GetLastDragCell(void)
						{ return mLastDropCell; }					// Get last drag cell
	virtual STableCell&		GetLastDragCursor(void)
						{ return mLastDropCursor; }					// Get last drag cursor

	virtual void	AddDragFlavor(unsigned int theFlavor) // Set its drag flavor
						{ mDragFlavors.push_back(theFlavor); }
	virtual void	AddDropFlavor(unsigned int theFlavor) // Set its drop flavor
						{ mDropFlavors.push_back(theFlavor); }

	virtual void OnCreate();

	virtual void	SelectionChanged(void);

// Drag methods
protected:
	unsigned char* mDropData;
	JSize mDataLen;
	JXSelectionManager::DeleteMethod mDelMethod;
	Atom mDropFlavor;
	Atom mDragFlavor;
	bool mDropActionMove;
	virtual bool GetDropData(Time time);

	static CTableDragAndDrop*		sTableDragSource;			// Source of drag
	static CTableDragAndDrop*		sTableDropTarget;			// Target of drag
	STableCell			mLastHitCell;				// Last cell hit by mouse
	STableCell			mLastDropCell;				// Last row for cell hilite
	STableCell			mLastDropCursor;			// Last row for cursor
	bool				mReadOnly;					// Table not editable
	bool				mDropCell;					// Drop into individual cells
	bool				mDropCursor;				// Drop at cell
	bool				mAllowDrag;					// Allow drag from self
	bool				mSelfDrag;					// Allow drag to self
	bool				mExternalDrag;				// Allow drag to others
	bool				mAllowMove;					// Allow drag to delete originals
	bool				mHandleMove;				// Do delete original on drag move
	typedef std::vector<Atom> CFlavorsList;
	CFlavorsList	mDragFlavors;				// List of flavors to send
  	CFlavorsList	mDropFlavors;				// List of flavors to accept
	JArray<Atom>	mCurrentDropFlavors;   		// List of flavors of the current drop
	bool			mIsDropTarget; 				//This is in CTable on windows.  Don't know if
                             					//we need it there or if we need it at all

	//To do drag and drop we need a bit more info than
	//the LClickCell way of doing things allows us.
	//So we'll directly override HandleMouseDown
	virtual void HandleMouseDown(const JPoint& pt, const JXMouseButton button,
										 const JSize clickCount, 
										 const JXButtonStates& buttonStates,
										 const JXKeyModifiers& modifiers);
	virtual void HandleMouseDrag (const JPoint& pt,
										const JXButtonStates&	buttonStates,
										const JXKeyModifiers&	modifiers);

	virtual bool DoDrag(const JPoint&pt,
								const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers);
	virtual Atom GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers);
	JBoolean WillAcceptDrop(const JArray<Atom>& typeList, 
													Atom* action,
													const Time time, 
													const JXWidget* source);

	virtual void HandleDNDEnter();
	virtual void HandleDNDLeave();
	virtual void HandleDNDHere(const JPoint& pt, const JXWidget* source);
	virtual void HandleDNDDrop(const JPoint& pt,
										 const JArray<Atom>& typeList,
										 const Atom action,
										 const Time time,
										 const JXWidget* source);
	virtual void HandleDNDDidDrop(const Atom& action);

	JPoint startPoint;
	static const int kDebounceWidth = 8;

	//This function should set atom to the proper X Atom for the current
	//selection and return true, or return false if the current selection isn't
	//of a type valid for dragging
	virtual bool	GetSelectionAtom(CFlavorsList& atom);
	virtual void	DNDInit(const JPoint& pt, const JXButtonStates& buttonStates,
												const JXKeyModifiers& modifiers);
			
	virtual bool	ItemIsAcceptable(const JArray<Atom>& typeList); // Check its suitable
	virtual bool	ItemIsAcceptable(Atom type); // Check its suitable

	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);

	virtual bool	IsDropCell(JArray<Atom>& typeList, const STableCell& cell); // Can cell be dropped into
	virtual bool	IsDropAtCell(JArray<Atom>& typeList, STableCell& cell);// Can cell be dropped at (modify if not)

	virtual void	GetLocalRowRect(TableIndexT row, JRect& theRect);	// Get rect of row
	virtual void	DrawDropCell(const STableCell& cell); 				// Draw drag row frame
	virtual void	ClearDropCell(const STableCell& cell); 				// Clear drag row frame
	virtual void	DrawDropCursor(const STableCell &cell); 			// Draw drag row cursor

	//returns true if 2 lists have at least one flavor in common
	//if match is not NULL, *match will be set to the first match found
	bool FlavorsMatch(const JArray<Atom>& list1, const CFlavorsList& list2, 
										Atom* match=NULL) const;
	bool FlavorsMatch(const CFlavorsList& list1, const JArray<Atom>& list2,
										Atom* match=NULL) const
		{ return FlavorsMatch(list2, list1, match);	}
	virtual Atom	GetBestFlavor(const JArray<Atom>& list);			// Get best flavor from drag

	virtual bool	DropData(Atom theFlavor,
								unsigned char* drag_data,
								unsigned long data_size);						// Drop data into whole table
	virtual bool	DropDataIntoCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell);		// Drop data into cell
	virtual bool	DropDataAtCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& cell);	// Drop data at cell
};

#endif
