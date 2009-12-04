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


// Source for CCalendarWindow class


#include "CCalendarWindow.h"

#include "CCalendarDoc.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CCalendarWindow

IMPLEMENT_DYNCREATE(CCalendarWindow, CView)

BEGIN_MESSAGE_MAP(CCalendarWindow, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWPREVIEW, OnUpdateMenuShowPreview)
	ON_COMMAND(IDM_WINDOWS_SHOWPREVIEW, OnShowPreview)
	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetDefaultState)
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWPREVIEW, OnUpdateShowPreview)
	ON_COMMAND(IDC_TOOLBARSHOWPREVIEW, OnShowPreview)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarWindow construction/destruction

cdmutexprotect<CCalendarWindow::CCalendarWindowList> CCalendarWindow::sCalendarWindows;
CMultiDocTemplate* CCalendarWindow::sCalendarDocTemplate = NULL;

// Default constructor
CCalendarWindow::CCalendarWindow()
{
	mPreviewVisible = true;
	mNode = NULL;

	{
		cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
		sCalendarWindows->push_back(this);
	}
}

// Default destructor
CCalendarWindow::~CCalendarWindow()
{
	// Deactivate the node (will close calendar if no more references to it)
	if (mNode != NULL)
		calstore::CCalendarStoreManager::sCalendarStoreManager->DeactivateNode(mNode);
	
	// Remove from list
	{
		cdmutexprotect<CCalendarWindowList>::lock _lock(sCalendarWindows);
		CCalendarWindowList::iterator found = ::find(sCalendarWindows->begin(), sCalendarWindows->end(), this);
		if (found != sCalendarWindows->end())
			sCalendarWindows->erase(found);
	}
}

void CCalendarWindow::MakeWindow(calstore::CCalendarStoreNode* node)
{
	// Look for existing window with this node
	
	CCalendarWindow* newWindow = NULL;
	try
	{
		// Activate the node
		if (node != NULL)
			calstore::CCalendarStoreManager::sCalendarStoreManager->ActivateNode(node);
		
		// Create the message window
		newWindow = CCalendarWindow::ManualCreate();
		newWindow->SetNode(node);
		newWindow->ShowWindow(SW_SHOW);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete if it still exists
		if (WindowExists(newWindow))
			FRAMEWORK_DELETE_WINDOW(newWindow);

		// Should throw out of here
		CLOG_LOGRETHROW;
		throw;
	}
}

// Create it or bring it to the front
void CCalendarWindow::CreateSubscribedWindow()
{
	// Create find & replace window or bring to front
	CCalendarWindow* found = FindWindow(NULL);
	if (found != NULL)
	{
		FRAMEWORK_WINDOW_TO_TOP(found)
	}
	else
	{
		MakeWindow(NULL);
	}
}

// Manually create document
CCalendarWindow* CCalendarWindow::ManualCreate()
{
	CDocument* aDoc = sCalendarDocTemplate->OpenDocumentFile(NULL, false);
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CCalendarWindow)))
		return (CCalendarWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CCalendarWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// Do inherited
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Create toolbar pane
	CRect rect = CRect(0, 0, cWindowWidth, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), IDC_STATIC);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);

	// Create splitter view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mSplitterView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mToolbarView.SetSibling(&mSplitterView);
	
	// Create calendar views
	mCalendarView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mPreview.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	GetCalendarView()->SetPreview(&mPreview);
	GetCalendarView()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());
	
	// Install the splitter items
	mSplitterView.InstallViews(GetCalendarView(), &mPreview, false);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetCalendarViewOptions().GetShowPreview())
		ShowPreview(false);

	GetCalendarView()->InitView();
	GetCalendarView()->Focus();

	return 0;
}

// Adjust splits
void CCalendarWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	CRect rect(0, 0, cx, cy);
	mSplitterView.MoveWindow(rect);
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CView::OnSize(nType, cx, cy);
}

// Get child windows
void CCalendarWindow::OnInitialUpdate(void)
{
	// Force focus to view
	if (GetCalendarView())
		GetCalendarView()->SetFocus();
}

BOOL CCalendarWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route command to current target
	if (CCommander::ProcessCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Tell window to focus on this one
void CCalendarWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to view
	if (GetCalendarView())
		GetCalendarView()->SetFocus();
}

void CCalendarWindow::OnUpdateMenuShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	::OnUpdateMenuTitle(pCmdUI, txt);
}

void CCalendarWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

// Tidy up when window closed
void CCalendarWindow::OnShowPreview(void)
{
	ShowPreview(!mPreviewVisible);
}

void CCalendarWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change preview state
	mSplitterView.ShowView(true, show);
	GetCalendarView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetCalendarViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetCalendarViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}

// Reset to default state
void CCalendarWindow::OnSaveDefaultState(void)
{
	SaveDefaultState();
}

// Reset to default state
void CCalendarWindow::OnResetDefaultState(void)
{
	ResetState(true);
}
