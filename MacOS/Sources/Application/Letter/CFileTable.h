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


//	CFileTable.h

#ifndef __CFILETABLE__MULBERRY__
#define __CFILETABLE__MULBERRY__

#include "CHierarchyTableDrag.h"

#include "CAttachment.h"
#include "CMessageWindow.h"

// Constants

// Classes

class CAttachment;
class CDataAttachment;
class CFileTableAddAction;
class CLetterWindow;
class CMessage;
class CMessageList;
class CSimpleTitleTable;

class	CFileTable : public CHierarchyTableDrag
{

	friend class CLetterTextDisplay;
	friend class CMessageWindow;

private:
	enum
	{
		eColType_Diamond,
		eColType_RW,
		eColType_MIME,
		eColType_MIMEIcon,
		eColType_Name,
		eColType_NameIcon,
		eColType_Size,
		eColType_Encoding
	};

	CAttachment*		 	mBody;
	CFileTableAddAction*	mAddAction;
	CLetterWindow*			mWindow;
	CSimpleTitleTable*		mTitles;
	TableIndexT				mRowShow;
	bool					mDirty;
	bool					mAttachmentsOnly;
	bool					mLocked;

public:
	enum { class_ID = 'FStb' };
	
						CFileTable(LStream *inStream);
	virtual				~CFileTable();

	void		SetDirty(bool dirty)
		{ mDirty = dirty; }
	bool		IsDirty(void)
		{ return mDirty; }

	void		SetAttachmentsOnly(bool attachments);
	bool		GetAttachmentsOnly() const
		{ return mAttachmentsOnly; }

	void		SetTitles(CSimpleTitleTable* titles)
	{
		mTitles = titles;
	}

	void		SetBody(CAttachment* aBody);
	CAttachment*	GetBody(void)
							{ return mBody; }

	void		SetRowShow(CAttachment* attach);
	CAttachment*		GetPartShow(void);

	unsigned long	CountParts() const;
	bool			HasAttachments() const;

protected:
	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

public:
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	void		DoAttachFile(void);
	void		DoNewTextPart(EContentSubType subType);
	void		DoMultipartMixed(void);
	void		DoMultipartParallel(void);
	void		DoMultipartDigest(void);
	void		DoMultipartAlternative(void);
	void		DoMultipart(EContentSubType subType);

	void		ForwardMessages(CMessageList* msgs, EForwardOptions forward);
	void		BounceMessages(CMessageList* msgs);
	void		RejectMessages(CMessageList* msgs, bool return_msg);
	void		SendAgainMessages(CMessageList* msgs);
	void		DigestMessages(CMessageList* msgs);

	void		AddFile(const cdstring& file);
	void		AddAttachment(CDataAttachment* attach);

	void		ServerReset(const CMboxProtocol* proto);		// Server reset
	void		MailboxReset(const CMbox* mbox);				// Mailbox reset
	void		MessageRemoved(const CMessage* msg);			// Message removed

	void		ChangedCurrent(void);							// Current part changed

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

	virtual void		BeTarget(void);
	virtual void		DontBeTarget(void);

	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);
	virtual void		ClickCell(const STableCell &inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell

public:
	virtual void	ResetTable(void);								// Reset the table from the body
	virtual void	AdaptToNewSurroundings(void);					// Adjust column widths

	virtual void	DeleteSelection(void);							// Delete selected cells and update visible
	virtual bool	DeleteRow(TableIndexT row);								// DeleteRow
	virtual void	RemoveRows(UInt32 inHowMany,
								TableIndexT inFromRow,				// WideOpen Index
								Boolean inRefresh);					// Remove rows and adjust parts

private:
	bool TestSelectionChangeable(TableIndexT row);					// Test for changeable attachments
	bool TestSelectionUnchangeable(TableIndexT row);				// Test for unchangeable attachments

	TableIndexT		AddPart(CAttachment* attach, CAttachment* parent, int parent_row, int pos, bool refresh);
	TableIndexT		InsertPart(TableIndexT& atRow,
								CAttachment* part,
								bool child,
								int pos = -1);
	bool	ConvertMultipart(bool add);							// Convert between single/multipart
	void	UpdateRowShow(void);									// Find suitable row show

	bool	ShowPart(TableIndexT row);

	void	GetDropAtParent(TableIndexT& at_row, TableIndexT& parent_row, CAttachment*& parent, TableIndexT& pos);

	void	DoEditProperties(void);								// Do properties dialog

	void	DoViewParts();										// View selected parts
	bool	CheckViewPart(TableIndexT row);						// Check for valid view of specified part
	bool	ViewPart(TableIndexT row);							// View specified part

	void	ExposePartsList();									// Make sure list is visible in draft window

	CAttachment* GetAttachment(TableIndexT row, bool is_worow = false);			// Get attachment from row

// Drag methods
private:
	virtual void	AddCellToDrag(CDragIt* theDragTask,
									const STableCell& theCell,
									Rect& dragRect);				// Add cell to drag with rect

protected:
	virtual bool		IsDropCell(DragReference inDragRef, STableCell aCell);		// Test drop into cell
	virtual bool		IsDropAtCell(DragReference inDragRef, STableCell& aCell);	// Test drop at cell
	virtual void		DropData(FlavorType theFlavor,
										char* drag_data,
										Size data_size);							// Drop data into whole table
	virtual void		DropDataIntoCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& theCell);					// Drop data into cell
	virtual void		DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell);				// Drop data at cell

};

#endif
