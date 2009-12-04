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


// CRegistrationDialog.cpp : implementation file
//


#include "CRegistrationDialog.h"

#include "CErrorHandler.h"
#include "CLicenseDialog.h"
#include "CMulberryApp.h"
#include "CRegistration.h"
#include "CSDIFrame.h"
#include "CStringUtils.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

#define V3_REGISTRATION	1

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDialog dialog


CRegistrationDialog::CRegistrationDialog(bool v3, CWnd* pParent /*=NULL*/)
	: CHelpDialog(v3 ? CRegistrationDialog::IDD2 : CRegistrationDialog::IDD1, pParent)
{
	mV3 = v3;

	//{{AFX_DATA_INIT(CRegistrationDialog)
	//}}AFX_DATA_INIT
	mAllowDemo = true;
}


void CRegistrationDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegistrationDialog)
	DDX_Control(pDX, IDOK, mOKBtn);
	DDX_Control(pDX, IDC_LICENSEE, mLicenseeNameCtrl);
	DDX_Control(pDX, IDC_ORGANISATION, mOrganisationCtrl);
	DDX_Control(pDX, IDC_SERIALNUMBER, mSerialNumberCtrl);
	if (mV3)
	{
		DDX_Control(pDX, IDC_REGKEY1, mRegKeyCtrl1);
		DDX_Control(pDX, IDC_REGKEY2, mRegKeyCtrl2);
		DDX_Control(pDX, IDC_REGKEY3, mRegKeyCtrl3);
		DDX_Control(pDX, IDC_REGKEY4, mRegKeyCtrl4);
	}
	else
		DDX_Control(pDX, IDC_REGKEY, mRegKeyCtrl);

	DDX_UTF8Text(pDX, IDC_LICENSEE, mLicenseeName);
	DDX_UTF8Text(pDX, IDC_ORGANISATION, mOrganisation);
	DDX_UTF8Text(pDX, IDC_SERIALNUMBER, mSerialNumber);
	if (mV3)
	{
		DDX_UTF8Text(pDX, IDC_REGKEY1, mRegKey1);
		DDV_UTF8MaxChars(pDX, mRegKey1, 4);
		DDX_UTF8Text(pDX, IDC_REGKEY2, mRegKey2);
		DDV_UTF8MaxChars(pDX, mRegKey2, 4);
		DDX_UTF8Text(pDX, IDC_REGKEY3, mRegKey3);
		DDV_UTF8MaxChars(pDX, mRegKey3, 4);
		DDX_UTF8Text(pDX, IDC_REGKEY4, mRegKey4);
		DDV_UTF8MaxChars(pDX, mRegKey4, 4);
	}
	else
	{
		DDX_UTF8Text(pDX, IDC_REGKEY, mRegKey);
		DDV_UTF8MaxChars(pDX, mRegKey, 8);
	}
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRegistrationDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CRegistrationDialog)
	ON_COMMAND(IDC_RUNDEMO, OnRunAsDemo)
	ON_EN_CHANGE(IDC_LICENSEE, OnChangeEntry)
	ON_EN_CHANGE(IDC_ORGANISATION, OnChangeEntry)
	ON_EN_CHANGE(IDC_SERIALNUMBER, OnChangeEntry)
	ON_EN_CHANGE(IDC_REGKEY, OnChangeEntry)
	ON_EN_CHANGE(IDC_REGKEY1, OnChangeEntry1)
	ON_EN_CHANGE(IDC_REGKEY2, OnChangeEntry2)
	ON_EN_CHANGE(IDC_REGKEY3, OnChangeEntry3)
	ON_EN_CHANGE(IDC_REGKEY4, OnChangeEntry4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDialog message handlers

BOOL CRegistrationDialog::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Hide registration button if already registered
	if (!mAllowDemo)
		GetDlgItem(IDC_RUNDEMO)->ShowWindow(SW_HIDE);

	// Force update to OK button in case of re-entry
	OnChangeEntry();

	return true;
}

void CRegistrationDialog::OnRunAsDemo()
{
	EndDialog(IDC_RUNDEMO);
}

void CRegistrationDialog::OnChangeEntry()
{
	cdstring licensee_name = CUnicodeUtils::GetWindowTextUTF8(&mLicenseeNameCtrl);
	bool valid_name = (licensee_name.length() != 0);

	cdstring serial_number = CUnicodeUtils::GetWindowTextUTF8(mSerialNumberCtrl);
	bool valid_serial = CRegistration::sRegistration.LegalSerial(cdstring(serial_number));

	bool valid_reg = false;
	if (mV3)
	{
		valid_reg = true;

		CString reg_key;
		mRegKeyCtrl1.GetWindowText(reg_key);
		valid_reg = (reg_key.GetLength() == 4); 

		mRegKeyCtrl2.GetWindowText(reg_key);
		valid_reg &= (reg_key.GetLength() == 4); 

		mRegKeyCtrl3.GetWindowText(reg_key);
		valid_reg &= (reg_key.GetLength() == 4); 

		mRegKeyCtrl4.GetWindowText(reg_key);
		valid_reg &= (reg_key.GetLength() == 4); 
	}
	else
	{
		CString reg_key;
		mRegKeyCtrl.GetWindowText(reg_key);
		valid_reg = (reg_key.GetLength() != 0); 
	}

	if (valid_name && valid_serial && valid_reg)
		mOKBtn.EnableWindow(true);
	else
		mOKBtn.EnableWindow(false);

}

