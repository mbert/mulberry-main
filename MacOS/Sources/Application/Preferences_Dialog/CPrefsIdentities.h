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


// Header for CPrefsIdentities class

#ifndef __CPREFSIDENTITIES__MULBERRY__
#define __CPREFSIDENTITIES__MULBERRY__

#include "CPrefsPanel.h"
#include "CListener.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsIdentities = 5009;
const	PaneIDT		paneid_IdentitiesListHeader = 'IHDR';
const	PaneIDT		paneid_IdentitiesListTitle = 'ITIT';
const	PaneIDT		paneid_IdentitiesList = 'IDTS';
const	PaneIDT		paneid_AddIdentitiesBtn = 'ADDI';
const	PaneIDT		paneid_ChangeIdentitiesBtn = 'CHGI';
const	PaneIDT		paneid_DuplicateIdentitiesBtn = 'DUPL';
const	PaneIDT		paneid_DeleteIdentitiesBtn = 'DELI';
const	PaneIDT		paneid_ContextTied = 'TIEC';
const	PaneIDT		paneid_MsgTied = 'MsgT';
const	PaneIDT		paneid_TiedMboxInherit = 'MbxI';
const	PaneIDT		paneid_SmartText = 'STXT';

// Mesages
const	MessageT	msg_AddIdentities = 5190;
const	MessageT	msg_ChangeIdentities = 5191;
const	MessageT	msg_DuplicateIdentities = 5192;
const	MessageT	msg_DeleteIdentities = 5193;
const	MessageT	msg_SelectIdentities = 'IDTS';

// Resources
const	ResIDT		RidL_CPrefsIdentitiesBtns = 5009;

const	ResIDT		STRx_PrefsIdentitiesHelp = 5009;

// Classes
class CTextDisplay;
class CTextTable;
class LCheckBox;
class LPushButton;

class	CPrefsIdentities : public CPrefsPanel,
						public LListener,
						public CListener {

private:
	CTextTable*			mIdentitiesList;
	LPushButton*		mChangeBtn;
	LPushButton*		mDuplicateBtn;
	LPushButton*		mDeleteBtn;
	LCheckBox*			mContextTied;
	LCheckBox*			mMsgTied;
	LCheckBox*			mTiedMboxInherit;
	CTextDisplay*		mSmartAddressText;

public:
	enum { class_ID = 'Pidt' };

					CPrefsIdentities();
					CPrefsIdentities(LStream *inStream);
	virtual 		~CPrefsIdentities();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

	virtual void	InitIdentitiesList(void);				// Initialise the list
	
	virtual void	DoAddIdentities(void);					// Set identities
	virtual void	DoEditIdentities(void);					// Set identities
	virtual void	DoDuplicateIdentities(void);			// Duplicate identities
	virtual void	DoDeleteIdentities(void);				// Clear identities
	virtual void	DoMoveIdentities(const ulvector& from,	// Move identities
										unsigned long to);
};

#endif
