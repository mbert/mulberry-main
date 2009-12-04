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


// Source for CIMAPLabelsDialog class

#include "CIMAPLabelsDialog.h"

#include "CCharSpecials.h"
#include "CErrorHandler.h"
#include "CStaticText.h"
#include "CTextField.h"

#include "cdstring.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIMAPLabelsDialog::CIMAPLabelsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CIMAPLabelsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 350,325, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,325);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Displayed Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 105,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("IMAP Label Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,10, 125,20);
    assert( obj3 != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 170,290, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 260,290, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

// end JXLayout
// begin JXLayout2

    mName[0] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,42, 95,20);
    assert( mName[0] != NULL );

    mLabel[0] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,40, 210,20);
    assert( mLabel[0] != NULL );

    mName[1] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,72, 95,20);
    assert( mName[1] != NULL );

    mLabel[1] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,70, 210,20);
    assert( mLabel[1] != NULL );

    mName[2] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,102, 95,20);
    assert( mName[2] != NULL );

    mLabel[2] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,100, 210,20);
    assert( mLabel[2] != NULL );

    mName[3] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,132, 95,20);
    assert( mName[3] != NULL );

    mLabel[3] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,130, 210,20);
    assert( mLabel[3] != NULL );

    mName[4] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,162, 95,20);
    assert( mName[4] != NULL );

    mLabel[4] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,160, 210,20);
    assert( mLabel[4] != NULL );

    mName[5] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,192, 95,20);
    assert( mName[5] != NULL );

    mLabel[5] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,190, 210,20);
    assert( mLabel[5] != NULL );

    mName[6] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,222, 95,20);
    assert( mName[6] != NULL );

    mLabel[6] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,220, 210,20);
    assert( mLabel[6] != NULL );

    mName[7] =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,252, 95,20);
    assert( mName[7] != NULL );

    mLabel[7] =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,250, 210,20);
    assert( mLabel[7] != NULL );

// end JXLayout2

	window->SetTitle("Change IMAP Label Names");
	SetButtons(mOkBtn, mCancelBtn);
}

void CIMAPLabelsDialog::SetDetails(const cdstrvect& names, const cdstrvect& labels)
{
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		mName[i]->SetText(names[i]);
		mLabel[i]->SetText(labels[i]);
	}
}

// Called during idle
bool CIMAPLabelsDialog::GetDetails(cdstrvect& labels)
{
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		labels[i] = mLabel[i]->GetText();
	}
	
	return true;
}

bool CIMAPLabelsDialog::ValidLabel(const cdstring& tlabel)
{
	// Cannot be empty
	if (tlabel.empty())
		return false;

	// Cannot start with backslash
	if (tlabel[0UL] == '\\')
		return false;
	
	// Must be an atom
	const char* p = tlabel.c_str();
	while(*p)
	{
		if (cINETChar[(unsigned char)*p] != 0)
			return false;
		p++;
	}
	
	return true;
}

JBoolean CIMAPLabelsDialog::OKToDeactivate()
{
	if (Cancelled())
	{
		return kTrue;
	}
	else
		return JBoolean(OnOK());
}

bool CIMAPLabelsDialog::OnOK()
{
	// Make sure all labels are valid
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		cdstring label = mLabel[i]->GetText();
		if (!ValidLabel(label))
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Preferences::InvalidIMAPLabel");
			return false;
		}
	}
	
	// Do OK action
	return true;
}

bool CIMAPLabelsDialog::PoseDialog(const cdstrvect& names, cdstrvect& labels)
{
	bool result = false;

	CIMAPLabelsDialog* dlog = new CIMAPLabelsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(names, labels);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(labels);
		result = true;
		dlog->Close();
	}

	return result;
}
