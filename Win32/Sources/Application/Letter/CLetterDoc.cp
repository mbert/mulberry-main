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


// CLetterDoc.cp : implementation of the CLetterDoc class
//


#include "CLetterDoc.h"

#include "CAdminLock.h"
#include "CCopyToMenu.h"
#include "CErrorDialog.h"
#include "CLetterFrame.h"
#include "CLetterWindow.h"
#include "CLocalCommon.h"
#include "CMailAccountManager.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSaveDraftDialog.h"
#include "CStringUtils.h"
#include "CUnicodeStdLib.h"

#include "diriterator.h"

unsigned long	CLetterDoc::sTemporaryCount = 1;
const char* cTemporaryDraftName = "MulberryDraft";
const char* cTemporaryDraftExtension = ".mbd";
const char* cRecoveredDraftName = "Recovered Draft ";

/////////////////////////////////////////////////////////////////////////////
// CLetterDoc

IMPLEMENT_DYNCREATE(CLetterDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CLetterDoc, CSDIDoc)
	//{{AFX_MSG_MAP(CLetterDoc)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_UPDATE_COMMAND_UI(IDM_FILE_REVERT, OnUpdateAdminLockDraftSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterDoc construction/destruction

CLetterDoc::CLetterDoc()
{
	// TODO: add one-time construction code here
	mLtrWnd = NULL;
	mSaveToChoiceDone = false;
	mSaveToMailbox = false;
}

CLetterDoc::~CLetterDoc()
{
	DeleteTemporary();
}

/////////////////////////////////////////////////////////////////////////////
// CLetterDoc commands

void CLetterDoc::SaveTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue() || (mLtrWnd == NULL))
		return;

	cdstring newTemporary;

	try
	{
		// Find temporary folder
		cdstring dir = CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory();

		// Find unused temp name
		while(true)
		{
			cdstring tempname(cTemporaryDraftName);
			tempname += cdstring(sTemporaryCount++);
			tempname += cTemporaryDraftExtension;

			newTemporary = dir;
			::addtopath(newTemporary, tempname);
			if (!::fileexists(newTemporary))
				break;
		}

		// Create new file
		mLtrWnd->SafetySave(newTemporary);

		// Delete the existing file here
		DeleteTemporary();
		
		// Assign temporary file
		mTemporary = newTemporary;
		
		// Reset timer
		mLtrWnd->ResetAutoSaveTimer();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

unsigned long CLetterDoc::SaveAllTemporary()
{
	// Init counters
	unsigned long total = 0;

	// Protect against all exceptions
	try
	{
		// Now look for any drafts
		cdmutexprotect<CLetterWindow::CLetterWindowList>::lock _lock(CLetterWindow::sLetterWindows);
		for(CLetterWindow::CLetterWindowList::iterator iter = CLetterWindow::sLetterWindows->begin(); iter != CLetterWindow::sLetterWindows->end(); iter++)
		{
			// Adjust counters and attempt safety save
			static_cast<CLetterDoc*>((*iter)->GetDocument())->SaveTemporary();
			total++;
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
	
	return total;
}

void CLetterDoc::DeleteTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	try
	{
		if (!mTemporary.empty() && ::fileexists(mTemporary))
			::remove_utf8(mTemporary);
		mTemporary.clear();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CLetterDoc::ReadTemporary()
{
	// Only if allowed by preferences and admin
	if (CAdminLock::sAdminLock.mNoLocalDrafts || (CPreferences::sPrefs != NULL) && !CPreferences::sPrefs->mAutoSaveDrafts.GetValue())
		return;

	unsigned long ctr = 1;
	try
	{
		// Find temporary folder
		cdstring dir = CConnectionManager::sConnectionManager.GetSafetySaveDraftDirectory();

		diriterator diter(dir, cTemporaryDraftExtension);
		const char* fname = NULL;
		while(diter.next(&fname))
		{
			// Convert to full path and attempt open
			cdstring fullpath = dir;
			::addtopath(fullpath, fname);
			
			// Create new draft from the temp file
			MakeTemporary(fullpath, ctr++);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Not much we can do so do not throw up
	}
}

void CLetterDoc::MakeTemporary(const cdstring& path, unsigned long ctr)
{
	// Protect against all execptions so that failure to open this one
	// will not prevent others from opening
	try
	{
		// Use default MFC document open - this assumes the file extension is
		// properly set to .mbd
		CLetterDoc* doc = dynamic_cast<CLetterDoc*>(CMulberryApp::sApp->OpenDocumentFile(path.win_str()));
		if (doc != NULL)
		{
			// Now make sure draft nows about the current temp file
			doc->mTemporary = path;

			// Give window a suitable title
			cdstring tempname(cRecoveredDraftName);
			tempname += cdstring(ctr);
			doc->SetTitle(tempname.win_str());
		}
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}
}

BOOL CLetterDoc::IsModified()
{
	return (mLtrWnd ? mLtrWnd->IsDirty() : CSDIDoc::IsModified());
}

// Update command
void CLetterDoc::OnUpdateAdminLockDraftSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!CAdminLock::sAdminLock.mNoLocalDrafts);
}

// Is closing allowed
BOOL CLetterDoc::SaveModified()
{
	// Check if save locked out
	if (CAdminLock::sAdminLock.mNoLocalDrafts && IsModified())
	{
		// See whether hide is really wanted
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ServerCloseClose",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		NULL,
																		"ErrorDialog::Text::NoLocalDraftSave");
		if (result == CErrorDialog::eBtn1)
			return true;
		else
			return false;
	}
	else
		// Do default action
		return CSDIDoc::SaveModified();
}

void CLetterDoc::OnFileSaveAs()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		DoSave(NULL);
		break;

	case eSaveDraftToMailbox:
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		break;

	case eSaveDraftChoose:
		{
			CSaveDraftDialog::SSaveDraft details;
			if (CSaveDraftDialog::PoseDialog(details, !CAdminLock::sAdminLock.mNoLocalDrafts))
			{
				mSaveToMailbox = !details.mFile;
				mMailboxSave = details.mMailboxName;
				if (mSaveToMailbox)
					DoMailboxSave();
				else
					DoSave(NULL);
			}
		}
	}
}

