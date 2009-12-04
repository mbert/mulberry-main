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

// Header for CSpellOptionsDialog class

#ifndef __CSPELLOPTIONSDIALOG__MULBERRY__
#define __CSPELLOPTIONSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "HPopupMenu.h"

// Constants

// Classes
class CSpellPlugin;
class JXColorButton;
class JXTextButton;
class JXTextCheckbox;

class CSpellOptionsDialog : public CDialogDirector
{
public:
			CSpellOptionsDialog(JXDirector* supervisor);
	virtual ~CSpellOptionsDialog();

	static bool PoseDialog(CSpellPlugin* speller);

	void OnCreate();

protected:
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnSpellColourBackground();

private:
// begin JXLayout

    JXTextButton*   mOKBtn;
    JXTextButton*   mCancelBtn;
    JXTextCheckbox* mAutoPositionDialog;
    JXTextCheckbox* mSpellOnSend;
    JXTextCheckbox* mSpellAsYouType;
    JXTextCheckbox* mSpellColourBackground;
    JXColorButton*  mSpellBackgroundColour;
    HPopupMenu*     mDictionaries;

// end JXLayout

	void SetOptions(CSpellPlugin* speller);			// Set the speller options
	void GetOptions(CSpellPlugin* speller);			// Get the speller options
			
	void InitDictionaries(CSpellPlugin* speller);
};

#endif
