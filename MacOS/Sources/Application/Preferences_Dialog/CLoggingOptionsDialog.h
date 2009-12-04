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


// Header for CLoggingOptionsDialog class

#ifndef __CLOGGINGOPTIONSDIALOG__MULBERRY__
#define __CLOGGINGOPTIONSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CLog.h"

// Constants

// Panes
const	PaneIDT		paneid_LoggingOptionsDialog = 5500;
const	PaneIDT		paneid_LoggingOptionsEnabled = 'ACTI';
const	PaneIDT		paneid_LoggingOptions[] = { 0L,
												'IMAP',
												'POP3',
												'IMSP',
												'ACAP',
												'SMTP',
												'HTTP',
												'PLUG',
												'FILT',
												'ERRS'};
const	PaneIDT		paneid_LoggingOptionsAuthentication = 'AUTH';
const	PaneIDT		paneid_LoggingOptionsPlayback = 'PLAY';
const	PaneIDT		paneid_LoggingOptionsOverwrite = 'OVER';
const	PaneIDT		paneid_LoggingOptionsAppDir = 'ADIR';
const	PaneIDT		paneid_LoggingOptionsUserDir = 'UDIR';

const MessageT		msg_LoggingOptionsFlush = 'FLSH';
const MessageT		msg_LoggingOptionsClear = 'CLEA';

// Resources
const	ResIDT		RidL_CLoggingOptionsDialog = 5500;

// Classes
class	LCheckBox;
class	LRadioButton;
class	CSpellPlugin;

class	CLoggingOptionsDialog : public LDialogBox
{

private:
	LCheckBox*		mEnabled;
	LCheckBox*		mLogs[CLog::eLogTypeLast];
	LCheckBox*		mAuthentication;
	LCheckBox*		mPlayback;
	LCheckBox*		mOverwrite;
	LRadioButton*	mApplicationDirectory;
	LRadioButton*	mUserDirectory;

public:
	enum { class_ID = 'Logp' };

					CLoggingOptionsDialog();
					CLoggingOptionsDialog(LStream *inStream);
	virtual 		~CLoggingOptionsDialog();

	static bool PoseDialog(CLog::SLogOptions& options);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	void	ListenToMessage(MessageT inMessage, void *ioParam);

public:
			void	SetOptions(const CLog::SLogOptions& options);		// Set options in dialog
			void	GetOptions(CLog::SLogOptions& options);			// Get options from dialog
};

#endif
