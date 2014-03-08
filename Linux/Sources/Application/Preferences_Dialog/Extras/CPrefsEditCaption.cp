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


// Source for CPrefsEditCaption class

#include "CPrefsEditCaption.h"

#include "CCaptionParser.h"
#include "CPreferences.h"
#include "CTextDisplay.h"
#include "CWindowStates.h"

#include <JXColormap.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// Default constructor
CPrefsEditCaption::CPrefsEditCaption(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
}

// Get details of sub-panes
void CPrefsEditCaption::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 560,345, "");
    assert( window != NULL );
    SetWindow(window);

    mBtn1 =
        new JXTextButton("My Name", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 80,20);
    assert( mBtn1 != NULL );

    mBtn2 =
        new JXTextButton("My Email", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,20, 80,20);
    assert( mBtn2 != NULL );

    mBtn3 =
        new JXTextButton("My Full", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,40, 80,20);
    assert( mBtn3 != NULL );

    mBtn4 =
        new JXTextButton("My First", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,60, 80,20);
    assert( mBtn4 != NULL );

    mBtn5 =
        new JXTextButton("Smart Name", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,0, 80,20);
    assert( mBtn5 != NULL );

    mBtn6 =
        new JXTextButton("Smart Email", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,20, 80,20);
    assert( mBtn6 != NULL );

    mBtn7 =
        new JXTextButton("Smart Full", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,40, 80,20);
    assert( mBtn7 != NULL );

    mBtn8 =
        new JXTextButton("Smart First", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,60, 80,20);
    assert( mBtn8 != NULL );

    mBtn9 =
        new JXTextButton("From Name", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,0, 80,20);
    assert( mBtn9 != NULL );

    mBtn10 =
        new JXTextButton("From Email", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,20, 80,20);
    assert( mBtn10 != NULL );

    mBtn11 =
        new JXTextButton("From Full", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,40, 80,20);
    assert( mBtn11 != NULL );

    mBtn12 =
        new JXTextButton("From First", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,60, 80,20);
    assert( mBtn12 != NULL );

    mBtn13 =
        new JXTextButton("To Name", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,0, 80,20);
    assert( mBtn13 != NULL );

    mBtn14 =
        new JXTextButton("To Email", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,20, 80,20);
    assert( mBtn14 != NULL );

    mBtn15 =
        new JXTextButton("To Full", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,40, 80,20);
    assert( mBtn15 != NULL );

    mBtn16 =
        new JXTextButton("To First", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 240,60, 80,20);
    assert( mBtn16 != NULL );

    mBtn17 =
        new JXTextButton("CC Name", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,0, 80,20);
    assert( mBtn17 != NULL );

    mBtn18 =
        new JXTextButton("CC Email", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,20, 80,20);
    assert( mBtn18 != NULL );

    mBtn19 =
        new JXTextButton("CC Full", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,40, 80,20);
    assert( mBtn19 != NULL );

    mBtn20 =
        new JXTextButton("CC First", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,60, 80,20);
    assert( mBtn20 != NULL );

    mBtn21 =
        new JXTextButton("Subject", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,0, 80,20);
    assert( mBtn21 != NULL );

    mBtn22 =
        new JXTextButton("Date Short", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,20, 80,20);
    assert( mBtn22 != NULL );

    mBtn23 =
        new JXTextButton("Date Long", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,40, 80,20);
    assert( mBtn23 != NULL );

    mBtn24 =
        new JXTextButton("Date Now", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,60, 80,20);
    assert( mBtn24 != NULL );

    mBtn25 =
        new JXTextButton("Page Number", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 480,0, 80,20);
    assert( mBtn25 != NULL );

    mSummary =
        new JXTextCheckbox("Print Summary Headers", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,80, 150,20);
    assert( mSummary != NULL );

    mOkBtn =
        new JXTextButton("OK", window,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 470,310, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", window,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 380,310, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mRevertBtn =
        new JXTextButton("Revert", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,310, 70,25);
    assert( mRevertBtn != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Header", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,85, 65,20);
    assert( obj1 != NULL );
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mMessageDraft1 =
        new JXRadioGroup(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,80, 170,20);
    assert( mMessageDraft1 != NULL );
    mMessageDraft1->SetBorderWidth(0);

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(1, "Message", mMessageDraft1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 80,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(2, "Draft", mMessageDraft1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,0, 70,20);
    assert( obj3 != NULL );

    mCursorTop =
        new JXTextCheckbox("Cursor at Top", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,80, 170,20);
    assert( mCursorTop != NULL );

    mUseBox1 =
        new JXTextCheckbox("Draw Box", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,80, 80,20);
    assert( mUseBox1 != NULL );

    mEditHeader =
        new CTextInputDisplay(window,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 0,100, 560,90);
    assert( mEditHeader != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Footer", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,195, 65,20);
    assert( obj4 != NULL );
    const JFontStyle obj4_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj4->SetFontStyle(obj4_style);

    mMessageDraft2 =
        new JXRadioGroup(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,190, 170,20);
    assert( mMessageDraft2 != NULL );
    mMessageDraft2->SetBorderWidth(0);

    JXTextRadioButton* obj5 =
        new JXTextRadioButton(1, "Message", mMessageDraft2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 80,20);
    assert( obj5 != NULL );

    JXTextRadioButton* obj6 =
        new JXTextRadioButton(2, "Draft", mMessageDraft2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,0, 70,20);
    assert( obj6 != NULL );

    mUseBox2 =
        new JXTextCheckbox("Draw Box", window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 320,190, 80,20);
    assert( mUseBox2 != NULL );

    mEditFooter =
        new CTextInputDisplay(window,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 0,210, 560,90);
    assert( mEditFooter != NULL );

// end JXLayout
	window->SetTitle("Edit Captions");
	SetButtons(mOkBtn, mCancelBtn);

	// Listen to all the items
	ListenTo(mBtn1);
	ListenTo(mBtn2);
	ListenTo(mBtn3);
	ListenTo(mBtn4);
	ListenTo(mBtn5);
	ListenTo(mBtn6);
	ListenTo(mBtn7);
	ListenTo(mBtn8);
	ListenTo(mBtn9);
	ListenTo(mBtn10);
	ListenTo(mBtn11);
	ListenTo(mBtn12);
	ListenTo(mBtn13);
	ListenTo(mBtn14);
	ListenTo(mBtn15);
	ListenTo(mBtn16);
	ListenTo(mBtn17);
	ListenTo(mBtn18);
	ListenTo(mBtn19);
	ListenTo(mBtn20);
	ListenTo(mBtn21);
	ListenTo(mBtn22);
	ListenTo(mBtn23);
	ListenTo(mBtn24);
	ListenTo(mBtn25);

	ListenTo(mMessageDraft1);
	ListenTo(mMessageDraft2);
	ListenTo(mRevertBtn);
}

void CPrefsEditCaption::Receive(JBroadcaster*	sender, const Message&	message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mMessageDraft1)
		{
			switch(index)
			{
			case 1:
				OnCaptionMessage1();
				break;
			case 2:
				OnCaptionDraft1();
				break;
			}
			return;
		}
		else if (sender == mMessageDraft2)
		{
			switch(index)
			{
			case 1:
				OnCaptionMessage2();
				break;
			case 2:
				OnCaptionDraft2();
				break;
			}
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mBtn1)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eMe]);
			return;
		}
		else if (sender == mBtn2)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eMeEmail]);
			return;
		}
		else if (sender == mBtn3)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eMeFull]);
			return;
		}
		else if (sender == mBtn4)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eMeFirst]);
			return;
		}
		else if (sender == mBtn5)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eSmart]);
			return;
		}
		else if (sender == mBtn6)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eSmartEmail]);
			return;
		}
		else if (sender == mBtn7)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eSmartFull]);
			return;
		}
		else if (sender == mBtn8)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eSmartFirst]);
			return;
		}
		else if (sender == mBtn9)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eFrom]);
			return;
		}
		else if (sender == mBtn10)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eFromEmail]);
			return;
		}
		else if (sender == mBtn11)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eFromFull]);
			return;
		}
		else if (sender == mBtn12)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eFromFirst]);
			return;
		}
		else if (sender == mBtn13)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eTo]);
			return;
		}
		else if (sender == mBtn14)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eToEmail]);
			return;
		}
		else if (sender == mBtn15)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eToFull]);
			return;
		}
		else if (sender == mBtn16)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eToFirst]);
			return;
		}
		else if (sender == mBtn17)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eCC]);
			return;
		}
		else if (sender == mBtn18)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eCCEmail]);
			return;
		}
		else if (sender == mBtn19)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eCCFull]);
			return;
		}
		else if (sender == mBtn20)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eCCFirst]);
			return;
		}
		else if (sender == mBtn21)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eSubject]);
			return;
		}
		else if (sender == mBtn22)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eDateSentShort]);
			return;
		}
		else if (sender == mBtn23)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eDateSentFull]);
			return;
		}
		else if (sender == mBtn24)
		{
			AddCaptionItem(cCaptions[CCaptionParser::eDateNow]);
			return;
		}
		else if (sender == mBtn25)
		{
			AddCaptionItem(cCaptions[CCaptionParser::ePage]);
			return;
		}
		else if (sender == mRevertBtn)
		{
			SetData(mText1Item, mText2Item, mText3Item, mText4Item, mCursorTopItem, mBox1Item, mBox2Item, mSummaryItem);
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

// Resize Window to wrap length
void CPrefsEditCaption::ResizeToWrap(CPreferences* prefs)
{
	// Set font in each text widget
	const SFontInfo& finfo = prefs->mDisplayTextFontInfo.GetValue();

    mEditHeader->SetFontName(finfo.fontname);
    mEditHeader->SetFontSize(finfo.size);
    mEditFooter->SetFontName(finfo.fontname);
    mEditFooter->SetFontSize(finfo.size);

	// Set spaces per tab
	mEditHeader->SetDefaultTabWidth(prefs->spaces_per_tab.GetValue());
	mEditFooter->SetDefaultTabWidth(prefs->spaces_per_tab.GetValue());

	unsigned long wrap = prefs->wrap_length.GetValue();
	
#ifdef NOTYET
	// Create ruler text
	char ruler_txt[256];
	
	// Clip wrap to fit in 256
	if (wrap > 120) wrap = 120;

	// Top line of ruler
	char* p = ruler_txt;
	for(unsigned long i = 1; i <= wrap; i++)
	{
		if ((i % 10 == 0) && (i != 0))
			*p++ = ((unsigned long) i/10) + '0';
		else
			*p++ = ' ';
	}
	*p++ = '\n';
	
	// Bottom line of ruler
	for(unsigned long i = 1; i <= wrap; i++)
		*p++ = (i % 10) + '0';
	*p++ = '\0';

	// Set ruler text
	mRuler->SetText(ruler_txt);

	// Now change window size
	JSize width_adjust = mRuler->GetBoundsWidth() - mRuler->GetApertureWidth();
	JSize height = mRuler->GetApertureHeight();
	JSize draw_height = mRuler->GetBoundsHeight();
	JSize height_adjust = (draw_height > height) ? draw_height - height : 0;

	AdjustSize(width_adjust, height_adjust);
#endif
}

// Handle buttons
void CPrefsEditCaption::AddCaptionItem(const char* item)
{
	cdstring add_text = "%";
	add_text += item;
	if (mEditHeader->HasFocus())
		mEditHeader->InsertUTF8(add_text);
	else if (mEditFooter->HasFocus())
		mEditFooter->InsertUTF8(add_text);
}

void CPrefsEditCaption::OnCaptionMessage1()
{
	UpdateDisplay1();
	SetDisplay1(true);
	showing1 = true;
}

void CPrefsEditCaption::OnCaptionDraft1()
{
	UpdateDisplay1();
	SetDisplay1(false);
	showing1 = false;
}

void CPrefsEditCaption::OnCaptionMessage2()
{
	UpdateDisplay2();
	SetDisplay2(true);
	showing3 = true;
}

void CPrefsEditCaption::OnCaptionDraft2()
{
	UpdateDisplay2();
	SetDisplay2(false);
	showing3 = false;
}

// Set text in editor
void CPrefsEditCaption::SetData(CPreferenceValueMap<cdstring>* txt1,
								CPreferenceValueMap<cdstring>* txt2,
								CPreferenceValueMap<cdstring>* txt3,
								CPreferenceValueMap<cdstring>* txt4,
								CPreferenceValueMap<bool>* cursor_top,
								CPreferenceValueMap<bool>* use_box1,
								CPreferenceValueMap<bool>* use_box2,
								CPreferenceValueMap<bool>* summary)
{
	// Cache ptr to prefs item
	mText1Item = txt1;
	mText2Item = txt2;
	mText3Item = txt3;
	mText4Item = txt4;
	mCursorTopItem = cursor_top;
	mBox1Item = use_box1;
	mBox2Item = use_box2;
	mSummaryItem = summary;

	// Set box item if present
	if (mBox1Item && mBox2Item)
	{
		mUseBox1->SetState(JBoolean(mBox1Item->GetValue()));
		mUseBox2->SetState(JBoolean(mBox2Item->GetValue()));
		mCursorTop->Hide();
	}
	else
	{
		mCursorTop->SetState(JBoolean(mCursorTopItem->GetValue()));
		mBtn25->Hide();
		mUseBox1->Hide();
		mUseBox2->Hide();
	}

	// Set summary item if present
	if (mSummaryItem)
		mSummary->SetState(JBoolean(mSummaryItem->GetValue()));
	else
		mSummary->Hide();

	// Keep copy of text
	text1 = mText1Item->GetValue();
	if (mText2Item)
	{
		text2 = mText2Item->GetValue();
		mMessageDraft1->SelectItem(1);
	}
	else
		mMessageDraft1->Hide();

	text3 = mText3Item->GetValue();
	if (mText4Item)
	{
		text4 = mText4Item->GetValue();
		mMessageDraft2->SelectItem(1);
	}
	else
		mMessageDraft2->Hide();

	// Update items
	showing1 = true;
	showing3 = true;
	SetDisplay2(true);
	SetDisplay1(true);
}

// Get text from editor
void CPrefsEditCaption::GetEditorText()
{
	// Get box item if present
	if (mCursorTopItem)
		mCursorTopItem->SetValue(mCursorTop->IsChecked());
	if (mBox1Item)
		mBox1Item->SetValue(mUseBox1->IsChecked());
	if (mBox2Item)
		mBox2Item->SetValue(mUseBox2->IsChecked());
	if (mSummaryItem)
		mSummaryItem->SetValue(mSummary->IsChecked());

	// Update existing text
	UpdateDisplay1();
	UpdateDisplay2();

	// Copy info from panel into prefs
	mText1Item->SetValue(text1);
	if (mText2Item)
		mText2Item->SetValue(text2);
	mText3Item->SetValue(text3);
	if (mText4Item)
		mText4Item->SetValue(text4);
}

void CPrefsEditCaption::SetDisplay1(bool show1)
{
	cdstring& item = (show1 ? text1 : text2);

	// Put text into editor
	if (item.empty())
		mEditHeader->SetText(cdstring::null_str);
	else
		mEditHeader->SetText(item.c_str());

	// Set selection at start
	mEditHeader->Focus();
}

void CPrefsEditCaption::UpdateDisplay1()
{
	cdstring& item = (showing1 ? text1 : text2);

	// Copy info from panel into prefs
	item = mEditHeader->GetText();
}

void CPrefsEditCaption::SetDisplay2(bool show3)
{
	cdstring& item = (show3 ? text3 : text4);

	// Put text into editor
	if (item.empty())
		mEditFooter->SetText(cdstring::null_str);
	else
		mEditFooter->SetText(item.c_str());

	// Set selection at start
	mEditFooter->Focus();
}

void CPrefsEditCaption::UpdateDisplay2()
{
	cdstring& item = (showing3 ? text3 : text4);

	// Copy info from panel into prefs
	item = mEditFooter->GetText();
}

void CPrefsEditCaption::PoseDialog(CPreferences* prefs,
									CPreferenceValueMap<cdstring>* txt1,
									CPreferenceValueMap<cdstring>* txt2,
									CPreferenceValueMap<cdstring>* txt3,
									CPreferenceValueMap<cdstring>* txt4,
									CPreferenceValueMap<bool>* cursor_top,
									CPreferenceValueMap<bool>* use_box1,
									CPreferenceValueMap<bool>* use_box2,
									CPreferenceValueMap<bool>* summary)
{
	CPrefsEditCaption* dlog = new CPrefsEditCaption(JXGetApplication());
	dlog->OnCreate();
	dlog->SetData(txt1, txt2, txt3, txt4, cursor_top, use_box1, use_box2, summary);
	dlog->ResizeToWrap(prefs);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetEditorText();
		dlog->Close();
	}
}
