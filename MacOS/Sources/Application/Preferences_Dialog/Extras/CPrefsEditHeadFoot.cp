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


#include "CBalloonDialog.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CStaticText.h"
#include "CStreamUtils.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextEngine.h"
#include "CTextFieldX.h"
#include "CURL.h"

#include "FullPath.h"

#include "cdfstream.h"

#include <LCaption.h>
#include <LPushButton.h>
#include <LRadioButton.h>
#include <UStandardDialogs.h>

#include <strstream>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S E D I T H E A D F O O T
// __________________________________________________________________________________________________

// Default constructor
CPrefsEditHeadFoot::CPrefsEditHeadFoot()
{
}

// Constructor from stream
CPrefsEditHeadFoot::CPrefsEditHeadFoot(LStream *inStream) :
				LDialogBox(inStream)
{
}

// Default destructor
CPrefsEditHeadFoot::~CPrefsEditHeadFoot()
{
}

// Get details of sub-panes
void CPrefsEditHeadFoot::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get controls
	mUseFile = (LRadioButton*) FindPaneByID(paneid_PrefsEditHeadFootUseFile);
	mUseText = (LRadioButton*) FindPaneByID(paneid_PrefsEditHeadFootUseText);
	mFileName = (CTextFieldX*) FindPaneByID(paneid_PrefsEditHeadFootFile);
	mChooseBtn = (LPushButton*) FindPaneByID(paneid_PrefsEditHeadFootChooseFile);
	mText = (CTextDisplay*) FindPaneByID(paneid_PrefsEditHeadFootText);
	mRevertBtn = (LPushButton*) FindPaneByID(paneid_PrefsEditHeadFootRevert);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, GetPaneID());

}

// Handle buttons
void CPrefsEditHeadFoot::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_PrefsEditHeadChooseFile:
		ChooseFile();
		break;
	case msg_PrefsEditHeadUseFile:
		if (*(long*)ioParam)
			SetUseFile(true);
		break;
	case msg_PrefsEditHeadUseText:
		if (*(long*)ioParam)
			SetUseFile(false);
		break;
	case msg_PrefsEditHeadRevert:
		// Set it back to what it was
		SetEditorText(mRevertText);
		break;
	}
}

// Set text in editor
void CPrefsEditHeadFoot::SetEditorText(const cdstring& text)
{
	if (mAllowFile && (::strncmpnocase(text.c_str(), cFileURLScheme, ::strlen(cFileURLScheme)) == 0))
	{
		// Set radio button if present
		if (mUseFile)
			mUseFile->SetValue(1);

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
			mUseText->SetValue(1);
			mFileName->SetText(cdstring::null_str);
		}

		// Cache ptr to prefs item
		mRevertText = text;

		// Put text into editor
		mText->SetText(text);

		mText->GetSuperCommander()->SetLatentSub(mText);
		SwitchTarget(mText);
	}

}

// Get text from editor
void CPrefsEditHeadFoot::GetEditorText(cdstring& text)
{
	// Look for choice of file rather than text
	if (mAllowFile && mUseFile->GetValue())
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
		// Copy info from panel into prefs
		mText->GetText(text);

		// Only add if non-zero
		if (text.length())
		{
			// Save old, and set current wrap length
			short old_wrap = CRFC822::GetWrapLength();
			CRFC822::SetWrapLength(mWrap);

			text.steal(const_cast<char*>(CTextEngine::WrapLines(text, text.length(), CRFC822::GetWrapLength(), false)));

			// Restore wrap length
			CRFC822::SetWrapLength(old_wrap);
		}
	}
}

// Set current wrap length
void CPrefsEditHeadFoot::SetRuler(unsigned long wrap, unsigned long spaces)
{
	mText->SetSpacesPerTab(spaces);

	// Make wrap safe
	if ((wrap == 0) || (wrap > 120))
		wrap = 120;

	// Cache value
	mWrap = wrap;

	// Resize to new wrap length
	short change_by = 6*(wrap - 80);
	Rect minmax;
	GetMinMaxSize(minmax);
	minmax.left += change_by;
	minmax.right += change_by;
	SetMinMaxSize(minmax);
	minmax = mUserBounds;
	minmax.right += change_by;
	DoSetBounds(minmax);

	// Get ruler
	CStaticText* ruler = (CStaticText*) FindPaneByID(paneid_PrefsEditHeadFootRuler);

	// Create ruler text
	char ruler_txt[256];

	// Clip wrap to fit in 256
	if (wrap > 120) wrap = 120;

	// Top line of ruler
	for(short i = 0; i<wrap; i++)
		ruler_txt[i] = '-';
	ruler_txt[wrap] = '\r';

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
	for(short i = 0; i < wrap; i++)
	{
		if (i % spaces)
			ruler_txt[wrap + i + 1] = '\'';
		else
			ruler_txt[wrap + i + 1] = '|';
	}
	ruler_txt[2*wrap+1] = '\0';

	// Set ruler text
	ruler->SetText(ruler_txt);

}

// Set text in editor
void CPrefsEditHeadFoot::ChooseFile()
{
	// Get folder from user
	PPx::FSObject fspec;
	if (PP_StandardDialogs::AskChooseOneFile(0, fspec, kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews))
	{
		cdstring temp(fspec.GetPath());
		mFileName->SetText(temp);

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
		cdifstream fin(fpath, std::ios::in | std::ios::binary);
		std::ostrstream out;
		::StreamCopy(fin, out, 0, ::StreamLength(fin));
		
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
		mFileName->Enable();
		mChooseBtn->Enable();
		mText->GetText(mOldText);
		mText->SetReadOnly(true);
		mRevertBtn->Disable();
		UpdateFile();
	}
	else
	{
		mFileName->Disable();
		mChooseBtn->Disable();
		mText->SetReadOnly(false);
		mText->SetText(mOldText);
		mRevertBtn->Enable();
	}
}

bool CPrefsEditHeadFoot::PoseDialog(const cdstring title, cdstring& text, bool allow_file, short spaces, short wrap)
{
	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(allow_file ? paneid_PrefsEditHeadFootFileDialog : paneid_PrefsEditHeadFootDialog, CMulberryApp::sApp);
		CPrefsEditHeadFoot* dlog = (CPrefsEditHeadFoot*) theHandler.GetDialog();
		dlog->SetAllowFile(allow_file);
		dlog->SetDescriptor(LStr255(title));
		dlog->SetRuler(wrap, spaces);
		dlog->SetEditorText(text);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				dlog->GetEditorText(text);
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
