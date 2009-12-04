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


// Header for CPropMailboxOptions class

#ifndef __CPROPMAILBOXOPTIONS__MULBERRY__
#define __CPROPMAILBOXOPTIONS__MULBERRY__

#include "CMailboxPropPanel.h"

#include "CMailAccountManager.h"

#include "HPopupMenu.h"

// Classes
class CIdentityPopup;
class JXImageWidget;
class JXTextButton;
class JXTextCheckbox3;

class CPropMailboxOptions : public CMailboxPropPanel
{
public:
		CPropMailboxOptions(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CMailboxPropPanel(enclosure, hSizing, vSizing, x, y, w, h)
			{ }

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
// begin JXLayout1

    JXImageWidget*   mIconState;
    JXTextCheckbox3* mCheck;
    HPopupMenu*      mAlertStylePopup;
    JXTextCheckbox3* mOpen;
    JXTextCheckbox3* mCopyTo;
    JXTextCheckbox3* mAppendTo;
    JXTextCheckbox3* mPuntOnClose;
    JXTextCheckbox3* mAutoSync;
    JXTextCheckbox3* mTieIdentity;
    CIdentityPopup*  mIdentityPopup;
    JXTextButton*    mRebuildLocal;

// end JXLayout1
	cdstring		mCurrentIdentity;
	bool			mDisableAutoSync;
	bool			mAllDir;
	bool			mHasLocal;

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	
private:
			void	InitAlertStylePopup();
			void	SetFavourite(bool set, CMailAccountManager::EFavourite fav_type);
			void	SetTiedIdentity(bool add);				// Add/remove tied identity
			void	OnRebuildLocal();

			CMailAccountManager::EFavourite GetStyleType(unsigned long index) const;
};

#endif
