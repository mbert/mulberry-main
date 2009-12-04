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

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CResizeNotifier.h"
#include "CStaticText.h"
#include "CXStringResources.h"

#include <JXUpRect.h>
#include <JXWindow.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXImageWidget.h>
#include <JXImage.h>

#include <jXGlobals.h>

#include <jx_un_error.xpm>
#include <jx_un_message.xpm>
#include <jx_un_warning.xpm>

bool CErrorDialog::sOnScreen = false;

const unsigned long cButtonTextLong = 9;
const unsigned long cButtonCharWidth = 6;

////////////////////////////////////////////////////////////////////////////
// CErrorDialog dialog

CErrorDialog::CErrorDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mDontShow = NULL;
}

CErrorDialog::~CErrorDialog()
{
}

void CErrorDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 410,150, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 410,150);
    assert( container != NULL );

    mBtn1Ctrl =
        new JXTextButton("btn1", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 315,115, 80,25);
    assert( mBtn1Ctrl != NULL );
    mBtn1Ctrl->SetShortcuts("^M");

    mBtn2Ctrl =
        new JXTextButton("btn2", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 215,115, 80,25);
    assert( mBtn2Ctrl != NULL );
    mBtn2Ctrl->SetShortcuts("^[");

    mBtn3Ctrl =
        new JXTextButton("btn3", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 115,115, 80,25);
    assert( mBtn3Ctrl != NULL );

    mBtn4Ctrl =
        new JXTextButton("btn4", container,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 15,115, 80,25);
    assert( mBtn4Ctrl != NULL );

    mIconCtrl =
        new JXImageWidget(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,10, 35,35);
    assert( mIconCtrl != NULL );

    mTextCtrl =
        new CStaticText("text", container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,10, 330,70);
    assert( mTextCtrl != NULL );

    mDontShowCtrl =
        new JXTextCheckbox("Don't show this alert from now on", container,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 65,85, 290,20);
    assert( mDontShowCtrl != NULL );

// end JXLayout

	mTextCtrl->SetBreakCROnly(false);

	ListenTo(container);
	ListenTo(mBtn1Ctrl);
	ListenTo(mBtn2Ctrl);
	ListenTo(mBtn3Ctrl);
	ListenTo(mBtn4Ctrl);
	ListenTo(mDontShowCtrl);

	if (mCancelBtn == 1)
		SetButtons(mBtn1Ctrl, mBtn1Ctrl);
	else if (mCancelBtn == 2)
		SetButtons(mBtn1Ctrl, mBtn2Ctrl);
	else if (mCancelBtn == 3)
		SetButtons(mBtn1Ctrl, mBtn3Ctrl);
	else if (mCancelBtn == 4)
		SetButtons(mBtn1Ctrl, mBtn4Ctrl);
}

