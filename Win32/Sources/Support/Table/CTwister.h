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


// CTwister.h : header file
//

#ifndef __CTWISTER__MULBERRY__
#define __CTWISTER__MULBERRY__

#include "CIconButton.h"

/////////////////////////////////////////////////////////////////////////////
// CTwister window

class CTwister : public CIconButton
{
	DECLARE_DYNAMIC(CTwister)

// Construction
public:
	CTwister();
	virtual ~CTwister();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
