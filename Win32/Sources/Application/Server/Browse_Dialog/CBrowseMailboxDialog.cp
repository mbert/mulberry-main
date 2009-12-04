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


// CBrowseMailboxDialog.cpp : implementation file
//

#include "CBrowseMailboxDialog.h"

#include "CFontCache.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CBrowseMailboxDialog dialog


CBrowseMailboxDialog::CBrowseMailboxDialog(bool browse_copy, CWnd* pParent /*=NULL*/)
	: CHelpDialog(browse_copy ? CBrowseMailboxDialog::IDD2 : CBrowseMailboxDialog::IDD1, pParent)
{
	//{{AFX_DATA_INIT(CBrowseMailboxDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	mBrowseCopy = browse_copy;
	mSending = false;
	mMultiple = false;
	mMbox = NULL;
	mSetAsDefault = FALSE; 
}


void CBrowseMailboxDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBrowseMailboxDialog)
	DDX_Control(pDX, IDOK, mOKButton);
	DDX_Control(pDX, IDC_SERVERBROWSE_CREATE, mCreateButton);
	if (mBrowseCopy && mSending)
	{
		DDX_Check(pDX, IDC_SERVERBROWSE_SETDEFAULT, mSetAsDefault);
		DDX_Control(pDX, IDC_SERVERBROWSE_SETDEFAULT, mSetAsDefaultBtn);
	}
	//}}AFX_DATA_MAP
	
	if (pDX->m_bSaveAndValidate)
	{
		// Do not allow exceptions to propogate out
		try
		{
			if (mMultiple)
				mTable.GetSelectedMboxes(mMboxList);
			else
				mMbox = mTable.GetSelectedMbox();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			mMbox = NULL;
		}
	}
	else
		mMbox = NULL;
}


BEGIN_MESSAGE_MAP(CBrowseMailboxDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CBrowseMailboxDialog)
	ON_BN_CLICKED(IDC_SERVERBROWSE_NOCOPY, OnBrowseMailboxNoCopy)
	ON_BN_CLICKED(IDC_SERVERBROWSE_CREATE, OnBrowseMailboxCreate)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBrowseMailboxDialog message handlers

BOOL CBrowseMailboxDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass table
	mTable.SubclassDlgItem(IDC_BROWSETABLE, this);
	mTable.SetManager();
	mTable.SetButtons(this, &mOKButton, &mCreateButton);
	mOKButton.EnableWindow(false);
	mTable.SetOpenMode(mOpenMode);
	if (mMultiple)
		mTable.SetMultipleSelection();
	mTable.ResetFont(CFontCache::GetListFont());
	mTable.InsertCols(1, 1);	// Create columns
	CRect rect;
	mTable.GetClientRect(rect);
	mTable.SetColWidth(rect.Width() - 16, 1, 1);	// Create columns
	mTable.ResetTable();

	// Hide set as default button and increase table size
	if (mBrowseCopy && !mSending)
	{
		mSetAsDefaultBtn.ShowWindow(SW_HIDE);
		::ResizeWindowBy(&mTable, 0, 20);
	}

	CRect wndRect;
	GetClientRect(wndRect);
	mOldWidth = wndRect.right;
	mOldHeight = wndRect.bottom;

	ResetState();

	return true;
}

// Hide instead of close
void CBrowseMailboxDialog::OnDestroy()
{
	// Save window state
	SaveState();
}

// Reposition items
void CBrowseMailboxDialog::OnSize(UINT nType, int cx, int cy)
{
	CHelpDialog::OnSize(nType, cx, cy);

	// Move by change
	int delta_width = cx - mOldWidth;
	int delta_height = cy - mOldHeight;

	// Only do when items actually exist
	if (GetDlgItem(IDOK))
	{
		// OK button - move right aligned
		CRect rect;
		GetDlgItem(IDOK)->GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(delta_width, 0);
		GetDlgItem(IDOK)->MoveWindow(rect);

		// No Copy button - move right aligned - may not be present
		if (mBrowseCopy)
		{
			GetDlgItem(IDC_SERVERBROWSE_NOCOPY)->GetWindowRect(rect);
			ScreenToClient(rect);
			rect.OffsetRect(delta_width, 0);
			GetDlgItem(IDC_SERVERBROWSE_NOCOPY)->MoveWindow(rect);
		}

		// Cancel button - move right aligned
		GetDlgItem(IDCANCEL)->GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(delta_width, 0);
		GetDlgItem(IDCANCEL)->MoveWindow(rect);

		// Create button - move right aligned
		mCreateButton.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(delta_width, 0);
		mCreateButton.MoveWindow(rect);

		// Set default button - move bottom aligned
		if (mBrowseCopy)
		{
			mSetAsDefaultBtn.GetWindowRect(rect);
			ScreenToClient(rect);
			rect.OffsetRect(0, delta_height);
			mSetAsDefaultBtn.MoveWindow(rect);
		}

		// Table - change width
		mTable.GetWindowRect(rect);
		ScreenToClient(rect);
		rect.right += delta_width;
		rect.bottom += delta_height;
		mTable.MoveWindow(rect);
		
		// Reset stored width
		mOldWidth = cx;
		mOldHeight = cy;
	}
}

void CBrowseMailboxDialog::OnBrowseMailboxNoCopy() 
{
	EndDialog(IDC_SERVERBROWSE_NOCOPY);
}

void CBrowseMailboxDialog::OnBrowseMailboxCreate() 
{
	mTable.SendMessage(WM_COMMAND, IDM_MAILBOX_CREATE);
	
	// Must set focus back on table
	mTable.SetFocus();
}

// Reset state from prefs
void CBrowseMailboxDialog::ResetState(void)
{
	CWindowState& state = CPreferences::sPrefs->mServerBrowseDefault.Value();

	// Do not set if empty
	CRect set_rect = state.GetBestRect(state);
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset position - keep width the same as dlog units may result in scaling
		SetWindowPos(NULL, set_rect.left, set_rect.top, set_rect.Width(), set_rect.Height(), SWP_NOZORDER);
	}
}

// Save state in prefs
void CBrowseMailboxDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;
	
	// Get bounds
	CRect bounds;
	GetWindowRect(bounds);

	// Add info to prefs
	CWindowState state(name, &bounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mServerBrowseDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerBrowseDefault.SetDirty();
}


bool CBrowseMailboxDialog::PoseDialog(bool open_mode, bool sending, CMbox*& mbox, bool& set_as_default)
{
	bool result = false;
	
	// Create the dialog
	CBrowseMailboxDialog dlog(sending, CSDIFrame::GetAppTopWindow());
	dlog.SetOpenMode(open_mode);
	dlog.SetSending(sending);

	// Let DialogHandler process events
	int dlog_result = dlog.DoModal();
	if (dlog_result == IDOK)
	{
		mbox = dlog.GetSelectedMbox();
		set_as_default = dlog.mSetAsDefault;
		result = true;
	}
	else if (dlog_result == IDC_SERVERBROWSE_NOCOPY)
	{
		mbox = (CMbox*) -1;
		result = true;
	}
	else
		mbox = NULL;
	
	return result;
}

bool CBrowseMailboxDialog::PoseDialog(CMboxList& mbox_list)
{
	bool result = false;
		
	// Create the dialog
	CBrowseMailboxDialog dlog(false, CSDIFrame::GetAppTopWindow());
	dlog.SetOpenMode(false);
	dlog.SetMultipleSelection();

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetSelectedMboxes(mbox_list);
		result = true;
	}
	
	return result;
}
