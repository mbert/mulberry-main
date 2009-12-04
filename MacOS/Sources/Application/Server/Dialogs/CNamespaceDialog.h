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


// Header for CNamespaceDialog class

#ifndef __CNAMESPACEDIALOG__MULBERRY__
#define __CNAMESPACEDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CTableDrag.h"

#include "CMboxProtocol.h"

// Constants

// Panes
const	PaneIDT		paneid_NamespaceDialog = 4004;
const	PaneIDT		paneid_NamespaceHelp = 'HELP';
const	PaneIDT		paneid_NamespaceText = 'STXT';
const	PaneIDT		paneid_NamespaceServer = 'SERV';
const	PaneIDT		paneid_NamespaceTable = 'TABL';
const	PaneIDT		paneid_NamespaceAuto = 'SADD';

// Resources
const	ResIDT		RidL_CNamespaceDialogBtns = 4004;

// Messages
const MessageT		msg_Namespace_Personal_Btn = 'SPER';
const MessageT		msg_Namespace_Shared_Btn = 'SSHA';
const MessageT		msg_Namespace_Public_Btn = 'SPUB';
const MessageT		msg_Namespace_All_Btn = 'SALL';
const MessageT		msg_Namespace_None_Btn = 'SNON';

// Classes
class CTextDisplay;
class LCheckBox;

class CNamespaceTable : public CTableDrag
{
public:
	enum { class_ID = 'NTAB' };

	enum ENamespaceSelect
	{
		eNamespace_Personal,
		eNamespace_Shared,
		eNamespace_Public,
		eNamespace_All,
		eNamespace_None
	};

					CNamespaceTable(LStream *inStream);
	virtual 		~CNamespaceTable();

			void	SetServerList(CMboxProtocol::SNamespace& servs, boolvector& servitems);		// Set namespaces
			void	ChangeSelection(ENamespaceSelect select);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);

	virtual void	ClickCell(const STableCell &inCell,
								const SMouseDownEvent &inMouseDown);	// Click in the cell

private:
	CMboxProtocol::SNamespace*	mData;
	boolvector*					mDataOn;
	cdstrvect					mTypeItems;
	TextTraitsH					mTextTraits;
};

class	CNamespaceDialog : public LDialogBox
{
public:
	enum { class_ID = 'NSPA' };

					CNamespaceDialog();
					CNamespaceDialog(LStream *inStream);
	virtual 		~CNamespaceDialog();

	static  bool	PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

			void	SetItems(const cdstrvect& items,						// Set namespaces
								CMboxProtocol::SNamespace& servs,
								boolvector& servitems, bool do_auto);
			void	GetItems(cdstrvect& items, bool& do_auto);				// Get selected items
	
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CTextDisplay*		mHelpText;
	CTextDisplay*		mUserItems;
	LView*				mServer;
	CNamespaceTable*	mItsTable;
	LCheckBox*			mDoAuto;
};

#endif
