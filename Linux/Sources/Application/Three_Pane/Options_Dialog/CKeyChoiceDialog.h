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


// Header for CKeyChoiceDialog class

#ifndef __CKEYCHOICEDIALOG__MULBERRY__
#define __CKEYCHOICEDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include <JXStaticText.h>

#include "CUserAction.h"

// Classes
class CKeyTrap;
class JXTextButton;

class CKeyChoiceDialog : public CDialogDirector
{
private:

public:
					CKeyChoiceDialog(JXDirector* supervisor);
	virtual 		~CKeyChoiceDialog();

	static bool 	PoseDialog(unsigned char& key, CKeyModifiers& mods);

	bool	IsDone() const
		{ return mDone; }

protected:
// begin JXLayout

    JXTextButton* mCancelBtn;
    CKeyTrap*     mKeyTrap;

// end JXLayout
	bool			mDone;

	virtual void	OnCreate();					// Do odds & ends
	virtual void	Receive(JBroadcaster* sender, const Message& message);
};

class CKeyTrap : public JXStaticText
{
public:
	CKeyTrap(const JCharacter* text, JXContainer* enclosure,
				 const HSizingOption hSizing, const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h);

	virtual ~CKeyTrap() {}

	virtual void	HandleKeyPress(const int key, const JXKeyModifiers& modifiers);

	unsigned char		mKey;
	CKeyModifiers		mMods;
	CKeyChoiceDialog*	mDialog;
};

#endif
