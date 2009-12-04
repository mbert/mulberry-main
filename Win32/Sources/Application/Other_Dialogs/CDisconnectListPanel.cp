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


// CDisconnectListPanel.cp : implementation of the CDisconnectListPanel class
//

#include "CDisconnectListPanel.h"

#include "CDisconnectDialog.h"
#include "CMboxList.h"

/////////////////////////////////////////////////////////////////////////////
// CDisconnectListPanel

BEGIN_MESSAGE_MAP(CDisconnectListPanel, CMailboxListPanel)
	ON_COMMAND(IDC_SEARCH_CLEARLIST, OnClearMailboxList)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDisconnectListPanel construction/destruction

CDisconnectListPanel::CDisconnectListPanel()
{
}

CDisconnectListPanel::~CDisconnectListPanel()
{
}

#pragma mark ____________________________Mailbox List

void CDisconnectListPanel::OnClearMailboxList()
{
	// Delete everything in list and reset table
	dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargets().DeleteAll();
	dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargetHits().clear();
	
	ResetMailboxList(dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargets(),
						dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargetHits());
}

// Add a list of mailboxes
void CDisconnectListPanel::AddMboxList(const CMboxList* list)
{
	CMboxRefList& targets = dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargets();
	ulvector& target_hits = dynamic_cast<CDisconnectDialog*>(mParentFrame)->GetTargetHits();

	for(CMboxList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		bool result = (targets.AddMbox(*iter) != NULL);
		if (result)
			target_hits.push_back(eNotHit);
	}

	ResetMailboxList(targets, target_hits);
}
