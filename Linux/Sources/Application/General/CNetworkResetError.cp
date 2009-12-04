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

#include "CNetworkResetError.h"

#include "CTCPSocket.h"
#include "CXStringResources.h"

#include <jXGlobals.h>

CNetworkResetError::CNetworkResetError(JXDirector* supervisor)
	: CErrorDialog(supervisor)
{}

void CNetworkResetError::Continue()
{
	if (CTCPSocket::CheckConnectionState())
		EndDialog(kDialogClosed_OK);
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
	const cdstring& title1 = rsrc::GetString(rsrc_btn1);
	const cdstring& title2 = rsrc::GetString(rsrc_btn2);
	const cdstring& title3 = rsrc::GetString(rsrc_btn3);
	const cdstring& title4 = rsrc::GetString(rsrc_btn4);
	const cdstring& main_text = rsrc::GetString(rsrc_text);

	CNetworkResetError* dlog = new CNetworkResetError(JXGetApplication());
	dlog->mCancelBtn = cancel_id;
	dlog->OnCreate();
	dlog->BuildWindow(type, title1, title2, title3, title4, main_text, extra1, extra2, txt_dontshow, dontshow, beep);
	dlog->mDontShow = dontshow;

	int result = dlog->DoModal(false);
	switch(result)
	{
	case kDialogClosed_OK:
		result = eBtn1;
		dlog->Close();
		break;
	case kDialogClosed_Cancel:
		result = eBtn2;
		break;
	case kDialogClosed_Btn3:
		result = eBtn3;
		dlog->Close();
		break;
	case kDialogClosed_Btn4:
		result = eBtn4;
		dlog->Close();
		break;
	}
	return (CErrorDialog::EDialogResult)result;
}

