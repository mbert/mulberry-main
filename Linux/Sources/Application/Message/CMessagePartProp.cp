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


// CMessagePartProp.cp : implementation file
//


#include "CMessagePartProp.h"

#include "CAttachment.h"
#include "CMIMESupport.h"
#include "CStaticText.h"
#include "CTextDisplay.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp dialog

CMessagePartProp::CMessagePartProp(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp message handlers

void CMessagePartProp::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 405,295, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 405,295);
    assert( obj1 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 315,260, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel All", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 215,260, 80,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 50,20);
    assert( obj2 != NULL );

    mName =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 305,20);
    assert( mName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Type:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,37, 50,20);
    assert( obj3 != NULL );

    mType =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,35, 305,20);
    assert( mType != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Encoding:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,62, 66,20);
    assert( obj4 != NULL );

    mEncoding =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,60, 305,20);
    assert( mEncoding != NULL );

    JXStaticText* obj5 =
        new JXStaticText("ID:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,87, 70,20);
    assert( obj5 != NULL );

    mID =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,85, 305,20);
    assert( mID != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Disposition:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,112, 72,20);
    assert( obj6 != NULL );

    mDisposition =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,110, 305,20);
    assert( mDisposition != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Description:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,137, 73,20);
    assert( obj7 != NULL );

    mDescription =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,135, 305,40);
    assert( mDescription != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Parameters:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,180, 73,20);
    assert( obj8 != NULL );

    mParams =
        new CTextDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,180, 305,40);
    assert( mParams != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Open with:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,227, 72,20);
    assert( obj9 != NULL );

    mOpenWith =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,225, 305,20);
    assert( mOpenWith != NULL );

// end JXLayout

	window->SetTitle("Message Part Properties");
	SetButtons(mOKBtn, mCancelBtn);

	mName->SetBorderWidth(kJXDefaultBorderWidth);
	mType->SetBorderWidth(kJXDefaultBorderWidth);
	mEncoding->SetBorderWidth(kJXDefaultBorderWidth);
	mID->SetBorderWidth(kJXDefaultBorderWidth);
	mDisposition->SetBorderWidth(kJXDefaultBorderWidth);
	mDescription->SetBorderWidth(kJXDefaultBorderWidth);
	mParams->SetBorderWidth(kJXDefaultBorderWidth);
	mOpenWith->SetBorderWidth(kJXDefaultBorderWidth);
}

void CMessagePartProp::SetFields(const CAttachment& attach, bool multi)
{
	const CMIMEContent& content = attach.GetContent();

	mName->SetText(attach.GetMappedName(true, true));
	mType->SetText(content.GetContentTypeText() + "/" + content.GetContentSubtypeText());
	mEncoding->SetText(content.GetTransferEncodingText());
	mID->SetText(content.GetContentId());
	mDisposition->SetText(content.GetContentDispositionText());
	mDescription->SetText(content.GetContentDescription());
	cdstring params;
	for(int i = 0; i < content.CountContentParameters(); i++)
	{
		params += content.GetContentParameterText(i);
		params += "\n";
	}
	mParams->SetText(params);
	mOpenWith->SetText(CMIMESupport::MapMIMEToApp(attach));

	if (!multi)
	{
		mCancelBtn->Hide();

		// Move OK to center
		mOKBtn->CenterWithinEnclosure(kTrue, kFalse);
	}
}

bool CMessagePartProp::PoseDialog(const CAttachment& attach, bool multi)
{
	// Create the dialog and give it the message
	CMessagePartProp* dlog = new CMessagePartProp(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFields(attach, multi);
	
	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->Close();
		return true;
	}

	return false;
}
