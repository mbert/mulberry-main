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


// Source for CNetworkResetError class

#include "CNetworkResetError.h"

#include "CLog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CTCPSocket.h"
#include "CXStringResources.h"

CNetworkResetError::CNetworkResetError(EErrDialogType type,
					const char* btn1, const char* btn2,
					const char* btn3, const char* btn4,
					const char* rsrc_text,
					const char* extra1, const char* extra2,
					const char* txt_dontshow, bool* dontshow, bool beep, CWnd* pParent) :
					CErrorDialog(type, btn1, btn2, btn3, btn4, rsrc_text,
									extra1, extra2, txt_dontshow, dontshow, beep, pParent)
{
	mTimerID = 0;
}

BEGIN_MESSAGE_MAP(CNetworkResetError, CErrorDialog)
	//{{AFX_MSG_MAP(CNetworkResetError)
		ON_WM_DESTROY()
		ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CNetworkResetError::OnInitDialog()
{
	CErrorDialog::OnInitDialog();

	// TODO: Add extra initialization here

	// Start a timer
	if (CPreferences::sPrefs->mSleepReconnect.GetValue())
		mTimerID = SetTimer('Cnre', 1000, NULL);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetworkResetError::OnDestroy()
{
	if (mTimerID != 0)
		KillTimer(mTimerID);

	CErrorDialog::OnDestroy();
}

void CNetworkResetError::OnTimer(UINT nIDEvent)
{
	if ((nIDEvent == mTimerID) && CTCPSocket::CheckConnectionState())
		EndDialog(IDOK);
}

bool CNetworkResetError::PoseDialog(bool allow_disconnect)
{
	bool disconnect = false;
	if (allow_disconnect)
	{
		CErrorDialog::EDialogResult result = PoseDialog(CErrorDialog::eErrDialog_Caution,
												"ErrorDialog::Btn::Continue",
												"ErrorDialog::Btn::Disconnect",
												NULL,
												NULL,
												"Alerts::General::ConnectionsResetDisconnect");
		disconnect = (result == CErrorDialog::eBtn2);
	}
	else
	{
		PoseDialog(CErrorDialog::eErrDialog_Caution,
												"ErrorDialog::Btn::Continue",
												NULL,
												NULL,
												NULL,
												"Alerts::General::ConnectionsReset");
	}
	
	return disconnect;
}

CErrorDialog::EDialogResult CNetworkResetError::PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
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

		CNetworkResetError dlog(type, title1, title2, title3, title4, main_text, extra1, extra2, txt_dontshow, dontshow, beep, CSDIFrame::GetAppTopWindow());

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


