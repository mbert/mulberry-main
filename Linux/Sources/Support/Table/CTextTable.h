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


// Header for CTextTable class

#ifndef __CTEXTTABLE__MULBERRY__
#define __CTEXTTABLE__MULBERRY__

#include "CTableDragAndDrop.h"
#include "cdstring.h"
#include "templs.h"

// Classes
class JPainter;

class CTextTable : public CTableDragAndDrop
{

public:
	static const JCharacter* kSelectionChanged;
	static const JCharacter* kLDblClickCell;

	class TextTableMessage : public JBroadcaster::Message
	{
	public:
		TextTableMessage(const JCharacter* type)
			: JBroadcaster::Message(type)
			{ };
	};

	enum
	{
		eBroadcast_Drag = 'drag'
	};

	struct SBroadcastDrag
	{
		CTextTable* mTable;
		ulvector mDragged;
		unsigned long mDropped;
	};

	CTextTable(JXScrollbarSet* scrollbarSet,
			  JXContainer* enclosure,
			  const HSizingOption hSizing, 
			  const VSizingOption vSizing,
			  const JCoordinate x, const JCoordinate y,
			  const JCoordinate w, const JCoordinate h);
	virtual	~CTextTable();

public:
	void SetDoubleClickMsg(bool inMessage)
		{ mDoubleClickMsg = inMessage; }
	
	void SetSelectionMsg(bool inMessage)
		{ mSelectionMsg = inMessage; }
						
	virtual void		EnableDragAndDrop();

	virtual void		DoSelectionChanged();
	virtual void		SetSingleSelection(void);
	virtual void		SetNoSelection(void);

	virtual void		AddItem(const cdstring& item);
	virtual void		SetContents(const cdstrvect& items);
	virtual void		GetSelection(ulvector& selection) const;
	virtual void		GetSelection(cdstrvect& selection) const;

	virtual void		SetItemText(const TableIndexT row, const cdstring& item);

protected:
	TableIndexT			mStringColumn;
	bool				mDoubleClickAll;
	SBroadcastDrag		mDragged;

	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);			// Click in the cell
	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell,
									const JRect& inLocalRect);					// Draw the string

			void		GetCellString(const STableCell& inCell, cdstring& str) const;

	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
	virtual bool	DropDataAtCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& before_cell);	// Drop data at cell

private:
	bool			mDoubleClickMsg;
	bool			mSelectionMsg;

		void		InitTextTable(void);
};

#endif
