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


// Header for CMatchOptionsDialog class

#ifndef __CMATCHOPTIONSDIALOG__MULBERRY__
#define __CMATCHOPTIONSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CMboxFwd.h"

// Constants

// Panes
const	PaneIDT		paneid_MatchOptionsDialog = 1004;
const	PaneIDT		paneid_MatchOptionsViewAllNoHilight = 'VALL';
const	PaneIDT		paneid_MatchOptionsViewAllHilight = 'VALH';
const	PaneIDT		paneid_MatchOptionsViewMatch = 'VMTC';
const	PaneIDT		paneid_MatchOptionsMatch = 'MTCH';
const	PaneIDT		paneid_MatchOptionsLastSearch = 'SRCH';
const	PaneIDT		paneid_MatchOptionsOr = 'OR  ';
const	PaneIDT		paneid_MatchOptionsAnd = 'AND ';
const	PaneIDT		paneid_MatchOptionsNor = 'NOR ';
const	PaneIDT		paneid_MatchOptionsNand = 'NAND';
const	PaneIDT		paneid_MatchOptionsNew = 'NEW ';
const	PaneIDT		paneid_MatchOptionsUnseen = 'UNSE';
const	PaneIDT		paneid_MatchOptionsImportant = 'IMPO';
const	PaneIDT		paneid_MatchOptionsAnswered = 'ANSW';
const	PaneIDT		paneid_MatchOptionsDeleted = 'DELE';
const	PaneIDT		paneid_MatchOptionsDraft = 'DRAF';
const	PaneIDT		paneid_MatchOptionsSentToday = 'STOD';
const	PaneIDT		paneid_MatchOptionsSentSinceYesterday = 'SYES';
const	PaneIDT		paneid_MatchOptionsSentThisWeek = 'SWEE';
const	PaneIDT		paneid_MatchOptionsSentWithSevenDays = 'SWIT';
const	PaneIDT		paneid_MatchOptionsSentByMe = 'SBME';
const	PaneIDT		paneid_MatchOptionsSentToMe = 'STME';
const	PaneIDT		paneid_MatchOptionsSentToList = 'STLI';
const	PaneIDT		paneid_MatchOptionsSelectedTo = 'SETO';
const	PaneIDT		paneid_MatchOptionsSelectedFrom = 'SEFR';
const	PaneIDT		paneid_MatchOptionsSelectedCorrespondents = 'SECO';
const	PaneIDT		paneid_MatchOptionsSelectedSubject = 'SESU';
const	PaneIDT		paneid_MatchOptionsSelectedDates = 'SEDA';
const	PaneIDT		paneid_MatchOptionsSets = 'SETS';

// Mesages
const	MessageT	msg_MatchOptionsClear = 'CLEA';

// Resources
const	ResIDT		RidL_CMatchOptionsDialogBtns = 1004;

class CCheckboxTable;
class CMatchItem;
class LCheckBox;
class LRadioButton;

class CMatchOptionsDialog : public LDialogBox
{
public:
	enum { class_ID = 'Mtch' };

					CMatchOptionsDialog();
					CMatchOptionsDialog(LStream *inStream);
	virtual 		~CMatchOptionsDialog();

	static bool		PoseDialog(NMbox::EViewMode& mbox_mode, CMatchItem& match, bool& last_search);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	LRadioButton*	mViewAllNoHilight;
	LRadioButton*	mViewAllHilight;
	LRadioButton*	mViewMatch;
	LRadioButton*	mMatch;
	LRadioButton*	mLastSearch;
	LRadioButton*	mOr;
	LRadioButton*	mAnd;
	LRadioButton*	mNor;
	LRadioButton*	mNand;
	LCheckBox*		mNew;
	LCheckBox*		mUnseen;
	LCheckBox*		mImportant;
	LCheckBox*		mAnswered;
	LCheckBox*		mDeleted;
	LCheckBox*		mDraft;
	LCheckBox*		mSentToday;
	LCheckBox*		mSentSinceYesterday;
	LCheckBox*		mSentThisWeek;
	LCheckBox*		mSentWithin7Days;
	LCheckBox*		mSentByMe;
	LCheckBox*		mSentToMe;
	LCheckBox*		mSentToList;
	LCheckBox*		mSelectedTo;
	LCheckBox*		mSelectedFrom;
	LCheckBox*		mSelectedCorrespondents;
	LCheckBox*		mSelectedSubjects;
	LCheckBox*		mSelectedDates;
	CCheckboxTable*	mTable;

	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search);
	CMatchItem GetMatchItems(NMbox::EViewMode& mbox_mode, bool& last_search) const;

	void ClearItems();
};

#endif
