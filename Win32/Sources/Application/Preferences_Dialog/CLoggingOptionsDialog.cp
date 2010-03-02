/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CLoggingOptionsDialog.cpp : implementation file
//

#include "CLoggingOptionsDialog.h"

#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog dialog


CLoggingOptionsDialog::CLoggingOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CLoggingOptionsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoggingOptionsDialog)
	mActivate = FALSE;
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		mLogs[i] = FALSE;
	mAuthentication = FALSE;
	mPlayback = FALSE;
	mOverwrite = FALSE;
	mUserCWD = 0;
	//}}AFX_DATA_INIT
}


void CLoggingOptionsDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoggingOptionsDialog)
	DDX_Check(pDX, IDC_LOGOPTIONS_ENABLE, mActivate);
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		DDX_Check(pDX, IDC_LOGOPTIONS_IMAP + i - CLog::eLogTypeFirst, mLogs[i]);
	DDX_Check(pDX, IDC_LOGOPTIONS_AUTH, mAuthentication);
	DDX_Check(pDX, IDC_LOGOPTIONS_PLAYBACK, mPlayback);
	DDX_Check(pDX, IDC_LOGOPTIONS_OVERWRITE, mOverwrite);
	DDX_Radio(pDX, IDC_LOGOPTIONS_APPLDIR, mUserCWD);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoggingOptionsDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CLoggingOptionsDialog)
		ON_BN_CLICKED(IDC_LOGOPTIONS_CLEAR, OnLoggingClearBtn)
		ON_BN_CLICKED(IDC_LOGOPTIONS_FLUSH, OnLoggingFlushBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog message handlers

void CLoggingOptionsDialog::OnLoggingClearBtn()
{
	CLog::ClearLogs();
}

void CLoggingOptionsDialog::OnLoggingFlushBtn()
{
	CLog::FlushLogs();
}

// Set options in dialog
void CLoggingOptionsDialog::SetOptions(const CLog::SLogOptions& options)
{
	mActivate = options.mActivate;
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		mLogs[i] = options.mEnable[i];
	mAuthentication = options.mAuthentication;
	mPlayback = options.mPlayback;
	mOverwrite = options.mOverwrite;
	mUserCWD = options.mUserCWD ? 1 : 0;
}

// Get options from dialog
void CLoggingOptionsDialog::GetOptions(CLog::SLogOptions& options)
{
	options.mActivate = mActivate;
	for(unsigned long i = CLog::eLogTypeFirst; i < CLog::eLogTypeLast; i++)
		options.mEnable[i] = mLogs[i];
	options.mAuthentication = mAuthentication;
	options.mPlayback = mPlayback;
	options.mOverwrite = mOverwrite;
	options.mUserCWD = (mUserCWD == 1);
}

bool CLoggingOptionsDialog::PoseDialog(CLog::SLogOptions& options)
{
	bool result = false;

	// Create the dialog
	CLoggingOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetOptions(options);

	if (dlog.DoModal() == IDOK)
	{
		dlog.GetOptions(options);
		result = true;
	}
	
	return result;
}
