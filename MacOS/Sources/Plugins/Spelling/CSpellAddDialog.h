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

// Header for CSpellAddDialog class

#ifndef __CSPELLADDDIALOG__MULBERRY__
#define __CSPELLADDDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_SpellAddDialog = 20200;
const	PaneIDT		paneid_SpellAddCheck1 = 'CHK1';
const	PaneIDT		paneid_SpellAddCheck2 = 'CHK2';
const	PaneIDT		paneid_SpellAddCheck3 = 'CHK3';
const	PaneIDT		paneid_SpellAddCheck4 = 'CHK4';
const	PaneIDT		paneid_SpellAddCheck5 = 'CHK5';
const	PaneIDT		paneid_SpellAddCheck6 = 'CHK6';
const	PaneIDT		paneid_SpellAddCheck7 = 'CHK7';
const	PaneIDT		paneid_SpellAddCheck8 = 'CHK8';
const	PaneIDT		paneid_SpellAddCheck9 = 'CHK9';
const	PaneIDT		paneid_SpellAddCheck10 = 'CHKA';
const	PaneIDT		paneid_SpellAddCheck11 = 'CHKB';
const	PaneIDT		paneid_SpellAddCheck12 = 'CHKC';
const	PaneIDT		paneid_SpellAddCheck13 = 'CHKD';
const	PaneIDT		paneid_SpellAddCheck14 = 'CHKE';
const	PaneIDT		paneid_SpellAddCheck15 = 'CHKF';
const	PaneIDT		paneid_SpellAddCheck16 = 'CHKG';
const	PaneIDT		paneid_SpellAddCheck17 = 'CHKH';
const	PaneIDT		paneid_SpellAddCheck18 = 'CHKI';
const	PaneIDT		paneid_SpellAddCheck19 = 'CHKJ';
const	PaneIDT		paneid_SpellAddSuggestion1 = 'SUG1';
const	PaneIDT		paneid_SpellAddSuggestion2 = 'SUG2';
const	PaneIDT		paneid_SpellAddSuggestion3 = 'SUG3';
const	PaneIDT		paneid_SpellAddSuggestion4 = 'SUG4';
const	PaneIDT		paneid_SpellAddSuggestion5 = 'SUG5';
const	PaneIDT		paneid_SpellAddSuggestion6 = 'SUG6';
const	PaneIDT		paneid_SpellAddSuggestion7 = 'SUG7';
const	PaneIDT		paneid_SpellAddSuggestion8 = 'SUG8';
const	PaneIDT		paneid_SpellAddSuggestion9 = 'SUG9';
const	PaneIDT		paneid_SpellAddSuggestion10 = 'SUGA';
const	PaneIDT		paneid_SpellAddSuggestion11 = 'SUGB';
const	PaneIDT		paneid_SpellAddSuggestion12 = 'SUGC';
const	PaneIDT		paneid_SpellAddSuggestion13 = 'SUGD';
const	PaneIDT		paneid_SpellAddSuggestion14 = 'SUGE';
const	PaneIDT		paneid_SpellAddSuggestion15 = 'SUGF';
const	PaneIDT		paneid_SpellAddSuggestion16 = 'SUGG';
const	PaneIDT		paneid_SpellAddSuggestion17 = 'SUGH';
const	PaneIDT		paneid_SpellAddSuggestion18 = 'SUGI';
const	PaneIDT		paneid_SpellAddSuggestion19 = 'SUGJ';
const	PaneIDT		paneid_SpellAddCaps = 'CAPS';

