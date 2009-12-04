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


// Header for CPrefsRemoteSets class

#ifndef __CPREFSREMOTESETS__MULBERRY__
#define __CPREFSREMOTESETS__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsRemoteSets = 5023;
const	PaneIDT		paneid_PrefsRemoteSetsList = 'SETS';
const	PaneIDT		paneid_PrefsRemoteSetsRefreshBtn = 'REFS';
const	PaneIDT		paneid_PrefsRemoteSetsRenameBtn = 'RENS';
const	PaneIDT		paneid_PrefsRemoteSetsDeleteBtn = 'DELS';

// Mesages
const	MessageT	msg_PrefsRemoteSetsSelect = 'SETS';
const	MessageT	msg_PrefsRemoteSetsRefresh = 'REFS';
const	MessageT	msg_PrefsRemoteSetsRename = 'RENS';
const	MessageT	msg_PrefsRemoteSetsDelete = 'DELS';

// Resources
const	ResIDT		RidL_CPrefsRemoteSetsBtns = 5023;

// Classes
class CPreferences;
class CTextTable;
class LPushButton;

class	CPrefsRemoteSets : public CPrefsTabSubPanel,
								public LListener
{
private:
	CTextTable*			mList;
	LPushButton*		mRefreshBtn;
	LPushButton*		mRenameBtn;
	LPushButton*		mDeleteBtn;

public:
	enum { class_ID = 'Aset' };

					CPrefsRemoteSets();
					CPrefsRemoteSets(LStream *inStream);
	virtual 		~CPrefsRemoteSets();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

	virtual void	DoRefresh(void);						// Refresh remote list
	virtual void	DoRename(void);							// Rename sets
	virtual void	DoDelete(void);							// Delete sets

private:
	virtual void	UpdateList(void);						// Update list
};

#endif
