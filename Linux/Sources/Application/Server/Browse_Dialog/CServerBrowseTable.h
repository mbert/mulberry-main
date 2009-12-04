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
class JXTextButton;

class CServerBrowseTable : public CServerBrowse
{
	friend class CBrowseMailboxDialog;

public:

	CServerBrowseTable(JXScrollbarSet* scrollbarSet,
						JXContainer* enclosure,
						const HSizingOption hSizing, 
						const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual 		~CServerBrowseTable();

	virtual void	SetButtons(CBrowseMailboxDialog* dialog, JXTextButton* ok_btn, JXTextButton* create_btn)					// Set buttons
		{ mDialog = dialog; mOKButton = ok_btn; mCreateButton = create_btn; }
	void	SetOpenMode(bool mode);						// Set to browse to open mailboxes
	void	SetMultipleSelection();						// Turn on multiple selection
	CMbox*	GetSelectedMbox();							// Get selected mbox
	void	GetSelectedMboxes(CMboxList& mbox_list);	// Get all selected mboxes

	virtual void	ResetTable();					// Reset the table from the mboxList

protected:
	virtual void 	OnCreate();

	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual void	LDblClickCell(const STableCell& cell, const JXKeyModifiers& modifiers);			// Click in the cell

	virtual void	DoSelectionChanged();
	virtual void	PostCreateAction(CMbox* mbox);					// Process mailbox after creation

	CMboxRef& GetLastBrowsed()							// Get last browsed mailbox
		{ return (mOpenMode ? sLastOpenBrowsed : sLastCreateBrowsed); }
private:
	CBrowseMailboxDialog* mDialog;
	JXTextButton* mOKButton;
	JXTextButton* mCreateButton;
	bool	mOpenMode;

	static CMboxRef	sLastCreateBrowsed;						// Last mailbox browsed in create mode
	static CMboxRef	sLastOpenBrowsed;						// Last mailbox browsed in open mode

			void	InitServerBrowseTable(void);
};

#endif
