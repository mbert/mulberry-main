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


// Header for CPrefsAlertsAttachment class

#ifndef __CPREFSALERTSATTACHMENT__MULBERRY__
#define __CPREFSALERTSATTACHMENT__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAlertsAttachment = 5081;
const	PaneIDT		paneid_AttachmentAlert = 'ALT2';
const	PaneIDT		paneid_AttachmentPlaySound = 'PLY2';
const	PaneIDT		paneid_AttachmentSound = 'SND2';
const	PaneIDT		paneid_AttachmentSpeak = 'SPK2';
const	PaneIDT		paneid_AttachmentSpeakText = 'SPT2';

// Mesages
const	MessageT	msg_AttachmentPlaySound = 'PLY2';
const	MessageT	msg_AttachmentSound = 'SND2';
const	MessageT	msg_AttachmentSpeak = 'SPK2';

// Resources
const	ResIDT		RidL_CPrefsAlertsAttachmentBtns = 5081;

// Classes
class LCheckBox;
class CSoundPopup;
class CTextFieldX;

class	CPrefsAlertsAttachment : public CPrefsTabSubPanel,
									public LListener
{
private:
	LCheckBox*			mAttachmentAlert;
	LCheckBox*			mAttachmentPlaySound;
	CSoundPopup*		mAttachmentSound;
	LCheckBox*			mAttachmentSpeak;
	CTextFieldX*		mAttachmentSpeakText;

public:
	enum { class_ID = 'Ratc' };

					CPrefsAlertsAttachment();
					CPrefsAlertsAttachment(LStream *inStream);
	virtual 		~CPrefsAlertsAttachment();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
