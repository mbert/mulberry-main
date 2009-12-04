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


// Header for CPrefsEditMappingDialog class

#ifndef __CPREFSEDITMAPPINGDIALOG__MULBERRY__
#define __CPREFSEDITMAPPINGDIALOG__MULBERRY__

#include <LDialogBox.h>


#include "CMessageWindow.h"
#include "CMIMEMap.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsEditMappingDialog = 5121;
const	PaneIDT		paneid_MIMEType = 'TYPE';
const	PaneIDT		paneid_MIMESubtype = 'SUBT';
const	PaneIDT		paneid_Application = 'APPL';
const	PaneIDT		paneid_FileType = 'FTYP';
const	PaneIDT		paneid_ChooseBtn = 'GETF';

// Mesages
const	MessageT	msg_ChooseApplication = 'GETF';

// Resources
const	ResIDT		RidL_CPrefsEditMappingDialogBtns = 5121;

// Classes

class	CMIMEMap;
class	CTextFieldX;
class	LPushButton;
class	LPopupButton;

class	CPrefsEditMappingDialog : public LDialogBox
{
public:
	enum { class_ID = 'MAPE' };

					CPrefsEditMappingDialog();
					CPrefsEditMappingDialog(LStream *inStream);
	virtual 		~CPrefsEditMappingDialog();

	static bool PoseDialog(CMIMEMap& mapping);

	virtual void	ListenToMessage(MessageT inMessage,
											void *ioParam);	// Respond to clicks in the buttons
protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	CTextFieldX*		mMIMEType;
	CTextFieldX*		mMIMESubtype;
	CTextFieldX*		mApplication;
	LPopupButton*		mTypeMenu;
	LPushButton*		mChooseBtn;
	LArray				mTypeArray;
	OSType				mCurrentCreator;
	OSType				mOriginalFileType;
	
			void	SetMapping(const CMIMEMap& mapping);
			void	GetMapping(CMIMEMap& mapping);

			void	DoChooseApplication();		// Choose an application

			void	ResetPopupMenu(OSType creator);	// Reset popup menu for new creator
};

#endif
