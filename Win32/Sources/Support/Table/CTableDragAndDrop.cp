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


// Source for CTableDragAndDrop class

#include "CTableDragAndDrop.h"

#include "CLog.h"

#pragma mark ____________________________________CTableDropSource

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDropSource::CTableDropSource()
{
}

// Default destructor
CTableDropSource::~CTableDropSource()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Special - do not lose mouse up events
BOOL CTableDropSource::OnBeginDrag(CWnd* pWnd)
{
	ASSERT_VALID(this);

	m_bDragStarted = FALSE;

	// opposite button cancels drag operation
#ifndef _MAC
	m_dwButtonCancel = 0;
	m_dwButtonDrop = 0;
	if (GetKeyState(VK_LBUTTON) < 0)
	{
		m_dwButtonDrop |= MK_LBUTTON;
		m_dwButtonCancel |= MK_RBUTTON;
	}
	else if (GetKeyState(VK_RBUTTON) < 0)
	{
		m_dwButtonDrop |= MK_RBUTTON;
		m_dwButtonCancel |= MK_LBUTTON;
	}
#endif

	DWORD dwLastTick = GetTickCount();
	pWnd->SetCapture();

	while (!m_bDragStarted)
	{
		// some applications steal capture away at random times
		if (CWnd::GetCapture() != pWnd)
			break;

		// peek for next input message
		MSG msg;
		if (PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
			PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
		{
			// check for button cancellation (any button down will cancel)
			if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP ||
				msg.message == WM_LBUTTONDOWN || msg.message == WM_RBUTTONDOWN)
				break;

			// Manually look for button up as WM_LBUTTONUP is not received
			if (::GetKeyState(VK_LBUTTON) >= 0)
				break;
	
			// check for keyboard cancellation
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
				break;

			// check for drag start transition
			m_bDragStarted = !m_rectStartDrag.PtInRect(msg.pt);
		}

		// if the user sits here long enough, we eventually start the drag
		if (GetTickCount() - dwLastTick > nDragDelay)
			m_bDragStarted = TRUE;
	}
	ReleaseCapture();

	return m_bDragStarted;
}

#pragma mark ____________________________________CTableDropSupport

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R O P S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDropSupport::CTableDropSupport(CTableDragAndDrop* itsTable)
{
	mTablePane = itsTable;
}

// Default destructor
CTableDropSupport::~CTableDropSupport()
{
	// No more dragging
	Revoke();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

DROPEFFECT CTableDropSupport::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return mTablePane->OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}

void CTableDropSupport::OnDragLeave(CWnd* pWnd)
{
	mTablePane->OnDragLeave(pWnd);
}

DROPEFFECT CTableDropSupport::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return mTablePane->OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

