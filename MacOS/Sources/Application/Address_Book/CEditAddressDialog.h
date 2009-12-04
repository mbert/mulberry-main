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


// Header for CEditAddressDialog class

#ifndef __CEDITADDRESSDIALOG__MULBERRY__
#define __CEDITADDRESSDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_EditAddress = 9001;
const	PaneIDT		paneid_EditAddressNickName = 'NICK';
const	PaneIDT		paneid_EditAddressFullName = 'FULL';
const	PaneIDT		paneid_EditAddressEmail = 'ADDR';
const	PaneIDT		paneid_EditAddressCalendar = 'CALA';
const	PaneIDT		paneid_EditAddressCompany = 'COMP';
const	PaneIDT		paneid_EditAddressAddress = 'MADD';
const	PaneIDT		paneid_EditAddressPhoneWork = 'WORK';
const	PaneIDT		paneid_EditAddressPhoneHome = 'HOME';
const	PaneIDT		paneid_EditAddressFax = 'FAX ';
const	PaneIDT		paneid_EditAddressURL = 'URL ';
const	PaneIDT		paneid_EditAddressNotes = 'NOTE';

// Resources
const	ResIDT		STRx_EditAddressHelp = 9001;

class	CTextFieldX;
class	CTextDisplay;
class	CAdbkAddress;

class	CEditAddressDialog : public LDialogBox {

private:
	CTextFieldX*		mNickName;
	CTextFieldX*		mFullName;
	CTextFieldX*		mEmail;
	CTextFieldX*		mCalendar;
	CTextFieldX*		mCompany;
	CTextDisplay*		mAddress;
	CTextDisplay*		mURL;
	CTextFieldX*		mPhoneWork;
	CTextFieldX*		mPhoneHome;
	CTextFieldX*		mFax;
	CTextDisplay*		mNotes;
	
public:
	enum { class_ID = 'EdAd' };

					CEditAddressDialog();
					CEditAddressDialog(LStream *inStream);
	virtual 		~CEditAddressDialog();

	static	bool	PoseDialog(CAdbkAddress* addr, bool allow_edit = true);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetFields(const CAdbkAddress* addr,		// Set fields in dialog
								bool allow_edit = true);
	virtual bool	GetFields(CAdbkAddress* addr);			// Get fields from dialog
};

#endif
