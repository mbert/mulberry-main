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


// Source for CPrefsAccountLocal class

#include "CPrefsAccountLocal.h"

#include "CConnectionManager.h"
#include "CLocalCommon.h"
#include "CMailAccount.h"
#include "CPreferences.h"
#include "CTextDisplay.h"

#include "TPopupMenu.h"

#include <JXChooseSaveFile.h>
#include <JXDownRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountLocal::CPrefsAccountLocal(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mLocalAddress = false;
	mDisconnected = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountLocal::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 340,117);
    assert( obj1 != NULL );

    mLocationGroup =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,10, 150,40);
    assert( mLocationGroup != NULL );
    mLocationGroup->SetBorderWidth(0);

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "Use Default Location", mLocationGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 150,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Specify Location:", mLocationGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 150,20);
    assert( obj3 != NULL );

    mPath =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 320,45);
    assert( mPath != NULL );

    mRelativeGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,65, 145,40);
    assert( mRelativeGroup != NULL );
    mRelativeGroup->SetBorderWidth(0);

    JXTextRadioButton* obj4 =
        new JXTextRadioButton(1, "Relative Path", mRelativeGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 145,20);
    assert( obj4 != NULL );

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(2, "Absolute Path", mRelativeGroup,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 145,20);
    assert( obj5 != NULL );

    mChooseBtn =
        new JXTextButton("Choose...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,65, 80,25);
    assert( mChooseBtn != NULL );

    mFileFormatTitle =
        new JXStaticText("File Format:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,173, 80,20);
    assert( mFileFormatTitle != NULL );

    mFileFormatPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,170, 150,20);
    assert( mFileFormatPopup != NULL );

// end JXLayout1

	mFileFormatPopup->SetMenuItems( "Automatic %r |"
									"Mac OS %r |"
									"Unix %r |"
									"Windows %r");
	mFileFormatPopup->SetUpdateAction(JXMenu::kDisableNone);

	// Start listening
	ListenTo(mLocationGroup);
	ListenTo(mRelativeGroup);
	ListenTo(mChooseBtn);
}

// Handle buttons
void CPrefsAccountLocal::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mLocationGroup)
		{
			SetUseLocal(mLocationGroup->GetSelectedItem() == 2);
			return;
		}
		else if (sender == mRelativeGroup)
		{
			if (mRelativeGroup->GetSelectedItem() == 1)
				SetRelative();
			else
				SetAbsolute();
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mChooseBtn)
		{
			DoChooseLocalFolder();
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAccountLocal::SetData(void* data)
{
	CINETAccount* account;
	CINETAccount temp;
	if (mLocalAddress)
		account = &static_cast<CPreferences*>(data)->mLocalAdbkAccount.Value();
	else if (mDisconnected)
	{
		temp.GetCWD().SetName(static_cast<CPreferences*>(data)->mDisconnectedCWD.GetValue());
		account = &temp;
	}
	else
		account = static_cast<CINETAccount*>(data);

	SetPath(account->GetCWD().GetName());

	mLocationGroup->SelectItem(account->GetCWD().GetName().empty() ? 1 : 2);
	SetUseLocal(!account->GetCWD().GetName().empty());
	
	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		mFileFormatPopup->SetValue(lacct->GetEndl() + 1);
	else
	{
		mFileFormatTitle->Hide();
		mFileFormatPopup->Hide();
	}
}

// Force update of prefs
bool CPrefsAccountLocal::UpdateData(void* data)
{
	CINETAccount* account;
	CINETAccount temp;
	if (mLocalAddress)
		account = &static_cast<CPreferences*>(data)->mLocalAdbkAccount.Value();
	else if (mDisconnected)
		account = &temp;
	else
		account = static_cast<CINETAccount*>(data);

	// Copy info from panel into prefs
	if (mLocationGroup->GetSelectedItem() == 2)
		account->GetCWD().SetName(cdstring(mPath->GetText()));
	else
		account->GetCWD().SetName(cdstring::null_str);

	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		lacct->SetEndl(static_cast<EEndl>(mFileFormatPopup->GetValue() - 1));

	if (mDisconnected)
		static_cast<CPreferences*>(data)->mDisconnectedCWD.SetValue(temp.GetCWD().GetName());

	return true;
}

void CPrefsAccountLocal::SetUseLocal(bool use)
{
	if (use)
	{
		mPath->Activate();
		mChooseBtn->Activate();
		mRelativeGroup->Activate();
	}
	else
	{
		mPath->Deactivate();
		mChooseBtn->Deactivate();
		mRelativeGroup->Deactivate();
	}
}

void CPrefsAccountLocal::SetPath(const char* path)
{
	// Get CWD
	const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

	// Does path start with this?
	if (IsRelativePath(path) || !::strncmp(path, cwd, cwd.length()))
	{
		// Set relative mode
		SetListening(kFalse);
		mRelativeGroup->SelectItem(1);
		SetListening(kTrue);

		cdstring copyStr = IsRelativePath(path) ? path : &path[cwd.length() - 1];
		mPath->SetText(copyStr);
	}
	else
	{
		// Set relative mode
		SetListening(kFalse);
		mRelativeGroup->SelectItem(2);
		SetListening(kTrue);

		cdstring copyStr = path;
		mPath->SetText(copyStr);
	}
}

// Choose default folder
void CPrefsAccountLocal::DoChooseLocalFolder(void)
{
	// Get folder from user
	JString pname;
	if (JXGetChooseSaveFile()->ChooseRWPath("Choose a directory:", NULL, mPath->GetText(), &pname))
		SetPath(pname);
}

void CPrefsAccountLocal::SetRelative()
{
	// Get current path
	cdstring path(mPath->GetText());

	// Get CWD
	const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

	// Path must start with CWD
	if (!::strncmp(path, cwd, cwd.length()))
	{
		cdstring copyStr = &path.c_str()[cwd.length() - 1];
		mPath->SetText(copyStr);
		SetListening(kFalse);
		mRelativeGroup->SelectItem(1);
		SetListening(kTrue);
	}
	else
	{
		// Stop relative mode
		SetListening(kFalse);
		mRelativeGroup->SelectItem(2);
		SetListening(kTrue);
	}
}

void CPrefsAccountLocal::SetAbsolute()
{
	// Get current path
	cdstring path(mPath->GetText());

	// Get CWD
	const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

	// Path must be relative
	if (IsRelativePath(path))
	{
		cdstring copyStr = cwd;
		copyStr += &path.c_str()[1];
		mPath->SetText(copyStr);
		SetListening(kFalse);
		mRelativeGroup->SelectItem(2);
		SetListening(kTrue);
	}
	else
	{
		// Stop absolute mode
		SetListening(kFalse);
		mRelativeGroup->SelectItem(1);
		SetListening(kTrue);
	}
}