DROPEFFECT CTableDropSupport::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	DROPEFFECT dropEffect = mTablePane->OnDragScroll(pWnd, dwKeyState, point);

	// DROPEFFECT_SCROLL means do the default
	if (dropEffect != DROPEFFECT_SCROLL)
		return dropEffect;

	// get client rectangle of destination window
	CRect rectClient;
	mTablePane->GetClientRect(&rectClient);
	CRect rect = rectClient;

	// hit-test against inset region
	UINT nTimerID = MAKEWORD(-1, -1);
	rect.InflateRect(-nScrollInset, -nScrollInset);
	if (rectClient.PtInRect(point) && !rect.PtInRect(point))
	{
		// determine which way to scroll along both X & Y axis
		if (point.x < rect.left)
		{
			// Check ability to scroll first
			if (mTablePane->GetScrollPos(SB_HORZ) > 0)
				nTimerID = MAKEWORD(SB_LINEUP, HIBYTE(nTimerID));
		}
		else if (point.x >= rect.right)
		{
			// Check ability to scroll first
			if (mTablePane->GetScrollPos(SB_HORZ) < mTablePane->GetScrollLimit(SB_HORZ))
				nTimerID = MAKEWORD(SB_LINEDOWN, HIBYTE(nTimerID));
		}
		if (point.y < rect.top)
		{
			// Check ability to scroll first
			if (mTablePane->GetScrollPos(SB_VERT) > 0)
				nTimerID = MAKEWORD(LOBYTE(nTimerID), SB_LINEUP);
		}
		else if (point.y >= rect.bottom)
		{
			// Check ability to scroll first
			if (mTablePane->GetScrollPos(SB_VERT) < mTablePane->GetScrollLimit(SB_VERT))
				nTimerID = MAKEWORD(LOBYTE(nTimerID), SB_LINEDOWN);
		}
	}

	if (nTimerID == MAKEWORD(-1, -1))
	{
		if (m_nTimerID != MAKEWORD(-1, -1))
		{
			// send fake OnDragStopScroll when transition from scroll->normal
			COleDataObject dataObject;
			dataObject.Attach(m_lpDataObject, FALSE);
			OnDragStopScroll(pWnd, &dataObject, dwKeyState, point);
			m_nTimerID = MAKEWORD(-1, -1);
		}
		return DROPEFFECT_NONE;
	}

	// save tick count when timer ID changes
	DWORD dwTick = GetTickCount();
	if (nTimerID != m_nTimerID)
	{
		m_dwLastTick = dwTick;
		m_nScrollDelay = nScrollDelay;
	}

	// scroll if necessary
	if (dwTick - m_dwLastTick > m_nScrollDelay)
	{
		if (LOBYTE(nTimerID) != -1)
			mTablePane->SendMessage(WM_HSCROLL, LOBYTE(nTimerID), 0L);
		if (HIBYTE(nTimerID) != -1)
			mTablePane->SendMessage(WM_VSCROLL, HIBYTE(nTimerID), 0L);
		m_dwLastTick = dwTick;
		m_nScrollDelay = nScrollInterval;
	}
	if (m_nTimerID == MAKEWORD(-1, -1))
	{
		// send OnDragStartScroll when transitioning from normal->scroll
		OnDragStartScroll(pWnd);
	}

	m_nTimerID = nTimerID;
	// check for force copy
	if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
		dropEffect = DROPEFFECT_SCROLL|DROPEFFECT_COPY;
	// check for force move
	else if ((dwKeyState & MK_ALT) == MK_ALT ||
		(dwKeyState & MK_SHIFT) == MK_SHIFT)
		dropEffect = DROPEFFECT_SCROLL|DROPEFFECT_MOVE;
	// default -- recommended action is move
	else
		dropEffect = DROPEFFECT_SCROLL|DROPEFFECT_MOVE;
	return dropEffect;
}

void CTableDropSupport::OnDragStartScroll(CWnd* pWnd)
{
	mTablePane->OnDragStartScroll(pWnd);
}

DROPEFFECT CTableDropSupport::OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	return mTablePane->OnDragStopScroll(pWnd, pDataObject, dwKeyState, point);
}

BOOL CTableDropSupport::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	return mTablePane->OnDrop(pWnd, pDataObject, dropEffect, point);
}

DROPEFFECT CTableDropSupport::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
	return mTablePane->OnDropEx(pWnd, pDataObject, dropDefault, dropList, point);
}

#pragma mark ____________________________________CTableDragSupport

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G S U P P O R T
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDragSupport::CTableDragSupport(CTableDragAndDrop* itsTable)
{
	mTablePane = itsTable;
}

// Default destructor
CTableDragSupport::~CTableDragSupport()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

BOOL CTableDragSupport::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	return mTablePane->OnRenderGlobalData(lpFormatEtc, phGlobal);
}

#pragma mark ____________________________________CTableDragAndDrop

IMPLEMENT_DYNAMIC(CTableDragAndDrop, CTable)

BEGIN_MESSAGE_MAP(CTableDragAndDrop, CTable)
	ON_WM_MOUSEACTIVATE()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E D R A G A N D D R O P S U P P O R T
// __________________________________________________________________________________________________

