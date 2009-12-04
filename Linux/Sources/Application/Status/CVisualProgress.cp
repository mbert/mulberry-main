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


// Source for CProgressDialog class

#include "CVisualProgress.h"

#include "CProgressBar.h"
#include "CStaticText.h"

#include <JXTextButton.h>
#include <JXStaticText.h>
#include <JXWindow.h>
#include <JXUpRect.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CProgressDialog::CProgressDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

// Default destructor
CProgressDialog::~CProgressDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CProgressDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 390,175, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 390,175);
    assert( obj1 != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,140, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mTitle =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,20, 360,80);
    assert( mTitle != NULL );

    mBarPane =
        new CBarPane(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,105, 350,20);
    assert( mBarPane != NULL );

// end JXLayout

	window->SetTitle("Waiting");
	SetButtons(mCancelBtn, mCancelBtn);

	mBarPane->SetIndeterminate(true);
	mBarPane->Start();
}

void CProgressDialog::SetDescriptor(const cdstring& title)
{
	mTitle->SetText(title);
}

void CProgressDialog::SetCount(unsigned long count) 
{
	mBarPane->SetCount(count);
}

void CProgressDialog::SetTotal(unsigned long total) 
{
	mBarPane->SetTotal(total);
}

void CProgressDialog::SetIndeterminate()
{
	mBarPane->SetIndeterminate(true);
}

CProgressDialog* CProgressDialog::StartDialog(const cdstring& rsrc)
{
	CProgressDialog* dlog = new CProgressDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->BeginDialog();

	JXApplication* app = JXGetApplication();
	app->DisplayInactiveCursor();

	return dlog;
}

void CProgressDialog::StopDialog(CProgressDialog* dlog)
{
	dlog->EndDialog(kDialogClosed_Cancel);
}
