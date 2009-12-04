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


// Header for CAdbkPropDialog class

#ifndef __CADBKPROPDIALOG__MULBERRY__
#define __CADBKPROPDIALOG__MULBERRY__

#include "CPropDialog.h"

#include "CAddressBook.h"

// Classes
class JXTextButton;
class JXIconTextButton;

class CAdbkPropDialog : public CPropDialog
{
public:
	CAdbkPropDialog(JXDirector* supervisor);

	static bool PoseDialog(CAddressBookList* adbk_list);

protected:
// begin JXLayout

    JXIconTextButton* mGeneralBtn;
    JXIconTextButton* mAccessBtn;
    JXCardFile*       mCards;
    JXTextButton*     mOKBtn;

// end JXLayout
	virtual void	OnCreate();								// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void	SetUpPanels(void);										// About to start dialog
	virtual void	DoPanelInit(void);										// About to display a panel

private:
	CAddressBookList*	mAdbkList;											// List of selected adbks

	void	SetAdbkList(CAddressBookList* adbk_list);								// Set mbox list
};

#endif
