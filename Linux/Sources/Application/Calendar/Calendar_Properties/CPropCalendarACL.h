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


// Header for CPropCalendarACL class

#ifndef __CPropCalendarACL__MULBERRY__
#define __CPropCalendarACL__MULBERRY__

#include "CCalendarPropPanel.h"
#include "CListener.h"

#include "CCalendarACL.h"
#include "CCalendarStoreNode.h"

#include "HPopupMenu.h"

// Classes
class CACLStylePopup;
class CACLTitleTable;
class CACLTable;
class CMbox;
class JXTextButton;

class CPropCalendarACL : public CCalendarPropPanel,
						public CListener
{
public:
		CPropCalendarACL(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CCalendarPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ mCanChange = false; mNoChange = false, mActionCal = NULL; }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	DoChangeACL(CCalendarACL::ECalACL acl, bool set);	// User clicked a button
	virtual void	ApplyChanges(void);						// Force update of mboxes

	virtual void	ListenTo_Message(long msg, void* param);

protected:
// begin JXLayout1

    HPopupMenu*     mCalendarPopup;
    JXTextButton*   mNewUserBtn;
    JXTextButton*   mDeleteUserBtn;
    CACLStylePopup* mACLStylePopup;
    JXTextButton*   mChangeAllBtn;

// end JXLayout1
	CACLTitleTable*	mACLTitleTable;
	CACLTable*		mACLTable;
	bool			mCanChange;
	bool			mNoChange;
	
	calstore::CCalendarStoreNode*	mActionCal;

	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
	void		SetCal(calstore::CCalendarStoreNode* cal);		// Set mbox item
	void		SetMyRights(SACLRight rights);					// Set indicators from rights
	void		SetButtons(bool enable, bool alloff);			// Enable/disable buttons
	void		InitCalendarPopup();

	// Handle actions
	void	DoChangeRights(SACLRight rights);			// User clicked a button
	void	DoNewUser(void);							// Do new user dialog
	void	DoDeleteUser(void);							// Delete user from list
	void	DoStylePopup(long index);					// Handle popup command
	void	DoChangeAll();								// Change rights
};

#endif
