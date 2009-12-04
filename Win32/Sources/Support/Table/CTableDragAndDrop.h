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

class CTableDragAndDrop;

class CTableDropSource : public COleDropSource
{
public:
					CTableDropSource();
	virtual 		~CTableDropSource();

// Drag methods
protected:
	virtual BOOL 	OnBeginDrag(CWnd* pWnd);
};

class CTableDropSupport : public COleDropTarget
{
public:
					CTableDropSupport(CTableDragAndDrop* itsTable);
	virtual 		~CTableDropSupport();

	BOOL RegisterNow(void)								// Tell the world to drag to us
			{ return Register((CWnd*) mTablePane); }

// Drag methods - pass up
protected:
	CTableDragAndDrop* mTablePane;					// Pane containing table

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);
	virtual void OnDragStartScroll(CWnd* pWnd);
	virtual DROPEFFECT OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
};

class CTableDragSupport : public COleDataSource
{
public:
					CTableDragSupport(CTableDragAndDrop* itsTable);
	virtual 		~CTableDragSupport();

// Drag methods - pass up
protected:
	CTableDragAndDrop* mTablePane;					// Pane containing table

	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
};

class CTableDragAndDrop : public CTable
{
	friend class CTableDragSupport;
	friend class CTableDropSupport;

	DECLARE_DYNAMIC(CTableDragAndDrop)

public:
					CTableDragAndDrop();
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
	virtual void	AddDragFlavor(unsigned int theFlavor)			// Set its drag flavor
						{ mDragFlavors.Add(theFlavor); }
	virtual void	AddDropFlavor(unsigned int theFlavor)			// Set its drop flavor
						{ mDropFlavors.Add(theFlavor); }
	
	virtual STableCell&		GetLastDragCell(void)
						{ return mLastDropCell; }					// Get last drag cell
	virtual STableCell&		GetLastDragCursor(void)
						{ return mLastDropCursor; }					// Get last drag cursor

	//{{AFX_MSG(CLetterHeaderView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	virtual BOOL 		SubclassDlgItem(UINT nID, CWnd* pParent);

// Drag methods
protected:
	CTableDropSupport	mDrop;						// Drag and drop capabilities
	CTableDragSupport	mDrag;						// Drag and drop capabilities
	static CTable*		sTableDragSource;			// Source of drag
	static CTable*		sTableDropTarget;			// Target of drag
	STableCell			mLastHitCell;				// Last cell hit by mouse
	STableCell			mLastDropCell;				// Last row for cell hilite
	STableCell			mLastDropCursor;			// Last row for cursor
	bool				mReadOnly;					// Table not editable
	bool				mDropCell;					// Drop into individual cells
	bool				mDropCursor;				// Drop at cell
	bool				mAllowDrag;					// Allow drag from self
	bool				mSelfDrag;					// Allow drag to self
	bool				mExternalDrag;				// Allow drag from external
	bool				mAllowMove;					// Allow drag to delete originals
	bool				mHandleMove;				// Do delete original on drag move
	CDWordArray			mDragFlavors;				// List of flavors to send
	CDWordArray			mDropFlavors;				// List of flavors to accept

	virtual void LClickCell(const STableCell& inCell, UINT 	nFlags);						// Initiate drag

	virtual BOOL DoDrag(TableIndexT row);
	virtual void SetDragFlavors(TableIndexT row);
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);
	virtual void OnDragStartScroll(CWnd* pWnd);
	virtual DROPEFFECT OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);

	virtual DROPEFFECT GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point); // Determine effect

	virtual bool	ItemIsAcceptable(COleDataObject* pDataObject);					// Check its suitable
	virtual bool	IsDropCell(COleDataObject* pDataObject, const STableCell& cell);				// Can cell be dropped into
	virtual bool	IsDropAtCell(COleDataObject* pDataObject, STableCell& cell);			// Can cell be dropped at (modify if not)
	virtual void	DrawDropCell(COleDataObject* pDataObject, const STableCell& cell);				// Draw drag row frame
	virtual void	DrawDropCursor(COleDataObject* pDataObject, const STableCell& cell);	// Draw drag row cursor

	virtual unsigned int GetBestFlavor(COleDataObject* pDataObject);			// Get best flavor from drag
	virtual bool	DropData(unsigned int theFlavor,
								char* drag_data,
								unsigned long data_size);						// Drop data into whole table
	virtual bool	DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size, const STableCell& cell);		// Drop data into cell
	virtual bool	DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size, const STableCell& cell);	// Drop data at cell

	DECLARE_MESSAGE_MAP()
};

#endif
