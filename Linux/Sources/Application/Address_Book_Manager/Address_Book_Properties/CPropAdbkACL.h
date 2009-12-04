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

// Classes
class CACLStylePopup;
class CACLTable;
class CACLTitleTable;
class CAddressBook;
class JXTextButton;

class CPropAdbkACL : public CAdbkPropPanel,
					 public CListener
{
public:
		CPropAdbkACL(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CAdbkPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ mCanChange = false; }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetAdbkList(CAddressBookList* adbk_list);		// Set adbk list
	virtual void	DoChangeACL(CAdbkACL::EAdbkACL acl, bool set);	// User clicked a button
	virtual void	ApplyChanges();							// Force update of adbks

	virtual void	ListenTo_Message(long msg, void* param);

protected:
// begin JXLayout1

    JXTextButton*   mNewUserBtn;
    JXTextButton*   mDeleteUserBtn;
    CACLStylePopup* mACLStylePopup;

// end JXLayout1
	CACLTitleTable*	mACLTitleTable;
	CACLTable*		mACLTable;
	bool			mCanChange;

	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
			void		SetAddressBook(CAddressBook* adbk);		// Set adbk item
			void		SetMyRights(SACLRight rights);			// Set indicators from rights
			void		SetButtons(bool enable);				// Enable/disable buttons

	// Handle actions
			void	DoChangeRights(SACLRight rights);			// User clicked a button
			void	DoNewUser();								// Do new user dialog
			void	DoDeleteUser();								// Delete user from list
			void	DoStylePopup(JIndex index);					// Handle popup command
};

#endif
