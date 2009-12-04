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


// CEditGroupDialog.cpp : implementation file
//


#include "CEditGroupDialog.h"

#include "CGroup.h"
#include "CStringUtils.h"
#include "CTextField.h"
#include "CTextDisplay.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDialog dialog


CEditGroupDialog::CEditGroupDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDialog message handlers

bool comp_strnocase(const cdstring& str1, const cdstring& str2);
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

void CEditGroupDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 435,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 435,220);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Group Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,12, 80,20);
    assert( obj2 != NULL );

    mGroupName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 330,20);
    assert( mGroupName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Nick-Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,37, 80,20);
    assert( obj3 != NULL );

    mNickName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,35, 330,20);
    assert( mNickName != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Addresses:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,62, 70,20);
    assert( obj4 != NULL );

    mAddressList =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,60, 330,100);
    assert( mAddressList != NULL );

    mSortBtn =
        new JXTextButton("Sort", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,165, 50,20);
    assert( mSortBtn != NULL );
    mSortBtn->SetFontSize(10);

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,185, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 345,185, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Group Edit");
	SetButtons(mOKBtn, mCancelBtn);

	ListenTo(mSortBtn);
}

void CEditGroupDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mSortBtn)
		{
			OnGroupEditSort();
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

void CEditGroupDialog::OnGroupEditSort() 
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
	std::sort(list.begin(), list.end(), comp_strnocase);

	// Put back in field
	for(unsigned long i = 0; i < list.size(); i++)
	{
		mOriginalListText += list.at(i);
		mOriginalListText += os_endl;
	}

	mAddressList->SetText(mOriginalListText);
}

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

	txt = mAddressList->GetText();
	if (mOriginalListText != txt)
	{
		grp->GetAddressList().clear();
		char* s = txt.c_str_mod();
		s = ::strtok(s, "\n");
		while(s)
		{
			grp->GetAddressList().push_back(cdstring(s));

			s = ::strtok(nil, "\n");
		}
		done_edit = true;
	}

	return done_edit;
}

void CEditGroupDialog::SetFields(const CGroup* grp)
{
	mNickName->SetText(grp->GetNickName());

	mGroupName->SetText(grp->GetName());

	cdstring txt;
	for(unsigned long i = 0; i < ((CGroup*) grp)->GetAddressList().size(); i++)
	{
		txt += ((CGroup*) grp)->GetAddressList().at(i);
		txt += os_endl;
	}
	mAddressList->SetText(txt);
}

bool CEditGroupDialog::PoseDialog(CGroup* grp)
{
	bool result = false;

	CEditGroupDialog* dlog = new CEditGroupDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFields(grp);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		// Get the address
		dlog->GetFields(grp);

		result = true;
		dlog->Close();
	}

	return result;
}
