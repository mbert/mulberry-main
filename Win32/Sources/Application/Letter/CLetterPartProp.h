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


// CLetterPartProp.h : header file
//

#ifndef __CLETTERPARTPROP__MULBERRY__
#define __CLETTERPARTPROP__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp dialog

class CMIMEContent;

class CLetterPartProp : public CHelpDialog
{
// Construction
public:
	CLetterPartProp(CWnd* pParent = NULL);   // standard constructor
	~CLetterPartProp();

	virtual void GetFields(CMIMEContent& content);
	virtual void SetFields(CMIMEContent& content);

// Dialog Data
	//{{AFX_DATA(CLetterPartProp)
	enum { IDD = IDD_LETTERPARTSPROP };
	CComboBox	mTypeCtrl;
	CComboBox	mSubtypeCtrl;
	CComboBox	mEncodingCtrl;
	CComboBox	mDispositionCtrl;
	CComboBox	mCharsetCtrl;
	CStatic		mCharsetTitleCtrl;
	cdstring	mName;
	CString		mType;
	CString		mSubtype;
	int			mEncoding;
	cdstring	mDescription;
	int			mDisposition;
	int			mCharset;
	//}}AFX_DATA
	bool		mHasDisposition;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLetterPartProp)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void SetSubtypeCombo(CString& str);

	// Generated message map functions
	//{{AFX_MSG(CLetterPartProp)
		// NOTE: the ClassWizard will add member functions here
	virtual BOOL OnInitDialog();
	afx_msg void OnEditChangeType();
	afx_msg void OnSelChangeType();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
