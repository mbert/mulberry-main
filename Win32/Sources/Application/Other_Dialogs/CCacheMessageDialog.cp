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


// CCacheMessageDialog.cpp : implementation file
//

#include "CCacheMessageDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCacheMessageDialog dialog


CCacheMessageDialog::CCacheMessageDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCacheMessageDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCacheMessageDialog)
	mNumber = 1;
	//}}AFX_DATA_INIT
}


void CCacheMessageDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCacheMessageDialog)
	DDX_UTF8Text(pDX, IDC_CACHEMESSAGE_NUMBER, mNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCacheMessageDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCacheMessageDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCacheMessageDialog message handlers

// Get the details
void CCacheMessageDialog::GetDetails(unsigned long& goto_num)
{
	goto_num = mNumber;
}

bool CCacheMessageDialog::PoseDialog(unsigned long& goto_num)
{
	bool result = false;

	// Create the dialog
	CCacheMessageDialog	dlog(CSDIFrame::GetAppTopWindow());

	// Run modal loop and look for OK
	if (dlog.DoModal() == msg_OK)
	{
		dlog.GetDetails(goto_num);
		result = true;
	}
	
	return result;
}
