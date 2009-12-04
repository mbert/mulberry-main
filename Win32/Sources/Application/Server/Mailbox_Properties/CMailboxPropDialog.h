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


// CMailboxPropDialog.h : header file
//

#ifndef __CMAILBOXPROPDIALOG__MULBERRY__
#define __CMAILBOXPROPDIALOG__MULBERRY__

#include "CHelpPropertySheet.h"
#include "CPropMailboxOptions.h"
#include "CPropMailboxGeneral.h"
#include "CPropMailboxServer.h"
#include "CPropMailboxACL.h"
#include "CPropMailboxQuota.h"

/////////////////////////////////////////////////////////////////////////////
// CMailboxPropDialog dialog

class CMailboxPropDialog : public CHelpPropertySheet
{
	DECLARE_DYNAMIC(CMailboxPropDialog)

// Construction
public:
	CMailboxPropDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CMboxList* mbox_list)
		{ return PoseDialog(mbox_list, NULL); }
	static bool PoseDialog(CMboxProtocol* proto)
		{ return PoseDialog(NULL, proto); }
	static bool PoseDialog(CMboxList* mbox_list, CMboxProtocol* proto);

	//virtual void BuildPropPageArray();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMailboxPropDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG

private:
	CMboxList*			mMboxList;											// List of selected mboxes
	CMboxProtocol*		mMboxProtocol;									// List of selected mboxes

	CPropMailboxOptions	mPropMailboxOptions;
	CPropMailboxGeneral	mPropMailboxGeneral;
	CPropMailboxServer	mPropMailboxServer;
	CPropMailboxACL		mPropMailboxACL;
	CPropMailboxQuota	mPropMailboxQuota;

	void	SetMboxList(CMboxList* mbox_list);							// Set mbox list
	void	SetProtocol(CMboxProtocol* proto);							// Set mbox protocol

	DECLARE_MESSAGE_MAP()
};

#endif
