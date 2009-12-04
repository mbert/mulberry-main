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
#include "CGroup.h"
#include "CMulberryApp.h"
#include "CStringUtils.h"
#include "CUnicodeUtils.h"
#include "C3PaneWindow.h"

#include "cdstring.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupPreview::CGroupPreview()
{
	mGroup = NULL;
}

// Default destructor
CGroupPreview::~CGroupPreview()
{
}

void CGroupPreview::DoDataExchange(CDataExchange* pDX)
{
	CAddressPreviewBase::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupPreview)
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_GROUPNAME, mGroupName);
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_NICKNAME, mNickName);
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_LIST, mAddressList);
	DDX_Control(pDX, IDC_GROUPEDIT_LIST, mAddressListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGroupPreview, CAddressPreviewBase)
	//{{AFX_MSG_MAP(CGroupPreview)
	ON_BN_CLICKED(IDC_GROUPEDIT_SORT, OnGroupEditSort)

	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnZoomPane)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

void CGroupPreview::InitDialog()
{
	CAddressPreviewBase::InitDialog();

	// Get window that owns this one
	CWnd* super = GetParent();
	while(super && !dynamic_cast<CView*>(super))
		super = super->GetParent();

	bool is3Pane = (super == C3PaneWindow::s3PaneWindow);

	const int cSmallIconBtnSize = 20;
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	CRect crect;
	GetClientRect(crect);
	int width = crect.Width();
	int height = crect.Height();

	// Header with alignment
	const int cCaptionHeight = 16;
	const int cHeaderHeight = cCaptionHeight + 8;
	const int cBtnVertOffset = (cHeaderHeight - cSmallIconBtnSize) / 2;
	mHeader.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width - cSmallIconBtnSize - 8, cHeaderHeight), this, IDC_STATIC);
	mHeader.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	AddAlignment(new CWndAlignment(&mHeader, CWndAlignment::eAlign_TopWidth));

	// Icon and descriptor
	mIcon.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(0, 0, 16, 16), &mHeader, IDC_STATIC);
	mIcon.SetIconID(IDI_3PANEPREVIEWGROUP);
	CRect rect;
	mHeader.GetClientRect(rect);
	rect.left += 24;
	mDescriptor.Create(NULL, WS_CHILD | WS_VISIBLE, rect, &mHeader, IDC_STATIC);
	mHeader.AddAlignment(new CWndAlignment(&mDescriptor, CWndAlignment::eAlign_WidthHeight));

	CString s;
	mZoom.Create(s, CRect(width - cSmallIconBtnSize, 0, width, cSmallIconBtnSize), this, IDC_3PANETOOLBAR_ZOOMPREVIEW, 0, IDI_3PANE_ZOOM);
	AddAlignment(new CWndAlignment(&mZoom, CWndAlignment::eAlign_TopRight));
	mZoom.SetSmallIcon(true);
	mZoom.SetShowIcon(true);
	mZoom.SetShowCaption(false);
	
	// No zoom/close if not 3-pane
	if (!is3Pane)
		mZoom.ShowWindow(SW_HIDE);

	// Add alignments
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_GROUPEDIT_GROUPNAME), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_GROUPEDIT_NICKNAME), CWndAlignment::eAlign_TopWidth));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_GROUPEDIT_LIST), CWndAlignment::eAlign_WidthHeight));
	AddAlignment(new CWndAlignment(GetDlgItem(IDC_GROUPEDIT_SORT), CWndAlignment::eAlign_BottomRight));
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
	GetDlgItem(IDC_GROUPEDIT_GROUPNAME)->SetFocus();
}

void CGroupPreview::OnZoomPane()
{
	C3PaneWindow::s3PaneWindow->SendMessage(WM_COMMAND, IDC_3PANETOOLBAR_ZOOMPREVIEW);
}

void CGroupPreview::SetGroup(CGroup* grp)
{
	mGroup = grp;
	cdstring title;
	if (grp)
		title = grp->GetName();
	CUnicodeUtils::SetWindowTextUTF8(&mDescriptor, title);

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
	CString s;
	mGroupName = (grp ? grp->GetName() : s);

	mNickName = (grp ? grp->GetNickName() : s);

	mAddressList = s;
	if (grp)
	{
		for(short i = 0; i < grp->GetAddressList().size(); i++)
		{
			mAddressList += grp->GetAddressList().at(i);
			mAddressList += "\r\n";
		}
	}
	
	// Now force control update
	UpdateData(false);
}

// Get fields from dialog
bool CGroupPreview::GetFields(CGroup* grp)
{
	// Only for valid group
	if (!grp)
		return false;
	
	// Now force control update
	UpdateData(true);

	bool done_edit = false;

	if (grp->GetName() != mGroupName)
	{
		grp->GetName() = mGroupName;
		done_edit = true;
	}
	
	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(mNickName);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	if (grp->GetNickName() != nickname)
	{
		grp->GetNickName() = nickname;
		done_edit = true;
	}

	char* s = mAddressList.c_str_mod();
	cdstrvect tmp;
	if (s)
	{
		s = ::strtok(s, "\r\n");
		while(s)
		{
			cdstring copyStr(s);
			tmp.push_back(copyStr);
			
			s = ::strtok(NULL, "\r\n");
		}
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

void CGroupPreview::OnGroupEditSort() 
{
	cdstrvect list;
	
	// Parse into list
	cdstring txt = CUnicodeUtils::GetWindowTextUTF8(&mAddressListCtrl);
	char* s = txt.c_str_mod();
	if (s)
	{
		s = ::strtok(s, "\r\n");
		while(s)
		{
			cdstring copyStr(s);
			list.push_back(copyStr);
			
			s = ::strtok(nil, "\r\n");
		}

		// Sort list
		sort(list.begin(), list.end(), comp_strnocase_1);

		// Put back in field
		txt.clear();
		for(short i = 0; i < list.size(); i++)
		{
			const cdstring& aStr = list.at(i);
			txt += aStr;
			txt += "\r\n";
		}
	}

	CUnicodeUtils::SetWindowTextUTF8(&mAddressListCtrl, txt);
}