CTable* CTableDragAndDrop::sTableDragSource = NULL;
CTable* CTableDragAndDrop::sTableDropTarget = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDragAndDrop::CTableDragAndDrop() : mDrop(this), mDrag(this)
{
	mLastHitCell.SetCell(0, 0);
	mLastDropCell.SetCell(0, 0);
	mLastDropCursor.SetCell(0, 0);
	mReadOnly = false;
	mDropCell = false;
	mDropCursor = false;
	mAllowDrag = true;
	mSelfDrag = false;
	mExternalDrag = true;
	mAllowMove = true;
	mHandleMove = true;
}

// Default destructor
CTableDragAndDrop::~CTableDragAndDrop()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CTableDragAndDrop::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTable::OnCreate(lpCreateStruct) == -1)
		return -1;
		
	// register drop target
	mDrop.RegisterNow();
	
	return 0;
}

int CTableDragAndDrop::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	// If dragging is support must prevent initial window activation to
	// allow drags from background windows
	return (mAllowDrag && (nHitTest == HTCLIENT) && (message == WM_LBUTTONDOWN)) ?
				MA_NOACTIVATE : CTable::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

BOOL CTableDragAndDrop::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	// Do inherited
	BOOL result = CTable::SubclassDlgItem(nID, pParent);
		
	// register drop target
	mDrop.RegisterNow();

	return result;
}

// Initiate drag
void CTableDragAndDrop::LClickCell(const STableCell& inCell, UINT nFlags)
{
	// Try drag and drop
	if (mAllowDrag)
	{
		if (DragDetect(mLDownPoint))
		{
			DoDrag(inCell.row);
			return;
		}
	}

	// Active if mouse activate not done
	GetParentFrame()->ActivateFrame();
	SetFocus();

	CTable::LClickCell(inCell, nFlags);
}

BOOL CTableDragAndDrop::DoDrag(TableIndexT row)
{
	// Always empty before adding new flavors
	mDrag.Empty();

	// Add allowed flavors to source
	SetDragFlavors(row);
	
	// Get row rect
	CRect rowRect;
	GetLocalRowRect(row, rowRect);
	ClientToScreen(rowRect);

	sTableDragSource = this;
	sTableDropTarget = NULL;
	CTableDropSource* pDropSource = new CTableDropSource;
	DROPEFFECT de = mDrag.DoDragDrop(DROPEFFECT_COPY | (mAllowMove ? DROPEFFECT_MOVE : 0), rowRect, pDropSource);
	delete pDropSource;
	sTableDragSource = NULL;

	// Handle move operation
	if ((de == DROPEFFECT_MOVE) && mAllowMove && mHandleMove)
		DeleteSelection();
	
	// If drag to self re-enable drawing
	if (sTableDropTarget == this)
		Changing(false);

	// Always empty after drag
	mDrag.Empty();

	return (de != DROPEFFECT_NONE);
}

void CTableDragAndDrop::SetDragFlavors(TableIndexT row)
{
	// Add allowed flavors to source
	for(int i = 0; i < mDragFlavors.GetSize(); i++)
	{
		unsigned int theFlavor = mDragFlavors[i];
	
		mDrag.DelayRenderData(theFlavor);
	}
}

BOOL CTableDragAndDrop::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	return false;
}

DROPEFFECT CTableDragAndDrop::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Set current droptarget as this
	sTableDropTarget = this;

	// Set flag for drawing
	mIsDropTarget = true;

	// Reset row hiliting
	mLastHitCell.SetCell(0, 0);
	mLastDropCell.SetCell(0, 0);
	mLastDropCursor.SetCell(0, 0);

	// Check flavor and self drag
	if (!ItemIsAcceptable(pDataObject) ||
		((sTableDragSource == this) && !mSelfDrag) ||
		((sTableDragSource != this) && !mExternalDrag))
		return DROPEFFECT_NONE;
	else
		return OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

