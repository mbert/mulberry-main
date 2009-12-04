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


// CVacationActionDialog.cp : implementation file
//

#include "CVacationActionDialog.h"

#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CVacationActionDialog dialog


CVacationActionDialog::CVacationActionDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CVacationActionDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVacationActionDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CVacationActionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVacationActionDialog)
	DDX_UTF8Text(pDX, IDC_VACATIONACTION_DAYS, mDays);
	DDV_MinMaxInt(pDX, mDays, 0, 32767);
	DDX_UTF8Text(pDX, IDC_VACATIONACTION_SUBJECT, mSubject);
	DDX_Control(pDX, IDC_VACATIONACTION_ADDRESSES, mAddressList);
	DDX_UTF8Text(pDX, IDC_VACATIONACTION_ADDRESSES, mAddresses);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CVacationActionDialog, CDialog)
	//{{AFX_MSG_MAP(CVacationActionDialog)
	ON_BN_CLICKED(IDC_VACATIONACTION_TEXT, OnText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVacationActionDialog message handlers

// Set the details
void CVacationActionDialog::SetDetails(CActionItem::CActionVacation& details)
{
	mDays = details.GetDays();

	mSubject = details.GetSubject();

	mText = details.GetText();

	mAddresses.clear();
	for(short i = 0; i < details.GetAddresses().size(); i++)
		mAddresses += details.GetAddresses()[i] + "\r\n";
}

// Get the details
void CVacationActionDialog::GetDetails(CActionItem::CActionVacation& details)
{
	details.SetDays(mDays);

	details.SetSubject(mSubject);

	details.SetText(mText);

	// Copy handle to text with null terminator
	char* s = ::strtok(mAddresses.c_str_mod(), "\r\n");
	cdstrvect accumulate;
	while(s)
	{
		cdstring copyStr(s);
		accumulate.push_back(copyStr);

		s = ::strtok(NULL, "\r\n");
	}
	details.SetAddresses(accumulate);
}

void CVacationActionDialog::OnText() 
{
	CPrefsEditHeadFoot::PoseDialog(CPreferences::sPrefs, mText, false);
}

bool CVacationActionDialog::PoseDialog(CActionItem::CActionVacation& details)
{
	bool result = false;

	// Create the dialog
	CVacationActionDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if ((dlog.DoModal() == IDOK))
	{
		dlog.GetDetails(details);
		result = true;
	}

	return result;
}
