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


// Header for CPrefsDisplayMessage class

#ifndef __CPREFSDISPLAYMESSAGE__MULBERRY__
#define __CPREFSDISPLAYMESSAGE__MULBERRY__

#include "CPrefsDisplayPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayMessage = 5042;
const	PaneIDT		paneid_DMUnseenColour = 'UCOL';
const	PaneIDT		paneid_DMSeenColour = 'SCOL';
const	PaneIDT		paneid_DMAnsweredColour = 'ACOL';
const	PaneIDT		paneid_DMImportantColour = 'ICOL';
const	PaneIDT		paneid_DMDeletedColour = 'DCOL';
const	PaneIDT		paneid_DMUnseenBold = 'UBLD';
const	PaneIDT		paneid_DMSeenBold = 'SBLD';
const	PaneIDT		paneid_DMAnsweredBold = 'ABLD';
const	PaneIDT		paneid_DMImportantBold = 'IBLD';
const	PaneIDT		paneid_DMDeletedBold = 'DBLD';
const	PaneIDT		paneid_DMUnseenItalic = 'UITL';
const	PaneIDT		paneid_DMSeenItalic = 'SITL';
const	PaneIDT		paneid_DMAnsweredItalic = 'AITL';
const	PaneIDT		paneid_DMImportantItalic = 'IITL';
const	PaneIDT		paneid_DMDeletedItalic = 'DITL';
const	PaneIDT		paneid_DMUnseenStrike = 'USTK';
const	PaneIDT		paneid_DMSeenStrike = 'SSTK';
const	PaneIDT		paneid_DMAnsweredStrike = 'ASTK';
const	PaneIDT		paneid_DMImportantStrike = 'ISTK';
const	PaneIDT		paneid_DMDeletedStrike = 'DSTK';
const	PaneIDT		paneid_DMUnseenUnderline = 'UUND';
const	PaneIDT		paneid_DMSeenUnderline = 'SUND';
const	PaneIDT		paneid_DMAnsweredUnderline = 'AUND';
const	PaneIDT		paneid_DMImportantUnderline = 'IUND';
const	PaneIDT		paneid_DMDeletedUnderline = 'DUND';
const	PaneIDT		paneid_DMMultiAddressBold = 'MBLD';
const	PaneIDT		paneid_DMMultiAddressItalic = 'MITL';
const	PaneIDT		paneid_DMMultiAddressStrike = 'MSTK';
const	PaneIDT		paneid_DMMultiAddressUnderline = 'MUND';
const	PaneIDT		paneid_DMMatchColour = 'MCOL';
const	PaneIDT		paneid_DMUseMatch = 'MTCH';
const	PaneIDT		paneid_DMNonMatchColour = 'NCOL';
const	PaneIDT		paneid_DMUseNonMatch = 'UMCH';
const	PaneIDT		paneid_DMUseLocalTimezone = 'ZONE';

// Mesages
const	MessageT	msg_DMUseMatch = 'MTCH';
const	MessageT	msg_DMUseNonMatch = 'UMCH';

// Resources
const	ResIDT		RidL_CPrefsDisplayMessageBtns = 5042;

// Classes
class LCheckBox;
class LGAColorSwatchControl;

class	CPrefsDisplayMessage : public CPrefsDisplayPanel,
							public LListener
{
private:
	SFullStyleItems		mUnseen;
	SFullStyleItems		mSeen;
	SFullStyleItems		mAnswered;
	SFullStyleItems		mImportant;
	SFullStyleItems		mDeleted;
	LCheckBox*			mMultiAddressBold;
	LCheckBox*			mMultiAddressItalic;
	LCheckBox*			mMultiAddressStrike;
	LCheckBox*			mMultiAddressUnderline;
	LGAColorSwatchControl*	mMatchColour;
	LCheckBox*				mUseMatch;
	LGAColorSwatchControl*	mNonMatchColour;
	LCheckBox*				mUseNonMatch;
	LCheckBox*				mUseLocalTimezone;

public:
	enum { class_ID = 'Dmsg' };

					CPrefsDisplayMessage();
					CPrefsDisplayMessage(LStream *inStream);
	virtual 		~CPrefsDisplayMessage();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
