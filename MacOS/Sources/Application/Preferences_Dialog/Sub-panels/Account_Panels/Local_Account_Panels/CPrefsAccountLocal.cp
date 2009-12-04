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
#include "CTextFieldX.h"

#include <LPopupButton.h>
#include <LPushButton.h>
#include <LRadioButton.h>

#include <UStandardDialogs.h>

#include <unistd.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountLocal::CPrefsAccountLocal()
{
	mLocalAddress = false;
	mDisconnected = false;
}

// Constructor from stream
CPrefsAccountLocal::CPrefsAccountLocal(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
	mLocalAddress = false;
	mDisconnected = false;
}

// Default destructor
CPrefsAccountLocal::~CPrefsAccountLocal()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountLocal::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mDefaultPath = (LRadioButton*) FindPaneByID(paneid_DefaultPath);
	mUsePath = (LRadioButton*) FindPaneByID(paneid_UsePath);
	mChoose = (LPushButton*) FindPaneByID(paneid_ChooseLocalFolder);
	mPath = (CTextFieldX*) FindPaneByID(paneid_AccountLocalPath);
	mRelative = (LRadioButton*) FindPaneByID(paneid_RelativeLocalFolder);
	mAbsolute = (LRadioButton*) FindPaneByID(paneid_AbsoluteLocalFolder);

	mFileFormatPopup = (LPopupButton*) FindPaneByID(paneid_FileFormatPopup);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAccountLocalBtns);
}

// Handle buttons
void CPrefsAccountLocal::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_DefaultPath:
			if (*((long*) ioParam))
				SetUseLocal(false);
			break;
		case msg_UsePath:
			if (*((long*) ioParam))
				SetUseLocal(true);
			break;
		case msg_RelativeLocalFolder:
			if (*((long*) ioParam))
				SetRelative();
			break;
		case msg_AbsoluteLocalFolder:
			if (*((long*) ioParam))
				SetAbsolute();
			break;
		case msg_ChooseLocalFolder:
			DoChooseLocalFolder();
			break;
	}
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

	mDefaultPath->SetValue(account->GetCWD().GetName().empty());
	mUsePath->SetValue(!account->GetCWD().GetName().empty());
	SetUseLocal(!account->GetCWD().GetName().empty());
	
	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		mFileFormatPopup->SetValue(lacct->GetEndl() + 1);
	else
		mFileFormatPopup->Hide();
}

// Force update of prefs
void CPrefsAccountLocal::UpdateData(void* data)
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
	if (mUsePath->GetValue())
	{
		account->GetCWD().SetName(mPath->GetText());
	}
	else
		account->GetCWD().SetName(cdstring::null_str);

	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		lacct->SetEndl(static_cast<EEndl>(mFileFormatPopup->GetValue() - 1));

	if (mDisconnected)
		static_cast<CPreferences*>(data)->mDisconnectedCWD.SetValue(temp.GetCWD().GetName());
		
}

void CPrefsAccountLocal::SetUseLocal(bool use)
{
	if (use)
	{
		mPath->Enable();
		mChoose->Enable();
		mRelative->Enable();
		mAbsolute->Enable();
	}
	else
	{
		mPath->Disable();
		mChoose->Disable();
		mRelative->Disable();
		mAbsolute->Disable();
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
		StopListening();
		mRelative->SetValue(1);
		mAbsolute->SetValue(0);
		StartListening();

		cdstring temp = IsRelativePath(path) ? path : &path[cwd.length() - 1];
		mPath->SetText(temp);
	}
	else
	{
		// Set relative mode
		StopListening();
		mAbsolute->SetValue(1);
		mRelative->SetValue(0);
		StartListening();

		cdstring temp = path;
		mPath->SetText(temp);
	}
}

// Choose default folder
void CPrefsAccountLocal::DoChooseLocalFolder(void)
{
	// Get folder from user
	PPx::FSObject fspec;
	SInt32 dirID;
	if (PP_StandardDialogs::AskChooseFolder(fspec, dirID))
	{
		cdstring path(fspec.GetPath());
		if (!path.empty())
			SetPath(path);
	}
}

void CPrefsAccountLocal::SetRelative()
{
	// Get current path
	cdstring path = mPath->GetText();

	// Get CWD
	const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

	// Path must start with CWD
	if (!::strncmp(path, cwd, cwd.length()))
	{
		cdstring temp = &path.c_str()[cwd.length() - 1];
		mPath->SetText(temp);
		mAbsolute->SetValue(0);
	}
	else
	{
		// Stop relative mode
		StopListening();
		mRelative->SetValue(0);
		mAbsolute->SetValue(1);
		StartListening();
	}
}

void CPrefsAccountLocal::SetAbsolute()
{
	// Get current path
	cdstring path = mPath->GetText();

	// Get CWD
	const cdstring& cwd = CConnectionManager::sConnectionManager.GetCWD();

	// Path must be relative
	if (IsRelativePath(path))
	{
		cdstring temp(cwd);
		temp += &path.c_str()[1];
		mPath->SetText(temp);
		mRelative->SetValue(0);
	}
	else
	{
		// Stop absolute mode
		StopListening();
		mAbsolute->SetValue(0);
		mRelative->SetValue(1);
		StartListening();
	}
}
