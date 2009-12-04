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

// CPluginRegisterDialog.h : header file
//

#ifndef __CPLUGINREGISTERDIALOG__MULBERRY__
#define __CPLUGINREGISTERDIALOG__MULBERRY__

#include "CDialogDirector.h"

/////////////////////////////////////////////////////////////////////////////
// CPluginRegisterDialog dialog

class CPlugin;

class cdstring;
class JXStaticText;
class JXTextButton;
class CTextInputField;
class JXImageWidget;

class CPluginRegisterDialog : public CDialogDirector
{
// Construction
public:
	enum
	{
		eRegister = 1,
		eCancel,
		eRemove,
		eDemo
	};

	CPluginRegisterDialog(JXDirector* supervisor);   // standard constructor

	static int PoseDialog(const CPlugin& plugin, bool allow_demo, bool allow_delete, cdstring& regkey);

// Implementation
protected:
// begin JXLayout

    JXImageWidget*   mLogo;
    JXStaticText*    mName;
    JXStaticText*    mVersion;
    JXStaticText*    mType;
    JXStaticText*    mManufacturer;
    JXStaticText*    mDescription;
    CTextInputField* mRegKey;
    JXTextButton*    mRegisterBtn;
    JXTextButton*    mCancelBtn;
    JXTextButton*    mRemoveBtn;
    JXTextButton*    mDemoBtn;

// end JXLayout

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual void SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete);
};

#endif