BOOL CLetterDoc::DoFileSave()
{
	// Look at preference option
	switch(CPreferences::sPrefs->mSaveOptions.GetValue().GetValue())
	{
	case eSaveDraftToFile:
	default:
		// Do normal file save to disk
		return CSDIDoc::DoFileSave();

	case eSaveDraftToMailbox:
		// Save to specified mailbox
		mMailboxSave = CPreferences::sPrefs->mSaveMailbox.GetValue();
		DoMailboxSave();
		return true;

	case eSaveDraftChoose:
		// Check whether choice has already been done on this document
		if (!mSaveToChoiceDone)
		{
			// Get user choice for save mode
			CSaveDraftDialog::SSaveDraft details;
			if (CSaveDraftDialog::PoseDialog(details, !CAdminLock::sAdminLock.mNoLocalDrafts))
			{
				mSaveToMailbox = !details.mFile;
				mMailboxSave = details.mMailboxName;
				mSaveToChoiceDone = true;
			}
			else
				// Cancel save
				return false;
		}
		
		// Now do the user's chosen save operation
		if (mSaveToMailbox)
		{
			DoMailboxSave();
			return true;
		}
		else
			return CSDIDoc::DoFileSave();
	}
}

// Save to a mailbox
void CLetterDoc::DoMailboxSave()
{
	CMbox* mbox = NULL;

	// Must have a mailbox name
	if (mMailboxSave.empty())
	{
	}	
	// Resolve mailbox name
	else if (mMailboxSave == "\1")
	{
		// Fake mailbox popup choice to do browse dialog - always return if cancelled
		const int cPopupChoose = 1;
		if (!CCopyToMenu::GetPopupMbox(false, cPopupChoose, mbox, true))
			return;
	}
	else
	{
		// Resolve mailbox name
		mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(mMailboxSave);		
	}

	// Force mailbox choice if mailbox is missing
	if (!mbox)
	{
		// Fake mailbox popup choice to do browse dialog - return if cancelled
		const int cPopupChoose = 1;
		if (!CCopyToMenu::GetPopupMbox(false, cPopupChoose, mbox, true))
			return;
	}

	// Do copy, always as draft
	mLtrWnd->CopyNow(mbox, !CPreferences::sPrefs->mAppendDraft.GetValue());
}

// Handle illegal file name
BOOL CLetterDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	// Make file name safe for file system
	cdstring fname = m_strTitle;
	MakeSafeFileName(fname);

	// Do inherited call
	BOOL result = CSDIDoc::DoSave(lpszPathName, bReplace);

	// Recover actual title
	SetTitle(fname.win_str());
	
	// Mark as saved
	if (result)
		mLtrWnd->DraftSaved();
	
	return result;
}

// Save data to file
void CLetterDoc::Serialize(CArchive& ar)
{
	// try to recover window if not already set
	if (!mLtrWnd)
	{
		POSITION pos = GetFirstViewPosition();
		CView* pView = GetNextView(pos);
		CFrameWnd* aFrame = pView->GetParentFrame();
		mLtrWnd = static_cast<CLetterWindow*>(aFrame->GetTopWindow());
	}

	// Get window to do it
	mLtrWnd->Serialize(ar);
}