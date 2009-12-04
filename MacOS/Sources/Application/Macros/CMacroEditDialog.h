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


// Header for CMacroEditDialog class

#ifndef __CMACROEDITDIALOG__MULBERRY__
#define __CMACROEDITDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CTextTable.h"
#include "CTextMacros.h"

// Constants

// Panes
const	PaneIDT		paneid_MacroEditDialog = 8010;
const	PaneIDT		paneid_MacroEditTable = 'TABL';
const	PaneIDT		paneid_MacroEditQuoteNew = 'NEWM';
const	PaneIDT		paneid_MacroEditQuoteEdit = 'EDIT';
const	PaneIDT		paneid_MacroEditAddToDelete = 'DELE';
const	PaneIDT		paneid_MacroEditActionKey = 'TKEY';
const	PaneIDT		paneid_MacroEditActionKeyChange = 'CHOS';

// Resources
const	ResIDT		RidL_CMacroEditDialogBtns = 8010;

// Messages
const	MessageT	msg_MacroEditQuoteNew = 'NEWM';
const	MessageT	msg_MacroEditQuoteEdit = 'EDIT';
const	MessageT	msg_MacroEditAddToDelete = 'DELE';
const	MessageT	msg_MacroEditDoubleClick = 'TABD';
const	MessageT	msg_MacroEditSelect = 'TABS';
const	MessageT	msg_MacroEditKeyChange = 'CHOS';

// Classes
class CStaticText;
class LPushButton;

class CMacroEditTable : public CTextTable
{
public:
	enum { class_ID = 'ETAB' };

					CMacroEditTable(LStream *inStream);
	virtual 		~CMacroEditTable();

			void	SetData(CTextMacros* macros);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	DrawCell(const STableCell &inCell,
								const Rect &inLocalRect);

private:
	CTextMacros*			mCopy;

};

class	CMacroEditDialog : public LDialogBox
{
public:
	enum { class_ID = 'MACR' };

					CMacroEditDialog();
					CMacroEditDialog(LStream *inStream);
	virtual 		~CMacroEditDialog();

	static  void	PoseDialog();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

			void	SetDetails();
			void	GetDetails();

private:
	CMacroEditTable*	mItsTable;
	LPushButton*		mEditBtn;
	LPushButton*		mDeleteBtn;
	CTextMacros			mCopy;
	CStaticText*		mKey;

	unsigned char		mActualKey;
	CKeyModifiers		mActualMods;

	void	NewMacro();
	void	EditMacro();
	void	DeleteMacro();
	void	GetKey();
};

#endif
