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

#include <LDialogBox.h>

#include "CUserAction.h"

// Constants

// Panes
const	PaneIDT		paneid_KeyChoiceDialog = 1791;

// Mesages

// Resources
const	ResIDT		RidL_CKeyChoiceDialogBtns = 1791;

// Type

class CKeyChoiceDialog : public LDialogBox
{
private:

public:
	enum { class_ID = 'KeyC' };

					CKeyChoiceDialog();
					CKeyChoiceDialog(LStream *inStream);
	virtual 		~CKeyChoiceDialog();

	static bool 	PoseDialog(unsigned char& key, CKeyModifiers& mods);

	virtual Boolean	HandleKeyPress(const EventRecord &inKeyEvent);

	bool	IsDone() const
		{ return mDone; }

protected:
	unsigned char	mKey;
	CKeyModifiers	mMods;
	bool			mDone;

	virtual void	FinishCreateSelf(void);					// Do odds & ends
};

#endif
