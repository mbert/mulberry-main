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

// Header for CAboutPluginsDialog class

#ifndef __CABOUTPLUGINSDIALOG__MULBERRY__
#define __CABOUTPLUGINSDIALOG__MULBERRY__

#include <LDialogBox.h>
#include "CPlugin.h"

// Constants

// Panes
const	PaneIDT		paneid_AboutPluginsDialog = 20000;
const	PaneIDT		paneid_AboutPluginsName = 'NAME';
const	PaneIDT		paneid_AboutPluginsVersion = 'VERS';
const	PaneIDT		paneid_AboutPluginsType = 'TYPE';
const	PaneIDT		paneid_AboutPluginsManufacturer = 'MANU';
const	PaneIDT		paneid_AboutPluginsDescription = 'DESC';
const	PaneIDT		paneid_AboutPluginsMode = 'MODE';
const	PaneIDT		paneid_AboutPluginsList = 'LIST';
const	PaneIDT		paneid_AboutPluginsRegister = 'REGP';

// Mesages
const	MessageT	msg_Selection = 'LIST';
const	MessageT	msg_AboutRegister = 'REGP';

// Resources

// Type
class LPushButton;
class CTextFieldX;
class CTextHierarchy;

class CAboutPluginsDialog : public LDialogBox
{
private:
	LPushButton*	mRegisterBtn;
	CTextFieldX*	mName;
	CTextFieldX*	mVersion;
	CTextFieldX*	mType;
	CTextFieldX*	mManufacturer;
	CTextFieldX*	mDescription;
	CTextFieldX*	mMode;
	CTextHierarchy*	mList;
	CPluginList		mHierPlugins;

public:
	enum { class_ID = 'AbPl' };

					CAboutPluginsDialog();
					CAboutPluginsDialog(LStream *inStream);
	virtual 		~CAboutPluginsDialog();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	virtual void	ShowPluginInfo(const CPlugin* plugin);

};

#endif
