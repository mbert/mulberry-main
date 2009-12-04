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

#include <LDialogBox.h>

#include "cdstring.h"


// Panes
const	PaneIDT		paneid_PrefsEditHeadFootDialog = 5100;
const	PaneIDT		paneid_PrefsEditHeadFootFileDialog = 5101;
const	PaneIDT		paneid_PrefsEditHeadFootFileHeader = 'FHDR';
const	PaneIDT		paneid_PrefsEditHeadFootUseFile = 'UFIL';
const	PaneIDT		paneid_PrefsEditHeadFootUseText = 'UTXT';
const	PaneIDT		paneid_PrefsEditHeadFootFile = 'FILE';
const	PaneIDT		paneid_PrefsEditHeadFootChooseFile = 'CHOS';
const	PaneIDT		paneid_PrefsEditHeadFootHeader = 'MHDR';
const	PaneIDT		paneid_PrefsEditHeadFootRuler = 'RULE';
const	PaneIDT		paneid_PrefsEditHeadFootScroller = 'SCRL';
const	PaneIDT		paneid_PrefsEditHeadFootText = 'TEXT';
const	PaneIDT		paneid_PrefsEditHeadFootRevert = 'REVR';

// Resources
const	ResIDT		RidL_CPrefsEditHeadFootBtns = 5100;
const	ResIDT		RidL_CPrefsEditHeadFootFileBtns = 5101;

// Messages
const	MessageT	msg_PrefsEditHeadChooseFile = 'CHOS';
const	MessageT	msg_PrefsEditHeadUseFile = 'UFIL';
const	MessageT	msg_PrefsEditHeadUseText = 'UTXT';
const	MessageT	msg_PrefsEditHeadRevert = 5100;

// Classes
class CTextDisplay;
class CTextFieldX;
class LPushButton;
class LRadioButton;

class CPrefsEditHeadFoot : public LDialogBox
{
private:

public:
	enum { class_ID = 'HeFo' };

					CPrefsEditHeadFoot();
					CPrefsEditHeadFoot(LStream *inStream);
	virtual 		~CPrefsEditHeadFoot();

	static bool PoseDialog(const cdstring title, cdstring& text, bool allow_file, short spaces, short wrap);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	LRadioButton*	mUseFile;
	LRadioButton*	mUseText;
	CTextFieldX*	mFileName;
	LPushButton*	mChooseBtn;
	CTextDisplay*	mText;
	LPushButton*	mRevertBtn;

	bool			mAllowFile;								// Allow choice of file
	cdstring		mRevertText;							// Text to revert to
	cdstring		mOldText;								// Text used prior to switching to file
	short			mWrap;									// Current wrap length

			void	SetAllowFile(bool allow_file)
		{ mAllowFile = allow_file; }
			void	SetEditorText(const cdstring& text);		// Set text in editor
			void	GetEditorText(cdstring& text);				// Get text from editor
			void	SetRuler(unsigned long wrap, unsigned long spaces);			// Set current wrap length

			void	ChooseFile();
			void	UpdateFile();
			void	SetUseFile(bool use_file);
};

#endif
