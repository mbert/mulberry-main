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


//	CBodyTable.h

#ifndef __CBODYTABLE__MULBERRY__
#define __CBODYTABLE__MULBERRY__

#include "CHierarchyTableDrag.h"
#include "CTabber.h"

// Classes

class CAttachment;
class CAttachmentList;
class CSimpleTitleTable;

class CBodyTable : public CHierarchyTableDrag, public CTabber
{
	friend class CMessageView;
	friend class CMessageWindow;

	DECLARE_DYNCREATE(CBodyTable)

public:
						CBodyTable();
	virtual				~CBodyTable();

	void		SetTitles(CSimpleTitleTable* titles)
	{
		mTitles = titles;
	}

	void		SetBody(CAttachment* aBody);
	CAttachment*	GetBody()
		{ return mBody; }
	void		ClearBody();

	void		SetFlat(bool flat);
	bool		GetFlat() const
		{ return mFlat; }

	void		SetRowShow(CAttachment* attach);
	CAttachment*		GetPartShow();

	void	ResetTable();						// Reset the table from the mboxList

	// Common updaters
	afx_msg void	OnUpdateExtractParts(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnEditProperties();
	afx_msg void	OnMessageViewParts();
	afx_msg void	OnMessageExtractParts();
			void	ExtractViewParts(bool view);
			void	DoExtractCurrentPart(bool view);							// Extract current part
			bool	CheckExtractPart(TableIndexT row, bool* worow);				// Check for valid extract of specified part
			bool	ExtractPart(TableIndexT row, bool* view, bool* worow);

	// Key/Mouse related
protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);
	virtual	void	LClickCell(const STableCell& inCell, UINT nFlags);			// Clicked somewhere

	CAttachment* GetAttachment(TableIndexT row, bool worow = false);			// Get attachment from row

	bool TestSelectionCached(TableIndexT row);				// Test for cached item

	// Drag & Drop
	virtual BOOL	DoDrag(TableIndexT row);
	virtual BOOL	OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);

private:
	CAttachment*		 	mBody;
	CMessageWindow*			mWindow;
	CMessageView*			mView;
	CSimpleTitleTable*		mTitles;
	SInt32					mRowShow;
	bool					mFlat;

			void	InsertPart(TableIndexT& atRow, CAttachment* part, bool child);
			bool	ShowPart(TableIndexT row);

	virtual bool		AddEntryToList(TableIndexT row, CAttachmentList* list);		// Add cell to list

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items
	virtual HICON	GetAttachIcon(CDC* pDC, CAttachment* attach);

	DECLARE_MESSAGE_MAP()
};

#endif
