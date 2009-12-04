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


// CViewCertDialog.cp : implementation file
//


#include "CViewCertDialog.h"

#include "CTextDisplay.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXTextButton.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CViewCertDialog::CViewCertDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CViewCertDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 450,300, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 450,300);
    assert( obj1 != NULL );

    mCertificate =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,15, 420,240);
    assert( mCertificate != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 345,265, 85,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Certificate");
	SetButtons(mOKBtn, mOKBtn);
}

/////////////////////////////////////////////////////////////////////////////
// CViewCertDialog message handlers

void CViewCertDialog::PoseDialog(const cdstring& certificate)
{
	CViewCertDialog* dlog = new CViewCertDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->mCertificate->SetText(certificate);

	if (dlog->DoModal(false) == kDialogClosed_OK)
		dlog->Close();
}
