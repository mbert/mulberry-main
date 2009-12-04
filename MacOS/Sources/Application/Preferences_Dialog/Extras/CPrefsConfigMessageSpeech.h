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


// Header for CPrefsConfigMessageSpeech class

#ifndef __CPREFSCONFIGMESSAGESPEECH__MULBERRY__
#define __CPREFSCONFIGMESSAGESPEECH__MULBERRY__

#include <LDialogBox.h>

#include "CSpeechSynthesis.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsConfigMessageSpeech = 5102;
const	PaneIDT		paneid_SpeechTable = 'TABL';
const	PaneIDT		paneid_SpeechNewBtn = 'NEWB';

// Mesages
const	MessageT	msg_NewSpeechItem = 5106;

// Resources
const	ResIDT		RidL_CPrefsConfigMessageSpeechBtns = 5102;

// Classes

class	CMessageSpeechTable;
class	LPushButton;

class	CPrefsConfigMessageSpeech : public LDialogBox {

public:
	enum { class_ID = 'CSPC' };

					CPrefsConfigMessageSpeech();
					CPrefsConfigMessageSpeech(LStream *inStream);
	virtual 		~CPrefsConfigMessageSpeech();


	virtual void	SetList(CMessageSpeakVector* list);

	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CMessageSpeakVector*	mList;
	CMessageSpeechTable*	mItsTable;
	LPushButton*			mNewBtn;
	
	virtual void	DoNewItem(void);				// Create a new item
};

#endif
