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


// CGetPassphraseDialog.h : header file
//

#ifndef __CGETPASSPHRASEDIALOG__MULBERRY__
#define __CGETPASSPHRASEDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "HPopupMenu.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CGetPassphraseDialog dialog

template <class T> class CInputField;
class JXInputField;
class JXPasswordInput;
class JXTextButton;
class JXTextCheckbox;

class CGetPassphraseDialog : public CDialogDirector
{
// Construction
public:
	CGetPassphraseDialog(JXDirector* supervisor);   // standard constructor

	static bool PoseDialog(cdstring& passphrase, const char* title);
	static bool PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index, const char* title = NULL);

protected:
// begin JXLayout

    CInputField<JXPasswordInput>* mPassphrase1;
    CInputField<JXInputField>*    mPassphrase2;
    HPopupMenu*                   mKeyPopup;
    JXTextCheckbox*               mHideTyping;
    JXTextButton*                 mOKBtn;
    JXTextButton*                 mCancelBtn;

// end JXLayout
	const char** mKeys;

	void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
	void OnHideTyping();
	void OnKeyPopup(JIndex nID);

	
	void InitKeysMenu(void);
};
#endif
