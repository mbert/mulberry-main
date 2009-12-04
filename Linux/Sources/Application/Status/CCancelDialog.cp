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


// Source for CCancelDialog class

#include "CCancelDialog.h"

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
CCancelDialog::CCancelDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
	mLastSecs = 0;
}

// Default destructor
CCancelDialog::~CCancelDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CCancelDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 390,195, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 390,195);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Waiting for Server Response:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,20, 175,20);
    assert( obj2 != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,160, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mTime =
        new JXStaticText("0", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 195,20, 40,20);
    assert( mTime != NULL );

    JXStaticText* obj3 =
        new JXStaticText("secs.", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,20, 40,20);
    assert( obj3 != NULL );

    mDesc =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,40, 360,80);
    assert( mDesc != NULL );

    mBar =
        new CProgressBar(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,125, 350,20);
    assert( mBar != NULL );

// end JXLayout

	window->SetTitle("Waiting");
	SetButtons(mCancelBtn, mCancelBtn);

	mDesc->SetBreakCROnly(false);

	mBar->SetIndeterminate(true);
	mBar->Start();
}

// Set descriptor text
void CCancelDialog::SetBusyDescriptor(const cdstring& desc)
{
	mDesc->SetText(desc);
}

// Called during idle
void CCancelDialog::SetTime(unsigned long secs)
{
	// Only if different
	if (mLastSecs != secs)
	{
		cdstring txt(secs);
		mTime->SetText(txt);
		mLastSecs = secs;
	}
	
	// Always ping the progress bar as this dialog will run with idlers turned
	// off so the pbar won't update
	mBar->SpendTime();
}
