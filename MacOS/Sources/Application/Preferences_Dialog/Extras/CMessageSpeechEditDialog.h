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


// Header for CMessageSpeechEditDialog class

#ifndef __CMESSAGESPEECHEDITDIALOG__MULBERRY__
#define __CMESSAGESPEECHEDITDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_MessageSpeechEdit = 5103;
const	PaneIDT		paneid_MessageSpeechEditItem = 'ITEM';
const	PaneIDT		paneid_MessageSpeechEditText = 'TEXT';

// Resources
const	ResIDT		RidL_CMessageSpeechEditDialogBtns = 5103;

// Classes
class	CTextFieldX;
class	LPopupButton;
class	CMessageSpeak;

class	CMessageSpeechEditDialog : public LDialogBox {

private:
	LPopupButton*	mItem;
	CTextFieldX*	mText;
	
public:
	enum { class_ID = 'EdMS' };

					CMessageSpeechEditDialog();
					CMessageSpeechEditDialog(LStream *inStream);
	virtual 		~CMessageSpeechEditDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetItem(CMessageSpeak* item);
	virtual void	GetItem(CMessageSpeak* item);
};

#endif
