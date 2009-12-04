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


// Header for CPrefsEditHeadFoot class

#ifndef __CPREFSEDITHEADFOOT__MULBERRY__
#define __CPREFSEDITHEADFOOT__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

// Classes

class CPreferences;

class CTextInputDisplay;
class CTextInputField;
class JXRadioGroup;
class JXStaticText;
class JXTextButton;

class CPrefsEditHeadFoot : public CDialogDirector
{

public:
	CPrefsEditHeadFoot(bool allow_file, JXDirector* supervisor);

	static bool		PoseDialog(cdstring& edit, cdstring& title, const CPreferences* prefs, bool allow_file);

protected:
// begin JXLayout1

    JXRadioGroup*      mUseFileGroup;
    JXTextButton*      mOkBtn;
    JXTextButton*      mCancelBtn;
    JXStaticText*      mRuler;
    CTextInputDisplay* mText;
    CTextInputField*   mFileName;
    JXTextButton*      mChooseFile;

// end JXLayout1
/* This one is not used
// begin JXLayout2

    JXTextButton*      mOkBtn;
    JXTextButton*      mCancelBtn;
    JXStaticText*      mRuler;
    CTextInputDisplay* mText;

// end JXLayout2
*/
	const CPreferences*	mCopyPrefs;
	bool				mAllowFile;
	cdstring			mOldText;

	virtual void	OnCreate();								// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);

private:
	cdstring*		mTextItem;								// Prefs field

	void ResizeToWrap(const CPreferences* prefs);					// Resize Window to wrap length

	void	SetEditorText(const cdstring& text);		// Set text in editor
	void	GetEditorText(cdstring& text);				// Get text from editor
	void	ChooseFile();
	void	UpdateFile();
	void	SetUseFile(bool use_file);
};

#endif
