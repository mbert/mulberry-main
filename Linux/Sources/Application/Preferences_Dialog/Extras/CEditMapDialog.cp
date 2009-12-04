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


// CEditMapDialog.cpp : implementation file
//


#include "CEditMapDialog.h"

#include "CMIMEMap.h"
#include "CTextField.h"

#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditMapDialog dialog


CEditMapDialog::CEditMapDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

void CEditMapDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 290,195, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 290,195);
    assert( obj1 != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 200,160, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 110,160, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj2 =
        new JXStaticText("MIME Type:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 90,20);
    assert( obj2 != NULL );

    mType =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,10, 160,20);
    assert( mType != NULL );

    JXStaticText* obj3 =
        new JXStaticText("MIME Subtype:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,37, 95,20);
    assert( obj3 != NULL );

    mSubtype =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,35, 160,20);
    assert( mSubtype != NULL );

    JXStaticText* obj4 =
        new JXStaticText("File Extension:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,62, 95,20);
    assert( obj4 != NULL );

    mSuffix =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,60, 60,20);
    assert( mSuffix != NULL );

    mAppLaunch =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,100, 250,40);
    assert( mAppLaunch != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Open File after Download:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,90, 155,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(1, "Always", mAppLaunch,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 70,20);
    assert( obj6 != NULL );

    JXTextRadioButton* obj7 =
        new JXTextRadioButton(2, "Ask", mAppLaunch,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,10, 70,20);
    assert( obj7 != NULL );

    JXTextRadioButton* obj8 =
        new JXTextRadioButton(3, "Never", mAppLaunch,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,10, 70,20);
    assert( obj8 != NULL );

// end JXLayout
	window->SetTitle("MIME Mapping");
	SetButtons(mOkBtn, mCancelBtn);
}

/////////////////////////////////////////////////////////////////////////////
// CEditMapDialog message handlers

void CEditMapDialog::SetMap(const CMIMEMap& aMap)
{
	mType->SetText(aMap.GetMIMEType());
	mSubtype->SetText(aMap.GetMIMESubtype());
	mSuffix->SetText(aMap.GetFileSuffix());
	mAppLaunch->SelectItem(aMap.GetAppLaunch() + 1);
}

void CEditMapDialog::GetMap(CMIMEMap& aMap)
{
	aMap.SetMIMEType(cdstring(mType->GetText()));
	aMap.SetMIMESubtype(cdstring(mSubtype->GetText()));
	aMap.SetFileSuffix(cdstring(mSuffix->GetText()));
	aMap.SetAppLaunch((EAppLaunch) (mAppLaunch->GetSelectedItem() - 1));
}

bool CEditMapDialog::PoseDialog(CMIMEMap& aMap)
{
	bool result = false;

	CEditMapDialog* dlog = new CEditMapDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetMap(aMap);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetMap(aMap);
		result = true;
		dlog->Close();
	}

	return result;
}
