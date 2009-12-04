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
		eServerView_New = 0,
		eServerView_Separator,
		eServerView_First
	};

					CServerViewPopup();
	virtual 		~CServerViewPopup();

	virtual BOOL Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd,
							UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel = 0,
							UINT nIDIconPushed = 0, UINT nIDIconPushedSel = 0, bool frame = true);

	virtual void	ListenTo_Message(long msg, void* param);	// Respond to list changes

	virtual void	SyncMenu(void);

protected:
	bool	mDirty;

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

};

#endif