void CTableDragAndDrop::OnDragLeave(CWnd* pWnd)
{
	// Unhighlight if drop into cell or cursor
	if (mDropCell)
		DrawDropCell(NULL, STableCell(0, 0));
	if (mDropCursor)
		DrawDropCursor(NULL, STableCell(0, 0));

	// Set flag for drawing
	mIsDropTarget = false;
}

DROPEFFECT CTableDragAndDrop::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Check flavor and self drag
	if (!ItemIsAcceptable(pDataObject) ||
		((sTableDragSource == this) && !mSelfDrag) ||
		((sTableDragSource != this) && !mExternalDrag))
		return DROPEFFECT_NONE;

	DROPEFFECT de = GetDropEffect(pWnd, pDataObject, dwKeyState, point);

	CPoint	imagePt;
	LocalToImagePoint(point, imagePt);

	TableIndexT rows, cols;
	GetTableSize(rows, cols);

	// Do cell highlight if drop into cell
	bool hilite = false;
	if (mDropCell && rows)
	{
		// Get the hit cell
		STableCell	hitCell;
		GetCellHitBy(imagePt, hitCell);
		mLastHitCell = hitCell;

		// Must be able to drop
		if (!IsDropCell(pDataObject, hitCell))
			hitCell.SetCell(0, 0);
		else if (mDropCursor)
		{
			// Check if close to edge of cell and drop cursor allowed
			CRect rowRect;
			GetLocalCellRect(hitCell, rowRect);
			
			if ((point.y < rowRect.top + 2) || (point.y > rowRect.bottom - 2))
				hitCell.SetCell(0, 0);
			else
				hilite = true;
		}
		else
			hilite = true;

		// Draw new cursor
		DrawDropCell(pDataObject, hitCell);
	}

	if (mDropCursor && rows)
	{
		// Get the hit cell
		STableCell	hitCell;
		GetCellHitBy(imagePt, hitCell);

		CRect rowRect;
		GetLocalCellRect(hitCell, rowRect);

		CPoint offset = imagePt;
		offset.Offset(0, rowRect.Height()/2);

		// Get the hit cell
		GetCellHitBy(imagePt, hitCell);
		
		if (!hilite)
		{
			if (!IsValidCell(hitCell))
				hitCell.SetCell(rows + 1, 1);
			
			if (!IsDropAtCell(pDataObject, hitCell))
				hitCell.SetCell(0, 0);
		}
		else
			hitCell.SetCell(0, 0);

		DrawDropCursor(pDataObject, hitCell);
	}

	return de;
}

DROPEFFECT CTableDragAndDrop::OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point)
{
	return DROPEFFECT_SCROLL;
}

void CTableDragAndDrop::OnDragStartScroll(CWnd* pWnd)
{
	// Just pretend to leave
	OnDragLeave(pWnd);
}

DROPEFFECT CTableDragAndDrop::OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Just pretend to enter
	return OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}

