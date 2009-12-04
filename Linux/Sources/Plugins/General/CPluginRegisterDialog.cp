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

// CPluginRegisterDialog.cpp : implementation file
//

#include "CPluginRegisterDialog.h"

#include "CPlugin.h"
#include "CTextField.h"
#include "CUtils.h"
#include "CXStringResources.h"

#include "HResourceMap.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXColormap.h>
#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXImageWidget.h>

#include <jXGlobals.h>

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog dialog


CPluginRegisterDialog::CPluginRegisterDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog message handlers

// Get details of sub-panes
void CPluginRegisterDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 416,461, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 416,461);
    assert( obj1 != NULL );

    mLogo =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 260,100);
    assert( mLogo != NULL );

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 395,290);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Plug-in Registration", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,105, 130,20);
    assert( obj3 != NULL );
    const JFontStyle obj3_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj3->SetFontStyle(obj3_style);

    JXStaticText* obj4 =
        new JXStaticText("Name:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 40,15);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    mName =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,10, 270,20);
    assert( mName != NULL );
    mName->SetFontSize(10);
    mName->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj5 =
        new JXStaticText("Version:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 50,15);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);
    const JFontStyle obj5_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj5->SetFontStyle(obj5_style);

    mVersion =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,35, 270,20);
    assert( mVersion != NULL );
    mVersion->SetFontSize(10);
    mVersion->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj6 =
        new JXStaticText("Type:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,65, 40,15);
    assert( obj6 != NULL );
    obj6->SetFontSize(10);
    const JFontStyle obj6_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj6->SetFontStyle(obj6_style);

    mType =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,60, 270,20);
    assert( mType != NULL );
    mType->SetFontSize(10);
    mType->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj7 =
        new JXStaticText("Manufacturer:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 80,15);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);
    const JFontStyle obj7_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj7->SetFontStyle(obj7_style);

    mManufacturer =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,85, 270,20);
    assert( mManufacturer != NULL );
    mManufacturer->SetFontSize(10);
    mManufacturer->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj8 =
        new JXStaticText("Description:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 70,15);
    assert( obj8 != NULL );
    obj8->SetFontSize(10);
    const JFontStyle obj8_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj8->SetFontStyle(obj8_style);

    mDescription =
        new JXStaticText("", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,110, 270,145);
    assert( mDescription != NULL );
    mDescription->SetFontSize(10);
    mDescription->SetBorderWidth(kJXDefaultBorderWidth);

    JXStaticText* obj9 =
        new JXStaticText("Registration Key:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,264, 110,20);
    assert( obj9 != NULL );
    const JFontStyle obj9_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj9->SetFontStyle(obj9_style);

    mRegKey =
        new CTextInputField(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,262, 150,20);
    assert( mRegKey != NULL );

    mRegisterBtn =
        new JXTextButton("Plugin Register", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,420, 110,25);
    assert( mRegisterBtn != NULL );
    mRegisterBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 210,420, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mRemoveBtn =
        new JXTextButton("Remove Plugin", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 170,420, 110,25);
    assert( mRemoveBtn != NULL );

    mDemoBtn =
        new JXTextButton("Run as Demo", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,420, 95,25);
    assert( mDemoBtn != NULL );

// end JXLayout

	window->SetTitle("Plugin Registration");
	mLogo->SetImage(bmpFromResource(IDB_LOGO, mLogo), kTrue);
	SetButtons(mRegisterBtn, mCancelBtn);
	mRegisterBtn->Deactivate();

	// Hide registration button if already registered
	mRegKey->ShouldBroadcastAllTextChanged(kTrue);
	ListenTo(mRegKey);
	ListenTo(mRemoveBtn);
	ListenTo(mDemoBtn);
}

void CPluginRegisterDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == mRegKey) && message.Is(JTextEditor16::kTextChanged))
	{
		if (mRegKey->GetTextLength())
			mRegisterBtn->Activate();
		else
			mRegisterBtn->Deactivate();
		return;
	}
	else if ((sender == mRemoveBtn) && message.Is(JXButton::kPushed))
	{
		EndDialog(kDialogClosed_Btn3);
		return;
	}
	else if ((sender == mDemoBtn) && message.Is(JXButton::kPushed))
	{
		EndDialog(kDialogClosed_Btn4);
		return;
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

void CPluginRegisterDialog::SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete)
{
	mName->SetText(plugin.GetName());
	mVersion->SetText(::GetVersionText(plugin.GetVersion()));
	mType->SetText(rsrc::GetIndexedString("Alerts::Plugins::Type", plugin.GetType()));
	mManufacturer->SetText(plugin.GetManufacturer());
	mDescription->SetText(plugin.GetDescription());

	if (!allow_delete)
	{
		mRemoveBtn->Deactivate();
		mRemoveBtn->Hide();
	}
	else
	{
		mCancelBtn->Deactivate();
		mCancelBtn->Hide();
	}

	if (!allow_demo)
	{
		mDemoBtn->Deactivate();
		mDemoBtn->Hide();
	}
}

int CPluginRegisterDialog::PoseDialog(const CPlugin& plugin, bool allow_demo, bool allow_delete, cdstring& regkey)
{
	CPluginRegisterDialog* dlog = new CPluginRegisterDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetPlugin(plugin, allow_demo, allow_delete);

	// Let Dialog process events
	switch(dlog->DoModal(false))
	{
	case kDialogClosed_OK:
		regkey = dlog->mRegKey->GetText();
		dlog->Close();
		return eRegister;
	case kDialogClosed_Cancel:
	default:
		return eCancel;
	case kDialogClosed_Btn3:
		dlog->Close();
		return eRemove;
	case kDialogClosed_Btn4:
		dlog->Close();
		return eDemo;
	}
}
