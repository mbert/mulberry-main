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


// Header for CCalendarStoreTable class

#ifndef __CCALENDARSTORETABLE__MULBERRY__
#define __CCALENDARSTORETABLE__MULBERRY__

#include "CHierarchyTableDrag.h"
#include "CListener.h"

#include "CCalendarStoreNode.h"
#include "CWindowStatesFwd.h"

#include "cdstring.h"

class CCalendarStoreView;
class CKeyModifiers;

namespace calstore
{
	class CCalendarProtocol;
	class CCalendarStoreManager;
}

// ===========================================================================
//	CCalendarStoreTable

class	CCalendarStoreTable : public CHierarchyTableDrag,
								public CListener
{
	DECLARE_DYNCREATE(CCalendarStoreTable)

public:
						CCalendarStoreTable();
	virtual				~CCalendarStoreTable();
	
	virtual void		ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

			void		SetManager(calstore::CCalendarStoreManager* manager);

	virtual void		ResetTable();
	virtual void		ClearTable();

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void		CollapseRow(TableIndexT inWideOpenRow);
	virtual void		DeepCollapseRow(TableIndexT inWideOpenRow);
	
	virtual void		ExpandRow(TableIndexT inWideOpenRow);
	virtual void		DeepExpandRow(TableIndexT inWideOpenRow);

	virtual void		ProcessExpansion(TableIndexT inWideOpenRow, bool expand);

	virtual void	SetColumnInfo(CColumnInfoArray& col_info)	// Reset header details from array
						{ mColumnInfo = &col_info; }

protected:
	CCalendarStoreView*						mTableView;
	bool									mListChanging;			// In the process of changing the list
	vector<calstore::CCalendarStoreNode*>	mData;					// data

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);						// Handle key down
	virtual void	LClickCell(const STableCell& inCell, UINT nFlags);							// Clicked item
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);						// Double-clicked item

	virtual void	DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);				// Draw the items
			
	int		GetPlotIcon(const calstore::CCalendarStoreNode* node,
									calstore::CCalendarProtocol* proto);			// Get appropriate icon id
	void	SetTextStyle(CDC* pDC, const calstore::CCalendarStoreNode* node,
							calstore::CCalendarProtocol* proto, bool& strike);					// Get appropriate text style
	bool 	UsesBackgroundColor(TableIndexT row) const;
	COLORREF GetBackgroundColor(TableIndexT row) const;

	afx_msg void		OnUpdateSelectionCalendar(CCmdUI* pCmdUI);

	virtual void	DoSelectionChanged(void);

			void		DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
			void		DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
			void		DoPreview();
			void		DoPreview(calstore::CCalendarStoreNode* node);
			void		DoFullView();

			void		PreviewCalendar(bool clear = false);										// Do preview of calendar
			void		PreviewCalendar(calstore::CCalendarStoreNode* node, bool clear = false);	// Do preview of calendar

	afx_msg void		OnImport();
			void		DoImportCalendar(calstore::CCalendarStoreNode* node, bool merge);
	afx_msg void		OnExport();
			bool		ExportCalendar(TableIndexT row);
	afx_msg void		OnUpdateLogin(CCmdUI* pCmdUI);
	afx_msg void		OnEditProperties();
			void		OnServerProperties();
			void		OnCalendarProperties();
	afx_msg void		OnLogin();
	afx_msg void		OnNewCalendar();
			calstore::CCalendarStoreNode*	DoCreateCalendar();
	afx_msg void		OnUpdateNewWebCalendar(CCmdUI* pCmdUI);
	afx_msg void		OnNewWebCalendar();
	afx_msg void		OnUpdateRefreshWebCalendar(CCmdUI* pCmdUI);
	afx_msg void		OnRefreshWebCalendar();
			bool		RefreshWebCalendar(TableIndexT row);
	afx_msg void		OnUpdateUploadWebCalendar(CCmdUI* pCmdUI);
	afx_msg void		OnUploadWebCalendar();
			bool		UploadWebCalendar(TableIndexT row);
			void		OnOpenCalendar();
			bool		OpenCalendar(TableIndexT row);
	afx_msg void		OnRenameCalendar();
			bool		RenameCalendar(TableIndexT row);
	afx_msg void		OnDeleteCalendar();
	afx_msg void		OnUpdateRefreshList(CCmdUI* pCmdUI);
	afx_msg void		OnRefreshList();
	afx_msg void		OnFreeBusyCalendar();
			bool		FreeBusyCalendar(TableIndexT row);
	afx_msg void		OnSendCalendar();
			bool		SendCalendar(TableIndexT row);

			void		DoChangeColour(TableIndexT row);

			void	AddNode(calstore::CCalendarStoreNode* node,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
			void	AddChildren(calstore::CCalendarStoreNode* node,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
			void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, UInt32 inFromRow, bool inRefresh);
			void	ExpandRestore(TableIndexT worow);
			void	ExpandAction(TableIndexT worow, bool deep);

			void	AddProtocol(calstore::CCalendarProtocol* proto);
			void	InsertProtocol(calstore::CCalendarProtocol* proto);
			void	RemoveProtocol(calstore::CCalendarProtocol* proto);
			void	ClearProtocol(calstore::CCalendarProtocol* proto);
			void	RefreshProtocol(calstore::CCalendarProtocol* proto);
			void	SwitchProtocol(calstore::CCalendarProtocol* proto);

			void	InsertNode(calstore::CCalendarStoreNode* node);					// Insert a node to the list
			void	DeleteNode(calstore::CCalendarStoreNode* node);					// Delete a node from the list
			void	RefreshNode(calstore::CCalendarStoreNode* node);				// Refresh a node from the list

			void	ClearSubList(calstore::CCalendarStoreNode* node);
			void	RefreshSubList(calstore::CCalendarStoreNode* node);

			bool	TestSelectionServer(TableIndexT row);					// Test for selected servers only
			bool	TestSelectionCalendar(TableIndexT row);					// Test for selected calendars only
			bool	TestSelectionRealCalendar(TableIndexT row);				// Test for selected real (not directory) calendars only
			bool	TestSelectionWebCalendar(TableIndexT row);				// Test for selected web calendars only
			bool	TestSelectionUploadWebCalendar(TableIndexT row);		// Test for selected uploadable web calendars only

	virtual BOOL	DoDrag(TableIndexT row);											// Prevent drag if improper selection
	virtual void	SetDragFlavors(TableIndexT row);
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

	virtual bool	IsDropCell(COleDataObject* pDataObject, const STableCell& cell);			// Draw drag row frame
	virtual bool	IsDropAtCell(COleDataObject* pDataObject, STableCell& cell);		// Test drop at cell
	virtual bool	ValidDragSelection(void) const;										// Check for valid drag selection
	virtual bool	CanDropExpand(COleDataObject* pDataObject, TableIndexT row);		// Can row expand for drop

	virtual DROPEFFECT	GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point); // Determine effect
	virtual bool		DropData(unsigned int theFlavor,
									char* drag_data,
									unsigned long data_size);					// Drop data into whole table
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);						// Drop data into cell
	virtual bool		DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);				// Drop data at cell

private:
	CColumnInfoArray*		mColumnInfo;

			void		InitCalendarStoreTable();

	calstore::CCalendarStoreNode*	GetCellNode(TableIndexT row, bool worow = false) const;			// Get the selected node
	calstore::CCalendarProtocol*	GetCellCalendarProtocol(TableIndexT row) const;			// Get the selected adbk protocol

	bool	AddSelectionToList(TableIndexT row,
										calstore::CCalendarStoreNodeList* list);		// Add selected nodes to list


protected:
	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
