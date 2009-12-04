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

#include "CBalloonDialog.h"
#include "CCaptionParser.h"
#include "CMulberryApp.h"
#include "CPreferenceValue.h"
#include "CTextDisplay.h"

#include <LCheckBox.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S E D I T H E A D F O O T
// __________________________________________________________________________________________________

// Default constructor
CPrefsEditCaption::CPrefsEditCaption()
{
}

// Constructor from stream
CPrefsEditCaption::CPrefsEditCaption(LStream *inStream) :
				LDialogBox(inStream)
{
}

// Default destructor
CPrefsEditCaption::~CPrefsEditCaption()
{
}

// Get details of sub-panes
void CPrefsEditCaption::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	mCursorTop = (LCheckBox*) FindPaneByID(paneid_PrefsEditCaptionCursorTop);
	mUseBox1 = (LCheckBox*) FindPaneByID(paneid_PrefsEditCaptionBox1);
	mSummary = (LCheckBox*) FindPaneByID(paneid_PrefsEditCaptionSummary);
	mText1 = (CTextDisplay*) FindPaneByID(paneid_PrefsEditCaptionText1);
	mUseBox2 = (LCheckBox*) FindPaneByID(paneid_PrefsEditCaptionBox2);
	mText2 = (CTextDisplay*) FindPaneByID(paneid_PrefsEditCaptionText2);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsEditCaptionBtns);

}

