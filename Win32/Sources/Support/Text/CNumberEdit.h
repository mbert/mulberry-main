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


// CNumberEdit.h : header file
//

#ifndef __CNUMBEREDIT__MULBERRY__
#define __CNUMBEREDIT__MULBERRY__

#include "CCmdEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CNumberEdit window

class CNumberEdit : public CCmdEdit
{
// Construction
public:
	CNumberEdit();
	virtual ~CNumberEdit();

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, bool read_only = false);

	virtual void SetValue(int value);		// Set text from number
	virtual int GetValue(void) const;		// Get number from text

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
