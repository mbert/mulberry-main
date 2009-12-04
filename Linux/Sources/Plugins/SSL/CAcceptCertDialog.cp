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


// CAcceptCertDialog.cp : implementation file
//


#include "CAcceptCertDialog.h"

#include "CTableScrollbarSet.h"
#include "CTextDisplay.h"
#include "CTextTable.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CAcceptCertDialog::CAcceptCertDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CAcceptCertDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 450,300, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 450,300);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("The server certificate shown below has the following status:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 375,20);
    assert( obj2 != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,30, 420,65);
    assert( sbs != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Do you want to accept it for this session only, accept and save it\nfor future use or cancel the session?", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,95, 420,35);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Certificate:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,125, 70,20);
    assert( obj4 != NULL );

    mCertificate =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,145, 420,110);
    assert( mCertificate != NULL );

    mAcceptOnceBtn =
        new JXTextButton("Accept Once", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 200,265, 105,25);
    assert( mAcceptOnceBtn != NULL );

    mCancelBtn =
        new JXTextButton("Don't Accept", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 325,265, 105,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^M");

    mAcceptSaveBtn =
        new JXTextButton("Accept & Save", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 20,265, 105,25);
    assert( mAcceptSaveBtn != NULL );

// end JXLayout

	sbs->NoTitles();
	mErrors = new CTextTable(sbs,sbs->GetScrollEnclosure(),
															JXWidget::kHElastic,
															JXWidget::kVElastic,
															0,0, 10, 10);
	mErrors->OnCreate();
	mErrors->SetNoSelection();

	window->SetTitle("Certificate");
	SetButtons(mAcceptOnceBtn, mCancelBtn);


	ListenTo(mAcceptSaveBtn);
}

/////////////////////////////////////////////////////////////////////////////
// CAcceptCertDialog message handlers

void CAcceptCertDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mAcceptSaveBtn)
		{
			EndDialog(kDialogClosed_Btn3);
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

int CAcceptCertDialog::PoseDialog(const char* certificate, const cdstrvect& errors)
{
	CAcceptCertDialog* dlog = new CAcceptCertDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->mErrors->SetContents(errors);
	dlog->mCertificate->SetText(certificate);

	int result = dlog->DoModal(false);
	switch(result)
	{
	case kDialogClosed_OK:
	case kDialogClosed_Btn3:
		dlog->Close();
		break;
	default:;
	}

	return result;
}
