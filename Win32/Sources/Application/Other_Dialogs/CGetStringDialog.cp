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


// CGetStringDialog.cp : implementation file
//


#include "CGetStringDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CGetStringDialog::CGetStringDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CGetStringDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenameMailboxDialog)
	//}}AFX_DATA_INIT
}


void CGetStringDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetStringDialog)
	DDX_UTF8Text(pDX, IDC_GETSTRINGNAME, mName);
	DDX_UTF8Text(pDX, IDC_GETSTRINGTITLE, mTitle);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetStringDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CGetStringDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetStringDialog message handlers

bool CGetStringDialog::PoseDialog(short nWindow, short nTitle, cdstring& name)
{
	cdstring window;
	window.FromResource(nWindow);
	cdstring title;
	title.FromResource(nTitle);
	return _PoseDialog(window, title, name);
}

bool CGetStringDialog::PoseDialog(const char* window, const char* title, cdstring& name)
{
	return _PoseDialog(rsrc::GetString(window), rsrc::GetString(title), name);
}

bool CGetStringDialog::PoseDialog(const char* title, cdstring& name)
{
	return _PoseDialog(NULL, rsrc::GetString(title), name);
}

bool CGetStringDialog::_PoseDialog(const char* window, const char* title, cdstring& name)
{
	// Create the dialog (use old name as starter)
	CGetStringDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mName = name;
	dlog.mTitle = title;
	if (window != NULL)
	{
		CUnicodeUtils::SetWindowTextUTF8(&dlog, window);
	}
	
	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		name = dlog.mName;
		return true;
	}
	else
		return false;
}
