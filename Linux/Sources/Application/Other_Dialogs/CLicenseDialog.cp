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


// CLicenseDialog.cpp : implementation file
//


#include "CLicenseDialog.h"
#include "CTextDisplay.h"

#include "LicenseText.txt"

#include "HResourceMap.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXImageWidget.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CLicenseDialog dialog


CLicenseDialog::CLicenseDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

void CLicenseDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 355,320, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 355,320);
    assert( obj1 != NULL );

    mOKBtn =
        new JXTextButton("I Agree", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,285, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,285, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mLicenseText =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,130, 335,140);
    assert( mLicenseText != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Demo License", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,115, 85,15);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);
    const JFontStyle obj2_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj2->SetFontStyle(obj2_style);

    mLogo =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 260,100);
    assert( mLogo != NULL );

// end JXLayout

	window->SetTitle("Mulberry Demo License");
	mLogo->SetImage(bmpFromResource(IDB_LOGO, mLogo), kTrue);
	SetButtons(mOKBtn, mCancelBtn);

	mLicenseText->SetText(cLicenseText);

}
