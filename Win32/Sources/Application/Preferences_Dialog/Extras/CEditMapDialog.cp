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


// CEditMapDialog.cpp : implementation file
//


#include "CEditMapDialog.h"

#include "CMIMEMap.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEditMapDialog dialog


CEditMapDialog::CEditMapDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CEditMapDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditMapDialog)
	mAppLaunch = -1;
	//}}AFX_DATA_INIT
}


void CEditMapDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMapDialog)
	DDX_UTF8Text(pDX, IDC_EDITMAP_TYPE, mType);
	DDX_UTF8Text(pDX, IDC_EDITMAP_SUBTYPE, mSubtype);
	DDX_UTF8Text(pDX, IDC_EDITMAP_SUFFIX, mSuffix);
	DDX_Radio(pDX, IDC_EDITMAP_ALWAYS, mAppLaunch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditMapDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CEditMapDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditMapDialog message handlers

void CEditMapDialog::SetMap(CMIMEMap& aMap)
{
	mType = aMap.GetMIMEType();
	mSubtype = aMap.GetMIMESubtype();
	mSuffix = aMap.GetFileSuffix();
	mAppLaunch = aMap.GetAppLaunch();
}

void CEditMapDialog::GetMap(CMIMEMap& aMap)
{
	aMap.SetMIMEType(mType);
	aMap.SetMIMESubtype(mSubtype);
	aMap.SetFileSuffix(mSuffix);
	aMap.SetAppLaunch((EAppLaunch) mAppLaunch);
}
