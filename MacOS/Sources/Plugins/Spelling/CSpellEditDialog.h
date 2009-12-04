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

// Header for CSpellEditDialog class

#ifndef __CSPELLEDITDIALOG__MULBERRY__
#define __CSPELLEDITDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_SpellEditDialog = 20400;
const	PaneIDT		paneid_SpellEditDictName = 'DICT';
const	PaneIDT		paneid_SpellEditScroller = 'HSCR';
const	PaneIDT		paneid_SpellEditWordTop = 'WTOP';
const	PaneIDT		paneid_SpellEditWordBottom = 'WBTM';
const	PaneIDT		paneid_SpellEditList = 'LIST';
const	PaneIDT		paneid_SpellEditWord = 'WORD';
const	PaneIDT		paneid_SpellEditAdd = 'ADDW';
const	PaneIDT		paneid_SpellEditFind = 'FIND';
const	PaneIDT		paneid_SpellEditRemove = 'RMVE';
const	PaneIDT		paneid_SpellEditCancel = 'CANC';

// Mesages
const	MessageT	msg_SpellEditList = 'LIST';
const	MessageT	msg_SpellEditHScroll = 'SCRL';
const	MessageT	msg_SpellEditAdd = 'ADDW';
const	MessageT	msg_SpellEditFind = 'FIND';
const	MessageT	msg_SpellEditRemove = 'RMVE';

// Resources
const	ResIDT		RidL_CSpellEditDialogBtns = 20400;

// Classes
class	CDictionaryPageScroller;
class	CStaticText;
class	CTextFieldX;
class	LPushButton;
class	CSpellPlugin;
class	CTextTable;

class CSpellEditDialog : public LDialogBox
{
	friend class CDictionaryPageScroller;

public:
	enum { class_ID = 'EdDi' };

					CSpellEditDialog();
					CSpellEditDialog(LStream *inStream);
	virtual 		~CSpellEditDialog();

	static bool PoseDialog(CSpellPlugin* speller);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual Boolean	HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	CStaticText*		mDictName;
	CDictionaryPageScroller*	mScroller;
	CStaticText*		mWordTop;
	CStaticText*		mWordBottom;
	CTextTable*			mList;
	CTextFieldX*		mWord;
	LPushButton*		mAdd;
	LPushButton*		mFind;
	LPushButton*		mRemove;
	LPushButton*		mCancel;

	CSpellPlugin*		mSpeller;

			void	SetSpeller(CSpellPlugin* speller);			// Set the speller

			void	RotateDefault(void);							// Rotate default button

			void	DoAdd(void);								// Add word to dictionary
			void	DoFind(void);								// Remove word from dicitonary
			void	DoRemove(void);								// Remove word from dicitonary
};

#endif