void CRegistrationDialog::OnChangeEntry1()
{
	// Do button activation
	OnChangeEntry();
	
	// Check length and move to next field
	CString reg_key;
	mRegKeyCtrl1.GetWindowText(reg_key);
	if (reg_key.GetLength() == 4)
		mRegKeyCtrl2.SetFocus();
}

void CRegistrationDialog::OnChangeEntry2()
{
	// Do button activation
	OnChangeEntry();
	
	// Check length and move to next field
	CString reg_key;
	mRegKeyCtrl2.GetWindowText(reg_key);
	if (reg_key.GetLength() == 4)
		mRegKeyCtrl3.SetFocus();
}

void CRegistrationDialog::OnChangeEntry3()
{
	// Do button activation
	OnChangeEntry();
	
	// Check length and move to next field
	CString reg_key;
	mRegKeyCtrl3.GetWindowText(reg_key);
	if (reg_key.GetLength() == 4)
		mRegKeyCtrl4.SetFocus();
}

void CRegistrationDialog::OnChangeEntry4()
{
	// Do button activation
	OnChangeEntry();
}

bool CRegistrationDialog::DoRegistration(bool initial)
{
	bool okayed = false;

	// Create the dialog
#ifdef V3_REGISTRATION
	CRegistrationDialog dlog(true, CSDIFrame::GetAppTopWindow());
#else
	CRegistrationDialog dlog(false, CSDIFrame::GetAppTopWindow());
#endif
	dlog.mAllowDemo = initial;

	// Loop until valid data entry
	while(true)
	{
		// Let Dialog process events
		int result = dlog.DoModal();

		if (result == IDOK)
		{
			cdstring licensee(dlog.mLicenseeName);
			cdstring organisation(dlog.mOrganisation);
			cdstring serial(dlog.mSerialNumber);

			cdstring regkey;
			if (dlog.mV3)
			{
				regkey = dlog.mRegKey1;
				regkey += dlog.mRegKey2;
				regkey += dlog.mRegKey3;
				regkey += dlog.mRegKey4;
				
				// Must be uppercase
				::strupper(regkey.c_str_mod());
			}
			else
				regkey = dlog.mRegKey;

			// Must have valid registration input first
			if (CRegistration::sRegistration.ValidInfoCurrent(serial, regkey))
			{
				// Remove existing resources
				CRegistration::sRegistration.ClearAppInfo(AfxGetResourceHandle());
				CRegistration::sRegistration.SetLicensee(licensee);
				CRegistration::sRegistration.SetOrganisation(organisation);
				CRegistration::sRegistration.SetSerialNumber(serial);
				CRegistration::sRegistration.SetAppUserCode(regkey);
				
				// Validate - should wotrk unless resource failure
				if (CRegistration::sRegistration.WriteAppInfo(AfxGetResourceHandle()))
				{
					// Mark to run as not demo
					CMulberryApp::sApp->RunDemo(false);
					return true;
				}
			}
		}
		else if (result == IDCANCEL)
		{
			return false;
		}
		else if (result == IDC_RUNDEMO)
		{
			// Look for date resource
			time_t timeout = CRegistration::sRegistration.LoadAppTimeout(0);

			// Check for existing time
			if (timeout)
			{
				// Get time difference
				double diff = ::difftime(::time(nil), timeout);

				// Check that it can run
				if (diff < 31L*24L*60L*60L)
				{
					// Put up alert warning of time left
					cdstring txt(rsrc::GetString("Alerts::General::Timeleft"));
					txt.Substitute((long) (31L*24L*60L*60L - diff)/(24L*60L*60L));
					CErrorHandler::PutNoteAlert(txt);

					// Mark to run as demo
					CMulberryApp::sApp->RunDemo(true);

					// Set registration details
					CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
					CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
					CRegistration::sRegistration.SetSerialNumber("Evaluation");
					
					return true;
				}
				else
				{
					// Time expired
					CErrorHandler::PutNoteAlertRsrc("Alerts::General::NoDemoTime");
					
					// Recyle the reg dialog but disable demo
					dlog.mAllowDemo = false;
				}
			}
			// Try to set new time in resource
			else
			{
				// Check for licence agreement
				CLicenseDialog dlog1(CSDIFrame::GetAppTopWindow());

				// Load text resource
				HGLOBAL hResource = ::LoadResource(AfxGetResourceHandle(),
													::FindResource(AfxGetResourceHandle(),
																	MAKEINTRESOURCE(IDT_DEMOLICENSE),
																	_T("TEXT")));
				char* p = (char*) ::LockResource(hResource);
				
				// Terminate with null
				char* q = p;
				while(*q != '*')
					q++;
				*q = 0;

				// Give to dialog
				dlog1.mDemoLicense = p;

				// Let DialogHandler process events
				if (dlog1.DoModal() == IDOK)
				{
					// Mark to run as demo
					CMulberryApp::sApp->RunDemo(true);

					// Set new time in resource
					okayed = CRegistration::sRegistration.WriteAppTimeout(0);

					// Set registration details
					CRegistration::sRegistration.SetLicensee("Unlicensed - for evaluation only");
					CRegistration::sRegistration.SetSerialNumber("Evaluation");
					CRegistration::sRegistration.SetOrganisation(cdstring::null_str);
					return true;
				}
				
				//::FreeResource(hResource); <- not required: crashes NT
			}
		}
	}

	return okayed;
}