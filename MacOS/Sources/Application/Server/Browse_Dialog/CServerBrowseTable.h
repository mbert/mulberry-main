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

class LPushButton;

class CServerBrowseTable : public CServerBrowse,
							public LBroadcaster
{
protected:
	LPushButton*		mOKButton;					// OK button
	LPushButton*		mCreateButton;				// Create button

public:
	enum { class_ID = 'BrCr' };

					CServerBrowseTable();
					CServerBrowseTable(LStream *inStream);
	virtual 		~CServerBrowseTable();

	void	SetOpenMode(bool mode);						// Set to browse to open mailboxes
	void	SetMultipleSelection();						// Turn on multiple selection
	CMbox*	GetSelectedMbox();							// Get selected mbox
	void	GetSelectedMboxes(CMboxList& mbox_list);	// Get all selected mboxes

	virtual void	ResetTable();						// Reset the table from the mboxList
	virtual void	AdaptToNewSurroundings(void);		// Adjust column widths

protected:
	virtual void		FinishCreateSelf(void);			// Get details of sub-panes

	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);	// Respond to clicks in the icon buttons

	virtual void	ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown);// Click in the cell

	virtual void	DoSelectionChanged();
	virtual void	PostCreateAction(CMbox* mbox);					// Process mailbox after creation

	CMboxRef& GetLastBrowsed()							// Get last browsed mailbox
		{ return (mOpenMode ? sLastOpenBrowsed : sLastCreateBrowsed); }
private:
	bool	mOpenMode;

	static CMboxRef	sLastCreateBrowsed;						// Last mailbox browsed in create mode
	static CMboxRef	sLastOpenBrowsed;						// Last mailbox browsed in open mode

			void	InitServerBrowseTable(void);
};

#endif
