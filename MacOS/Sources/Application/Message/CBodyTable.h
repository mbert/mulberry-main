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

#include "CAttachment.h"


// Constants

const FlavorType cDragPromiseFSSpec = 'fssP';

// Classes

class CAttachment;
class CMessageView;
class CMessageWindow;
class CStaticText;

class	CBodyTable : public CHierarchyTableDrag
{

	friend class CMessageView;
	friend class CMessageWindow;

private:
	CAttachment*		 	mBody;
	CMessageWindow*			mWindow;
	CMessageView*			mView;
	CStaticText*			mContentTitle;
	SInt32					mRowShow;
	bool					mFlat;
	bool					mDragInProgress;

public:
	enum { class_ID = 'BDtb' };
	
						CBodyTable(LStream *inStream);
	virtual				~CBodyTable();

	void		SetBody(CAttachment* aBody);
	CAttachment*	GetBody()
		{ return mBody; }
	void		ClearBody();					// Remove reference to attachment

	void		SetFlat(bool flat);
	bool		GetFlat() const
		{ return mFlat; }

	void		SetRowShow(CAttachment* attach);
	CAttachment*		GetPartShow();

protected:
	virtual void		FinishCreateSelf();				// Get details of sub-panes

public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void		PlotAttachIcon(CAttachment* attach, Rect& iconRect, bool inHilite);
	virtual void		HiliteCellActively(const STableCell &inCell,
											Boolean inHilite);
	virtual void		HiliteCellInactively(const STableCell &inCell,
											Boolean inHilite);

	virtual void		CalcCellFlagRect(
								const STableCell		&inCell,
								Rect					&outRect);						

	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);
	virtual void		ClickCell(const STableCell &inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell

public:
	virtual void	ResetTable();								// Reset the table from the body
	virtual void	AdaptToNewSurroundings();					// Adjust column widths

private:
	void	InsertPart(TableIndexT& atRow,
								CAttachment* part,
								bool child);

	void	DoProperties();								// Do properties dialog

	void	DoExtractParts(bool view);											// Extract selected parts
	void	DoExtractCurrentPart(bool view);									// Extract current part
	bool	CheckExtractPart(TableIndexT row, bool* worow);						// Check for valid extract of specified part
	bool	ExtractPart(TableIndexT row, bool* view, bool* worow);				// Extract/view specified part

	bool	ShowPart(TableIndexT row);

	CAttachment* GetAttachment(TableIndexT row, bool worow = false);			// Get attachment from row

	bool TestSelectionCached(TableIndexT row);				// Test for cached item

// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	virtual void	DoDragSendData(FlavorType		inFlavor,
									ItemReference	inItemRef,
									DragReference	inDragRef);		// Other flavor requested by receiver

};

#endif
