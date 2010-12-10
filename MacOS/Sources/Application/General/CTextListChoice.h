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


// Header for CTextListChoice class

#ifndef __CTEXTLISTCHOICE__MULBERRY__
#define __CTEXTLISTCHOICE__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"
#include "templs.h"

// Panes
const	PaneIDT		paneid_TextListDialog = 10003;
const	PaneIDT		paneid_TextListDescription = 'DESC';
const	PaneIDT		paneid_TextListList = 'TABL';
const	PaneIDT		paneid_TextListItems = 'ENTR';
const	PaneIDT		paneid_TextListItemTitle = 'TITL';
const	PaneIDT		paneid_TextListItem = 'ITEM';
const	PaneIDT		paneid_TextListButton = 'OKBT';
const	PaneIDT		paneid_TextListCancel = 'CANC';

// Mesages
const	MessageT	msg_TextListSingleClick = 'TAB1';
const	MessageT	msg_TextListDblClick = 'TAB2';

// Resoucres
const	ResIDT		STRx_TextList = 10005;
enum
{
	str_TextList_CreateAddressBook_Title = 1,
	str_TextList_CreateAddressBook_Desc,
	str_TextList_CreateAddressBook_Item
};

// Classes
class CTextTable;
class CTextFieldX;

class	CTextListChoice : public LDialogBox
{
public:
	enum { class_ID = 'TxLI' };

					CTextListChoice();
					CTextListChoice(LStream *inStream);
	virtual 		~CTextListChoice();

	virtual cdstring	GetItem(void);						// Get details from dialog
	virtual ulvector	GetSelection(void);					// Get details from dialog

	virtual	void	SetUpDetails(const char* title, const char* description, const char* item,
								bool use_entry, bool single_selection, bool no_selection, bool select_first,
								cdstrvect& items, cdstring& text, const char* btn = NULL);

	static bool	PoseDialog(const char* title, const char* description, const char* item,
								bool use_entry, bool single_selection, bool no_selection, bool select_first,
								cdstrvect& items, cdstring& text, ulvector& selection, const char* btn = NULL);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CTextTable*		mList;
	CTextFieldX*	mName;
};

#endif
