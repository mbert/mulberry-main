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


// Source for CAboutDialog class

#include "CAboutDialog.h"

#include "CPreferences.h"
#include "CStaticText.h"

#include "HResourceMap.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXImageWidget.h>

#include <jXGlobals.h>

#include <cassert>

// __________________________________________________________________________________________________
// C L A S S __ C A B O U T D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAboutDialog::CAboutDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAboutDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 360,230, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 360,230);
    assert( obj1 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 145,185, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mVersion =
        new CStaticText("Version v%s", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 340,20);
    assert( mVersion != NULL );
    mVersion->SetFontSize(10);

    JXStaticText* obj2 =
        new JXStaticText("http://www.mulberrymail.com", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,135, 340,20);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

    JXStaticText* obj3 =
        new JXStaticText("Copyright Cyrus Daboo, 2006-2009. All Rights Reserved.", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,155, 340,20);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);

    JXImageWidget* obj4 =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 50,5, 260,100);
    assert( obj4 != NULL );

// end JXLayout

	obj2->SetFontAlign(JXStaticText::kAlignCenter);
	obj3->SetFontAlign(JXStaticText::kAlignCenter);

	window->SetTitle("About Mulberry");
	obj4->SetImage(bmpFromResource(IDB_LOGO, obj4), kTrue);
	SetButtons(mOKBtn, NULL);

	cdstring txt = mVersion->GetText();
	txt.Substitute(CPreferences::sPrefs->GetVersionText());
	mVersion->SetBreakCROnly(false);
	mVersion->SetText(txt);
	mVersion->SetFontAlign(CStaticText::kAlignCenter);
}

bool CAboutDialog::PoseDialog()
{
	CAboutDialog* dlog = new CAboutDialog(JXGetApplication());

	// Test for OK
	if (dlog->DoModal() == kDialogClosed_OK)
		dlog->Close();

	return true;
}
