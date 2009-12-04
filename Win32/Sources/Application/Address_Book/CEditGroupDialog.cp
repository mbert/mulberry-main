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
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditGroupDialog dialog


CEditGroupDialog::CEditGroupDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CEditGroupDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditGroupDialog)
	mNickName = _T("");
	mGroupName = _T("");
	mAddressList = _T("");
	//}}AFX_DATA_INIT
}


void CEditGroupDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditGroupDialog)
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_NICKNAME, mNickName);
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_GROUPNAME, mGroupName);
	DDX_UTF8Text(pDX, IDC_GROUPEDIT_LIST, mAddressList);
	DDX_Control(pDX, IDC_GROUPEDIT_LIST, mAddressListCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditGroupDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CEditGroupDialog)
	ON_BN_CLICKED(IDC_GROUPEDIT_SORT, OnGroupEditSort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

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

void CEditGroupDialog::OnGroupEditSort() 
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
		sort(list.begin(), list.end(), comp_strnocase);

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

bool CEditGroupDialog::GetFields(CGroup* grp)
{
	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(mNickName);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	grp->GetNickName() = nickname;
	grp->GetName() = mGroupName;
	
	grp->GetAddressList().clear();
	char* s = mAddressList.c_str_mod();
	if (s)
	{
		s = ::strtok(s, "\r\n");
		while(s)
		{
			cdstring copyStr(s);
			grp->GetAddressList().push_back(copyStr);
			
			s = ::strtok(NULL, "\r\n");
		}
	}

	return true;
}

void CEditGroupDialog::SetFields(const CGroup* grp)
{
	mNickName = grp->GetNickName();

	mGroupName = grp->GetName();

	for(short i = 0; i < ((CGroup*) grp)->GetAddressList().size(); i++)
	{
		mAddressList += ((CGroup*) grp)->GetAddressList().at(i);
		mAddressList += "\r\n";
	}
}