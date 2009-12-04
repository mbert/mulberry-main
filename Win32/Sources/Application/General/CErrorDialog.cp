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


// CErrorDialog.cpp : implementation file
//


#include "CErrorDialog.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

const int cButtonTextLong = 10;
const int cButtonCharWidth = 6;

bool CErrorDialog::sOnScreen = false;

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog dialog

CErrorDialog::CErrorDialog(EErrDialogType type,
							const char*  btn1, const char*  btn2,
							const char*  btn3, const char*  btn4,
							const char* rsrc_text,
							const char* extra1, const char* extra2,
							const char* txt_dontshow, bool* dontshow, bool beep,
							CWnd* pParent /*=NULL*/)
	: CDialog((dontshow != NULL) ? CErrorDialog::IDD2 : CErrorDialog::IDD1, pParent)
{
	mType = type;
	mBtn1 = btn1;
	mBtn2 = btn2;
	mBtn3 = btn3;
	mBtn4 = btn4;
	mRsrcText = rsrc_text;
	mExtra1 = extra1;
	mExtra2 = extra2;
	mTextDontShow = txt_dontshow;
	mDontShow = dontshow;
	mDontShowValue = FALSE;
}

void CErrorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CErrorDialog)
	if (mDontShow != NULL)
		DDX_Check(pDX, IDC_ERRORDONTSHOW, mDontShowValue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CErrorDialog, CDialog)
	//{{AFX_MSG_MAP(CDialog)
	ON_COMMAND(IDC_ERRORBTN1, OnBtn1)
	ON_COMMAND(IDC_ERRORBTN2, OnBtn2)
	ON_COMMAND(IDC_ERRORBTN3, OnBtn3)
	ON_COMMAND(IDC_ERRORBTN4, OnBtn4)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog message handlers

