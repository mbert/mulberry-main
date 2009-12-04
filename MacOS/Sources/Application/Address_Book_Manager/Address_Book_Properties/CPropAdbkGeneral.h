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


// Header for CPropAdbkGeneral class

#ifndef __CPROPADBKGENERAL__MULBERRY__
#define __CPROPADBKGENERAL__MULBERRY__

#include "CAdbkPropPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PropAdbkGeneral = 9511;
const	PaneIDT		paneid_AdbkGeneralState = 'ICON';
const	PaneIDT		paneid_AdbkGeneralName = 'NAME';
const	PaneIDT		paneid_AdbkGeneralServer = 'SERV';
const	PaneIDT		paneid_AdbkGeneralTotal = 'TOTL';
const	PaneIDT		paneid_AdbkGeneralSingle = 'SING';
const	PaneIDT		paneid_AdbkGeneralGroup = 'GRUP';
const	PaneIDT		paneid_AdbkGeneralOpenStart = 'OPEN';
const	PaneIDT		paneid_AdbkGeneralNickName = 'NICK';
const	PaneIDT		paneid_AdbkGeneralSearch = 'SRCH';
const	PaneIDT		paneid_AdbkGeneralAdd = 'ADDR';
const	PaneIDT		paneid_AdbkGeneralAutoSync = 'SYNC';

// Mesages
const	MessageT	msg_AdbkGeneralOpenStart = 'OPEN';
const	MessageT	msg_AdbkGeneralNickName = 'NICK';
const	MessageT	msg_AdbkGeneralSearch = 'SRCH';
const	MessageT	msg_AdbkGeneralAdd = 'ADDR';
const	MessageT	msg_AdbkGeneralAutoSync = 'SYNC';

// Resources
const ResIDT	RidL_CPropAdbkGeneralBtns = 9511;

// Classes
class CAddressBook;
class CTextFieldX;
class LCheckBox;
class LIconControl;

class CPropAdbkGeneral : public CAdbkPropPanel,
								public LListener
{
private:
	LIconControl*		mIconState;
	CTextFieldX*		mName;
	CTextFieldX*		mServer;
	CTextFieldX*		mTotal;
	CTextFieldX*		mSingle;
	CTextFieldX*		mGroup;
	LCheckBox*			mOpen;
	LCheckBox*			mNickName;
	LCheckBox*			mSearch;
	LCheckBox*			mAdd;
	LCheckBox*			mAutoSync;

public:
	enum { class_ID = 'AGen' };

					CPropAdbkGeneral();
					CPropAdbkGeneral(LStream *inStream);
	virtual 		~CPropAdbkGeneral();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetAdbkList(CAddressBookList* adbk_list);		// Set adbk list
	virtual void	ApplyChanges(void);							// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);						// Do odds & ends
	
private:
	virtual void	SetAddressBook(CAddressBook* adbk);			// Set adbk item

	virtual void	OnCheckOpenAtStart(bool set);
	virtual void	OnCheckNickName(bool set);
	virtual void	OnCheckSearch(bool set);
	virtual void	OnCheckAutoSync(bool set);
};

#endif
