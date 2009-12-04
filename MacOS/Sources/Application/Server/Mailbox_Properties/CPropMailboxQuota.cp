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


// Source for CPropMailboxQuota class

#include "CPropMailboxQuota.h"

#include "CMbox.h"
#include "CMboxList.h"
#include "CQuotaTable.h"



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPropMailboxQuota::CPropMailboxQuota()
{
}

// Constructor from stream
CPropMailboxQuota::CPropMailboxQuota(LStream *inStream)
		: CMailboxPropPanel(inStream)
{
}

// Default destructor
CPropMailboxQuota::~CPropMailboxQuota()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPropMailboxQuota::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxPropPanel::FinishCreateSelf();

	// Get items
	mTable = (CQuotaTable*) FindPaneByID(paneid_MailboxQuotaTable);
}

// Set mbox list
void CPropMailboxQuota::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());
}


// Set mbox list
void CPropMailboxQuota::SetMbox(CMbox* mbox)
{
	mTable->SetList(mbox->GetQuotas());
}

// Set protocol
void CPropMailboxQuota::SetProtocol(CMboxProtocol* protocol)
{
	// Does nothing
}

// Force update of items
void CPropMailboxQuota::ApplyChanges(void)
{
	// Nothing to update
}