// Called during idle
BOOL CErrorDialog::OnInitDialog(void)
{
	CMulberryApp* app = CMulberryApp::sApp;	// Need this to fix bug in CW8 compiler causing garbage for sApp later on

	CDialog::OnInitDialog();

	CButton* aBtn1 = (CButton*) GetDlgItem(IDC_ERRORBTN1);
	CButton* aBtn2 = (CButton*) GetDlgItem(IDC_ERRORBTN2);
	CButton* aBtn3 = (CButton*) GetDlgItem(IDC_ERRORBTN3);
	CButton* aBtn4 = (CButton*) GetDlgItem(IDC_ERRORBTN4);

	int distance = 0;
	if (mBtn1)
	{
		cdstring s(mBtn1);
		short len = s.length();
		if (len)
		{
			CUnicodeUtils::SetWindowTextUTF8(aBtn1, s);

			// Resize this button
			if (len > cButtonTextLong)
			{
				distance = max(distance, cButtonCharWidth * (len - cButtonTextLong));
			}

		}
		else
			aBtn1->ShowWindow(SW_HIDE);
	}
	else
		aBtn1->ShowWindow(SW_HIDE);

	if (mBtn2)
	{
		cdstring s(mBtn2);
		short len = s.length();
		if (len)
		{
			CUnicodeUtils::SetWindowTextUTF8(aBtn2, s);

			// Resize this button
			if (len > cButtonTextLong)
			{
				distance = max(distance, cButtonCharWidth * (len - cButtonTextLong));
			}

		}
		else
			aBtn2->ShowWindow(SW_HIDE);
	}
	else
		aBtn2->ShowWindow(SW_HIDE);

	if (mBtn3)
	{
		cdstring s(mBtn3);
		short len = s.length();
		if (len)
		{
			CUnicodeUtils::SetWindowTextUTF8(aBtn3, s);

			// Resize this button
			if (len > cButtonTextLong)
			{
				distance = max(distance, cButtonCharWidth * (len - cButtonTextLong));
			}

		}
		else
			aBtn3->ShowWindow(SW_HIDE);
	}
	else
		aBtn3->ShowWindow(SW_HIDE);

	if (mBtn4)
	{
		cdstring s(mBtn4);
		short len = s.length();
		if (len)
		{
			CUnicodeUtils::SetWindowTextUTF8(aBtn4, s);

			// Resize this button
			if (len > cButtonTextLong)
			{
				distance = max(distance, cButtonCharWidth * (len - cButtonTextLong));
			}

		}
		else
			aBtn4->ShowWindow(SW_HIDE);
	}
	else
		aBtn4->ShowWindow(SW_HIDE);
	
	// Now adjust all button sizes and dialog width
	if (distance != 0)
	{
		::ResizeWindowBy(aBtn1, distance, 0, false);
		::ResizeWindowBy(aBtn2, distance, 0, false);
		::ResizeWindowBy(aBtn3, distance, 0, false);
		::ResizeWindowBy(aBtn4, distance, 0, false);
		::ResizeWindowBy(this, distance, 0, false);
	}

	// Set icon
	CStatic* anIcon = (CStatic*) GetDlgItem(IDC_ERRORICON);
	HICON hicon;
	cdstring title;
	switch(mType)
	{
	case eErrDialog_Stop:
		hicon = CMulberryApp::sApp->LoadStandardIcon(IDI_HAND);
		title.FromResource(IDS_DialogError);
		break;
	case eErrDialog_Note:
		hicon = CMulberryApp::sApp->LoadStandardIcon(IDI_ASTERISK);
		title.FromResource(IDS_DialogNote);
		break;
	case eErrDialog_Caution:
	default:
		hicon = CMulberryApp::sApp->LoadStandardIcon(IDI_EXCLAMATION);
		title.FromResource(IDS_DialogWarning);
		break;
	}
	anIcon->SetIcon(hicon);
	CUnicodeUtils::SetWindowTextUTF8(this, title);

	// Insert extra text and leave as c-str in case > 255 chars
	cdstring txt1;
	if (mExtra1 && !mExtra2)
	{
		// Load in from resource if there
		if ((mRsrcText != NULL) && (::strlen(mRsrcText) != 0))
		{
			size_t txt_reserve = ::strlen(mRsrcText) + ::strlen(mExtra1) + 1;
			txt1.reserve(txt_reserve);
			::snprintf(txt1.c_str_mod(), txt_reserve, mRsrcText, mExtra1);
		}
		else
			// Just use extra as main text
			txt1 = mExtra1;
	}
	else if (mExtra1 && mExtra2)
	{
		// Load in from resource if there
		if ((mRsrcText != NULL) && (::strlen(mRsrcText) != 0))
		{
			size_t txt_reserve = ::strlen(mRsrcText) + ::strlen(mExtra1) + ::strlen(mExtra2) + 1;
			txt1.reserve(txt_reserve);
			::snprintf(txt1.c_str_mod(), txt_reserve, mRsrcText, mExtra1, mExtra2);
		}
		else
		{
			// Just use extras as main text
			txt1 = mExtra1;
			txt1 += mExtra2;
		}
	}
	else if (mRsrcText != NULL)
	{
		// Set alert message
		txt1 = mRsrcText;
	}

	// Give text to pane
	CStatic* theText = (CStatic*) GetDlgItem(IDC_ERRORTEXT);
	CUnicodeUtils::SetWindowTextUTF8(theText, txt1);

	
	// Do don't show checkbox
	if (mDontShow != NULL)
	{
		CButton* cbox = (CButton*) GetDlgItem(IDC_ERRORDONTSHOW);
		if (cbox && (mTextDontShow != NULL) && (*mTextDontShow != 0))
			CUnicodeUtils::SetWindowTextUTF8(cbox, mTextDontShow);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CErrorDialog::OnBtn1()
{
	UpdateData(TRUE);
	EndDialog(IDC_ERRORBTN1);
}

void CErrorDialog::OnBtn2()
{
	UpdateData(TRUE);
	EndDialog(IDC_ERRORBTN2);
}

void CErrorDialog::OnBtn3()
{
	UpdateData(TRUE);
	EndDialog(IDC_ERRORBTN3);
}

void CErrorDialog::OnBtn4()
{
	UpdateData(TRUE);
	EndDialog(IDC_ERRORBTN4);
}

// Handle key down
void CErrorDialog::OnCancel()
{
	// Find the button corresponding to Cancel
	switch(mCancelBtn)
	{
	case 1:
		OnBtn1();
		break;
	case 2:
		OnBtn2();
		break;
	case 3:
		OnBtn3();
		break;
	case 4:
		OnBtn4();
		break;
	}
}

CErrorDialog::EDialogResult CErrorDialog::PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
								const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
								unsigned long cancel_id, const char* extra1, const char* extra2, const char* txt_dontshow, bool* dontshow, bool beep)
{
	EDialogResult result;

	// Create the dialog
	HWND old_focus = CWnd::GetFocus()->GetSafeHwnd();
	sOnScreen = true;
	CMulberryApp::sApp->ErrorPause(true);
	{
		const cdstring& title1 = rsrc::GetString(rsrc_btn1);
		const cdstring& title2 = rsrc::GetString(rsrc_btn2);
		const cdstring& title3 = rsrc::GetString(rsrc_btn3);
		const cdstring& title4 = rsrc::GetString(rsrc_btn4);
		const cdstring& main_text = rsrc::GetString(rsrc_text);

		CErrorDialog dlog(type, title1, title2, title3, title4, main_text, extra1, extra2, txt_dontshow, dontshow, beep, CSDIFrame::GetAppTopWindow());

		// Determine cancel button
		dlog.mCancelBtn = cancel_id;

		// Sound if required
		if (beep) ::MessageBeep(-1);

		CSDIFrame::EnterModal(&dlog);
		try
		{
			// Let DialogHandler process events and map result to
			// button code
			switch(dlog.DoModal())
			{
			case IDC_ERRORBTN1:
				result = eBtn1;
				break;
			case IDC_ERRORBTN2:
				result = eBtn2;
				break;
			case IDC_ERRORBTN3:
				result = eBtn3;
				break;
			case IDC_ERRORBTN4:
				result = eBtn4;
				break;
			}

			if (dontshow != NULL)
			{
				*dontshow = dlog.mDontShowValue;
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			CSDIFrame::ExitModal();
			CLOG_LOGRETHROW;
			throw;
		}
		
		CSDIFrame::ExitModal();
	}
	sOnScreen = false;
	CMulberryApp::sApp->ErrorPause(false);
	if (old_focus && ::IsWindow(old_focus))
		::SetFocus(old_focus);

	return result;
}

// Standard alert
CErrorDialog::EDialogResult CErrorDialog::Alert(const char* txt, bool beep)
{
	// Just map to NoteAlert
	return CErrorDialog::NoteAlert(txt, NULL, NULL, beep);
}

// Stop alert
CErrorDialog::EDialogResult CErrorDialog::StopAlert(const char* txt, bool beep)
{
	return PoseDialog(eErrDialog_Stop,
						"ErrorDialog::Btn::OK",
						NULL,
						NULL,
						NULL,
						NULL, 2, txt, NULL, NULL, NULL, beep);
}

// Caution alert
CErrorDialog::EDialogResult CErrorDialog::CautionAlert(bool yesno, const char* txt, const char* txt_dontshow, bool* dontshow, bool beep)
{
	return PoseDialog(eErrDialog_Caution,
						yesno ? "ErrorDialog::Btn::Yes" :  "ErrorDialog::Btn::OK",
						yesno ? "ErrorDialog::Btn::No" : "ErrorDialog::Btn::Cancel",
						NULL,
						NULL,
						NULL, 2, txt, NULL, txt_dontshow, dontshow, beep);
}

// Note alert
CErrorDialog::EDialogResult CErrorDialog::NoteAlert(const char* txt, const char* txt_dontshow, bool* dontshow, bool beep)
{
	return PoseDialog(eErrDialog_Note,
						"ErrorDialog::Btn::OK",
						NULL,
						NULL,
						NULL,
						NULL, 2, txt, NULL, txt_dontshow, dontshow, beep);
}

// Note alert
CErrorDialog::EDialogResult CErrorDialog::NoteAlertRsrc(const char* txt, const char* txt_dontshow, bool* dontshow, bool beep)
{
	cdstring alert = rsrc::GetString(txt);
	return CErrorDialog::NoteAlert(alert, txt_dontshow, dontshow, beep);
}
