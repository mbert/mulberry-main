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


// Header for CPropMailboxQuota class

#ifndef __CPROPMAILBOXQUOTA__MULBERRY__
#define __CPROPMAILBOXQUOTA__MULBERRY__

#include "CMailboxPropPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PropMailboxQuota = 4014;
const	PaneIDT		paneid_MailboxQuotaTable = 'TABL';

// Classes
class CMbox;
class CQuotaTable;

class CPropMailboxQuota : public CMailboxPropPanel
{
public:
	enum { class_ID = 'MQuo' };

					CPropMailboxQuota();
					CPropMailboxQuota(LStream *inStream);
	virtual 		~CPropMailboxQuota();

	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
private:
	CQuotaTable*	mTable;

	virtual void	SetMbox(CMbox* mbox);					// Set mbox item

};

#endif
