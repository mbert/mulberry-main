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


// Header for CPrefsSpeech class

#ifndef __CPREFSSPEECH__MULBERRY__
#define __CPREFSSPEECH__MULBERRY__

#include "CPrefsPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsSpeech = 5008;
const	PaneIDT		paneid_SpeechNewMessage = 'NEWM';
const	PaneIDT		paneid_SpeechOnArrival = 'OPEN';
const	PaneIDT		paneid_SpeechMaxChars = 'MAXC';
const	PaneIDT		paneid_SpeechEmptyText = 'EMPT';

// Mesages
const	MessageT	msg_ConfigureMessageSpeech = 5181;
const	MessageT	msg_ConfigureLetterSpeech = 5182;

// Resources
const	ResIDT		RidL_CPrefsSpeechBtns = 5008;
const	ResIDT		STRx_PrefsSpeechHelp = 5008;
enum {
	str_PrefsSpeechOnAlertText = 7
};

// Classes
class CTextFieldX;
class LCheckBox;

class	CPrefsSpeech : public CPrefsPanel,
						public LListener {

private:
	LCheckBox*		mSpeechNewMessage;
	LCheckBox*		mSpeechOnArrival;
	CTextFieldX*	mSpeechMaxChars;
	CTextFieldX*	mSpeechEmptyText;

public:
	enum { class_ID = 'Pspk' };

					CPrefsSpeech();
					CPrefsSpeech(LStream *inStream);
	virtual 		~CPrefsSpeech();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

private:
	virtual void	ConfigureMessage(void);					// Configure message speech
	virtual void	ConfigureLetter(void);					// Configure letter speech

};

#endif
