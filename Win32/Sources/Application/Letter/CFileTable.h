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
#include "CTabber.h"

#include "CMessageWindow.h"

// Classes

class CAttachment;
class CAttachmentList;
class CDataAttachment;
class CFileTableAddAction;
class CLetterWindow;
class CMboxProtocol;
class CMessage;
class CMessageList;
class CSimpleTitleTable;

class CFileTable : public CHierarchyTableDrag, public CTabber
{
	friend class CLetterWindow;
	friend class CLetterTextEditView;

	DECLARE_DYNCREATE(CFileTable)

public:
						CFileTable();
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
	CAttachment*		GetPartShow();

	unsigned long	CountParts() const;
	bool			HasAttachments() const;

	void	ResetTable();							// Reset the table from the mboxList

	virtual void	DeleteSelection();						// Delete selected cells and update visible
	virtual bool	DeleteRow(TableIndexT row);				// DeleteRow
	virtual void	RemoveRows(UInt32 inHowMany,
								TableIndexT inFromRow,
								bool inRefresh);		// Remove rows and adjust parts

	// Common updaters
	afx_msg void	OnUpdateNotLocked(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateNotLockedSelection(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateAttachFile(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateAllowStyled(CCmdUI* pCmdUI);

	// Command handlers
	afx_msg void	OnEditClear();
	afx_msg void	OnEditProperties();
	afx_msg void	OnDraftAttachFile();
	afx_msg void	OnDraftNewPlainPart();
	afx_msg void	OnDraftNewEnrichedPart();
	afx_msg void	OnDraftNewHTMLPart();
			void	DoNewTextPart(EContentSubType subType);
	afx_msg void	OnDraftMultipartMixed();
	afx_msg void	OnDraftMultipartParallel();
	afx_msg void	OnDraftMultipartDigest();
	afx_msg void	OnDraftMultipartAlternative();
			void	DoMultipart(EContentSubType subType);
	afx_msg void	OnCmdActionDeleted();
	afx_msg void	OnViewParts();
			bool	CheckViewPart(TableIndexT row);				// Check for valid extract of specified part
			bool	ViewPart(TableIndexT row);

			void	ExposePartsList();							// Make sure list is visible in draft window

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
			void	MessageRemoved(const CMessage* msg);	// Message removed

			void	ChangedCurrent();					// Current part changed

	// Key/Mouse related
protected:
	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	// Handle key down
	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);
	virtual	void	LClickCell(const STableCell& inCell, UINT nFlags);							// Clicked somewhere

	// Drag & Drop
	virtual bool		IsDropCell(COleDataObject* pDataObject, const STableCell& cell);		// Test drop into cell
	virtual bool		IsDropAtCell(COleDataObject* pDataObject, STableCell& cell);	// Test drop at cell
	virtual BOOL		OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual bool		DropDataIntoCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);							// Drop data into cell
	virtual bool		DropDataAtCell(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size,
										const STableCell& cell);					// Drop data at cell
	virtual bool		DropData(unsigned int theFlavor,
										char* drag_data,
										unsigned long data_size);				// Drop data
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
	int						mRowShow;
	bool					mDirty;
	bool					mAttachmentsOnly;
	bool					mLocked;

	virtual bool TestSelectionChangeable(TableIndexT row);						// Test for changeable attachments
	virtual bool TestSelectionUnchangeable(TableIndexT row);					// Test for unchangeable attachments

			TableIndexT		AddPart(CAttachment* attach, CAttachment* parent, TableIndexT parent_row, int pos, bool refresh);
			TableIndexT		InsertPart(TableIndexT& atRow,
								CAttachment* part,
								bool child,
								int pos = -1);
			bool	ConvertMultipart(bool add);							// Convert between single/multipart
			void	UpdateRowShow();									// Find suitable row show
			bool	ShowPart(TableIndexT row);

			void	GetDropAtParent(TableIndexT& at_row, TableIndexT& parent_row, CAttachment*& parent, int& pos);

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDropFiles(HDROP hDropInfo);

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items
	virtual HICON	GetAttachIcon(CDC* pDC, CAttachment* attach);

	DECLARE_MESSAGE_MAP()
};

#endif
