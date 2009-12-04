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

class JXImage;

// Classes

class CAttachment;
class CAttachmentList;
class CMessageView;
class CMessageWindow;
class CSimpleTitleTable;

class CBodyTable : public CHierarchyTableDrag
{
	friend class CMessageWindow;
	friend class CMessageView;

public:
	CBodyTable(JXScrollbarSet* scrollbarSet, 
				JXContainer* enclosure,
				const HSizingOption hSizing, 
				const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);
	virtual ~CBodyTable();

	void		SetTitles(CSimpleTitleTable* titles)
	{
		mTitles = titles;
	}

	void SetBody(CAttachment* aBody);
	CAttachment* GetBody()
		{ return mBody; }
	void		ClearBody();					// Remove reference to attachment

	void SetFlat(bool flat);
	bool GetFlat() const
		{ return mFlat; }

	void SetRowShow(CAttachment *attach);
	CAttachment* GetPartShow();

	void	ResetTable();						// Reset the table from the mboxList

	// Common updaters
	void 	OnUpdateExtractParts(CCmdUI* cmdui);

	// Command handlers
	void	OnEditProperties();
	void	OnMessageViewParts();
	void	OnMessageExtractParts();
	void	ExtractViewParts(bool view);
	void	DoExtractCurrentPart(bool view);							// Extract current part
	bool	CheckExtractPart(TableIndexT row, bool* worow);				// Check for valid extract of specified part
	bool	ExtractPart(TableIndexT row, bool* view, bool* worow);

	// Key/Mouse related
protected:
	virtual bool	HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);
	virtual void	LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);				// Clicked somewhere

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual void ApertureResized(const JCoordinate dw, const JCoordinate dh);

	CAttachment* GetAttachment(TableIndexT row, bool worow = false);			// Get attachment from row

	bool TestSelectionCached(TableIndexT row);				// Test for cached item


private:
	CMessageWindow* 		mWindow;
	CMessageView*	 		mView;
	CAttachment*			mBody;
	CSimpleTitleTable*		mTitles;
	long					mRowShow;
	bool					mFlat;

			void	InsertPart(TableIndexT& atRow, CAttachment* part, bool child);
	virtual bool	AddEntryToList(TableIndexT row, CAttachmentList* list);		// Add cell to list
			bool 	ShowPart(TableIndexT row);

public:
	virtual void OnCreate();

	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,
							 const JRect& inLocalRect);				// Draw the items
	virtual JXImage*	GetAttachIcon(CAttachment* attach);

protected:
// Drag methods
	virtual bool	RenderSelectionData(CMulSelectionData* selection, Atom type);
};

#endif
