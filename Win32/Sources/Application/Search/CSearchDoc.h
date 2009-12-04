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


// CSearchDoc.h : interface of the CSearchDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CSEARCHDOC__MULBERRY__
#define __CSEARCHDOC__MULBERRY__

#include "CSDIDoc.h"

class CSearchDoc : public CSDIDoc
{
protected: // create from serialization only
	CSearchDoc() {}
	DECLARE_DYNCREATE(CSearchDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CSearchDoc() {}

// Generated message map functions
protected:
	//{{AFX_MSG(CSearchDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
