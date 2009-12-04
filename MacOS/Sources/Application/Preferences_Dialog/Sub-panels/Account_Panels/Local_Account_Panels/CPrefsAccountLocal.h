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


// Header for CPrefsAccountLocal class

#ifndef __CPREFSACCOUNTLOCAL__MULBERRY__
#define __CPREFSACCOUNTLOCAL__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountLocal = 5027;
const	PaneIDT		paneid_DefaultPath = 'DEFL';
const	PaneIDT		paneid_UsePath = 'USEL';
const	PaneIDT		paneid_AccountLocalPath = 'PATH';
const	PaneIDT		paneid_ChooseLocalFolder = 'CHOS';
const	PaneIDT		paneid_RelativeLocalFolder = 'RELP';
const	PaneIDT		paneid_AbsoluteLocalFolder = 'ABSP';
const	PaneIDT		paneid_FileFormatPopup = 'FILE';

// Mesages
const	MessageT	msg_DefaultPath = 'DEFL';
const	MessageT	msg_UsePath = 'USEL';
const	MessageT	msg_ChooseLocalFolder = 'CHOS';
const	MessageT	msg_RelativeLocalFolder = 'RELP';
const	MessageT	msg_AbsoluteLocalFolder = 'ABSP';

// Resources
const	ResIDT		RidL_CPrefsAccountLocalBtns = 5027;

// Classes
class CTextFieldX;
class LPopupButton;
class LPushButton;
class LRadioButton;

class	CPrefsAccountLocal : public CPrefsTabSubPanel, public LListener
{
private:
	LRadioButton*		mDefaultPath;
	LRadioButton*		mUsePath;
	CTextFieldX*		mPath;
	LPushButton*		mChoose;
	LRadioButton*		mRelative;
	LRadioButton*		mAbsolute;
	bool				mLocalAddress;
	bool				mDisconnected;
	LPopupButton*		mFileFormatPopup;

public:
	enum { class_ID = 'Alcl' };

					CPrefsAccountLocal();
					CPrefsAccountLocal(LStream *inStream);
	virtual 		~CPrefsAccountLocal();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

			void	SetLocalAddress()
		{ mLocalAddress = true; }
			void	SetDisconnected()
		{ mDisconnected = true; }

	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data

private:
	virtual void	SetUseLocal(bool use);
	virtual void	SetPath(const char* path);
	virtual void	DoChooseLocalFolder();				// Choose local folder using browser
	virtual void	SetRelative();
	virtual void	SetAbsolute();
};

#endif
