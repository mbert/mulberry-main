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


// Header for CEditGroupDialog class

#ifndef __CEDITGROUPDIALOG__MULBERRY__
#define __CEDITGROUPDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_EditGroup = 9002;
const	PaneIDT		paneid_EditGroupNickName = 'NICK';
const	PaneIDT		paneid_EditGroupGroupName = 'FULL';
const	PaneIDT		paneid_EditGroupAddresses = 'ADDS';
const	PaneIDT		paneid_EditGroupSort = 'SORT';

// Resources
const	ResIDT		RidL_CEditGroupDialogBtns = 9002;
const	ResIDT		STRx_EditGroupHelp = 9005;

// Messages
const	MessageT	msg_EditGroupSort = 9002;

// Classes
class	CTextFieldX;
class	CTextDisplay;
class	CGroup;

class	CEditGroupDialog : public LDialogBox {

private:
	CTextFieldX*	mNickName;
	CTextFieldX*	mGroupName;
	CTextDisplay*	mAddresses;
	
public:
	enum { class_ID = 'EdGr' };

					CEditGroupDialog();
					CEditGroupDialog(LStream *inStream);
	virtual 		~CEditGroupDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetFields(const CGroup* grp);			// Set fields in dialog
	virtual bool	GetFields(CGroup* grp);					// Get fields from dialog

private:
	virtual void	SortAddresses(void);					// Sort addresses in list

};

#endif
