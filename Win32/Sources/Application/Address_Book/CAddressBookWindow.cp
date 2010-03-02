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


// Source for CAddressBookWindow class


#include "CAddressBookWindow.h"

#include "CAddressBook.h"
#include "CAddressBookDoc.h"
#include "CAddressBookDocTemplate.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CAddressBookWindow

IMPLEMENT_DYNCREATE(CAddressBookWindow, CView)

BEGIN_MESSAGE_MAP(CAddressBookWindow, CTableViewWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWPREVIEW, OnUpdateMenuShowPreview)
	ON_COMMAND(IDM_WINDOWS_SHOWPREVIEW, OnShowPreview)
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWPREVIEW, OnUpdateShowPreview)
	ON_COMMAND(IDC_TOOLBARSHOWPREVIEW, OnShowPreview)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddressBookWindow construction/destruction

CAddressBookDocTemplate* CAddressBookWindow::sAddressBookDocTemplate = nil;
cdmutexprotect<CAddressBookWindow::CAddressBookWindowList> CAddressBookWindow::sAddressBookWindows;

// Default constructor
CAddressBookWindow::CAddressBookWindow()
{
	mPreviewVisible = true;

	{
		cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
		sAddressBookWindows->push_back(this);
	}
}

// Default destructor
CAddressBookWindow::~CAddressBookWindow()
{
	// Remove from list
	{
		cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
		CAddressBookWindowList::iterator found = std::find(sAddressBookWindows->begin(), sAddressBookWindows->end(), this);
		if (found != sAddressBookWindows->end())
			sAddressBookWindows->erase(found);
	}
}

// Manually create document
CAddressBookDoc* CAddressBookWindow::ManualCreate(CAddressBook* adbk, const char* fname)
{
	CDocument* aDoc = sAddressBookDocTemplate->OpenDocumentFile(adbk, cdstring(fname).win_str(), false);
	if (!aDoc)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();
	CAddressBookWindow* pWnd = static_cast<CAddressBookWindow*>(aFrame->GetTopWindow());

	// Give it to document
	((CAddressBookDoc*) aDoc)->SetAddressBookWindow(pWnd);
	
	// Reset lists after load
	pWnd->SetAddressBook(adbk);

	return (CAddressBookDoc*) aDoc;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

bool CAddressBookWindow::OpenWindow(CAddressBook* adbk)
{
	// Does window already exist?
	CAddressBookWindow* theWindow = FindWindow(adbk);

	if (theWindow)
	{
		// Found existing window so make visible and select
		FRAMEWORK_WINDOW_TO_TOP(theWindow)
		
		return true;
	}
	else
	{
		CAddressBookDoc* doc = NULL;
		try
		{
			doc = CAddressBookWindow::ManualCreate(adbk, NULL);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		
		return doc != NULL;
	}
}

CAddressBookWindow* CAddressBookWindow::FindWindow(const CAddressBook* adbk)
{
	cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
	for(CAddressBookWindowList::const_iterator iter = sAddressBookWindows->begin(); iter != sAddressBookWindows->end(); iter++)
	{
		if (adbk == (*iter)->GetAddressBook())
			return *iter;
	}

	return NULL;
}

// Check for window
bool CAddressBookWindow::WindowExists(const CAddressBookWindow* wnd)
{
	cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
	CAddressBookWindowList::iterator found = std::find(sAddressBookWindows->begin(), sAddressBookWindows->end(), wnd);
	return found != sAddressBookWindows->end();
}

int CAddressBookWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Do inherited
	if (CTableViewWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Create toolbar pane
	CRect rect = CRect(0, 0, cWindowWidth, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), ID_VIEW_TOOLBAR);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);

	// Create splitter view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mSplitterView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	
	// Create mailbox view
	mAddressBookView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mAddressBookView);
	
	// Create message view
	mAddressView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	GetAddressBookView()->SetPreview(&mAddressView);

	// Install the splitter items
	mSplitterView.InstallViews(GetAddressBookView(), &mAddressView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(&mSplitterView);
	GetAddressBookView()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetAddressBookView()->SetOpen();

	// Add import/export menu items
	CMenu* menu = GetParentFrame()->GetMenu();
	CMenu* addr_menu = menu->GetSubMenu(2);
	for(int i = 0; i < addr_menu->GetMenuItemCount(); i++)
	{
		if (addr_menu->GetMenuItemID(i) == IDM_ADDR_RENAME)
		{
			CMenu* import_menu = addr_menu->GetSubMenu(i + 2);
			CMenu* export_menu = addr_menu->GetSubMenu(i + 3);
			CPluginManager::sPluginManager.SyncAdbkIOPluginMenu(import_menu, export_menu);
			break;
		}
	}

	return 0;
}

// Adjust splits
void CAddressBookWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	CRect rect(0, 0, cx, cy);
	mSplitterView.MoveWindow(rect);
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CView::OnSize(nType, cx, cy);
}

// Tell window to focus on this one
void CAddressBookWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to table
	if (GetAddressBookView())
		GetAddressBookView()->SetFocus();
}

//	Set dirty state
void CAddressBookWindow::SetDirty(bool dirty)
{
	//GetAddressBookView()->SetDirty(dirty);
}

// Get dirty state
bool CAddressBookWindow::IsDirty(void)
{
	//return 	GetAddressBookView()->IsDirty();
	return false;
}

CAddressBook* CAddressBookWindow::GetAddressBook()
{
	return GetAddressBookView()->GetAddressBook();
}

// Set address and group lists
void CAddressBookWindow::SetAddressBook(CAddressBook* adbk)
{
	GetAddressBookView()->SetAddressBook(adbk);
}

// Refresh address and group lists
void CAddressBookWindow::ResetAddressBook()
{
	GetAddressBookView()->ResetAddressBook();
}

void CAddressBookWindow::OnUpdateMenuShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	::OnUpdateMenuTitle(pCmdUI, txt);
}

void CAddressBookWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

// Tidy up when window closed
void CAddressBookWindow::OnShowPreview(void)
{
	ShowPreview(!mPreviewVisible);
}

void CAddressBookWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change preview state
	mSplitterView.ShowView(true, show);
	GetAddressBookView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetAddressViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}

