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
#include "CSimpleTitleTable.h"
#include "CTableScrollbarSet.h"

#include <JXColormap.h>
#include <JXStaticText.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Get details of sub-panes
void CPropMailboxQuota::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Quota", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 55,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 430,230);
    assert( sbs != NULL );

// end JXLayout1

    sbs->SetBorderWidth(kJXDefaultBorderWidth);

	CSimpleTitleTable* titles = new CSimpleTitleTable(sbs, sbs->GetScrollEnclosure(),
																	 JXWidget::kHElastic,
																	 JXWidget::kFixedTop,
																	 0, 0, 100, cTitleHeight);

	mTable = new CQuotaTable(sbs, sbs->GetScrollEnclosure(),
								JXWidget::kHElastic, JXWidget::kVElastic,
								0,cTitleHeight, 100, 100);

	mTable->OnCreate();
	titles->OnCreate();

	// Setup titles to sync with table and add titles
	titles->SyncTable(mTable, true);
	titles->LoadTitles("UI::Titles::Quota", 4);

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