// Mesages
const	MessageT	msg_SpellAddCheck1 = 'CHK1';
const	MessageT	msg_SpellAddCheck2 = 'CHK2';
const	MessageT	msg_SpellAddCheck3 = 'CHK3';
const	MessageT	msg_SpellAddCheck4 = 'CHK4';
const	MessageT	msg_SpellAddCheck5 = 'CHK5';
const	MessageT	msg_SpellAddCheck6 = 'CHK6';
const	MessageT	msg_SpellAddCheck7 = 'CHK7';
const	MessageT	msg_SpellAddCheck8 = 'CHK8';
const	MessageT	msg_SpellAddCheck9 = 'CHK9';
const	MessageT	msg_SpellAddCheck10 = 'CHKA';
const	MessageT	msg_SpellAddCheck11 = 'CHKB';
const	MessageT	msg_SpellAddCheck12 = 'CHKC';
const	MessageT	msg_SpellAddCheck13 = 'CHKD';
const	MessageT	msg_SpellAddCheck14 = 'CHKE';
const	MessageT	msg_SpellAddCheck15 = 'CHKF';
const	MessageT	msg_SpellAddCheck16 = 'CHKG';
const	MessageT	msg_SpellAddCheck17 = 'CHKH';
const	MessageT	msg_SpellAddCheck18 = 'CHKI';
const	MessageT	msg_SpellAddCheck19 = 'CHKJ';
const	MessageT	msg_SpellAddCaps = 'CAPS';

const PaneIDT paneid_SpellAddCheck[] =
	{	paneid_SpellAddCheck1,
		paneid_SpellAddCheck2,
		paneid_SpellAddCheck3,
		paneid_SpellAddCheck4,
		paneid_SpellAddCheck5,
		paneid_SpellAddCheck6,
		paneid_SpellAddCheck7,
		paneid_SpellAddCheck8,
		paneid_SpellAddCheck9,
		paneid_SpellAddCheck10,
		paneid_SpellAddCheck11,
		paneid_SpellAddCheck12,
		paneid_SpellAddCheck13,
		paneid_SpellAddCheck14,
		paneid_SpellAddCheck15,
		paneid_SpellAddCheck16,
		paneid_SpellAddCheck17,
		paneid_SpellAddCheck18,
		paneid_SpellAddCheck19};

const PaneIDT paneid_SpellAddSuggestion[] =
	{	paneid_SpellAddSuggestion1,
		paneid_SpellAddSuggestion2,
		paneid_SpellAddSuggestion3,
		paneid_SpellAddSuggestion4,
		paneid_SpellAddSuggestion5,
		paneid_SpellAddSuggestion6,
		paneid_SpellAddSuggestion7,
		paneid_SpellAddSuggestion8,
		paneid_SpellAddSuggestion9,
		paneid_SpellAddSuggestion10,
		paneid_SpellAddSuggestion11,
		paneid_SpellAddSuggestion12,
		paneid_SpellAddSuggestion13,
		paneid_SpellAddSuggestion14,
		paneid_SpellAddSuggestion15,
		paneid_SpellAddSuggestion16,
		paneid_SpellAddSuggestion17,
		paneid_SpellAddSuggestion18,
		paneid_SpellAddSuggestion19};

const MessageT msg_SpellAddCheck[] =
	{	msg_SpellAddCheck1,
		msg_SpellAddCheck2,
		msg_SpellAddCheck3,
		msg_SpellAddCheck4,
		msg_SpellAddCheck5,
		msg_SpellAddCheck6,
		msg_SpellAddCheck7,
		msg_SpellAddCheck8,
		msg_SpellAddCheck9,
		msg_SpellAddCheck10,
		msg_SpellAddCheck11,
		msg_SpellAddCheck12,
		msg_SpellAddCheck13,
		msg_SpellAddCheck14,
		msg_SpellAddCheck15,
		msg_SpellAddCheck16,
		msg_SpellAddCheck17,
		msg_SpellAddCheck18,
		msg_SpellAddCheck19};

// Resources
const	ResIDT		RidL_CSpellAddDialogBtns = 20200;

// Classes
class	LCheckBox;
class	CTextFieldX;
class	CSpellPlugin;

class	CSpellAddDialog : public LDialogBox
{
public:
	enum { class_ID = 'AddW' };

					CSpellAddDialog();
					CSpellAddDialog(LStream *inStream);
	virtual 		~CSpellAddDialog();

	static bool PoseDialog(CSpellPlugin* speller, const char* add_word);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	LCheckBox*		mCheck[19];
	CTextFieldX*	mSuggestion[19];
	LCheckBox*		mCapitalise;

	CSpellPlugin*	mSpeller;

			void	SetDetails(CSpellPlugin* speller, const char* word);			// Set the speller
			void	GetDetails(CSpellPlugin* speller);
			void	Capitalise(bool capitals);
};

#endif
