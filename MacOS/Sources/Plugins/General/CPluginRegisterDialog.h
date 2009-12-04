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

// Header for CPluginRegisterDialog class

#ifndef __CPLUGINREGISTERDIALOG__MULBERRY__
#define __CPLUGINREGISTERDIALOG__MULBERRY__

#include <LDialogBox.h>
#include "CPlugin.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_PluginRegisterDialog = 5310;
const	PaneIDT		paneid_PluginRegisterName = 'NAME';
const	PaneIDT		paneid_PluginRegisterVersion = 'VERS';
const	PaneIDT		paneid_PluginRegisterType = 'TYPE';
const	PaneIDT		paneid_PluginRegisterManufacturer = 'MANU';
const	PaneIDT		paneid_PluginRegisterDescription = 'DESC';
const	PaneIDT		paneid_PluginRegisterRegKey = 'RKEY';
const	PaneIDT		paneid_PluginRegisterRunDemoBtn = 'DEMO';
const	PaneIDT		paneid_PluginRegisterRemovePluginBtn = 'REMV';
const	PaneIDT		paneid_PluginRegisterCancelBtn = 'CANC';

// Mesages
const	MessageT	msg_RunPluginDemo = 'DEMO';
const	MessageT	msg_RemovePlugin = 'REMV';

// Type
class CTextFieldX;
class LPushButton;
class CTextFieldX;
class CPlugin;

class CPluginRegisterDialog : public LDialogBox
{
private:
	CTextFieldX*	mName;
	CTextFieldX*	mVersion;
	CTextFieldX*	mType;
	CTextFieldX*	mManufacturer;
	CTextFieldX*	mDescription;
	CTextFieldX*	mRegKey;
	LPushButton*	mRunDemoBtn;
	LPushButton*	mRemovePluginBtn;
	LPushButton*	mCancelBtn;

public:
	enum { class_ID = 'RgPl' };

					CPluginRegisterDialog();
					CPluginRegisterDialog(LStream *inStream);
	virtual 		~CPluginRegisterDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	 SetPlugin(const CPlugin& plugin, bool allow_demo, bool allow_delete);
	virtual cdstring GetRegKey(void) const;
};

#endif
