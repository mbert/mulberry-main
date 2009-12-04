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


// CMessagePartProp.h : header file
//

#ifndef __CMESSAGEPARTPROP__MULBERRY__
#define __CMESSAGEPARTPROP__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp dialog

class CAttachment;

class CMessagePartProp : public CHelpDialog
{
// Construction
public:
	CMessagePartProp(CWnd* pParent = NULL);   // standard constructor
	~CMessagePartProp();

	virtual void SetFields(CAttachment& attach, bool multi);	

// Dialog Data
	//{{AFX_DATA(CMessagePartProp)
	enum { IDD = IDD_MESSAGEPARTSPROP };
	CButton	mCancelAll;
	cdstring	mName;
	cdstring	mType;
	cdstring	mEncoding;
	cdstring	mID;
	cdstring	mDisposition;
	cdstring	mDescription;
	cdstring	mParams;
	cdstring	mOpenWith;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMessagePartProp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mMultiview;

	// Generated message map functions
	//{{AFX_MSG(CMessagePartProp)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
