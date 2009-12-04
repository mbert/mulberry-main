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


// CServerBrowseTable.h

// Class to browse servers in a dialog box

#ifndef __CSERVERBROWSETABLE__MULBERRY__
#define __CSERVERBROWSETABLE__MULBERRY__

#include "CServerBrowse.h"

#include "CMboxRef.h"

class CBrowseMailboxDialog;

class CServerBrowseTable : public CServerBrowse
{
public:
					CServerBrowseTable();
	virtual 		~CServerBrowseTable();

	DECLARE_DYNCREATE(CServerBrowseTable)

	virtual void	SetButtons(CBrowseMailboxDialog* dialog, CButton* ok_btn, CButton* create_btn)					// Set buttons
		{ mDialog = dialog; mOKButton = ok_btn; mCreateButton = create_btn; }
	virtual void	SetOpenMode(bool mode);				// Set to browse to open mailboxes
	virtual void	SetMultipleSelection();				// Turn on multiple selection
	virtual CMbox*	GetSelectedMbox(void);				// Get selected mbox
	virtual void	GetSelectedMboxes(CMboxList& mbox_list);	// Get all selected mboxes

	virtual void	ResetTable(void);					// Reset the table from the mboxList

protected:
	CBrowseMailboxDialog* mDialog;
	CButton* mOKButton;
	CButton* mCreateButton;

	virtual void	LDblClickCell(const STableCell& inCell, UINT nFlags);						// Double-clicked item

	virtual void	SelectionChanged(void);
	virtual void	PostCreateAction(CMbox* mbox);					// Process mailbox after creation

	virtual CMboxRef& GetLastBrowsed(void)							// Get last browsed mailbox
		{ return (mOpenMode ? sLastOpenBrowsed : sLastCreateBrowsed); }

	afx_msg void	OnCreateMailbox(void);							// Create Mailbox
	afx_msg void	OnSize(UINT nType, int cx, int cy);

private:
	bool	mOpenMode;

	static CMboxRef	sLastCreateBrowsed;						// Last mailbox browsed in create mode
	static CMboxRef	sLastOpenBrowsed;						// Last mailbox browsed in open mode

			void	InitServerBrowseTable(void);

	DECLARE_MESSAGE_MAP()
};

#endif
