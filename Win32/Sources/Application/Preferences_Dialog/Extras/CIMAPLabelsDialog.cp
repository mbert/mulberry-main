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


// CIMAPLabelsDialog.cpp : implementation file
//

#include "CIMAPLabelsDialog.h"

#include "CCharSpecials.h"
#include "CErrorHandler.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CIMAPLabelsDialog dialog


CIMAPLabelsDialog::CIMAPLabelsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CIMAPLabelsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIMAPLabelsDialog)
	//}}AFX_DATA_INIT
}


void CIMAPLabelsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIMAPLabelsDialog)
	for(int i = 0; i < 8; i++)
	{
		DDX_UTF8Text(pDX, IDC_IMAPLABELS_NAME1 + i, mNames[i]);
		DDX_UTF8Text(pDX, IDC_IMAPLABELS_LABEL1 + i, mLabels[i]);
	}
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIMAPLabelsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CIMAPLabelsDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIMAPLabelsDialog message handlers

BOOL CIMAPLabelsDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CIMAPLabelsDialog::SetDetails(const cdstrvect& names, const cdstrvect& labels)
{
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		mNames[i] = names[i];
		mLabels[i] = labels[i];
	}
}

// Called during idle
bool CIMAPLabelsDialog::GetDetails(cdstrvect& labels)
{
	// First make sure all labels are valid
	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		cdstring label = mLabels[i];
		if (!ValidLabel(label))
			return false;
	}

	for(int i = 0; i < NMessage::eMaxLabels; i++)
	{
		labels[i] = mLabels[i];
	}
	
	return true;
}

bool CIMAPLabelsDialog::ValidLabel(const cdstring& tlabel)
{
	// Cannot be empty
	if (tlabel.empty())
		return false;

	// Cannot start with backslash
	if (tlabel[(cdstring::size_type)0] == '\\')
		return false;
	
	// Must be an atom
	const char* p = tlabel.c_str();
	while(*p)
	{
		if (cINETChar[(unsigned char)*p] != 0)
			return false;
		p++;
	}
	
	return true;
}

bool CIMAPLabelsDialog::PoseDialog(const cdstrvect& names, cdstrvect& labels)
{
	bool result = false;

	// Create dlog
	CIMAPLabelsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(names, labels);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetDetails(labels);
		result = true;
	}
	
	return result;
}
