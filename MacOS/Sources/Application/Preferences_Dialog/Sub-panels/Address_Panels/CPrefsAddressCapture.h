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


// Header for CPrefsAddressCapture class

#ifndef __CPREFSADDRESSCAPTURE__MULBERRY__
#define __CPREFSADDRESSCAPTURE__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddressCapture = 5091;
const 	PaneIDT 	paneid_ACCaptureAddressBook = 'ADBK';
const 	PaneIDT 	paneid_ACCapturePopup = 'APOP';
const 	PaneIDT 	paneid_ACCaptureAllowEdit = 'EDIT';
const 	PaneIDT 	paneid_ACCaptureAllowChoice = 'CHOI';
const 	PaneIDT 	paneid_ACCaptureRead = 'CRED';
const 	PaneIDT 	paneid_ACCaptureRespond = 'CRES';
const 	PaneIDT 	paneid_ACCaptureFrom = 'CFRM';
const 	PaneIDT 	paneid_ACCaptureCc = 'CCC ';
const 	PaneIDT 	paneid_ACCaptureReplyTo = 'CRPL';
const 	PaneIDT 	paneid_ACCaptureTo = 'CTO ';

// Mesages
const	MessageT	msg_ACCapturePopup = 'APOP';

// Resources
const	ResIDT		RidL_CPrefsAddressCaptureBtns = 5091;

// Classes
class CTextFieldX;
class LCheckBox;
class LPopupButton;

class CPrefsAddressCapture : public CPrefsTabSubPanel,
									public LListener
{
private:
	CTextFieldX*	mCaptureAddressBook;
	LPopupButton*	mCapturePopup;
	LCheckBox*		mCaptureAllowEdit;
	LCheckBox*		mCaptureAllowChoice;
	LCheckBox*		mCaptureRead;
	LCheckBox*		mCaptureRespond;
	LCheckBox*		mCaptureFrom;
	LCheckBox*		mCaptureCc;
	LCheckBox*		mCaptureReplyTo;
	LCheckBox*		mCaptureTo;

public:
	enum { class_ID = 'Acap' };

					CPrefsAddressCapture();
					CPrefsAddressCapture(LStream *inStream);
	virtual 		~CPrefsAddressCapture();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

private:
			void	InitAddressBookPopup();
};

#endif
