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


// Header for CUserActionOptions class

#ifndef __CUSERACTIONOPTIONS__MULBERRY__
#define __CUSERACTIONOPTIONS__MULBERRY__

#include "CUserAction.h"

// Constants
const	PaneIDT		paneid_UserActionSelect = 'SELC';
const	PaneIDT		paneid_UserActionSingleClick = 'CLCK';
const	PaneIDT		paneid_UserActionDoubleClick = 'DBLC';
const	PaneIDT		paneid_UserActionUseKey = 'UKEY';
const	PaneIDT		paneid_UserActionKey = 'TKEY';
const	PaneIDT		paneid_UserActionKeyChange = 'CHOS';

// Messages
const	MessageT	msg_UserActionKeyChange = 'CHOS';

// Resources

// Classes
class CStaticText;
class LCheckBox;
class LCheckBoxGroupBox;

class CUserActionOptions : public LView,
							public LListener
{
public:
	enum { class_ID = 'UAct' };

					CUserActionOptions();
					CUserActionOptions(LStream *inStream);
	virtual 		~CUserActionOptions();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void	SetData(const CUserAction& action);
	void	GetData(CUserAction& action);
	void	DisableItems();

protected:
	LCheckBox*			mSelect;
	LCheckBox*			mSingleClick;
	LCheckBox*			mDoubleClick;
	LCheckBoxGroupBox*	mUseKey;
	CStaticText*		mKey;

	unsigned char		mActualKey;
	CKeyModifiers		mActualMods;

	virtual void	FinishCreateSelf(void);

private:
	void GetKey();
};

#endif