// Handle buttons
void CPrefsEditCaption::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	cdstring add_text = "%";

	CTextDisplay* text_insert = (mText1->IsOnDuty() ? mText1 : mText2);
	switch (inMessage)
	{
		case msg_PrefsEditCaptionMeName:
			add_text += cCaptions[CCaptionParser::eMe];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionMeEmail:
			add_text += cCaptions[CCaptionParser::eMeEmail];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionMeFull:
			add_text += cCaptions[CCaptionParser::eMeFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionMeFirst:
			add_text += cCaptions[CCaptionParser::eMeFirst];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionSmartName:
			add_text += cCaptions[CCaptionParser::eSmart];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionSmartEmail:
			add_text += cCaptions[CCaptionParser::eSmartEmail];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionSmartFull:
			add_text += cCaptions[CCaptionParser::eSmartFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionSmartFirst:
			add_text += cCaptions[CCaptionParser::eSmartFirst];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionFromName:
			add_text += cCaptions[CCaptionParser::eFrom];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionFromEmail:
			add_text += cCaptions[CCaptionParser::eFromEmail];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionFromFull:
			add_text += cCaptions[CCaptionParser::eFromFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionFromFirst:
			add_text += cCaptions[CCaptionParser::eFromFirst];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionToName:
			add_text += cCaptions[CCaptionParser::eTo];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionToEmail:
			add_text += cCaptions[CCaptionParser::eToEmail];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionToFull:
			add_text += cCaptions[CCaptionParser::eToFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionToFirst:
			add_text += cCaptions[CCaptionParser::eToFirst];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionCCName:
			add_text += cCaptions[CCaptionParser::eCC];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionCCEmail:
			add_text += cCaptions[CCaptionParser::eCCEmail];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionCCFull:
			add_text += cCaptions[CCaptionParser::eCCFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionCCFirst:
			add_text += cCaptions[CCaptionParser::eCCFirst];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionSubject:
			add_text += cCaptions[CCaptionParser::eSubject];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionDateShort:
			add_text += cCaptions[CCaptionParser::eDateSentShort];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionDateLong:
			add_text += cCaptions[CCaptionParser::eDateSentFull];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionNow:
			add_text += cCaptions[CCaptionParser::eDateNow];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionPage:
			add_text += cCaptions[CCaptionParser::ePage];
			text_insert->FocusDraw();
			text_insert->InsertUTF8(add_text);
			break;
		case msg_PrefsEditCaptionRevert:
			// Set it back to what it was
			SetData(mText1Item, mText2Item, mText3Item, mText4Item, mCursorTopItem, mBox1Item, mBox2Item, mSummaryItem);
			break;
		case msg_PrefsEditCaptionButton1:
			if (*((long*) ioParam))
			{
				UpdateDisplay1();
				SetDisplay1(true);
				showing1 = true;
			}
			break;
		case msg_PrefsEditCaptionButton2:
			if (*((long*) ioParam))
			{
				UpdateDisplay1();
				SetDisplay1(false);
				showing1 = false;
			}
			break;
		case msg_PrefsEditCaptionButton3:
			if (*((long*) ioParam))
			{
				UpdateDisplay2();
				SetDisplay2(true);
				showing3 = true;
			}
			break;
		case msg_PrefsEditCaptionButton4:
			if (*((long*) ioParam))
			{
				UpdateDisplay2();
				SetDisplay2(false);
				showing3 = false;
			}
			break;
	}
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
		mUseBox1->SetValue(mBox1Item->GetValue());
		mUseBox2->SetValue(mBox2Item->GetValue());
		mCursorTop->Hide();
	}
	else
	{
		mCursorTop->SetValue(mCursorTopItem->GetValue());
		FindPaneByID(paneid_PrefsEditCaptionPage)->Hide();
		mUseBox1->Hide();
		mUseBox2->Hide();
	}

	// Set summary item if present
	if (mSummaryItem)
		mSummary->SetValue(mSummaryItem->GetValue());
	else
		mSummary->Hide();

	// Keep copy of text
	text1 = mText1Item->GetValue();
	if (mText2Item)
		text2 = mText2Item->GetValue();
	else
	{
		FindPaneByID(paneid_PrefsEditCaptionButton1)->Hide();
		FindPaneByID(paneid_PrefsEditCaptionButton2)->Hide();
	}
	text3 = mText3Item->GetValue();
	if (mText4Item)
		text4 = mText4Item->GetValue();
	else
	{
		FindPaneByID(paneid_PrefsEditCaptionButton3)->Hide();
		FindPaneByID(paneid_PrefsEditCaptionButton4)->Hide();
	}

	// Update items
	showing1 = true;
	showing3 = true;
	SetDisplay2(true);
	SetDisplay1(true);
}

// Get text from editor
void CPrefsEditCaption::GetEditorText(void)
{
	// Get box item if present
	if (mCursorTopItem)
		mCursorTopItem->SetValue(mCursorTop->GetValue());
	if (mBox1Item)
		mBox1Item->SetValue(mUseBox1->GetValue());
	if (mBox2Item)
		mBox2Item->SetValue(mUseBox2->GetValue());
	if (mSummaryItem)
		mSummaryItem->SetValue(mSummary->GetValue());

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

// Set current set spaces per tab
void CPrefsEditCaption::SetCurrentSpacesPerTab(short num)
{
	mText1->SetSpacesPerTab(num);
	mText2->SetSpacesPerTab(num);
}

void CPrefsEditCaption::SetDisplay1(bool show1)
{
	cdstring& item = (show1 ? text1 : text2);

	// Put text into editor
	mText1->SetText(item);

	SwitchTarget(mText1);
}

void CPrefsEditCaption::UpdateDisplay1()
{
	cdstring& item = (showing1 ? text1 : text2);

	// Copy info from panel into prefs
	mText1->GetText(item);
}

void CPrefsEditCaption::SetDisplay2(bool show3)
{
	cdstring& item = (show3 ? text3 : text4);

	// Put text into editor
	mText2->SetText(item);

	SwitchTarget(mText2);
}

void CPrefsEditCaption::UpdateDisplay2()
{
	cdstring& item = (showing3 ? text3 : text4);

	// Copy info from panel into prefs
	mText2->GetText(item);
}

bool CPrefsEditCaption::PoseDialog(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary,
							unsigned long spaces)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_PrefsEditCaptionDialog, CMulberryApp::sApp);
	CPrefsEditCaption* dlog = (CPrefsEditCaption*) theHandler.GetDialog();
	dlog->SetData(txt1, txt2, txt3, txt4, cursor_top, use_box1, use_box2, summary);
	dlog->SetCurrentSpacesPerTab(spaces);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			dlog->GetEditorText();
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
