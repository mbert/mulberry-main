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


// Header for CPropAdbkACL class

#ifndef __CPROPADBKACL__MULBERRY__
#define __CPROPADBKACL__MULBERRY__

#include "CAdbkPropPanel.h"
#include "CListener.h"

#include "CAdbkACL.h"

// Constants

// Panes
const	PaneIDT		paneid_PropAdbkACL = 9512;
const	PaneIDT		paneid_AdbkACLTable = 'TABL';
const	PaneIDT		paneid_AdbkACLLookupMark = 'IND1';
const	PaneIDT		paneid_AdbkACLReadMark = 'IND2';
const	PaneIDT		paneid_AdbkACLWriteMark = 'IND3';
const	PaneIDT		paneid_AdbkACLCreateMark = 'IND4';
const	PaneIDT		paneid_AdbkACLDeleteMark = 'IND5';
const	PaneIDT		paneid_AdbkACLAdminMark = 'IND6';
const	PaneIDT		paneid_AdbkACLNewUserBtn = 'NEWU';
const	PaneIDT		paneid_AdbkACLDeleteUserBtn = 'DELU';
const	PaneIDT		paneid_AdbkACLStylePopup = 'STYL';

// Resources

const	ResIDT		RidL_CPropAdbkACLBtns = 9512;

// Mesages
const	MessageT	msg_ACLAdbkNewUser = 'NEWU';
const	MessageT	msg_ACLAdbkDeleteUser = 'DELU';
const	MessageT	msg_ACLAdbkStylePopup = 'STYL';

// Classes
class CACLStylePopup;
class CACLTable;
class CAddressBook;
class LIconControl;
class LPushButton;

class CPropAdbkACL : public CAdbkPropPanel,
						public LListener,
						public CListener
{
public:
	enum { class_ID = 'AAcl' };

					CPropAdbkACL();
					CPropAdbkACL(LStream *inStream);
	virtual 		~CPropAdbkACL();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetAdbkList(CAddressBookList* adbk_list);		// Set adbk list
	virtual void	DoChangeACL(CAdbkACL::EAdbkACL acl, bool set);	// User clicked a button
	virtual void	ApplyChanges(void);						// Force update of adbks

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
private:
	LIconControl*	mLookupMark;
	LIconControl*	mReadMark;
	LIconControl*	mWriteMark;
	LIconControl*	mCreateMark;
	LIconControl*	mDeleteMark;
	LIconControl*	mAdminMark;
	CACLTable*		mACLTable;
	LPushButton*	mNewUserBtn;
	LPushButton*	mDeleteUserBtn;
	CACLStylePopup*	mStylePopup;
	bool			mCanChange;

	virtual void		SetAddressBook(CAddressBook* adbk);		// Set adbk item
	virtual void		SetMyRights(SACLRight rights);			// Set indicators from rights
	virtual void		SetButtons(bool enable);				// Enable/disable buttons

	// Handle actions
	virtual void	DoChangeRights(SACLRight rights);					// User clicked a button
	virtual void	DoNewUser(void);						// Do new user dialog
	virtual void	DoDeleteUser(void);						// Delete user from list
	virtual void	DoStylePopup(long index);				// Handle popup command
};

#endif
