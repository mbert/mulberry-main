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


// Source for CPrefsEditHeadFoot class

#include "CPrefsEditHeadFoot.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResizeNotifier.h"
#include "CRFC822.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextEngine.h"
#include "CTextField.h"
#include "CURL.h"

#include "cdfstream.h"

#include <JXChooseSaveFile.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextRadioButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

#include <strstream>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S E D I T H E A D F O O T
// __________________________________________________________________________________________________

// Default constructor
CPrefsEditHeadFoot::CPrefsEditHeadFoot(bool allow_file, JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
	mTextItem = NULL;
	mAllowFile = allow_file;
}

void CPrefsEditHeadFoot::OnCreate()
{
    JXWindow* window = new JXWindow(this, 500, mAllowFile ? 275 : 230, "");
    assert( window != NULL );
    SetWindow(window);

	if (mAllowFile)
	{
// begin JXLayout1

    CResizeNotifier* obj1 =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,275);
    assert( obj1 != NULL );

    mUseFileGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 95,40);
    assert( mUseFileGroup != NULL );
    mUseFileGroup->SetBorderWidth(0);

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 410,240, 70,25);
    assert( mOkBtn != NULL );
    mOkBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 320,240, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mRuler =
        new JXStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 480,30);
    assert( mRuler != NULL );

    mText =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,85, 480,140);
    assert( mText != NULL );

    JXTextRadioButton* obj2 =
        new JXTextRadioButton(0, "From File:", mUseFileGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,0, 90,20);
    assert( obj2 != NULL );

    JXTextRadioButton* obj3 =
        new JXTextRadioButton(1, "Enter Text:", mUseFileGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,20, 90,20);
    assert( obj3 != NULL );

    mFileName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,10, 290,20);
    assert( mFileName != NULL );

    mChooseFile =
        new JXTextButton("Choose...", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 400,8, 75,25);
    assert( mChooseFile != NULL );

// end JXLayout1

		ListenTo(obj1);
		ListenTo(mUseFileGroup);
		ListenTo(mChooseFile);
	}
	else
	{
// begin JXLayout2

    CResizeNotifier* obj1 =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,230);
    assert( obj1 != NULL );

    mOkBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 410,195, 70,25);
    assert( mOkBtn != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 320,195, 70,25);
    assert( mCancelBtn != NULL );

    mRuler =
        new JXStaticText("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 480,30);
    assert( mRuler != NULL );

    mText =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 10,40, 480,140);
    assert( mText != NULL );

// end JXLayout2
		mUseFileGroup = NULL;
		mChooseFile = NULL;

		ListenTo(obj1);
	}
	SetButtons(mOkBtn, mCancelBtn);

	// Make sure ruler does not word wrap
	mRuler->SetBreakCROnly(kTrue);
}

void CPrefsEditHeadFoot::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mUseFileGroup)
		{
			SetUseFile(index == 0);
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mChooseFile)
		{
			ChooseFile();
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

// Resize Window to wrap length
void CPrefsEditHeadFoot::ResizeToWrap(const CPreferences* prefs)
{
	// Set font in each text widget
	const SFontInfo& finfo = prefs->mDisplayTextFontInfo.GetValue();

    mRuler->SetFontName(finfo.fontname);
    mRuler->SetFontSize(finfo.size);
    mText->SetFontName(finfo.fontname);
    mText->SetFontSize(finfo.size);

	// Set spaces per tab
	mText->SetDefaultTabWidth(prefs->spaces_per_tab.GetValue());

	// Make wrap safe
	unsigned long wrap = prefs->wrap_length.GetValue();
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	unsigned long spaces = prefs->spaces_per_tab.GetValue();
	
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

// Set text in editor
void CPrefsEditHeadFoot::SetEditorText(const cdstring& text)
{
	if (mAllowFile && (::strncmpnocase(text.c_str(), cFileURLScheme, ::strlen(cFileURLScheme)) == 0))
	{
		// Set radio button if present
		if (mUseFileGroup)
			mUseFileGroup->SelectItem(0);

		// Decode the URL to local path
		cdstring fpath(text.c_str() + ::strlen(cFileURLScheme));
		cdstring temp(cURLHierarchy);
		::strreplace(fpath.c_str_mod(), temp, os_dir_delim);
		fpath.DecodeURL();
		mFileName->SetText(fpath);

		mText->SetReadOnly(true);
		UpdateFile();
	}
	else
	{
		// Set radio button if present
		if (mAllowFile)
		{
			mUseFileGroup->SelectItem(1);
			mFileName->SetText(cdstring::null_str);
		}

		// Put text into editor
		mText->SetText(text);
	}
}

// Get text from editor
void CPrefsEditHeadFoot::GetEditorText(cdstring& text)
{
	// Look for choice of file rather than text
	if (mAllowFile && (mUseFileGroup->GetSelectedItem() == 0))
	{
		// Convert path to URL & convert directories
		cdstring fpath = mFileName->GetText();
		fpath.EncodeURL(os_dir_delim);
		cdstring temp(os_dir_delim);
		::strreplace(fpath.c_str_mod(), temp, cURLHierarchy);
			
		// File spec uses file URL scheme at start
		text = cFileURLScheme;
		text += fpath;
	}
	else
	{
		text = mText->GetText();

		// Only add if non-zero
		if (text.length())
		{
			// Save old, and set current wrap length
			short old_wrap = CRFC822::GetWrapLength();
			CRFC822::SetWrapLength(mCopyPrefs->wrap_length.GetValue());

			// Create space for footer
			const char* p = CTextEngine::WrapLines(text, text.length(), CRFC822::GetWrapLength(), false);
			text = p;
			delete p;

			// Restore wrap length
			CRFC822::SetWrapLength(old_wrap);
		}
	}
}

// Set text in editor
void CPrefsEditHeadFoot::ChooseFile()
{
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Signature file to open:", NULL, &fname))
	{
		mFileName->SetText(cdstring(fname));

		// Now read the file and put data into text
		UpdateFile();
	}
}

// Now read the file and put data into text
void CPrefsEditHeadFoot::UpdateFile()
{
	cdstring fpath = mFileName->GetText();
	
	if (fpath.empty())
		mText->SetText(cdstring::null_str);
	else
	{
		// Open file and read content
		cdifstream fin(fpath, ios::in | ios::binary);
		std::ostrstream out;
		::StreamCopy(fin, out, 0, ::StreamLength(fin));
		out << std::ends;

		cdstring result;
		result.steal(out.str());
		result.ConvertEndl();
		mText->SetText(result);
	}
}

// Set text in editor
void CPrefsEditHeadFoot::SetUseFile(bool use_file)
{
	if (use_file)
	{
		mFileName->Activate();
		mChooseFile->Activate();
		mOldText = mText->GetText();
		mText->SetReadOnly(true);
		UpdateFile();
	}
	else
	{
		mFileName->Deactivate();
		mChooseFile->Deactivate();
		mText->SetReadOnly(false);
		mText->SetText(mOldText);
	}
}

bool CPrefsEditHeadFoot::PoseDialog(cdstring& edit, cdstring& title, const CPreferences* prefs, bool allow_file)
{
	bool result = false;

	CPrefsEditHeadFoot* dlog = new CPrefsEditHeadFoot(allow_file, JXGetApplication());
	dlog->mCopyPrefs = prefs;
	dlog->OnCreate();
	dlog->mTextItem = &edit;
	dlog->ResizeToWrap(prefs);
	dlog->GetWindow()->SetTitle(title);
	dlog->SetEditorText(edit);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetEditorText(edit);
		result = true;
		dlog->Close();
	}

	return result;
}
