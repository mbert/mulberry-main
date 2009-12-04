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


// CErrorDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog dialog

#ifndef __CERRORDIALOG__MULBERRY__
#define __CERRORDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include <stdint.h>

class CStaticText;
class JXTextButton;
class JXImageWidget;
class JXDirector;
class JXTextCheckbox;

class CErrorDialog : public CDialogDirector
{
public:

	enum EDialogResult
	{
		eOK,
		eCancel,
		eBtn1 = 0,
		eBtn2,
		eBtn3,
		eBtn4
	};
	
	enum EErrDialogType
	{
		eErrDialog_Stop = 1,
		eErrDialog_Note,
		eErrDialog_Caution,
		eErrDialog_Other
	};
  
	CErrorDialog(JXDirector* supervisor);
	virtual	~CErrorDialog();

	// Construction
	static EDialogResult PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
								const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
								unsigned long cancel_id = 2,
								const char* extra1 = NULL, const char* extra2 = NULL,
								const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);

	static EDialogResult	Alert(const char* txt, bool beep = true);
	static EDialogResult	StopAlert(const char* txt, bool beep = true);
	static EDialogResult	CautionAlert(bool yesno, const char* txt, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);
	static EDialogResult	NoteAlert(const char* txt, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);
	static EDialogResult	NoteAlertRsrc(const char* text, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);

	static bool	OnScreen(void)
		{ return sOnScreen; }		// Is dialog currently on screen

protected:
// begin JXLayout

    JXTextButton*   mBtn1Ctrl;
    JXTextButton*   mBtn2Ctrl;
    JXTextButton*   mBtn3Ctrl;
    JXTextButton*   mBtn4Ctrl;
    JXImageWidget*  mIconCtrl;
    CStaticText*    mTextCtrl;
    JXTextCheckbox* mDontShowCtrl;

// end JXLayout

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

protected:
	static bool sOnScreen;
	int mResult;

	EErrDialogType	mType;
	const char*		mBtn1;
	const char*		mBtn2;
	const char*		mBtn3;
	const char*		mBtn4;
	const char*		mStr;
	const char* 	mExtra1;
	const char* 	mExtra2;
	const char*		mTextDontShow;
	bool*			mDontShow;
	bool			mBeep;
	int32_t			mCancelBtn;

	void BuildWindow(CErrorDialog::EErrDialogType type,
					const char* btn1, const char* btn2,
					const char* btn3, const char* btn4,
					const char* rsrc_text,
					const char* extra1, const char* extra2,
					const char* txt_dontshow, bool* dontshow, bool beep);

};
#endif
