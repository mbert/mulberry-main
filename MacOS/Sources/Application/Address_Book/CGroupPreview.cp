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


// Source for CGroupPreview class

#include "CGroupPreview.h"

#include "CAddressBook.h"
#include "CCommands.h"
#include "CGroup.h"
#include "CStaticText.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include "cdstring.h"

#include "MyCFString.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupPreview::CGroupPreview()
{
	mGroup = NULL;
}

// Constructor from stream
CGroupPreview::CGroupPreview(LStream *inStream)
		: CAddressPreviewBase(inStream)
{
	mGroup = NULL;
}

// Default destructor
CGroupPreview::~CGroupPreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CGroupPreview::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPreviewBase::FinishCreateSelf();

	// See if we are a child of a 3-pane
	LView* super = GetSuperView();
	while(super && !dynamic_cast<C3PaneWindow*>(super))
		super = super->GetSuperView();
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(super);

	// Get items
	mDescriptor = (CStaticText*) FindPaneByID(paneid_GroupPreviewDesc);
	mGroupName = (CTextFieldX*) FindPaneByID(paneid_GroupPreviewGroupName);
	mNickName = (CTextFieldX*) FindPaneByID(paneid_GroupPreviewNickName);
	mAddresses = (CTextDisplay*) FindPaneByID(paneid_GroupPreviewAddresses);

	// Hide close/zoom if not 3pane
	if (!m3PaneWindow)
	{
		FindPaneByID(paneid_GroupPreviewZoom)->Hide();
	}
	else
	{
		CToolbarButton* btn = (CToolbarButton*) FindPaneByID(paneid_GroupPreviewZoom);
		btn->AddListener(this);
		btn->SetSmallIcon(true);
		btn->SetShowIcon(true);
		btn->SetShowCaption(false);
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());
}

// Respond to clicks in the icon buttons
void CGroupPreview::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_GroupPreviewSort:
		// Sort address field
		SortAddresses();
		break;
	case msg_GroupPreviewZoom:
		OnZoom();
		break;
	}
}

void CGroupPreview::OnZoom()
{
	m3PaneWindow->ObeyCommand(cmd_ToolbarZoomPreview, NULL);
}

void CGroupPreview::Close(void)
{
	// If it exits and its changed, update it
	if (mAdbk && mGroup && GetFields(mGroup))
	{
		// Need to set group to NULL bfore the update as the update
		// will generate cause a refresh and make this close again
		CGroup* temp = mGroup;
		mGroup = NULL;
		mAdbk->UpdateGroup(temp, true);
	}
}

void CGroupPreview::Focus()
{
	LCommander::SwitchTarget(mGroupName);
}

void CGroupPreview::SetGroup(CGroup* grp)
{
	mGroup = grp;
	cdstring title;
	if (grp)
		title = grp->GetName();
	MyCFString temp(title, kCFStringEncodingUTF8);
	mDescriptor->SetCFDescriptor(temp);

	// Set the data in the panel
	SetFields(grp);
}

// Address removed by someone else
void CGroupPreview::ClearGroup()
{
	mGroup = NULL;
	SetGroup(NULL);
}

// Set fields in dialog
void CGroupPreview::SetFields(const CGroup* grp)
{
	cdstring txt;
	
	if (grp)
		txt = grp->GetName();
	mGroupName->SetText(txt);

	if (grp)
		txt = grp->GetNickName();
	mNickName->SetText(txt);

	cdstring addrs;
	if (grp)
	{
		for(cdstrvect::const_iterator iter = grp->GetAddressList().begin();
				iter != grp->GetAddressList().end(); iter++)
		{
			addrs += *iter;
			addrs += os_endl;
		}
	}
	mAddresses->SetText(addrs);

	mNickName->SelectAll();
}

// Get fields from dialog
bool CGroupPreview::GetFields(CGroup* grp)
{
	// Only for valid group
	if (!grp)
		return false;

	bool done_edit = false;

	cdstring txt = mGroupName->GetText();
	if (grp->GetName() != txt)
	{
		grp->GetName() = txt;
		done_edit = true;
	}

	txt = mNickName->GetText();

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(txt);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	if (grp->GetNickName() != nickname)
	{
		grp->GetNickName() = nickname;
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

void CGroupPreview::SortAddresses(void)
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
