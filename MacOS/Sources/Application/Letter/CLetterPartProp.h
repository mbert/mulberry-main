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


// Header for CLetterPartProp class

#ifndef __CLETTERPARTPROP__MULBERRY__
#define __CLETTERPARTPROP__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_LetterPart = 3010;
const	PaneIDT		paneid_LetterPartName = 'NAME';
const	PaneIDT		paneid_LetterPartType = 'TYPE';
const	PaneIDT		paneid_LetterPartTypePopup = 'TPOP';
const	PaneIDT		paneid_LetterPartSubtype = 'STYP';
const	PaneIDT		paneid_LetterPartSubtypePopup = 'SPOP';
const	PaneIDT		paneid_LetterPartEncodingPopup = 'ENCD';
const	PaneIDT		paneid_LetterPartDescription = 'DESC';
const	PaneIDT		paneid_LetterPartDispositionPopup = 'DISP';
const	PaneIDT		paneid_LetterPartCharsetPopup = 'CHAR';

// Messages
const	MessageT	msg_TypePopup = 'TPOP';
const	MessageT	msg_SubtypePopup = 'SPOP';
const	MessageT	msg_EncodingPopup = 'ENCD';

// Resources
const	ResIDT		RidL_CLetterPartProp = 3010;
const	ResIDT		MENU_MIMETypes = 3003;
enum {
	cMIMEApplication = 1,
	cMIMEAudio,
	cMIMEImage,
	cMIMEMessage,
	cMIMEMultipart,
	cMIMEText,
	cMIMEVideo
};

// Classes
class	CTextFieldX;
class	LPopupButton;
class	CMIMEContent;

class	CLetterPartProp : public LDialogBox {

private:
	CTextFieldX*		mName;
	CTextFieldX*		mType;
	LPopupButton*		mTypePopup;
	CTextFieldX*		mSubtype;
	LPopupButton*		mSubtypePopup;
	LPopupButton*		mEncodingPopup;
	CTextFieldX*		mDescription;
	LPopupButton*		mDispositionPopup;
	LPopupButton*		mCharsetPopup;
	
public:
	enum { class_ID = 'LPrt' };

					CLetterPartProp();
					CLetterPartProp(LStream *inStream);
	virtual 		~CLetterPartProp();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to hits in items

public:
	virtual void	SetFields(CMIMEContent& content);			// Set fields in dialog
	virtual void	GetFields(CMIMEContent& content);			// Get fields from dialog

private:
	virtual void	SyncTypePopup(void);						// Sync menus and text
	virtual void	SyncSubtypePopup(long type);				// Sync menus and text
	virtual void	SyncEncodingPopup(long type);				// Sync menus and text
	virtual void	SyncDispositionPopup(long type);			// Sync menus and text
	virtual void	SyncCharsetPopup(long type);				// Sync menus and text
};

#endif
