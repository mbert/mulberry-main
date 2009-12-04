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


// Header for CPropMailboxGeneral class

#ifndef __CPROPMAILBOXGENERAL__MULBERRY__
#define __CPROPMAILBOXGENERAL__MULBERRY__

#include "CMailboxPropPanel.h"


// Classes
class CMbox;
class CStaticText;
class JXImageWidget;
class JXTextButton;

class CPropMailboxGeneral : public CMailboxPropPanel
{
public:
		CPropMailboxGeneral(JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
			: CMailboxPropPanel(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void	OnCreate();								// Do odds & ends
	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
// begin JXLayout1

    JXImageWidget* mIconState;
    CStaticText*   mName;
    CStaticText*   mServer;
    CStaticText*   mHierarchy;
    CStaticText*   mSeparator;
    CStaticText*   mTotal;
    CStaticText*   mRecent;
    CStaticText*   mUnseen;
    CStaticText*   mDeleted;
    CStaticText*   mFullSync;
    CStaticText*   mPartialSync;
    CStaticText*   mMissingSync;
    CStaticText*   mSize;
    JXTextButton*  mCalculateBtn;
    CStaticText*   mStatus;
    CStaticText*   mUIDValidity;
    CStaticText*   mLastSync;

// end JXLayout1

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	
private:
			void	SetMbox(CMbox* mbox);					// Set mbox item
			void	OnCalculateSize();

};

#endif
