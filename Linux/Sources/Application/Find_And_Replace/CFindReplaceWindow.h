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

#include <JXWindowDirector.h>

#include "HPopupMenu.h"

#include "cdstring.h"
#include "strfind.h"

// Classes

class CTextDisplay;
class CTextInputDisplay;
class JXTextButton;
class JXTextCheckbox;

class CFindReplaceWindow : public JXWindowDirector
{
public:
	static CFindReplaceWindow*		sFindReplaceWindow;
	static cdstring					sFindText;
	static cdstring					sReplaceText;
	static EFindMode				sFindMode;
	static cdstrvect				sFindHistory;
	static cdstrvect				sReplaceHistory;
	
			CFindReplaceWindow(JXDirector* owner);
	virtual	~CFindReplaceWindow();

	static void CreateFindReplaceWindow(CTextDisplay* target);	// Create it or bring it to the front
	static void DestroyFindReplaceWindow();						// Destroy it
	static void UpdateFindReplace();							// Update current details
	static void SetFindText(const char* text);					// Set find text and push into history

	virtual void	ResetState(bool force = false);				// Reset state from prefs

protected:
// begin JXLayout

    JXTextButton*      mFindBtn;
    JXTextButton*      mReplaceBtn;
    JXTextButton*      mReplaceFindBtn;
    JXTextButton*      mReplaceAllBtn;
    CTextInputDisplay* mFindText;
    HPopupMenu*        mFindPopup;
    CTextInputDisplay* mReplaceText;
    HPopupMenu*        mReplacePopup;
    JXTextCheckbox*    mCaseSensitive;
    JXTextCheckbox*    mBackwards;
    JXTextCheckbox*    mWrap;
    JXTextCheckbox*    mEntireWord;

// end JXLayout

	CTextDisplay*	mTargetDisplay;

	virtual void	OnCreate();

	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void	SaveDefaultState(void);					// Save state in prefs

	void InitItems();										// Initialise items
	void InitHistory(HPopupMenu* popup,						// Initialise menus
						const cdstrvect& history);
	void UpdateButtons();									// Update button state
	void SelectHistory(const cdstrvect& history,			// History item chosen
						JIndex menu_pos,
						CTextInputDisplay* field);
	void	CaptureState();									// Copy state information to statics

	void	OnFindText();
	void	OnReplaceText();
	void	OnReplaceFindText();
	void	OnReplaceAllText();
	
	void	PrepareTarget();								// Prepare target before execution
};

#endif
