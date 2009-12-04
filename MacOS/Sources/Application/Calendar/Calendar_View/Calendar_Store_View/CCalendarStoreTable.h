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

#include "CCalendarStoreFreeBusy.h"
#include "CCalendarStoreNode.h"

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
public:
	enum { class_ID = 'CSTb' };

						CCalendarStoreTable();
						CCalendarStoreTable(LStream *inStream);
	virtual				~CCalendarStoreTable();
	
			void		SetManager(calstore::CCalendarStoreManager* manager);

	virtual void		ResetTable();
	virtual void		ClearTable();

	virtual void		ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void		CollapseRow(TableIndexT inWideOpenRow);
	virtual void		DeepCollapseRow(TableIndexT inWideOpenRow);
	
	virtual void		ExpandRow(TableIndexT inWideOpenRow);
	virtual void		DeepExpandRow(TableIndexT inWideOpenRow);

	virtual void		ProcessExpansion(TableIndexT inWideOpenRow, bool expand);

protected:
	CCalendarStoreView*						mTableView;
	bool									mListChanging;			// In the process of changing the list
	short									mHierarchyCol;			// Column containing names
	std::vector<calstore::CCalendarStoreNode*>	mData;					// data

	virtual void		FinishCreateSelf();			// Get details of sub-panes

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

	virtual void		ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell
	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);	// Click

	virtual void		DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);			// Draw the message info

	virtual void	CalcCellFlagRect(const STableCell &inCell,
										Rect &outRect);						
			
	ResIDT	GetPlotIcon(const calstore::CCalendarStoreNode* node,
									calstore::CCalendarProtocol* proto);			// Get appropriate icon id
	void	SetTextStyle(const calstore::CCalendarStoreNode* node,
							calstore::CCalendarProtocol* proto, bool& strike);	// Get appropriate text style
	bool 	UsesBackgroundColor(const calstore::CCalendarStoreNode* node) const;
	const RGBColor& GetBackgroundColor(const calstore::CCalendarStoreNode* node) const;

	virtual void	DoSelectionChanged(void);

			void		DoSingleClick(unsigned long row, const CKeyModifiers& mods);				// Handle click user action
			void		DoDoubleClick(unsigned long row, const CKeyModifiers& mods);				// Handle double-click user action
			void		DoPreview();
			void		DoPreview(calstore::CCalendarStoreNode* node);
			void		DoFullView();

			void		PreviewCalendar(bool clear = false);										// Do preview of calendar
			void		PreviewCalendar(calstore::CCalendarStoreNode* node, bool clear = false);	// Do preview of calendar

			void		OnImport();
			void		DoImportCalendar(calstore::CCalendarStoreNode* node, bool merge);
			void		OnExport();
			bool		ExportCalendar(TableIndexT row);
			void		OnServerProperties();
			void		OnCalendarProperties();
			void		OnLogin();
			void		OnNewCalendar();
			calstore::CCalendarStoreNode*	DoCreateCalendar();
			void		OnNewWebCalendar();
			void		OnRefreshWebCalendar();
			bool		RefreshWebCalendar(TableIndexT row);
			void		OnUploadWebCalendar();
			bool		UploadWebCalendar(TableIndexT row);
			void		OnOpenCalendar();
			bool		OpenCalendar(TableIndexT row);
			void		OnRenameCalendar();
			bool		RenameCalendar(TableIndexT row);
			void		OnDeleteCalendar();
			void		OnCheckCalendar();
			bool		CheckCalendar(TableIndexT row);
			void		OnNewHierarchy();							// Create new hierarchy
			void		OnRenameHierarchy();						// Rename hierarchy
			void		OnDeleteHierarchy();						// Delete hierarchy
			
			void		OnRefreshList();
			void		OnFreeBusyCalendar();
			bool		FreeBusyCalendar(TableIndexT row,
											calstore::CCalendarStoreFreeBusyList* list);
			void		OnSendCalendar();
			bool		SendCalendar(TableIndexT row);

			void		DoChangeColour(TableIndexT row);

			void	AddNode(calstore::CCalendarStoreNode* node,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
			void	AddChildren(calstore::CCalendarStoreNode* node,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
			void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh);
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
			bool	TestSelectionCalendarStoreNode(TableIndexT row);		// Test for non-protocol node
			bool	TestSelectionCanChangeCalendar(TableIndexT row);		// Test editable calendar store node
			bool	TestSelectionRealCalendar(TableIndexT row);				// Test for selected real (not directory) calendars only
			bool	TestSelectionWebCalendar(TableIndexT row);				// Test for selected web calendars only
			bool	TestSelectionUploadWebCalendar(TableIndexT row);		// Test for selected uploadable web calendars only
			
			// Hierarchy related
			bool	TestSelectionHierarchy(TableIndexT row);				// Test for selected hierarchies only
	
private:
			void		InitCalendarStoreTable();

	calstore::CCalendarStoreNode*	GetCellNode(TableIndexT row, bool worow = false) const;			// Get the selected node
	calstore::CCalendarProtocol*	GetCellCalendarProtocol(TableIndexT row) const;					// Get the selected adbk protocol

	bool	AddSelectionToList(TableIndexT row,
										calstore::CCalendarStoreNodeList* list);		// Add selected nodes to list

// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	virtual bool	ValidDragSelection() const;									// Check for valid drag selection
	virtual bool	CanDropExpand(DragReference inDragRef,
									TableIndexT woRow);							// Can row expand for drop
	virtual bool	IsCopyCursor(DragReference inDragRef);						// Use copy cursor?

	virtual bool	IsDropCell(DragReference inDragRef, STableCell row);		// Can cell be dropped into
	virtual bool	IsDropAtCell(DragReference inDragRef, STableCell& aCell);	// Test drop at cell

	virtual void	DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size);						// Drop data into table
	virtual void	DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);				// Drop data into cell
	virtual void	DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);			// Drop data at cell
};

#endif
