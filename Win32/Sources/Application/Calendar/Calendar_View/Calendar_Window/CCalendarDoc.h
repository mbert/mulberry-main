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


// CCalendarDoc.h : interface of the CCalendarDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CCalendarDoc__MULBERRY__
#define __CCalendarDoc__MULBERRY__

#include "CSDIDoc.h"

// Classes

class CCalendarDoc : public CSDIDoc
{
protected: // create from serialization only
	CCalendarDoc() {}
	DECLARE_DYNCREATE(CCalendarDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CCalendarDoc() {}

// Generated message map functions
protected:
	//{{AFX_MSG(CCalendarDoc)
	afx_msg void	OnFileClose(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
