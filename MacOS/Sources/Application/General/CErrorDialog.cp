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


// Source for CErrorDialog class

#include "CErrorDialog.h"

#include "CBalloonDialog.h"
#include "CHelpAttach.h"
#include "CMulberryApp.h"
#include "CTextDisplay.h"
#include "CXStringResources.h"

#include <LCheckBox.h>
#include <LIconControl.h>
#include <LPushButton.h>
#include <UGAColorRamp.h>

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C E R R O R D I A L O G
// __________________________________________________________________________________________________

bool CErrorDialog::sOnScreen = false;
bool CErrorDialog::sErrorCritical = false;

const int cCharResize = 7;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CErrorDialog::CErrorDialog()
{
}

// Constructor from stream
CErrorDialog::CErrorDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CErrorDialog::~CErrorDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CErrorDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());
}

// Called during idle
void CErrorDialog::SetUpDetails(const char* rsrc_btn1,
								const char* rsrc_btn2,
								const char* rsrc_btn3,
								const char* rsrc_btn4,
								ResIDT icon, const char* rsrc_text,
								const char* extra1, const char* extra2,
								const char* txt_dontshow, bool* dontshow)
{
	cdstring txt;

	LPushButton* aBtn1 = (LPushButton*) FindPaneByID(paneid_ErrorBtn1);
	LPushButton* aBtn2 = (LPushButton*) FindPaneByID(paneid_ErrorBtn2);
	LPushButton* aBtn3 = (LPushButton*) FindPaneByID(paneid_ErrorBtn3);
	LPushButton* aBtn4 = (LPushButton*) FindPaneByID(paneid_ErrorBtn4);

	if ((rsrc_btn1 != NULL) && (::strlen(rsrc_btn1) != 0))
	{
		aBtn1->SetDescriptor(LStr255(rsrc_btn1));
		//aBtn1->AddAttachment(new CHelpAttach(STRx_ErrorButtons, btn1 + 1));

		// Resize this button and move buttons if required
		if (::strlen(rsrc_btn1) > 6)
		{
			short distance = cCharResize * (::strlen(rsrc_btn1) - 6);
			ResizeWindowBy(distance, 0);
			aBtn1->ResizeFrameBy(distance, 0, false);
			aBtn1->MoveBy(-distance, 0, false);
			aBtn2->MoveBy(-distance, 0, false);
			aBtn3->MoveBy(-distance, 0, false);
			aBtn4->MoveBy(-distance, 0, false);
		}

	}
	else
		aBtn1->Hide();

	if ((rsrc_btn2 != NULL) && (::strlen(rsrc_btn2) != 0))
	{
		aBtn2->SetDescriptor(LStr255(rsrc_btn2));
		//aBtn2->AddAttachment(new CHelpAttach(STRx_ErrorButtons, btn2 + 1));

		// Resize this button and move buttons if required
		if (::strlen(rsrc_btn2) > 6)
		{
			short distance = cCharResize * (::strlen(rsrc_btn2) - 6);
			ResizeWindowBy(distance, 0);
			aBtn2->ResizeFrameBy(distance, 0, false);
			aBtn2->MoveBy(-distance, 0, false);
			aBtn3->MoveBy(-distance, 0, false);
			aBtn4->MoveBy(-distance, 0, false);
		}
	}
	else
		aBtn2->Hide();

	if ((rsrc_btn3 != NULL) && (::strlen(rsrc_btn3) != 0))
	{
		aBtn3->SetDescriptor(LStr255(rsrc_btn3));
		//aBtn3->AddAttachment(new CHelpAttach(STRx_ErrorButtons, btn3 + 1));

		// Resize this button and move buttons if required
		if (::strlen(rsrc_btn3) > 6)
		{
			short distance = cCharResize * (::strlen(rsrc_btn3) - 6);
			ResizeWindowBy(distance, 0);
			aBtn3->ResizeFrameBy(distance, 0, false);
			aBtn3->MoveBy(-distance, 0, false);
			aBtn4->MoveBy(-distance, 0, false);
		}
	}
	else
		aBtn3->Hide();

	if ((rsrc_btn4 != NULL) && (::strlen(rsrc_btn4) != 0))
	{
		aBtn4->SetDescriptor(LStr255(rsrc_btn4));
		//aBtn4->AddAttachment(new CHelpAttach(STRx_ErrorButtons, btn4 + 1));

		// Resize this button and move buttons if required
		if (::strlen(rsrc_btn4) > 6)
		{
			short distance = cCharResize * (::strlen(rsrc_btn4) - 6);
			ResizeWindowBy(distance, 0);
			aBtn4->ResizeFrameBy(distance, 0, false);
			aBtn4->MoveBy(-distance, 0, false);
		}
	}
	else
		aBtn4->Hide();

	LIconControl* anIcon = (LIconControl*) FindPaneByID(paneid_ErrorIcon);
	anIcon->SetResourceID(icon);

	// Insert extra text and leave as c-str in case > 255 chars
	if (extra1 && !extra2)
	{
		// Load in from resource if there
		if ((rsrc_text != NULL) && (::strlen(rsrc_text) != 0))
		{
			size_t txt_reserve = ::strlen(rsrc_text) + ::strlen(extra1) + 1;
			txt.reserve(txt_reserve);
			::snprintf(txt.c_str_mod(), txt_reserve, rsrc_text, extra1);
		}
		else
			// Just use extra as main text
			txt = extra1;
	}
	else if (extra1 && extra2)
	{
		// Load in from resource if there
		if ((rsrc_text != NULL) && (::strlen(rsrc_text) != 0))
		{
			size_t txt_reserve = ::strlen(rsrc_text) + ::strlen(extra1) + ::strlen(extra2) + 1;
			txt.reserve(txt_reserve);
			::snprintf(txt.c_str_mod(), txt_reserve, rsrc_text, extra1, extra2);
		}
		else
		{
			// Just use extras as main text
			txt = extra1;
			txt += extra2;
		}
	}
	else if (rsrc_text != NULL)
	{
		// Set alert message (convert to c-str)
		txt = rsrc_text;
	}

	// Give text to pane
	CTextDisplay* theText = (CTextDisplay*) FindPaneByID(paneid_ErrorText);
	theText->SetDefaultBackground(UGAColorRamp::GetColor(colorRamp_Gray1));
	theText->SetText(txt);

	// Increase size of window if message uses more than 3 lines
	if (theText->GetLineCount() > 3)
	{
		Rect frameRect;
		theText->CalcLocalFrameRect(frameRect);

		Rect theBounds;
		CalcPortFrameRect(theBounds);
		PortToGlobalPoint(topLeft(theBounds));
		PortToGlobalPoint(botRight(theBounds));
		theBounds.bottom += theText->GetFullHeight() - (frameRect.bottom - frameRect.top) + 6;
		DoSetBounds(theBounds);
	}
	
	// Do don't show checkbox
	if (dontshow != NULL)
	{
		LCheckBox* cbox = (LCheckBox*) FindPaneByID(paneid_ErrorDontShow);
		if (cbox && (txt_dontshow != NULL) && (*txt_dontshow != 0))
			cbox->SetDescriptor(LStr255(txt_dontshow));
	}
}

