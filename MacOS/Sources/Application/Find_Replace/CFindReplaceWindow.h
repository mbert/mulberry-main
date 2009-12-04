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


// Header for CFindReplaceWindow class

#ifndef __CFINDREPLACEWINDOW__MULBERRY__
#define __CFINDREPLACEWINDOW__MULBERRY__

#include "LWindow.h"

#include "cdstring.h"
#include "strfind.h"

// Constants
const	PaneIDT		paneid_FindReplaceWindow = 3020;
const	PaneIDT		paneid_FindReplaceFind = 'FINT';
const	PaneIDT		paneid_FindReplaceFindPopup = 'FINP';
const	PaneIDT		paneid_FindReplaceReplace = 'REPT';
const	PaneIDT		paneid_FindReplaceReplacePopup = 'REPP';
const	PaneIDT		paneid_FindReplaceCase = 'CASE';
const	PaneIDT		paneid_FindReplaceBackwards = 'BACK';
const	PaneIDT		paneid_FindReplaceWrap = 'WRAP';
const	PaneIDT		paneid_FindReplaceWord = 'WORD';
const	PaneIDT		paneid_FindReplaceFindBtn = 'FIND';
const	PaneIDT		paneid_FindReplaceReplaceBtn = 'REPL';
const	PaneIDT		paneid_FindReplaceBothBtn = 'REFI';
const	PaneIDT		paneid_FindReplaceAllBtn = 'REPA';

// Resources
const	ResIDT		RidL_CFindReplaceBtns = 3020;

// Messages
const	MessageT	msg_FindReplaceFindText = 'FINT';
const	MessageT	msg_FindReplaceFindPopup = 'FINP';
const	MessageT	msg_FindReplaceReplacePopup = 'REPP';
const	MessageT	msg_FindReplaceCase = 'CASE';
const	MessageT	msg_FindReplaceBackwards = 'BACK';
const	MessageT	msg_FindReplaceWrap = 'WRAP';
const	MessageT	msg_FindReplaceWord = 'WORD';
const	MessageT	msg_FindReplaceFindBtn = 'FIND';
const	MessageT	msg_FindReplaceReplaceBtn = 'REPL';
const	MessageT	msg_FindReplaceBothBtn = 'REFI';
const	MessageT	msg_FindReplaceAllBtn = 'REPA';

// Classes

class CTextDisplay;
class CTextFieldX;
class LCheckBox;
class LPushButton;
class LPopupButton;

class	CFindReplaceWindow : public LWindow,
						 		public LListener
{
public:
	static CFindReplaceWindow*		sFindReplaceWindow;
	static cdstring					sFindText;
	static cdstring					sReplaceText;
	static EFindMode				sFindMode;
	static cdstrvect				sFindHistory;
	static cdstrvect				sReplaceHistory;
	
	enum { class_ID = 'FWin' };

					CFindReplaceWindow();
					CFindReplaceWindow(LStream *inStream);
	virtual 		~CFindReplaceWindow();

	static void CreateFindReplaceWindow(CTextDisplay* target);	// Create it or bring it to the front
	static void DestroyFindReplaceWindow();						// Destroy it
	static void UpdateFindReplace();							// Update current details
	static void SetFindText(const char* text);					// Set find text and push into history

	virtual void	ResetState(bool force = false);				// Reset state from prefs

protected:
	CTextFieldX*	mFindText;
	LPopupButton*	mFindPopup;
	CTextFieldX*	mReplaceText;
	LPopupButton*	mReplacePopup;
	LCheckBox*		mCaseSensitive;
	LCheckBox*		mBackwards;
	LCheckBox*		mWrap;
	LCheckBox*		mEntireWord;
	LPushButton*	mFindBtn;
	LPushButton*	mReplaceBtn;
	LPushButton*	mReplaceFindBtn;
	LPushButton*	mReplaceAllBtn;

	CTextDisplay*	mTargetDisplay;

	virtual void	FinishCreateSelf(void);

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual void		ListenToMessage(MessageT inMessage,
										void *ioParam);	// Respond to clicks in the icon buttons

private:
	virtual void	SaveDefaultState(void);					// Save state in prefs

	void InitItems();										// Initialise items
	void InitHistory(LPopupButton* popup,					// Initialise menus
						const cdstrvect& history);
	void UpdateButtons();									// Update button state
	void SelectHistory(const cdstrvect& history,			// History item chosen
						long menu_pos,
						CTextFieldX* field);
	void CaptureState();									// Copy state information to statics

	void	OnFindText();
	void	OnReplaceText();
	void	OnReplaceFindText();
	void	OnReplaceAllText();
	
	void	PrepareTarget();								// Prepare target before execution
};

#endif
