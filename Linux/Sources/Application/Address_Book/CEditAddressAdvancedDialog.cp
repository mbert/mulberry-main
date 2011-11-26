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


// CEditAddressAdvancedDialog.cpp : implementation file
//


#include "CEditAddressAdvancedDialog.h"

#include "CAdbkAddress.h"
#include "CAddressFieldContainer.h"
#include "CBetterScrollbarSet.h"
#include "CBlankScrollable.h"

#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditAddressAdvancedDialog dialog


CEditAddressAdvancedDialog::CEditAddressAdvancedDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CEditAddressAdvancedDialog message handlers

void CEditAddressAdvancedDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 425,500, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 425,500);
    assert( obj1 != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,460, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 340,460, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mScroller =
        new CBetterScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 415,441);
    assert( mScroller != NULL );

    mScrollPane =
        new CBlankScrollable(mScroller, mScroller->GetScrollEnclosure(), //, mScroller,
                    JXWidget::kHElastic, JXWidget::kVElastic, 4,5, 398,431);
    assert( mScrollPane != NULL );

    mFields =
        new CAddressFieldContainer(mScrollPane,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 4,5, 394,421);
    assert( mFields != NULL );

// end JXLayout

	mScrollPane->Init();
	mScroller->SetAllowScroll(false, true);
	mFields->OnCreate();

	window->SetTitle("Address Edit");
	SetButtons(mOKBtn, mCancelBtn);
}

void CEditAddressAdvancedDialog::SetFields(CAdbkAddress* addr, bool allow_edit)
{
    mFields->SetAddress(addr);
}

bool CEditAddressAdvancedDialog::GetFields(CAdbkAddress* addr)
{
    return mFields->GetAddress(addr);
}

bool CEditAddressAdvancedDialog::PoseDialog(CAdbkAddress* addr, bool allow_edit)
{
	bool result = false;

	CEditAddressAdvancedDialog* dlog = new CEditAddressAdvancedDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFields(addr, allow_edit);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		// Get the address
		dlog->GetFields(addr);

		result = true;
		dlog->Close();
	}

	return result;
}