BOOL CTableDragAndDrop::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	bool done = false;
	unsigned int theFlavor = GetBestFlavor(pDataObject);
	bool free_hglb = false;
	HGLOBAL hglb = pDataObject->GetGlobalData(theFlavor);
	
	if (!hglb)
	{
		STGMEDIUM stg;
		if (pDataObject->GetData(theFlavor, &stg))
		{
			switch(stg.tymed)
			{
			case TYMED_HGLOBAL:
				hglb = ::GlobalAlloc(GMEM_DDESHARE, ::GlobalSize(stg.hGlobal));
				if (hglb)
				{
					unsigned char* dest = static_cast<unsigned char*>(::GlobalLock(hglb));
					unsigned char* src = static_cast<unsigned char*>(::GlobalLock(stg.hGlobal));
					::memcpy(dest, src, ::GlobalSize(stg.hGlobal));
					::GlobalUnlock(stg.hGlobal);
					::GlobalUnlock(hglb);
					free_hglb = true;
				}
				break;
			case TYMED_ISTREAM:
				// Count bytes in stream
				unsigned long ctr = 0;
				char buf[1024];
				unsigned long actual = 0;
				while(true)
				{
					stg.pstm->Read(buf, 1024, &actual);
					ctr += actual;
					if (actual != 1024)
						break;
				}
				_LARGE_INTEGER lint = {0, 0};
				stg.pstm->Seek(lint, STREAM_SEEK_SET, NULL);
				hglb = ::GlobalAlloc(GMEM_DDESHARE, ctr);
				if (hglb)
				{
					unsigned char* dest = static_cast<unsigned char*>(::GlobalLock(hglb));
					while(true)
					{
						stg.pstm->Read(dest, 1024, &actual);
						dest += actual;
						if (actual != 1024)
							break;
					}
					::GlobalUnlock(hglb);
					free_hglb = true;
				}
			}
			::ReleaseStgMedium(&stg);
		}
	}

	if (hglb)
	{
		// Force visual update off
		Changing(true);

		char* lptstr = (char*) ::GlobalLock(hglb);
		DWORD gsize = ::GlobalSize(hglb);			// This value is meaningless as it might be bigger than actual data
		try
		{
			// Check for drop into cell
			bool was_dropped = false;
			if (mDropCell)
			{
				STableCell dropCell;
				CPoint imagePt;
				LocalToImagePoint(point, imagePt);
				if (GetCellHitBy(imagePt, dropCell) && IsDropCell(pDataObject, dropCell))
				{
					if (mDropCursor)
					{
						// Check if close to edge of cell and drop cursor allowed
						CRect rowRect;
						GetLocalCellRect(dropCell, rowRect);
						
						if ((point.y < rowRect.top + 2) || (point.y > rowRect.bottom - 2))
							dropCell.row = 0;
					}

					if (dropCell.row > 0)
					{
						// Prevent drop in own selection
						if ((sTableDragSource != this) || !CellIsSelected(dropCell))
						{
							done = DropDataIntoCell(theFlavor, lptstr, gsize, dropCell);
							was_dropped = (dropCell.row > 0);
						}
					}
				}
			}
			if (mDropCursor && !was_dropped)
			{
				// Get the hit cell
				STableCell	hitCell;
				CPoint imagePt;
				LocalToImagePoint(point, imagePt);
				GetCellHitBy(imagePt, hitCell);

				CRect rowRect;
				GetLocalCellRect(hitCell, rowRect);

				CPoint offset = point;
				offset.Offset(0, rowRect.Height()/2);

				// Get the hit cell
				LocalToImagePoint(offset, imagePt);
				GetCellHitBy(imagePt, hitCell);
				
				if (mRows == 0)
					hitCell.row = 1;
				else if (!IsValidCell(hitCell))
					hitCell.row = mRows + 1;
				
				if ((hitCell.row > 0) && IsDropAtCell(pDataObject, hitCell))
				{
					done = DropDataAtCell(theFlavor, lptstr, gsize, hitCell);
					was_dropped = true;
				}
			}
			
			if (!was_dropped)
				done = DropData(theFlavor, lptstr, gsize);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
		
		::GlobalUnlock(hglb);
		if (free_hglb)
			::GlobalFree(hglb);

		// Allow drawing if not drop to self
		if (sTableDragSource != sTableDropTarget)
			Changing(false);
	}

	// Clean up highlighting here - must do this after drop in case
	// spring-loaded drop into row is collapsed away
	OnDragLeave(pWnd);


	return done;
}

DROPEFFECT CTableDragAndDrop::OnDropEx(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
	return (DROPEFFECT)-1;  // not implemented
}

DROPEFFECT CTableDragAndDrop::GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	DROPEFFECT de;

	// check for force copy
	if ((dwKeyState & MK_CONTROL) == MK_CONTROL)
		de = DROPEFFECT_COPY;
	// check for force move
	else if ((dwKeyState & MK_ALT) == MK_ALT)
		de = DROPEFFECT_MOVE;
	// default -- recommended action is move
	else
		de = DROPEFFECT_MOVE;

	return de;
}

