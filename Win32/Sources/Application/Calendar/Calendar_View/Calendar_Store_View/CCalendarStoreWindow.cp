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


// CCalendarStoreWindow.cp : implementation of the CCalendarStoreWindow class
//



#include "CCalendarStoreWindow.h"

#include "CCalendarStoreTable.h"
#include "CCalendarWindow.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "C3PaneWindow.h"

#include "CCalendarStoreManager.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CCalendarStoreWindow

IMPLEMENT_DYNCREATE(CCalendarStoreWindow, CView)

BEGIN_MESSAGE_MAP(CCalendarStoreWindow, CTableViewWindow)
	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCalendarStoreWindow construction/destruction

CCalendarStoreWindow*	CCalendarStoreWindow::sCalendarStoreManager = NULL;
CMultiDocTemplate* CCalendarStoreWindow::sCalendarStoreDocTemplate = NULL;

CCalendarStoreWindow::CCalendarStoreWindow()
{
	sCalendarStoreManager = this;
}

CCalendarStoreWindow::~CCalendarStoreWindow()
{
	sCalendarStoreManager = NULL;
}

// Create it or bring it to the front
void CCalendarStoreWindow::CreateCalendarStoreWindow()
{
	// Create find & replace window or bring to front
	if (sCalendarStoreManager)
	{
		FRAMEWORK_WINDOW_TO_TOP(sCalendarStoreManager)
	}
	else
	{
		CCalendarStoreWindow* calstoreman = CCalendarStoreWindow::ManualCreate();
		calstoreman->ResetState();
		calstoreman->ShowWindow(SW_SHOW);
		
		// Always display subscribed calendars when creating store window
		if (CCalendarWindow::FindWindow(NULL) == NULL)
		{
			// Create new subscribed window
			CCalendarWindow::MakeWindow(NULL);
			
			// Make sure this window is still focussed
			FRAMEWORK_WINDOW_TO_TOP(sCalendarStoreManager)
		}
	}
}

void CCalendarStoreWindow::DestroyCalendarStoreWindow()
{
	FRAMEWORK_DELETE_WINDOW(sCalendarStoreManager)
	sCalendarStoreManager = NULL;
}

// Manually create document
CCalendarStoreWindow* CCalendarStoreWindow::ManualCreate(void)
{
	CDocument* aDoc = sCalendarStoreDocTemplate->OpenDocumentFile(nil, false);
	CString strDocName;
	sCalendarStoreDocTemplate->GetDocString(strDocName, CDocTemplate::docName);
	aDoc->SetTitle(strDocName);

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CCalendarStoreWindow)))
		return (CCalendarStoreWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return nil;
}

int CCalendarStoreWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableViewWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Create toolbar pane
	CRect rect = CRect(0, 0, cWindowWidth, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), ID_VIEW_TOOLBAR);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);

	// Create CalendarStore view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mCalendarStoreView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mCalendarStoreView);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(GetCalendarStoreView());
	GetCalendarStoreView()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetCalendarStoreView()->GetTable()->SetManager(calstore::CCalendarStoreManager::sCalendarStoreManager);
	GetCalendarStoreView()->SetOpen();

	return 0;
}
