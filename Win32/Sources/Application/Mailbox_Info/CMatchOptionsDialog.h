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


// CMatchOptionsDialog.h : header file
//

#ifndef __CMATCHOPTIONSDIALOG__MULBERRY__
#define __CMATCHOPTIONSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CMboxFwd.h"

class CCheckboxTable;
class CMatchItem;

class CMatchOptionsDialog : public CHelpDialog
{
// Construction
public:
	CMatchOptionsDialog(CWnd* pParent = NULL);   // standard constructor

	static bool		PoseDialog(NMbox::EViewMode& mbox_mode, CMatchItem& match, bool& last_search);

// Dialog Data
	//{{AFX_DATA(CMatchOptionsDialog)
	enum { IDD = IDD_MATCHOPTIONS };
	int				mViewAllNoHilight;
	int				mMatch;
	int				mOr;
	BOOL			mNew;
	BOOL			mUnseen;
	BOOL			mImportant;
	BOOL			mAnswered;
	BOOL			mDeleted;
	BOOL			mDraft;
	BOOL			mSentToday;
	BOOL			mSentYesterday;
	BOOL			mSentThisWeek;
	BOOL			mSent7Days;
	BOOL			mSentByMe;
	BOOL			mSentToMe;
	BOOL			mSentToList;
	BOOL			mSelectedTo;
	BOOL			mSelectedFrom;
	BOOL			mSelectedSmart;
	BOOL			mSelectedSubject;
	BOOL			mSelectedDate;
	CCheckListBox	mTable;
	//}}AFX_DATA
	cdstrvect		mNames;
	boolvector		mSelected;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatchOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void SetMatchItems(NMbox::EViewMode mbox_mode, const CMatchItem& match, bool last_search);
	CMatchItem GetMatchItems(NMbox::EViewMode& mbox_mode, bool& last_search) const;

	// Generated message map functions
	//{{AFX_MSG(CMatchOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
