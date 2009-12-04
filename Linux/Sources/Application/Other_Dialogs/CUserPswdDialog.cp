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


// CUserPswdDialog.cpp : implementation file
//


#include "CUserPswdDialog.h"

#include "CIconLoader.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextField.h"
#include "CXStringResources.h"

#include <JXInputField.h>
#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXTextButton.h>
#include <JXStaticText.h>
#include <JXImageWidget.h>
#include <JXDisplay.h>
#include <JXPasswordInput.h>
#include "CInputField.h"

#include "jXGlobals.h"

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog dialog


CUserPswdDialog::CUserPswdDialog(JXDirector* supervisor)
  : CDialogDirector(supervisor)
{
	mCapsLockDown = false;
}


/////////////////////////////////////////////////////////////////////////////
// CUserPswdDialog message handlers

void CUserPswdDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 355,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 355,220);
    assert( obj1 != NULL );

    JXTextButton* okButton =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,185, 70,25);
    assert( okButton != NULL );
    okButton->SetShortcuts("^M");

    JXTextButton* cancelButton =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 175,185, 70,25);
    assert( cancelButton != NULL );
    cancelButton->SetShortcuts("^[");

    mIcon =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 32,32);
    assert( mIcon != NULL );

    mDescription =
        new CStaticText("Login to Server:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 50,10, 280,20);
    assert( mDescription != NULL );

    mServerName =
        new CStaticText("Server name", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 50,30, 280,20);
    assert( mServerName != NULL );

    JXStaticText* obj2 =
        new JXStaticText("User ID:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,65, 60,20);
    assert( obj2 != NULL );

    mUserID =
        new CTextInputField(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,65, 200,20);
    assert( mUserID != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Password:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,95, 70,20);
    assert( obj3 != NULL );

    mPassword =
        new CInputField<JXPasswordInput>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,95, 200,20);
    assert( mPassword != NULL );

    mCapsLock =
        new CStaticText("Caps Lock\nis Down", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 290,75, 60,30);
    assert( mCapsLock != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Method:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,130, 70,20);
    assert( obj4 != NULL );

    mMethod =
        new CStaticText("method", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,130, 250,20);
    assert( mMethod != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Connection:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 10,150, 80,20);
    assert( obj5 != NULL );

    mSecure =
        new CStaticText("connection", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 90,150, 250,20);
    assert( mSecure != NULL );

// end JXLayout

	window->SetTitle("Set User ID & Password");
	SetButtons(okButton, cancelButton);

	mCapsLock->SetBreakCROnly(false);

	JXKeyModifiers modifiers(GetDisplay());
	GetDisplay()->GetCurrentButtonKeyState(NULL, &modifiers);
	mCapsLockDown = modifiers.shiftLock();
	if (!mCapsLockDown)
		mCapsLock->Hide();
}

// Watch state of capslock
void CUserPswdDialog::Continue()
{
	if (mCapsLockDown != GetDisplay()->GetLatestKeyModifiers().shiftLock())
	{
		mCapsLockDown = !mCapsLockDown;
		if (mCapsLockDown)
			mCapsLock->Show();
		else
			mCapsLock->Hide();
	}
}

bool CUserPswdDialog::PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
									const cdstring& title, const cdstring& server_ip,
									const cdstring& method, bool secure, int icon)
{
	bool result = false;
	CUserPswdDialog* dlog = new CUserPswdDialog(JXGetApplication());
	dlog->OnCreate();

	dlog->mUserID->SetText(uid);
	if (save_user)
		dlog->mUserID->Deactivate();
	dlog->mPassword->SetText(pswd);
	if (save_pswd && !pswd.empty())
		dlog->mPassword->Deactivate();
	dlog->mDescription->SetText(title);
	dlog->mServerName->SetText(server_ip);
	dlog->mMethod->SetText(method);
	dlog->mSecure->SetText(rsrc::GetString(secure ? "UI::UserPswd::Secure" : "UI::UserPswd::Insecure"));
	dlog->mIcon->SetImage(CIconLoader::GetIcon(icon, dlog->mIcon, 32, 0x00CCCCCC), kFalse);

	// Set appropriate initial focus
	if (!(!save_user && (uid.empty() || save_pswd)))
	{
		// Horrible: we have to reset the order of the focus widgets in the window
		// as JX does not let us change the intial focus when the widgets are not visible
		dlog->GetWindow()->UnregisterFocusWidget(dlog->mUserID);
		dlog->GetWindow()->RegisterFocusWidget(dlog->mUserID);
	}

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		uid = dlog->mUserID->GetText();
		pswd = dlog->mPassword->GetText();

		dlog->Close();
		result = true;
	}

	return result;
}
