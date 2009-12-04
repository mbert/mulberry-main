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


// Header for CAboutDialog class

#ifndef __CERRORDIALOG__MULBERRY__
#define __CERRORDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_ErrorDialog = 10001;
const	PaneIDT		paneid_ErrorDontShowDialog = 10007;
const	PaneIDT		paneid_ErrorBtn1 = 'BTN1';
const	PaneIDT		paneid_ErrorBtn2 = 'BTN2';
const	PaneIDT		paneid_ErrorBtn3 = 'BTN3';
const	PaneIDT		paneid_ErrorBtn4 = 'BTN4';
const	PaneIDT		paneid_ErrorIcon = 'ICON';
const	PaneIDT		paneid_ErrorText = 'TEXT';
const	PaneIDT		paneid_ErrorDontShow = 'DSHW';

// Messages
const	MessageT	msg_Btn1 = 900;
const	MessageT	msg_Btn2 = 901;
const	MessageT	msg_Btn3 = 902;
const	MessageT	msg_Btn4 = 903;

// Resources
const	PaneIDT		RidL_CErrorDialogBtns = 10001;
const	PaneIDT		RidL_CErrorDontShowDialogBtns = 10007;

enum
{
	icnx_Stop = 0,
	icnx_Note,
	icnx_Caution
};

class LCommander;

class CErrorDialog : public LDialogBox
{

public:
	enum { class_ID = 'Errr' };

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

					CErrorDialog();
					CErrorDialog(LStream *inStream);
	virtual 		~CErrorDialog();

	virtual	void	SetUpDetails(const char* rsrc_btn1,
									const char* rsrc_btn2,
									const char* rsrc_btn3,
									const char* rsrc_btn4,
									ResIDT icon,
									const char* rsrc_text,
									const char* extra1 = NULL,
									const char* extra2 = NULL,
									const char* txt_dontshow = NULL,
									bool* dontshow = NULL);

	static EDialogResult	PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
								const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
								unsigned long cancel_id = 2,
								const char* extra1 = NULL, const char* extra2 = NULL,
								const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);

	static EDialogResult	Alert(const char* txt, bool beep = true);
	static EDialogResult	StopAlert(const char* txt, bool beep = true);
	static EDialogResult	CautionAlert(bool yesno, const char* txt, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);
	static EDialogResult	NoteAlert(const char* txt, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);
	static EDialogResult	NoteAlertRsrc(const char* text, const char* txt_dontshow = NULL, bool* dontshow = NULL, bool beep = true);

	static bool				OnScreen(void)
								{ return sOnScreen; }		// Is dialog currently on screen
	
	static void				SetCritical(bool critical)
								{ sErrorCritical = critical; }
protected:
	virtual void	FinishCreateSelf(void);

private:
	static bool sOnScreen;
	static bool sErrorCritical;
};

class StErrorCritical
{
public:
	StErrorCritical()
		{ CErrorDialog::SetCritical(true); }
	~StErrorCritical()
		{ CErrorDialog::SetCritical(false); }
};

#endif
