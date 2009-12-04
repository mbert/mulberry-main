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


// Header for CServerViewPopup class

#ifndef __CSERVERVIEWPOPUP__MULBERRY__
#define __CSERVERVIEWPOPUP__MULBERRY__

#include "CToolbarPopupButton.h"

#include "CListener.h"

// Consts

// Classes

class CServerViewPopup : public CToolbarPopupButton,
						 public CListener
{

public:
	enum
	{
	  eServerView_New = 1, //starts at 1 on Unix
	  //eServerView_Separator, separators aren't items in JX
	  eServerView_First
	};

	CServerViewPopup(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual 		~CServerViewPopup();
	
	virtual void	OnCreate();

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void	SyncMenu(void);

 protected:
	bool	mDirty;

	virtual void	UpdateMenu();
	
};

#endif
