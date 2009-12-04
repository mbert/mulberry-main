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


// CServerWindow.cp : implementation of the CServerWindow class
//



#include "CServerWindow.h"

#include "CErrorDialog.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "C3PaneWindow.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CServerWindow

IMPLEMENT_DYNCREATE(CServerWindow, CView)

BEGIN_MESSAGE_MAP(CServerWindow, CTableViewWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerWindow construction/destruction

cdmutexprotect<CServerWindow::CServerWindowList> CServerWindow::sServerWindows;
CMultiDocTemplate* CServerWindow::sServerDocTemplate = NULL;

CServerWindow::CServerWindow()
{
	// Add to list
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		sServerWindows->push_back(this);
	}
}

CServerWindow::~CServerWindow()
{
}

// Manually create document
CServerWindow* CServerWindow::ManualCreate(void)
{
	CDocument* aDoc = sServerDocTemplate->OpenDocumentFile(nil, false);
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CServerWindow)))
		return (CServerWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return nil;
}

// Check for window
bool CServerWindow::WindowExists(const CServerWindow* wnd)
{
	cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
	CServerWindowList::iterator found = ::find(sServerWindows->begin(), sServerWindows->end(), wnd);
	return found != sServerWindows->end();
}

// Hide instead of close
bool CServerWindow::AttemptClose()
{
	// Save window state
	SaveState();

	// See whether at least one server window is still visible
	bool one_visible = false;
	{
		cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
		for(CServerWindow::CServerWindowList::iterator iter = CServerWindow::sServerWindows->begin(); iter != CServerWindow::sServerWindows->end(); iter++)
		{
			if (((*iter) != this) && !(*iter)->IsIconic())
			{
				one_visible = true;
				break;
			}
		}
	}

	// If there are no 1-pane windows left and no 3-pane window the user
	// may be intending to quit
	if (!one_visible && !C3PaneWindow::s3PaneWindow)
	{
		// Ask user for quit
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ServerCloseQuit",
																		"ErrorDialog::Btn::ServerCloseClose",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::ServerClose", 3);

		if (result == CErrorDialog::eBtn2)
			// Cancel close attempt
			return false;
		else if (result == CErrorDialog::eBtn1)
		{
			// Do app quit
			CMulberryApp::sApp->GetMainWnd()->PostMessage(WM_CLOSE);
			return false;
		}
	}

	// Just hide if manager
	if (GetServerView()->GetTable()->IsManager())
	{
		GetParentFrame()->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		return false;
	}
	else
		// Carry on with close attempt
		return true;
}

int CServerWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableViewWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Create toolbar pane
	CRect rect = CRect(0, 0, cWindowWidth, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), IDC_STATIC);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);

	// Create server view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mServerView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mServerView);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(GetServerView());
	GetServerView()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetServerView()->SetOpen();

	return 0;
}

void CServerWindow::OnDestroy()
{
	// Remove from list
	cdmutexprotect<CServerWindowList>::lock _lock(sServerWindows);
	CServerWindowList::iterator found = ::find(sServerWindows->begin(), sServerWindows->end(), this);
	if (found != sServerWindows->end())
		sServerWindows->erase(found);
}
