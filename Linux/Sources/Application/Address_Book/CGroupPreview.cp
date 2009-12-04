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
#include "CIconLoader.h"
#include "CStaticText.h"
#include "CStringUtils.h"
#include "CTextField.h"
#include "CTextDisplay.h"
#include "CToolbarButton.h"
#include "C3PaneWindow.h"

#include "cdstring.h"

#include <JXColormap.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>

#include <algorithm>
#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupPreview::CGroupPreview(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
					: CAddressPreviewBase(enclosure, hSizing, vSizing, x, y, w, h)
{
	mGroup = NULL;
}

// Default destructor
CGroupPreview::~CGroupPreview()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CGroupPreview::OnCreate()
{
// begin JXLayout1

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 470,24);
    assert( obj1 != NULL );

    JXImageWidget* icon =
        new JXImageWidget(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,2, 16,16);
    assert( icon != NULL );

    mDescriptor =
        new CStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,2, 440,20);
    assert( mDescriptor != NULL );

    mZoomBtn =
        new CToolbarButton("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 475,2, 20,20);
    assert( mZoomBtn != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Group Name:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,37, 80,20);
    assert( obj2 != NULL );

    mGroupName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,35, 395,20);
    assert( mGroupName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Nick-Name:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,62, 80,20);
    assert( obj3 != NULL );

    mNickName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,60, 395,20);
    assert( mNickName != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Addresses:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,87, 70,20);
    assert( obj4 != NULL );

    mAddressList =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,85, 395,180);
    assert( mAddressList != NULL );

    mSortBtn =
        new JXTextButton("Sort", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,270, 50,20);
    assert( mSortBtn != NULL );
    mSortBtn->SetFontSize(10);

// end JXLayout1

	icon->SetImage(CIconLoader::GetIcon(IDI_3PANEPREVIEWGROUP, icon, 16, 0x00CCCCCC), kFalse);

	// See if we are a child of a 3-pane
	m3PaneWindow = dynamic_cast<C3PaneWindow*>(GetWindow()->GetDirector());

	// Hide close/zoom if not 3pane
	if (!m3PaneWindow)
	{
		mZoomBtn->Hide();
	}
	else
	{
		ListenTo(mZoomBtn);
		mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
		mZoomBtn->SetSmallIcon(true);
		mZoomBtn->SetShowIcon(true);
		mZoomBtn->SetShowCaption(false);
	}

	ListenTo(mSortBtn);
}

// Respond to clicks in the icon buttons
void CGroupPreview::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoom();
			return;
		}
		else if (sender == mSortBtn)
		{
			// Sort address field
			SortAddresses();
			return;
		}
	}
	
	CAddressPreviewBase::Receive(sender, message);
}

void CGroupPreview::OnZoom()
{
	m3PaneWindow->ObeyCommand(CCommand::eToolbarZoomPreview, NULL);
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
	mGroupName->Focus();
}

void CGroupPreview::SetGroup(CGroup* grp)
{
	mGroup = grp;
	cdstring title;
	if (grp)
		title = grp->GetName();
	mDescriptor->SetText(title);

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

	if (grp)
	{
		txt = cdstring::null_str;
		for(unsigned long i = 0; i < ((CGroup*) grp)->GetAddressList().size(); i++)
		{
			txt += ((CGroup*) grp)->GetAddressList().at(i);
			txt += os_endl;
		}
	}
	mAddressList->SetText(txt);
}

// Get fields from dialog
bool CGroupPreview::GetFields(CGroup* grp)
{
	// Only for valid group
	if (!grp)
		return false;

	cdstring txt;
	bool done_edit = false;

	txt = mGroupName->GetText();
	if (grp->GetName() != txt)
	{
		grp->GetName() = txt;
		done_edit = true;
	}

	// Nick-name cannot contain '@' and no spaces surrounding it
	txt = mNickName->GetText();
	::strreplace(txt.c_str_mod(), "@", '*');
	txt.trimspace();

	if (grp->GetNickName() != txt)
	{
		grp->GetNickName() = txt;
		done_edit = true;
	}

	// Only copy text if dirty
	cdstrvect tmp;
	// Copy handle to text with null terminator
	txt = mAddressList->GetText();

	char* s = ::strtok(txt.c_str_mod(), "\n");
	while(s)
	{
		cdstring copyStr(s);
		tmp.push_back(copyStr);

		s = ::strtok(NULL, "\n");
	}
	if (grp->GetAddressList() != tmp)
	{
		grp->GetAddressList().clear();
		grp->GetAddressList() = tmp;
		done_edit = true;
	}

	return done_edit;
}

bool comp_strnocase_1(const cdstring& str1, const cdstring& str2);
bool comp_strnocase_1(const cdstring& str1, const cdstring& str2)
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
	
	// Parse into list
	cdstring txt = mAddressList->GetText();
	char* s = txt.c_str_mod();
	s = ::strtok(s, "\n");
	while(s)
	{
		cdstring copyStr(s);
		list.push_back(copyStr);
		
		s = ::strtok(NULL, "\n");
	}

	// Sort list
	std::sort(list.begin(), list.end(), comp_strnocase_1);

	// Put back in field
	txt = cdstring::null_str;
	for(unsigned long i = 0; i < list.size(); i++)
	{
		txt += list.at(i);
		txt += os_endl;
	}

	mAddressList->SetText(txt);
}
