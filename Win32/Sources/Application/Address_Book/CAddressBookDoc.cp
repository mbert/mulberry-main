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


// CAddressBookDoc.cp : implementation of the CAddressBookDoc class
//


#include "CAddressBookDoc.h"

#include "CAddressBook.h"
#include "CAddressBookWindow.h"
#include "CErrorHandler.h"
#include "CLocalAddressBook.h"
#include "CLog.h"

#define COMMA_SPACE			", "

const char cAnon[]				= "Anonymous";
const char cGroupHeader[]		= "Group:";
const char cNewGroupHeader[]	= "Grp:";

/////////////////////////////////////////////////////////////////////////////
// CAddressBookDoc

IMPLEMENT_DYNCREATE(CAddressBookDoc, CSDIDoc)

BEGIN_MESSAGE_MAP(CAddressBookDoc, CSDIDoc)
	//{{AFX_MSG_MAP(CAddressBookDoc)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(IDM_FILE_REVERT, OnUpdateFileRevert)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(IDM_FILE_REVERT, OnFileRevert)

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressBookDoc construction/destruction

CAddressBookDoc::CAddressBookDoc()
{
	// TODO: add one-time construction code here
	mAddressBookWnd = NULL;
}

CAddressBookDoc::~CAddressBookDoc()
{
}

void CAddressBookDoc::InitDoc(CAddressBook* adbk)
{
	// try to recover window if not already set
	if (!mAddressBookWnd)
	{
		POSITION pos = GetFirstViewPosition();
		CView* pView = GetNextView(pos);
		CFrameWnd* aFrame = pView->GetParentFrame();
		mAddressBookWnd = static_cast<CAddressBookWindow*>(aFrame->GetTopWindow());
	}

	mAddressBookWnd->SetAddressBook(adbk);
	
	SetTitle(adbk->GetName().win_str());
}

CAddressBook* CAddressBookDoc::GetAddressBook()
{
	return mAddressBookWnd->GetAddressBook();
}

// Is closing allowed
BOOL CAddressBookDoc::CanCloseFrame(CFrameWnd* pFrame)
{
	// Look for dangling messages then process close actions
	if (!GetAddressBookWindow()->GetAddressBookView()->TestClose())
		return false;

	// Close the view  - this will close the actual window at idle time
	GetAddressBookWindow()->GetAddressBookView()->DoClose();
	
	return true;
}

// Save data to file
void CAddressBookDoc::Serialize(CArchive& ar)
{
	// Check Open or Save
	if (ar.IsLoading())	// Opening
	{
		try
		{
			ReadFile(ar);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// must trap all exceptions since CArchiveException trap not working
		}

		// Now reset the address book display
		mAddressBookWnd->ResetAddressBook();

		// Force window to update
		mAddressBookWnd->RedrawWindow();
	}
	else
		SaveFile(ar);
}

void CAddressBookDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(dynamic_cast<CLocalAddressBook*>(GetAddressBook()) ? true : false);	// Only if dirty
}

void CAddressBookDoc::OnUpdateFileRevert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(dynamic_cast<CLocalAddressBook*>(GetAddressBook()) && mAddressBookWnd->IsDirty());	// Only if dirty
}

void CAddressBookDoc::OnFileClose(void)
{
	// Do save first
	if (!SaveModified())
		return;

	// Only do after warning
	POSITION pos = GetFirstViewPosition();
	CView* pView = GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	if (!CanCloseFrame(aFrame))
		return;

	// shut it down
	OnCloseDocument();
		// this should destroy the document
}

void CAddressBookDoc::OnFileSave()
{
	// Do inherited
	CSDIDoc::OnFileSave();
	
	// Check for name change
	if (cdstring(GetTitle()) != GetAddressBook()->GetName())
		GetAddressBook()->SetName(cdstring(GetTitle()));
}

void CAddressBookDoc::OnFileSaveAs()
{
	// Do inherited
	CSDIDoc::OnFileSaveAs();
	
	// Check for name change
	if (cdstring(GetTitle()) != GetAddressBook()->GetName())
		GetAddressBook()->SetName(cdstring(GetTitle()));
}

void CAddressBookDoc::OnFileRevert(void)
{
	// Delete items in existing lists
	GetAddressBook()->Clear();
	

	try
	{
		// Reopen and read data
		OnOpenDocument(GetPathName());
	}
	catch (CFileException* ex)
	{
		CLOG_LOGCATCH(CFileException*);

		// Inform user
		CErrorHandler::PutFileErrAlertRsrc("Alerts::Adbk::RevertError", *ex);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Inform user
		CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::RevertError");
	}
}

// Read data from file
void CAddressBookDoc::ReadFile(CArchive& ar)
{
	// Read in string to CR
	CString s;
	char temp;
	while(ar.Read(&temp, 1) && (temp != '\r'))
	{
		s += temp;
	}

	while (!s.IsEmpty())
	{
		cdstring line = s.GetBuffer(0);
		s.ReleaseBuffer(-1);
		s.Empty();
		GetAddressBook()->ImportAddress(line.c_str_mod(), true, NULL, NULL);

		// Read in string to CR, ignore LF
		while(ar.Read(&temp, 1) && (temp != '\r'))
		{
			if (temp != '\n')
				s += temp;
		}

	}
}

void CAddressBookDoc::SaveFile(CArchive& ar)
{
	ExportTabbedAddresses(ar);
}

void CAddressBookDoc::ExportTabbedAddresses(CArchive& ar)
{
	for(CAddressList::const_iterator iter = GetAddressBook()->GetAddressList()->begin(); iter != GetAddressBook()->GetAddressList()->end(); iter++)
	{
		const char* out_addr = GetAddressBook()->ExportAddress(static_cast<const CAdbkAddress*>(*iter));
		ar.Write(out_addr, ::strlen(out_addr));
		delete out_addr;
	}

	for(CGroupList::iterator iter = GetAddressBook()->GetGroupList()->begin(); iter != GetAddressBook()->GetGroupList()->end(); iter++)
	{
		const char* out_grp = GetAddressBook()->ExportGroup(*iter);
		ar.Write(out_grp, ::strlen(out_grp));
		delete out_grp;
	}
}

