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


// CAdbkManagerWindow.cp : implementation of the CAdbkManagerWindow class
//



#include "CAdbkManagerWindow.h"

#include "CAddressBookManager.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CPreferences.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerWindow

IMPLEMENT_DYNCREATE(CAdbkManagerWindow, CView)

BEGIN_MESSAGE_MAP(CAdbkManagerWindow, CTableViewWindow)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdbkManagerWindow construction/destruction

CAdbkManagerWindow* CAdbkManagerWindow::sAdbkManager = NULL;
CMultiDocTemplate* CAdbkManagerWindow::sAdbkManagerDocTemplate = NULL;

CAdbkManagerWindow::CAdbkManagerWindow()
{
	sAdbkManager = this;
	mHidden = false;
}

CAdbkManagerWindow::~CAdbkManagerWindow()
{
	sAdbkManager = NULL;
}

void CAdbkManagerWindow::CreateAdbkManagerWindow()
{
	// Create find & replace window or bring to front
	if (sAdbkManager)
	{
		FRAMEWORK_WINDOW_TO_TOP(sAdbkManager)
	}
	else
	{
		// Create Adbk manager window
		CAdbkManagerWindow* adbkman = CAdbkManagerWindow::ManualCreate();
		adbkman->ResetState();
		adbkman->GetParentFrame()->ShowWindow(SW_SHOW);
	}
}

void CAdbkManagerWindow::DestroyAdbkManagerWindow()
{
	FRAMEWORK_DELETE_WINDOW(sAdbkManager)
	sAdbkManager = NULL;
}

// Manually create document
CAdbkManagerWindow* CAdbkManagerWindow::ManualCreate(void)
{
	CDocument* aDoc = sAdbkManagerDocTemplate->OpenDocumentFile(nil, false);
	CString strDocName;
	sAdbkManagerDocTemplate->GetDocString(strDocName, CDocTemplate::docName);
	aDoc->SetTitle(strDocName);

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CAdbkManagerWindow)))
		return (CAdbkManagerWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return nil;
}

int CAdbkManagerWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
	mAdbkView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mAdbkView);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(GetAdbkManagerView());
	GetAdbkManagerView()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetAdbkManagerView()->GetTable()->SetManager(CAddressBookManager::sAddressBookManager);
	GetAdbkManagerView()->SetOpen();

	// Add import/export menu itemss
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

// Control show/hide
void CAdbkManagerWindow::OnShowWindow(BOOL bShow, UINT nStatus)
{
	// Update hidden flag only when app is not quitting
	if (!CMulberryApp::sApp->IsQuitting())
		mHidden = !bShow;

	// Do default
	CView::OnShowWindow(bShow, nStatus);
}

