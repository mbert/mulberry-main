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


// CEditIdentityOutgoing.cpp : implementation file
//

#include "CEditIdentityOutgoing.h"

#include "CEditIdentityAddress.h"
#include "CEditIdentityDSN.h"
#include "CTabController.h"

#include "cdstring.h"

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityOutgoing message handlers

// Set data
// Get details of sub-panes
void CEditIdentityOutgoing::OnCreate()
{
// begin JXLayout1

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 430,175);
    assert( mTabs != NULL );

// end JXLayout1

	// Create tab panels
	CEditIdentityAddress* panel_to = new CEditIdentityAddress(true, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	panel_to->SetAddressType(false, false, false, true, false, false, false);
	cdstring title;
	title.FromResource(IDS_IDENTITY_PANEL_TO);
	mTabs->AppendCard(panel_to, title);

	CEditIdentityAddress* panel_cc = new CEditIdentityAddress(true, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	panel_cc->SetAddressType(false, false, false, false, true, false, false);
	title.FromResource(IDS_IDENTITY_PANEL_CC);
	mTabs->AppendCard(panel_cc, title);

	CEditIdentityAddress* panel_bcc = new CEditIdentityAddress(true, mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	panel_bcc->SetAddressType(false, false, false, false, false, true, false);
	title.FromResource(IDS_IDENTITY_PANEL_BCC);
	mTabs->AppendCard(panel_bcc, title);

	CEditIdentityDSN* panel_dsn = new CEditIdentityDSN(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 450, 150);
	mTabs->AppendCard(panel_dsn, "DSN");
}

// Set data
void CEditIdentityOutgoing::SetData(void* data)
{
	mTabs->SetData(data);
	mTabs->ShowCard(1);
}

// Force update of data
bool CEditIdentityOutgoing::UpdateData(void* data)
{
	// Get values
	return mTabs->UpdateData(data);
}
