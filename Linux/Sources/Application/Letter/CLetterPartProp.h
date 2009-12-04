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


// CLetterPartProp.h : header file
//

#ifndef __CLETTERPARTPROP__MULBERRY__
#define __CLETTERPARTPROP__MULBERRY__

#include "CDialogDirector.h"

#include "cdstring.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp dialog

class CMIMEContent;

class CTextInputDisplay;
class CTextInputField;
class JXTextButton;
class JXTextMenu;

class CLetterPartProp : public CDialogDirector
{
// Construction
public:
	CLetterPartProp(JXDirector* supervisor);   // standard constructor

	static bool PoseDialog(CMIMEContent& content);

protected:
	enum {
		cMIMEApplication = 1,
		cMIMEAudio,
		cMIMEImage,
		cMIMEMessage,
		cMIMEMultipart,
		cMIMEText,
		cMIMEVideo
	};

// begin JXLayout

    CTextInputField*   mName;
    CTextInputField*   mType;
    HPopupMenu*        mTypePopup;
    CTextInputField*   mSubtype;
    HPopupMenu*        mSubtypePopup;
    HPopupMenu*        mEncodingPopup;
    HPopupMenu*        mDispositionPopup;
    HPopupMenu*        mCharsetPopup;
    CTextInputDisplay* mDescription;
    JXTextButton*      mOKBtn;
    JXTextButton*      mCancelBtn;

// end JXLayout

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual void	SetFields(CMIMEContent& content);			// Set fields in dialog
	virtual void	GetFields(CMIMEContent& content);			// Get fields from dialog

private:
			void	SyncSubtypePopup(const cdstring& sel);			// Sync menus and text
};

#endif
