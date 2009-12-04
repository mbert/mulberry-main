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


// Header for CGetPassphraseDialog class

#ifndef __CGETPASSPHRASEDIALOG__MULBERRY__
#define __CGETPASSPHRASEDIALOG__MULBERRY__

#include <LDialogBox.h>
#include <LPopupButton.h>

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_GetPassphraseDialog = 21100;
const	PaneIDT		paneid_GetPassphraseTextP = 'TEXP';
const	PaneIDT		paneid_GetPassphraseTextC = 'TEXC';
const	PaneIDT		paneid_GetPassphraseKeys = 'KEYS';
const	PaneIDT		paneid_GetPassphraseHide = 'HIDE';

// Message
const	MessageT	msg_GetPassphraseHide = 'HIDE';

// Resources
const	PaneIDT		RidL_CGetPassphraseDialogBtns = 21100;

class CTextFieldX;

class CGetPassphraseDialog : public LDialogBox
{
public:
	enum { class_ID = 'Pass' };

					CGetPassphraseDialog();
					CGetPassphraseDialog(LStream *inStream);
	virtual 		~CGetPassphraseDialog();

	virtual	void	SetUpDetails(cdstring& change, const char** keys);
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	static bool		PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index);

protected:
	CTextFieldX*	mTextP;
	CTextFieldX*	mTextC;
	LPopupButton*	mKeys;
	bool			mHidden;

	virtual void	FinishCreateSelf(void);
	
			void	HideText(bool hide);
};

#endif
