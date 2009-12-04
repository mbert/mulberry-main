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

#include "CPickPopup.h"
#include "CINETAccount.h"

#include "CListener.h"

#include "cdstring.h"

// Consts

enum
{
	eServerPopup_All = 0,
	eServerPopup
};

// Classes

class CAdbkServerPopup : public CPickPopup,
							public CListener
{

public:
	enum { class_ID = 'Apop' };

	enum
	{
		eServerView_New = 1,
		eServerView_Separator,
		eServerView_First
	};

					CAdbkServerPopup(LStream *inStream);
	virtual 		~CAdbkServerPopup();

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual	void	SetValue(SInt32 inValue);
	
	virtual void 	InitAccountMenu(CINETAccount::EINETServerType type);
	virtual void 	SetupCurrentMenuItem(MenuHandle inMenuH, SInt16 inCurrentItem);
	virtual void 	SyncMenu(void);

	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);
	
	virtual cdstrvect	GetServers();
	
private:
	bool mActive;
	bool mDirty;
	CINETAccount::EINETServerType mType;
};

#endif
