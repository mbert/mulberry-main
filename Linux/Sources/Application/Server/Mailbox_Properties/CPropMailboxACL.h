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


// Header for CPropMailboxACL class

#ifndef __CPROPMAILBOXACL__MULBERRY__
#define __CPROPMAILBOXACL__MULBERRY__

#include "CMailboxPropPanel.h"
#include "CListener.h"

#include "CMboxACL.h"

#include "HPopupMenu.h"

// Classes
class CACLStylePopup;
class CACLTitleTable;
class CACLTable;
class CMbox;
class JXTextButton;

class CPropMailboxACL : public CMailboxPropPanel,
						public CListener
{
public:
		CPropMailboxACL(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CMailboxPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ mCanChange = false; mNoChange = false, mActionMbox = NULL; }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetMboxList(CMboxList* mbox_list);				// Set mbox list
	virtual void	DoChangeACL(CMboxACL::EMboxACL acl, bool set);	// User clicked a button
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges();									// Force update of mboxes

	virtual void	ListenTo_Message(long msg, void* param);

protected:
// begin JXLayout1

    HPopupMenu*     mMailboxPopup;
    JXTextButton*   mNewUserBtn;
    JXTextButton*   mDeleteUserBtn;
    CACLStylePopup* mACLStylePopup;
    JXTextButton*   mChangeAllBtn;

// end JXLayout1
	CACLTitleTable*	mACLTitleTable;
	CACLTable*		mACLTable;
	bool			mCanChange;
	bool			mNoChange;
	CMbox*			mActionMbox;

	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
			void		SetMbox(CMbox* mbox);					// Set mbox item
			void		SetMyRights(SACLRight rights);			// Set indicators from rights
			void		SetButtons(bool enable, bool alloff);	// Enable/disable buttons
			void		InitMailboxPopup();

	// Handle actions
			void	DoChangeRights(SACLRight rights);			// User clicked a button
			void	DoNewUser();								// Do new user dialog
			void	DoDeleteUser();								// Delete user from list
			void	DoStylePopup(JIndex index);					// Handle popup command
			void	DoChangeAll();								// Change rights
};

#endif
