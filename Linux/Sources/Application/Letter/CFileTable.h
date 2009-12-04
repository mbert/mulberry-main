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

#include "CMessageWindow.h"

// Classes

class CAttachment;
class CAttachmentList;
class CDataAttachment;
class CFileTableAddAction;
class CLetterWindow;
class CMbox;
class CMboxProtocol;
class CMessage;
class CMessageList;
class CSimpleTitleTable;

class JXKeyModifers;
class JXImage;

class CFileTable : public CHierarchyTableDrag

{
	friend class CLetterWindow;
	friend class CLetterTextEditView;

	enum IconID
	{
		eDiamond = 0,
		eDiamondTicked,
		eDiamondHighlight,
		eDiamondDisable,
		eSquare,
		eSquareHighlight,
		eTickMark,
		eMissingPart,
		eApplefile,
		eMultipart,
		eMessagePart,
		eUnknownfile,
		eReadWrite,
		eReadOnly
	};

public:
	CFileTable(JXScrollbarSet* scrollbarSet,
				JXContainer* enclosure,
				const HSizingOption hSizing, 
				const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h);

	virtual				~CFileTable();

	void		SetDirty(bool dirty)
	{
		mDirty = dirty;
	}
	bool		IsDirty() const
	{
		return mDirty;
	}

	void		SetAttachmentsOnly(bool attachments);
	bool		GetAttachmentsOnly() const
	{
		return mAttachmentsOnly;
	}

	void		SetTitles(CSimpleTitleTable* titles)
	{
		mTitles = titles;
	}

	void		SetBody(CAttachment* aBody);
	CAttachment*	GetBody()
	{
		return mBody;
	}

	void		SetRowShow(CAttachment* attach);
	CAttachment*		GetPartShow(void);

	unsigned long	CountParts() const;
	bool			HasAttachments() const;

	void	ResetTable();							// Reset the table from the mboxList

	virtual void	DeleteSelection();					// Delete selected cells and update visible
	virtual bool	DeleteRow(TableIndexT row);			// DeleteRow
	virtual void	RemoveRows(UInt32 inHowMany,
								TableIndexT inFromRow,
								bool inRefresh);		// Remove rows and adjust parts

	// Common updaters
	void	OnUpdateNotLocked(CCmdUI* pCmdUI);
	void	OnUpdateNotLockedSelection(CCmdUI* pCmdUI);
	void	OnUpdateAttachFile(CCmdUI* pCmdUI);
	void	OnUpdateAllowStyled(CCmdUI* pCmdUI);

	// Command handlers
	void	OnEditProperties();
	void	OnDraftAttachFile();
	void	OnDraftNewPlainPart();
	void	OnDraftNewEnrichedPart();
	void	OnDraftNewHTMLPart();
	void	DoNewTextPart(EContentSubType subType);
	void	OnDraftMultipartMixed();
	void	OnDraftMultipartParallel();
	void	OnDraftMultipartDigest();
	void	OnDraftMultipartAlternative();
	void	DoMultipart(EContentSubType subType);
	void	ActionDeleted();
	void	OnViewParts();
	bool	CheckViewPart(TableIndexT row);						// Check for valid extract of specified part
	bool	ViewPart(TableIndexT row);

	void	ExposePartsList();									// Make sure list is visible in draft window

	CAttachment*	GetAttachment(TableIndexT row, bool is_worow = false);				// Get attachment from row

	void	ForwardMessages(CMessageList* msgs, EForwardOptions forward);
	void	BounceMessages(CMessageList* msgs);
	void	RejectMessages(CMessageList* msgs, bool return_msg);
	void	SendAgainMessages(CMessageList* msgs);
	void	DigestMessages(CMessageList* msgs);

	void	AddFile(const cdstring& file);
	void	AddAttachment(CDataAttachment* attach);

	void	ServerReset(const CMboxProtocol* proto);		// Server reset
	void	MailboxReset(const CMbox* mbox);				// Mailbox reset
	void	MessageRemoved(const CMessage* msg);			// Message removed

	void	ChangedCurrent();				// Current part changed

	void OnCreate();

	 // Key/Mouse related
protected:
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);// Handle key down
	virtual void LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods);
	virtual	void LClickCell(const STableCell& inCell, const JXKeyModifiers& mods);							// Clicked somewhere

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	 // Drag & Drop
	virtual void		HandleDNDDidDrop(const Atom& action);
	virtual bool		IsDropCell(JArray<Atom>& typeList, const STableCell& cell);		// Test drop into cell
	virtual bool		IsDropAtCell(JArray<Atom>& typeList, STableCell& cell);	// Test drop at cell
	virtual bool		RenderSelectionData(CMulSelectionData* seldata, Atom type);
	virtual bool		DropDataIntoCell(Atom theFlavor,
											unsigned char* drag_data,
											unsigned long data_size,
											const STableCell& cell);		// Drop data into cell
	virtual bool		DropDataAtCell(Atom theFlavor,
											unsigned char* drag_data,
											unsigned long data_size,
											const STableCell& cell);		// Drop data at cell
	virtual bool		DropData(Atom theFlavor,
											unsigned char* drag_data,
											unsigned long data_size);		// Drop data at cell

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

	CAttachment*			mBody;
	CFileTableAddAction*	mAddAction;
	CLetterWindow*			mWindow;
	CSimpleTitleTable*		mTitles;
	TableIndexT				mRowShow;
	bool					mDirty;
	bool					mAttachmentsOnly;
	bool					mLocked;
	
	virtual bool TestSelectionChangeable(TableIndexT row);					// Test for changeable attachments
	virtual bool TestSelectionUnchangeable(TableIndexT row);				// Test for unchangeable attachments
	
	TableIndexT		AddPart(CAttachment* attach, CAttachment* parent, TableIndexT parent_row, int pos, bool refresh);
	TableIndexT		InsertPart(TableIndexT& atRow,
						 CAttachment* part,
						 bool child,
						 int pos = -1);
	bool	ConvertMultipart(bool add);							// Convert between single/multipart
	void	UpdateRowShow();									// Find suitable row show
	bool	ShowPart(TableIndexT row);
	
	void	GetDropAtParent(TableIndexT& at_row, TableIndexT& parent_row, CAttachment*& parent, int& pos);

	void ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual void DrawCell(JPainter* pDC, const STableCell&	inCell,
								const JRect& inLocalRect);				// Draw the items
	virtual JXImage* GetAttachIcon(CAttachment* attach);
	
};

#endif
