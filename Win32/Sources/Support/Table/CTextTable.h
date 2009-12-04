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

class CTextTable : public CTableDragAndDrop
{

public:
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

					CTextTable();
	virtual 		~CTextTable();

public:
	void SetMsgTarget(CWnd* target)
		{ mMsgTarget = target; }

	void SetDoubleClickMsg(UINT inMessage)
		{ mDoubleClickMsg = inMessage; }
	
	UINT GetDoubleClickMsg()
		{ return mDoubleClickMsg; }
	
	void SetSelectionMsg(UINT inMessage)
		{ mSelectionMsg = inMessage; }
						
	UINT GetSelectionMsg()
		{ return mSelectionMsg; }
	
	virtual void		EnableDragAndDrop();

	virtual void		SelectionChanged();
	virtual void		SetSingleSelection(void);

	virtual void		AddItem(const cdstring& item);
	virtual void		SetContents(const cdstrvect& items);
	virtual void		GetSelection(ulvector& selection) const;
	virtual void		GetSelection(cdstrvect& selection) const;

protected:
	int					mStringColumn;
	bool				mDoubleClickAll;

	virtual	bool		HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void		LDblClickCell(
								const STableCell&		inCell,
								UINT nFlags);			// Click in the cell
	virtual void		DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);					// Draw the string

			void		GetCellString(const STableCell& inCell, cdstring& str) const;

	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()

private:
	CWnd*			mMsgTarget;
	UINT			mDoubleClickMsg;
	UINT			mSelectionMsg;

		void		InitTextTable(void);

// Drag methods
protected:
	SBroadcastDrag	mDragged;

	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual bool	DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& before_cell);					// Drop data at cell
};

#endif
