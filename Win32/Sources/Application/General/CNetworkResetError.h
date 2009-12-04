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


// Header for CNetworkResetError class

#ifndef __CNetworkResetError__MULBERRY__
#define __CNetworkResetError__MULBERRY__

#include "CErrorDialog.h"

// Classes

class CNetworkResetError : public CErrorDialog
{
public:

		CNetworkResetError(EErrDialogType type,
					const char* btn1, const char* btn2,
					const char* btn3, const char* btn4,
					const char* rsrc_text,
					const char* extra1, const char* extra2,
					const char* txt_dontshow, bool* dontshow, bool beep, CWnd* pParent);

	static bool	PoseDialog(bool allow_disconnect);

private:
	UINT				mTimerID;

	static EDialogResult	PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
								const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
								unsigned long cancel_id = 2,
								const char* extra1 = NULL, const char* extra2 = NULL,
								const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);

	// Generated message map functions
	//{{AFX_MSG(CNetworkResetError)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy(void);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
