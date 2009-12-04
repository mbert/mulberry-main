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


// Header for CDisconnectDialog class

#ifndef __CDISCONNECTDIALOG__MULBERRY__
#define __CDISCONNECTDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CDisconnectListPanel.h"

#include "CMboxRefList.h"

#include "templs.h"

// Constants

// Panes
const	PaneIDT		paneid_DisconnectDialog = 1003;
const	PaneIDT		paneid_DisconnectDisconnect = 'DISC';
const	PaneIDT		paneid_DisconnectAll = 'SALL';
const	PaneIDT		paneid_DisconnectNew = 'SNEW';
const	PaneIDT		paneid_DisconnectNone = 'SNON';
const	PaneIDT		paneid_DisconnectSend = 'SEND';
const	PaneIDT		paneid_DisconnectConnect = 'CONN';
const	PaneIDT		paneid_DisconnectPermanent = 'PERM';
const	PaneIDT		paneid_DisconnectUpdate = 'UPDT';
const	PaneIDT		paneid_DisconnectPlayback = 'PLAY';
const	PaneIDT		paneid_DisconnectList = 'MLST';
const	PaneIDT		paneid_DisconnectPOP3 = 'POP3';
const	PaneIDT		paneid_DisconnectWait = 'WAIT';
const	PaneIDT		paneid_DisconnectAdbk1 = 'ADD1';
const	PaneIDT		paneid_DisconnectAdbk2 = 'ADD2';
const	PaneIDT		paneid_DisconnectMessage = 'MESS';
const	PaneIDT		paneid_DisconnectFull = 'FULL';
const	PaneIDT		paneid_DisconnectBelow = 'BELO';
const	PaneIDT		paneid_DisconnectSize = 'SIZE';
const	PaneIDT		paneid_DisconnectPartial = 'PART';
const	PaneIDT		paneid_DisconnectPlayProgress = 'PPRG';
const	PaneIDT		paneid_DisconnectProgress1 = 'PRG1';
const	PaneIDT		paneid_DisconnectProgress2 = 'PRG2';
const	PaneIDT		paneid_DisconnectProgress = 'PROG';
const	PaneIDT		paneid_DisconnectOKBtn = 'OKBT';
const	PaneIDT		paneid_DisconnectCancelBtn = 'CANC';

// Mesages
const	MessageT	msg_DisconnectAll = 'SALL';
const	MessageT	msg_DisconnectNew = 'SNEW';
const	MessageT	msg_DisconnectNone = 'SNON';
const	MessageT	msg_DisconnectPermanent = 'PERM';
const	MessageT	msg_DisconnectUpdate = 'UPDT';
const	MessageT	msg_DisconnectFull = 'FULL';
const	MessageT	msg_DisconnectBelow = 'BELO';
const	MessageT	msg_DisconnectPartial = 'PART';
const	MessageT	msg_DisconnectListDelete = 'LDEL';

// Resources
const	ResIDT		RidL_CDisconnectDialogBtns = 1003;

class CBarPane;
class CIconTextTable;
class CTextFieldX;
class LCheckBox;
class LRadioButton;

class	CDisconnectDialog : public LDialogBox
{
private:
	static CDisconnectDialog* sDisconnectDialog;

	LPane*				mDisconnectPanel;
	LRadioButton*		mAll;
	LRadioButton*		mNew;
	LRadioButton*		mNone;
	LCheckBox*			mSend;
	LPane*				mConnectPanel;
	LRadioButton*		mPermanent;
	LRadioButton*		mUpdate;
	LCheckBox*			mPlayback;
	LCheckBox*			mPOP3;
	LCheckBox*			mList;
	LCheckBox*			mWait;
	LCheckBox*			mAdbk1;
	LCheckBox*			mAdbk2;
	LPane*				mMessagePanel;
	LRadioButton*		mFull;
	LRadioButton*		mBelow;
	CTextFieldX*		mSize;
	LRadioButton*		mPartial;
	LPane*				mPlayProgressPanel;
	CBarPane*			mProgress1;
	CBarPane*			mProgress2;
	LPane*				mProgressPanel;
	LPane*				mOKBtn;
	LPane*				mCancelBtn;

	CDisconnectListPanel	mMailboxListPanel;

	CMboxRefList		mSyncTargets;
	ulvector			mSyncTargetHits;
	static bool			sIsTwisted;

public:
	enum { class_ID = 'DisC' };

					CDisconnectDialog();
					CDisconnectDialog(LStream *inStream);
	virtual 		~CDisconnectDialog();

	static CDisconnectDialog* GetDisconnectDialog()
		{ return sDisconnectDialog; }

	CMboxRefList&	GetTargets()
		{ return mSyncTargets; }
	ulvector&		GetTargetHits()
		{ return mSyncTargetHits; }

	void 			NextItem(unsigned long item);
	void 			DoneItem(unsigned long item, bool hit);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

			void	ShowProgressPanel(bool show);			// Show or hide the progress panel
			void	OnListDelete();							// Delete item from mailbox list
			void	SetDetails(bool connecting);			// Set the dialogs info
			void	GetDetailsDisconnect();					// Get the dialogs return info
			void	GetDetailsConnect();					// Get the dialogs return info

public:
	static  void	PoseDialog();
};

#endif