void CErrorDialog::Receive (JBroadcaster* sender, const Message&	message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mBtn1Ctrl)
		{
			EndDialog(kDialogClosed_Btn1);
		}
		else if (sender == mBtn2Ctrl)
		{
			EndDialog(kDialogClosed_Btn2);
		}
		else if (sender == mBtn3Ctrl)
		{
			EndDialog(kDialogClosed_Btn3);
		}
		else if (sender == mBtn4Ctrl)
		{
			EndDialog(kDialogClosed_Btn4);
		}

		return;
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if ((sender == mDontShowCtrl) && (mDontShow != NULL))
		{
			*mDontShow = mDontShowCtrl->IsChecked();
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

void CErrorDialog::BuildWindow(CErrorDialog::EErrDialogType type,
					const char* btn1, const char* btn2,
					const char* btn3, const char* btn4,
					const char* rsrc_text,
					const char* extra1, const char* extra2,
					const char* txt_dontshow, bool* dontshow, bool beep)
{
	JSize width_resize = 0;

	if (btn1)
	{
		cdstring s(btn1);
		if (s.length())
		{
			mBtn1Ctrl->SetLabel(s.c_str());

			// Resize this button
			if (s.length() > cButtonTextLong)
			{
				JSize distance = cButtonCharWidth * (s.length() - cButtonTextLong);
				width_resize += distance;
				mBtn1Ctrl->AdjustSize(distance, 0);
				mBtn1Ctrl->Move(-distance, 0);
				mBtn2Ctrl->Move(-distance, 0);
				mBtn3Ctrl->Move(-distance, 0);
				mBtn4Ctrl->Move(-distance, 0);
			}
		}
		else
			mBtn1Ctrl->Hide();
	}
	else
		mBtn1Ctrl->Hide();
	
	if (btn2)
	{
		cdstring s(btn2);
		if (s.length())
		{
			mBtn2Ctrl->SetLabel(s.c_str());

			// Resize this button
			if (s.length() > cButtonTextLong)
			{
				JSize distance = cButtonCharWidth * (s.length() - cButtonTextLong);
				width_resize += distance;
				mBtn2Ctrl->AdjustSize(distance, 0);
				mBtn2Ctrl->Move(-distance, 0);
				mBtn3Ctrl->Move(-distance, 0);
				mBtn4Ctrl->Move(-distance, 0);
			}
		}
		else
			mBtn2Ctrl->Hide();
	}
	else
		mBtn2Ctrl->Hide();

	if (btn3)
	{
		cdstring s(btn3);
		if (s.length())
		{
			mBtn3Ctrl->SetLabel(s.c_str());

			// Resize this button
			if (s.length() > cButtonTextLong)
			{
				JSize distance = cButtonCharWidth * (s.length() - cButtonTextLong);
				width_resize += distance;
				mBtn3Ctrl->AdjustSize(distance, 0);
				mBtn3Ctrl->Move(-distance, 0);
				mBtn4Ctrl->Move(-distance, 0);
			}
		}
		else
			mBtn3Ctrl->Hide();
	}
	else
		mBtn3Ctrl->Hide();

	if (btn4)
	{
		cdstring s(btn4);
		if (s.length())
		{
			mBtn4Ctrl->SetLabel(s.c_str());

			// Resize this button
			if (s.length() > cButtonTextLong)
			{
				JSize distance = cButtonCharWidth * (s.length() - cButtonTextLong);
				width_resize += distance;
				mBtn4Ctrl->AdjustSize(distance, 0);
				mBtn4Ctrl->Move(-distance, 0);
			}
		}
		else
			mBtn4Ctrl->Hide();
	}
	else
		mBtn4Ctrl->Hide();
	
	// Set icon
	JXImage* image;
  	cdstring title;
	switch(type)
	{
	// The JX names for the icons doesn't really match, but the images use
	// match up with what images were used on windows.
	case CErrorDialog::eErrDialog_Stop:
		image = new JXImage(GetDisplay(), GetWindow()->GetColormap(), JXPM(jx_un_warning));// This gets the hand
		title.FromResource(IDS_DialogError);
		break;
	case CErrorDialog::eErrDialog_Note:
		image = new JXImage(GetDisplay(), GetWindow()->GetColormap(), JXPM(jx_un_message));// hmm... well I guess this works 
		title.FromResource(IDS_DialogNote);
		break;
	case CErrorDialog::eErrDialog_Caution:
	default:
		image = new JXImage(GetDisplay(), GetWindow()->GetColormap(), JXPM(jx_un_error));// The exlamation
		title.FromResource(IDS_DialogWarning);
		break;
	}
	mIconCtrl->SetImage(image, kTrue);
	GetWindow()->SetTitle(title.c_str());

	// Insert extra text and leave as c-str in case > 255 chars
	cdstring txt;
	if (extra1 && !extra2)
	{
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
		if ((rsrc_text != NULL) && (::strlen(rsrc_text) != 0))
		{
			size_t txt_reserve = ::strlen(rsrc_text) + ::strlen(extra1) + ::strlen(extra2) + 1;
			txt.reserve(txt_reserve);
			::snprintf(txt.c_str_mod(), txt_reserve, rsrc_text, extra1, extra2);
		}
		else
		{
			// Just use extra as main text
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
	mTextCtrl->SetText(txt.c_str());
	
	// Do don't show checkbox
	if (dontshow != NULL)
	{
		if ((txt_dontshow != NULL) && (*txt_dontshow != 0))
			mDontShowCtrl->SetLabel(txt_dontshow);
	}
	else
		mDontShowCtrl->Hide();
	
	// Adjust height of window for size of text
	JSize height_resize = 0;
	{
		JSize txt_lines = mTextCtrl->GetLineCount();
		unsigned long txt_height = mTextCtrl->GetLineTop(txt_lines) + mTextCtrl->GetLineHeight(txt_lines);
		if (txt_height + 8UL > mTextCtrl->GetFrameHeight())
		{
			// Just resize main window - child windows will resize/move as appropriate
			height_resize = txt_height + 8 - mTextCtrl->GetFrameHeight();
		}
	}
	
	if ((width_resize != 0) || (height_resize != 0))
		AdjustSize(width_resize, height_resize);
}

CErrorDialog::EDialogResult CErrorDialog::PoseDialog(EErrDialogType type, const char* rsrc_btn1, const char* rsrc_btn2,
								const char* rsrc_btn3, const char* rsrc_btn4, const char* rsrc_text,
								unsigned long cancel_id, const char* extra1, const char* extra2, const char* txt_dontshow, bool* dontshow, bool beep)
{
	const cdstring& title1 = rsrc::GetString(rsrc_btn1);
	const cdstring& title2 = rsrc::GetString(rsrc_btn2);
	const cdstring& title3 = rsrc::GetString(rsrc_btn3);
	const cdstring& title4 = rsrc::GetString(rsrc_btn4);
	const cdstring& main_text = rsrc::GetString(rsrc_text);

	CErrorDialog* dlog = new CErrorDialog(JXGetApplication());
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
