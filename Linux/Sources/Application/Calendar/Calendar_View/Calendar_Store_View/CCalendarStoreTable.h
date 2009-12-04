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
class JXChooseColorDialog;

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
	CCalendarStoreTable(JXScrollbarSet* scrollbarSet, 
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual				~CCalendarStoreTable();
	
			void		SetManager(calstore::CCalendarStoreManager* manager);

	virtual void		ResetTable();
	virtual void		ClearTable();

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh);

	virtual void		ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void		CollapseRow(TableIndexT inWideOpenRow);
	virtual void		DeepCollapseRow(TableIndexT inWideOpenRow);
	
	virtual void		ExpandRow(TableIndexT inWideOpenRow);
	virtual void		DeepExpandRow(TableIndexT inWideOpenRow);

	virtual void		ProcessExpansion(TableIndexT inWideOpenRow, bool expand);

protected:
	CCalendarStoreView*						mTableView;
	bool									mListChanging;			// In the process of changing the list
	std::vector<calstore::CCalendarStoreNode*>	mData;					// data

	virtual void	LClickCell(const STableCell& inCell, const JXKeyModifiers& mods); // Clicked item
	virtual void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods); // Double Clicked item

	virtual void DrawCell(JPainter* pDC, const STableCell& inCell,
								const JRect& inLocalRect);// Draw the message info

	JIndex	GetPlotIcon(const calstore::CCalendarStoreNode* node,
									calstore::CCalendarProtocol* proto);			// Get appropriate icon id
	void	SetTextStyle(JPainter* pDC, const calstore::CCalendarStoreNode* node,
							calstore::CCalendarProtocol* proto, bool& strike);	// Get appropriate text style
	bool 	UsesBackgroundColor(const calstore::CCalendarStoreNode* node) const;
	JColorIndex GetBackgroundColor(const calstore::CCalendarStoreNode* node) const;

	void	OnUpdateSelectionCalendarStoreNode(CCmdUI* pCmdUI);
	void	OnUpdateSelectionCanChangeCalendar(CCmdUI* pCmdUI);

	virtual void	DoSelectionChanged(void);

			void		DoSingleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle click user action
			void		DoDoubleClick(TableIndexT row, const CKeyModifiers& mods);				// Handle double-click user action
			void		DoPreview();
			void		DoPreview(calstore::CCalendarStoreNode* node);
			void		DoFullView();

			void		PreviewCalendar(bool clear = false);										// Do preview of calendar
			void		PreviewCalendar(calstore::CCalendarStoreNode* node, bool clear = false);	// Do preview of calendar

			void		OnImport();
			void		DoImportCalendar(calstore::CCalendarStoreNode* node, bool merge);
			void		OnExport();
			bool		ExportCalendar(TableIndexT row);
			void		OnUpdateLogin(CCmdUI* pCmdUI);
			void		OnServerProperties();
			void		OnCalendarProperties();
			void		OnLogin();
			void		OnNewCalendar();
			calstore::CCalendarStoreNode*	DoCreateCalendar();
			void		OnUpdateNewWebCalendar(CCmdUI* pCmdUI);
			void		OnNewWebCalendar();
			void		OnUpdateRefreshWebCalendar(CCmdUI* pCmdUI);
			void		OnRefreshWebCalendar();
			bool		RefreshWebCalendar(TableIndexT row);
			void		OnUpdateUploadWebCalendar(CCmdUI* pCmdUI);
			void		OnUploadWebCalendar();
			bool		UploadWebCalendar(TableIndexT row);
			void		OnOpenCalendar();
			bool		OpenCalendar(TableIndexT row);
			void		OnRenameCalendar();
			bool		RenameCalendar(TableIndexT row);
			void		OnDeleteCalendar();
			void		OnCheckCalendar();
			bool		CheckCalendar(TableIndexT row);
			void		OnNewHierarchy();
			void		OnRenameHierarchy();
			void		OnDeleteHierarchy();
			void		OnUpdateRefreshList(CCmdUI* pCmdUI);
			void		OnRefreshList();
			void		OnFreeBusyCalendar();
			bool		FreeBusyCalendar(TableIndexT row, calstore::CCalendarStoreFreeBusyList* list);
			void		OnSendCalendar();
			bool		SendCalendar(TableIndexT row);

			void		DoChangeColour(TableIndexT row);

			void	AddNode(calstore::CCalendarStoreNode* node,
							TableIndexT& row, bool child, bool refresh = false);	// Add a node to the list
			void	AddChildren(calstore::CCalendarStoreNode* node,
							TableIndexT& parent_row, bool refresh = false);			// Add child nodes to the list
			void	RemoveChildren(TableIndexT& parent_row, bool refresh = false);	// Remove child nodes from the list
	virtual void	RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh);
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
			bool	TestSelectionCalendarStoreNode(TableIndexT row);					// Test for selected calendars only
			bool	TestSelectionCanChangeCalendar(TableIndexT row);					// Test for selected calendars only
			bool	TestSelectionRealCalendar(TableIndexT row);				// Test for selected real (not directory) calendars only
			bool	TestSelectionWebCalendar(TableIndexT row);				// Test for selected web calendars only
			bool	TestSelectionUploadWebCalendar(TableIndexT row);		// Test for selected uploadable web calendars only
			bool	TestSelectionHierarchy(TableIndexT row);					// Test for selected calendars only

private:
	JRGB					mColourChoose;
	JXChooseColorDialog*	mColourChooser;
	bool					mColourChosen;

			void		InitCalendarStoreTable();

	calstore::CCalendarStoreNode*	GetCellNode(TableIndexT row, bool worow = false) const;			// Get the selected node
	calstore::CCalendarProtocol*	GetCellCalendarProtocol(TableIndexT row) const;					// Get the selected adbk protocol

	bool	AddSelectionToList(TableIndexT row,
										calstore::CCalendarStoreNodeList* list);		// Add selected nodes to list

// Drag methods
protected:
	virtual bool RenderSelectionData(CMulSelectionData* selection, Atom type);

	virtual bool GetSelectionAtom(CFlavorsList& atoms);
	virtual bool DoDrag(const JPoint& pt,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers );

	// Prevent drag if improper selection
	virtual bool	IsDropCell(JArray<Atom>& typeList, const STableCell& cell); // Can cell be dropped into
	virtual bool	IsDropAtCell(JArray<Atom>& typeList, STableCell& cell);// Can cell be dropped at (modify if not)

	virtual bool ValidDragSelection(void) const;        // Check for valid drag selection
	virtual bool CanDropExpand(const JArray<Atom>& typeList, TableIndexT row);  // Can row expand for drop

	virtual Atom GetDNDAction(const JXContainer* target, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers);

	virtual bool  DropData(Atom theFlavor,
							 unsigned char* drag_data,
							 unsigned long data_size);     // Drop data into whole table
	virtual bool  DropDataIntoCell(Atom theFlavor,
									 unsigned char* drag_data,
									 unsigned long data_size,
									 const STableCell& cell);        // Drop data into cell
	virtual bool  DropDataAtCell(Atom theFlavor,
								 unsigned char* drag_data,
								 unsigned long data_size,
								 const STableCell& before_cell);     // Drop data at cell
};

#endif
