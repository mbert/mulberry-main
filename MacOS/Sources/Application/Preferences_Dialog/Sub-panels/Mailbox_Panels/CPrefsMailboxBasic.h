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


// Header for CPrefsMailboxBasic class

#ifndef __CPREFSMAILBOXBASIC__MULBERRY__
#define __CPREFSMAILBOXBASIC__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsMailboxBasic = 5060;
const	PaneIDT		paneid_MBOpenFirst = 'FRST';
const	PaneIDT		paneid_MBOpenLast = 'LAST';
const	PaneIDT		paneid_MBOpenFirstNew = 'FNEW';
const	PaneIDT		paneid_MBNoOpenPreview = 'NOPR';
const	PaneIDT		paneid_MBOldestToNewest = 'PRON';
const	PaneIDT		paneid_MBNewestToOldest = 'PRNO';
const	PaneIDT		paneid_MBUnseenNew = 'UNSN';
const	PaneIDT		paneid_MBRecentNew = 'RCNT';
const	PaneIDT		paneid_MBExpungeOnClose = 'EXPG';
const	PaneIDT		paneid_MBWarnOnExpunge = 'WARN';
const	PaneIDT		paneid_MBWarnPuntUnseen = 'PUNT';
const	PaneIDT		paneid_MBDoRollover = 'OPEN';
const	PaneIDT		paneid_MBRolloverWarn = 'NEXT';
const	PaneIDT		paneid_MBScrollForUnseen = 'SCNW';
const	PaneIDT		paneid_MBMoveFromINBOX = 'MOVE';
const	PaneIDT		paneid_MBClearMailbox = 'INCL';
const	PaneIDT		paneid_MBClearMailboxPopup = 'MOVM';
const	PaneIDT		paneid_MBWarnOnClear = 'WCLR';

// Mesages
const	MessageT	msg_MBAllowMove = 'MOVE';
const	MessageT	msg_MBClearMailboxPopup = 'MOVM';

// Resources
const	ResIDT		RidL_CPrefsMailboxBasicBtns = 5060;

// Classes
class LCheckBox;
class LCheckBoxGroupBox;
class LRadioButton;
class CMailboxPopup;
class CTextFieldX;

class CPrefsMailboxBasic : public CPrefsTabSubPanel,
							public LListener
{
private:
	LRadioButton*		mOpenFirst;
	LRadioButton*		mOpenLast;
	LRadioButton*		mOpenFirstNew;
	LCheckBox*			mNoOpenPreview;
	LRadioButton*		mOldestToNewest;
	LRadioButton*		mNewestToOldest;
	LCheckBox*			mExpungeOnClose;
	LCheckBox*			mWarnOnExpunge;
	LCheckBox*			mWarnPuntUnseen;
	LCheckBox*			mDoRollover;
	LCheckBox*			mRolloverWarn;
	LCheckBox*			mScrollForUnseen;
	LCheckBoxGroupBox*	mMoveFromINBOX;
	CTextFieldX*		mClearMailbox;
	CMailboxPopup*		mClearMailboxPopup;
	LCheckBox*			mWarnOnClear;

public:
	enum { class_ID = 'Mbas' };

					CPrefsMailboxBasic();
					CPrefsMailboxBasic(LStream *inStream);
	virtual 		~CPrefsMailboxBasic();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
