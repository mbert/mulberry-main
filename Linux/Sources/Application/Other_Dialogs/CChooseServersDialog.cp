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


// CChooseServersDialog.cp : implementation file
//


#include "CChooseServersDialog.h"

#include "CTextField.h"

#include <JXWindow.h>
#include <JXEngravedRect.h>
#include <JXTextButton.h>
#include <JXStaticText.h>

#include "jXGlobals.h"

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CChooseServersDialog::CChooseServersDialog(JXDirector* supervisor)
  : CDialogDirector(supervisor)
{
}

void CChooseServersDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 350,120, "");
    assert( window != NULL );
    SetWindow(window);

    JXEngravedRect* obj1 =
        new JXEngravedRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,120);
    assert( obj1 != NULL );

    JXTextButton* okButton =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,85, 70,25);
    assert( okButton != NULL );
    okButton->SetShortcuts("^M");

    JXTextButton* cancelButton =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,85, 70,25);
    assert( cancelButton != NULL );
    cancelButton->SetShortcuts("^[");

    JXStaticText* obj2 =
        new JXStaticText("Email Server:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,15, 86,20);
    assert( obj2 != NULL );

    mMailServer =
        new CTextInputField(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 115,15, 215,20);
    assert( mMailServer != NULL );

    JXStaticText* obj3 =
        new JXStaticText("SMTP Server:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,45, 90,20);
    assert( obj3 != NULL );

    mSMTPServer =
        new CTextInputField(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 115,45, 215,20);
    assert( mSMTPServer != NULL );

// end JXLayout

	window->SetTitle("Choose Servers");
	SetButtons(okButton, cancelButton);
}

/////////////////////////////////////////////////////////////////////////////
// CChooseServersDialog message handlers

bool CChooseServersDialog::PoseDialog(cdstring& mail, cdstring& smtp)
{
	// Create the dialog (use old name as starter)
	CChooseServersDialog* dlog = new CChooseServersDialog(JXGetApplication());
	
	// Let Dialog process events
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		mail = dlog->mMailServer->GetText();
		smtp = dlog->mSMTPServer->GetText();

		dlog->Close();
		return true;
	}
	else
		return false;
}
