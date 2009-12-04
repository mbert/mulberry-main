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


// Header for CAdbkServerPopup class

#ifndef __CADBKSERVERPOPUP__MULBERRY__
#define __CADBKSERVERPOPUP__MULBERRY__

#include "CPopupButton.h"

#include "CINETAccount.h"
#include "CListener.h"

#include "cdstring.h"

// Consts

// Classes

class CAdbkServerPopup : public CPopupButton,
							public CListener
{

public:
					CAdbkServerPopup();
	virtual 		~CAdbkServerPopup();

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void 	InitServersMenu(CINETAccount::EINETServerType type);
	virtual void	SetValue(UINT value);
	virtual void	SyncMenu(void);
	
	virtual cdstrvect	GetServers();

protected:
	bool	mDirty;
	CINETAccount::EINETServerType mType;

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

};

#endif