// Check whether drag item is acceptable
bool CTableDragAndDrop::ItemIsAcceptable(COleDataObject* pDataObject)
{
	// Cannot accept if read only
	if (mReadOnly)
		return false;

	// Check for mDropFlavors
	for(int i = 0; i < mDropFlavors.GetSize(); i++)
	{
		unsigned int theFlavor = mDropFlavors[i];
	
		if (pDataObject->IsDataAvailable(theFlavor))
			return true;
	}
	
	return false;

} // CTableDragAndDrop::ItemIsAcceptable

// Test drop into cell
bool CTableDragAndDrop::IsDropCell(COleDataObject* pDataObject, const STableCell& cell)
{
	return true;
}

// Test drop at cell
bool CTableDragAndDrop::IsDropAtCell(COleDataObject* pDataObject, STableCell& cell)
{
	return true;
}

// Draw drag insert cell
void CTableDragAndDrop::DrawDropCell(COleDataObject* pDataObject, const STableCell& cell)
{
	// Only do if different
	if (mLastDropCell.row == cell.row)
		return;

	CDC* pDC = GetDC();

	// First remove current drag hilite
	if (mLastDropCell.row > 0)
	{
		// Get row rect
		CRect rowRect;
		GetLocalCellRect(mLastDropCell, rowRect);

		pDC->DrawFocusRect(rowRect);
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		// Get row rect
		CRect rowRect;
		GetLocalCellRect(cell, rowRect);

		pDC->DrawFocusRect(rowRect);
	}
	
	ReleaseDC(pDC);

	// Reset current value
	mLastDropCell = cell;
}

// Draw drag insert cursor
void CTableDragAndDrop::DrawDropCursor(COleDataObject* pDataObject, const STableCell& cell)
{
	// Only do if different
	if (mLastDropCursor.row == cell.row)
		return;

	CDC* pDC = GetDC();

	// Get current table size
	TableIndexT rows, cols;
	GetTableSize(rows, cols);

	CRect clientRect;
	GetClientRect(clientRect);

	// First remove current drag hilite
	if (mLastDropCursor.row > 0)
	{
		// Get row rect
		CRect rowRect;
		if (mLastDropCursor <= rows)
		{
			GetLocalCellRect(mLastDropCursor, rowRect);
			rowRect.bottom = rowRect.top + 2;
		}
		else
		{
			STableCell temp = mLastDropCursor;
			temp.row--;
			GetLocalCellRect(temp, rowRect);
			rowRect.top = rowRect.bottom - 2;
		}

		// Line goes all the way across the visible part of the list
		rowRect.left = clientRect.left;
		rowRect.right = clientRect.right;
		pDC->DrawFocusRect(rowRect);
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		// Get row rect
		CRect rowRect;
		if (cell.row <= rows)
		{
			GetLocalCellRect(cell, rowRect);
			rowRect.bottom = rowRect.top + 2;
		}
		else
		{
			STableCell temp = cell;
			temp.row--;
			GetLocalCellRect(temp, rowRect);
			rowRect.top = rowRect.bottom - 2;
		}

		// Line goes all the way across the visible part of the list
		rowRect.left = clientRect.left;
		rowRect.right = clientRect.right;
		pDC->DrawFocusRect(rowRect);
	}
	
	ReleaseDC(pDC);

	// Reset current value
	mLastDropCursor = cell;
}

// Check whether drag item is acceptable
unsigned int CTableDragAndDrop::GetBestFlavor(COleDataObject* pDataObject)
{
	// Check against mDropFlavors in order
	for(int i = 0; i < mDropFlavors.GetSize(); i++)
	{
		unsigned int theFlavor = mDropFlavors[i];
	
		if (pDataObject->IsDataAvailable(theFlavor))
			return theFlavor;
	}
	
	return 0;

}

// Drop data into whole table
bool CTableDragAndDrop::DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size)
{
	return false;
}

// Drop data into cell
bool CTableDragAndDrop::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& cell)
{
	return false;
}

// Drop data at cell
bool CTableDragAndDrop::DropDataAtCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& before_cell)
{
	return false;
}
