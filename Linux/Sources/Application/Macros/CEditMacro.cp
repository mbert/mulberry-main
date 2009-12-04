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


// Source for CEditMacro class

#include "CEditMacro.h"

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResizeNotifier.h"
#include "CTextDisplay.h"
#include "CTextEngine.h"
#include "CTextField.h"
#include "CRFC822.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// Default constructor
CEditMacro::CEditMacro(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
	mTextItem = NULL;
}

// Default destructor
CEditMacro::~CEditMacro()
{
}

// Get details of sub-panes
void CEditMacro::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 420,260, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* obj1 =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 420,260);
    assert( obj1 != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 330,225, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 240,225, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mRuler =
        new JXStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 400,30);
    assert( mRuler != NULL );

    mText =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,70, 400,140);
    assert( mText != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 50,20);
    assert( obj2 != NULL );

    mName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,10, 250,20);
    assert( mName != NULL );

// end JXLayout
	window->SetTitle("Edit Macro");
	SetButtons(mOkBtn, mCancelBtn);

	// Make sure ruler does not word wrap
	mRuler->SetBreakCROnly(kTrue);

	ListenTo(obj1);
}

// Set text in editor
void CEditMacro::SetData(const cdstring& name, const cdstring& text)
{
	// Put name into edit field
	mName->SetText(name);

	// Put text into editor
	mText->SetText(text.c_str());
}

// Get text from editor
void CEditMacro::GetData(cdstring& name, cdstring& text)
{
	// Put name into edit field
	name = mName->GetText();

	// Copy info from panel into prefs
	text = mText->GetText();
}

// Set current wrap length
void CEditMacro::SetRuler()
{
	// Set font in each text widget
	const SFontInfo& finfo = CPreferences::sPrefs->mDisplayTextFontInfo.GetValue();

    mRuler->SetFontName(finfo.fontname);
    mRuler->SetFontSize(finfo.size);
    mText->SetFontName(finfo.fontname);
    mText->SetFontSize(finfo.size);

	// Set spaces per tab
	mText->SetDefaultTabWidth(CPreferences::sPrefs->spaces_per_tab.GetValue());

	unsigned long spaces = CPreferences::sPrefs->spaces_per_tab.GetValue();
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();

	// Make wrap safe
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	// Create ruler text
	char ruler_txt[256];
	
	// Top line of ruler
	char* p = ruler_txt;
	for(unsigned long i = 0; i<wrap; i++)
		*p++ = '-';
	*p++ = '\n';

	// Do top line text
	ruler_txt[0] = '<';
	ruler_txt[wrap - 1] = '>';

	cdstring wrapper = wrap;
	wrapper += " characters";
	
	if (wrapper.length() + 4 < wrap)
	{
		unsigned long start = (wrap - wrapper.length())/2;
		::memcpy(&ruler_txt[start], wrapper.c_str(), wrapper.length());
	}
	

	// Bottom line of ruler
	for(unsigned long i = 0; i < wrap; i++)
	{
		if (i % spaces)
			*p++ = '\'';
		else
			*p++ = '|';
	}
	*p++ = '\0';

	// Set ruler text
	mRuler->SetText(ruler_txt);

	// Now change window size
	JSize width_adjust = mRuler->GetBoundsWidth() - mRuler->GetApertureWidth();
	JSize height = mRuler->GetApertureHeight();
	JSize draw_height = mRuler->GetBoundsHeight();
	JSize height_adjust = (draw_height > height) ? draw_height - height : 0;

	AdjustSize(width_adjust, height_adjust);
}

bool CEditMacro::PoseDialog(cdstring& name, cdstring& macro)
{
	bool result = false;

	// Create the dialog
	CEditMacro* dlog = new CEditMacro(JXGetApplication());
	dlog->OnCreate();
	dlog->SetRuler();
	dlog->SetData(name, macro);

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetData(name, macro);
		dlog->Close();
		result = true;
	}

	return result;
}
