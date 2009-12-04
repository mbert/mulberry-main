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


// CSDIDoc.h : interface of the CSDIDoc class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __CSDIDOC__MULBERRY__
#define __CSDIDOC__MULBERRY__

class CSDIDoc : public CDocument
{
protected: // create from serialization only
	CSDIDoc() {}
	DECLARE_DYNCREATE(CSDIDoc)

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual ~CSDIDoc() {}

	virtual void SetTitle(LPCTSTR lpszTitle);

// Generated message map functions
protected:
	//{{AFX_MSG(CSDIDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif
