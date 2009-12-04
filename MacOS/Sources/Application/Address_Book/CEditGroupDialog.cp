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


// Source for CEditGroupDialog class

#include "CEditGroupDialog.h"

#include "CGroup.h"
#include "CHelpAttach.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CStringUtils.h"

#include "cdstring.h"

// __________________________________________________________________________________________________
// C L A S S __ C E D I T G R O U P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditGroupDialog::CEditGroupDialog()
{
}

// Constructor from stream
CEditGroupDialog::CEditGroupDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CEditGroupDialog::~CEditGroupDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditGroupDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Do nick-name
	mNickName = (CTextFieldX*) FindPaneByID(paneid_EditGroupNickName);

	// Do group name
	mGroupName = (CTextFieldX*) FindPaneByID(paneid_EditGroupGroupName);

	// Do addresses
	mAddresses = (CTextDisplay*) FindPaneByID(paneid_EditGroupAddresses);

	// Make text edit field active
	SetLatentSub(mGroupName);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CEditGroupDialogBtns);
}

// Handle buttons
void CEditGroupDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {
		case msg_EditGroupSort:

			// Sort address field
			SortAddresses();
			break;

	}
}

// Set fields in dialog
void CEditGroupDialog::SetFields(const CGroup* grp)
{
	mNickName->SetText(grp->GetNickName());

	mGroupName->SetText(grp->GetName());

	cdstring addrs;
	for(cdstrvect::const_iterator iter = grp->GetAddressList().begin(); iter != grp->GetAddressList().end(); iter++)
	{
		addrs += *iter;
		addrs += os_endl;
	}
	mAddresses->SetText(addrs);

	mNickName->SelectAll();
}

// Get fields from dialog
bool CEditGroupDialog::GetFields(CGroup* grp)
{
	bool done_edit = false;

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring txt = mNickName->GetText();
	::strreplace(txt.c_str_mod(), "@", '*');
	txt.trimspace();

	if (grp->GetNickName() != txt)
	{
		grp->GetNickName() = txt;
		done_edit = true;
	}

	txt = mGroupName->GetText();
	if (grp->GetName() != txt)
	{
		grp->GetName() = txt;
		done_edit = true;
	}

	// Only copy text if dirty
	if (mAddresses->IsDirty())
	{
		// Delete all existing addresses
		grp->GetAddressList().clear();

		// Copy handle to text with null terminator
		cdstring txt;
		mAddresses->GetText(txt);

		char* s = ::strtok(txt.c_str_mod(), CR);
		while(s)
		{
			cdstring copyStr(s);
			grp->GetAddressList().push_back(copyStr);

			s = ::strtok(NULL, CR);
		}

		done_edit = true;
	}

	return done_edit;
}

static bool comp_strnocase(const cdstring& str1, const cdstring& str2);
bool comp_strnocase(const cdstring& str1, const cdstring& str2)
{
	// Compare without case
	int result = ::strcmpnocase(str1, str2);

	// If equal compare with case
	if (!result)
		result = ::strcmp(str1, str2);

	// Return true if str1 < str2
	return (result < 0);
}

void CEditGroupDialog::SortAddresses(void)
{
	cdstrvect list;

	// Copy handle to text with null terminator
	cdstring txt;
	mAddresses->GetText(txt);

	// Parse into list
	char* s = ::strtok(txt.c_str_mod(), CR);
	while(s)
	{
		cdstring copyStr(s);
		list.push_back(copyStr);

		s = ::strtok(NULL, CR);
	}

	// Sort list
	std::sort(list.begin(), list.end(), comp_strnocase);

	// Put back in field
	cdstring addrs;
	for(cdstrvect::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		addrs += *iter;
		addrs += os_endl;
	}
	mAddresses->SetText(addrs);
	mAddresses->SetDirty(true);
}
