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


// CCmdBackground.h : header file
//

#ifndef __CCMDBACKGROUND__MULBERRY__
#define __CCMDBACKGROUND__MULBERRY__

#include "CCommanderProtect.h"

/////////////////////////////////////////////////////////////////////////////
// CCmdBackground window

class CCmdBackground : public CStatic
{
	DECLARE_DYNAMIC(CCmdBackground)

// Construction
public:
	CCmdBackground();
	virtual ~CCmdBackground();

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

protected:
	CCommanderProtect		mCmdProtect;					// Protect commands

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