CErrorDialog::EDialogResult CErrorDialog::PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
														const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
														unsigned long cancel_id,
														const char* extra1, const char* extra2,
														const char* txt_dontshow, bool* dontshow, bool beep)
{
	EDialogResult result;

	// Create the dialog
	sOnScreen = true;

	{
		// Determine dialog type
		PaneIDT paneid = (dontshow != NULL) ? paneid_ErrorDontShowDialog : paneid_ErrorDialog;

		// Must auto destroy handler in case of exception
		std::auto_ptr<CBalloonDialog> theHandler;
		if (sErrorCritical)
			theHandler.reset(new CCriticalBalloonDialog(paneid, CMulberryApp::sApp));
		else
			theHandler.reset(new CBalloonDialog(paneid, CMulberryApp::sApp));
		theHandler->SetCritical(sErrorCritical);

		CErrorDialog* dlog = (CErrorDialog*) theHandler->GetDialog();

		// Get icon id
		ResIDT icon;
		switch(type)
		{
		case eErrDialog_Stop:
			icon = icnx_Stop;
			break;
		case eErrDialog_Note:
			icon = icnx_Note;
			break;
		case eErrDialog_Caution:
		default:
			icon = icnx_Caution;
			break;
		}

		const cdstring& title1 = rsrc::GetString(rsrc_btn1);
		const cdstring& title2 = rsrc::GetString(rsrc_btn2);
		const cdstring& title3 = rsrc::GetString(rsrc_btn3);
		const cdstring& title4 = rsrc::GetString(rsrc_btn4);
		const cdstring& main_text = rsrc::GetString(rsrc_text);

		// Set dlog info
		dlog->SetUpDetails(title1, title2, title3, title4, icon, main_text, extra1, extra2, txt_dontshow, dontshow);

		// Make sure cancel button is always 'esc'
		if (cancel_id == 1)
			static_cast<CErrorDialog*>(theHandler->GetDialog())->SetCancelButton(paneid_ErrorBtn1);
		else if (cancel_id == 2)
			static_cast<CErrorDialog*>(theHandler->GetDialog())->SetCancelButton(paneid_ErrorBtn2);
		else if (cancel_id == 3)
			static_cast<CErrorDialog*>(theHandler->GetDialog())->SetCancelButton(paneid_ErrorBtn3);
		else if(cancel_id == 4)
			static_cast<CErrorDialog*>(theHandler->GetDialog())->SetCancelButton(paneid_ErrorBtn4);

		theHandler->StartDialog();
		if (beep) ::SysBeep(1);

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true) {
			hitMessage = theHandler->DoDialog();

			if ((hitMessage == msg_Btn1) ||
				(hitMessage == msg_Btn2) ||
				(hitMessage == msg_Btn3) ||
				(hitMessage == msg_Btn4))
				break;
		}

		switch(hitMessage)
		{
		case msg_Btn1:
			result = eBtn1;
			break;
		case msg_Btn2:
			result = eBtn2;
			break;
		case msg_Btn3:
			result = eBtn3;
			break;
		case msg_Btn4:
			result = eBtn4;
			break;
		}
		
		if (dontshow != NULL)
		{
			LCheckBox* cbox = (LCheckBox*) dlog->FindPaneByID(paneid_ErrorDontShow);
			if (cbox)
				*dontshow = (cbox->GetValue() == 1);
		}
	}

	sOnScreen = false;

	return result;
}

// Standard alert
CErrorDialog::EDialogResult CErrorDialog::Alert(const char* txt, bool beep)
{
	// Just map to NoteAlert
	return CErrorDialog::NoteAlert(txt, 0, NULL, beep);
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