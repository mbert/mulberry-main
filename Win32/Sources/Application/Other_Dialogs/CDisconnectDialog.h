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


// CDisconnectDialog.h : header file
//

#ifndef __CDISCONNECTDIALOG__MULBERRY__
#define __CDISCONNECTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CDisconnectListPanel.h"
#include "CMboxRefList.h"
#include "CProgress.h"

/////////////////////////////////////////////////////////////////////////////
// CDisconnectDialog dialog

class CDisconnectDialog : public CHelpDialog
{
// Construction
public:
	CDisconnectDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDisconnectDialog();

// Dialog Data
	//{{AFX_DATA(CDisconnectDialog)
	enum { IDD = IDD_DISCONNECT };
	int		mAll;
	BOOL	mList;
	BOOL	mWait;
	int		mPermanent;
	BOOL	mPlayback;
	BOOL	mPOP3;
	BOOL	mSend;
	int		mFull;
	UINT	mSize;
	BOOL	mAdbk1;
	BOOL	mAdbk2;
	CBarPane	mProgress1;
	CBarPane	mProgress2;
	//}}AFX_DATA
	bool 	mConnecting;
	bool	mProgressVisible;
	CDisconnectListPanel	mMailboxListPanel;

	static CDisconnectDialog* GetDisconnectDialog()
		{ return sDisconnectDialog; }

	CMboxRefList&	GetTargets()
		{ return mSyncTargets; }
	ulvector&		GetTargetHits()
		{ return mSyncTargetHits; }

	void 			NextItem(unsigned long item);
	void 			DoneItem(unsigned long item, bool hit);

	static  void	PoseDialog();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDisconnectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static CDisconnectDialog* sDisconnectDialog;

	CMboxRefList		mSyncTargets;
	ulvector			mSyncTargetHits;
	static bool			sIsTwisted;

	virtual void	SetDetails(bool connecting);								// Set the dialogs info
	virtual void	GetDetailsDisconnect();										// Get the dialogs return info
	virtual void	GetDetailsConnect();										// Get the dialogs return info

	virtual void	EnableMessageItems(bool enable);
			void	ShowProgressPanel(bool show);			// Show or hide the progress panel
			void	ShowPlaybackProgressPanel(bool show);	// Show or hide the progress panel

	// Generated message map functions
	//{{AFX_MSG(CDisconnectDialog)
	virtual BOOL OnInitDialog();
	virtual int DoModal();
	virtual void OnOK();
	afx_msg void OnDisconnectAll();
	afx_msg void OnDisconnectNew();
	afx_msg void OnDisconnectNone();
	afx_msg void OnDisconnectPermanent();
	afx_msg void OnDisconnectUpdate();
	afx_msg void OnDisconnectFull();
	afx_msg void OnDisconnectBelow();
	afx_msg void OnDisconnectPartial();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif