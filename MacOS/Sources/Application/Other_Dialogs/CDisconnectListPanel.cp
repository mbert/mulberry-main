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


// Source for CDisconnectListPanel class

#include "CDisconnectListPanel.h"

#include "CDisconnectDialog.h"
#include "CMboxList.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CDisconnectListPanel::CDisconnectListPanel()
{
}

// Default destructor
CDisconnectListPanel::~CDisconnectListPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________Mailbox List

void CDisconnectListPanel::OnClearMailboxList()
{
	// Delete everything in list and reset table
	dynamic_cast<CDisconnectDialog*>(mParent)->GetTargets().DeleteAll();
	dynamic_cast<CDisconnectDialog*>(mParent)->GetTargetHits().clear();
	
	ResetMailboxList(dynamic_cast<CDisconnectDialog*>(mParent)->GetTargets(),
						dynamic_cast<CDisconnectDialog*>(mParent)->GetTargetHits());
}

// Add a list of mailboxes
void CDisconnectListPanel::AddMboxList(const CMboxList* list)
{
	CMboxRefList& targets = dynamic_cast<CDisconnectDialog*>(mParent)->GetTargets();
	ulvector& target_hits = dynamic_cast<CDisconnectDialog*>(mParent)->GetTargetHits();

	for(CMboxList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		bool result = (targets.AddMbox(*iter) != NULL);
		if (result)
			target_hits.push_back(eNotHit);
	}

	ResetMailboxList(targets, target_hits);
}
