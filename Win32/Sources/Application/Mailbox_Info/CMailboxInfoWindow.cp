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


// Source for CMailboxInfoWindow class

#include "CMailboxInfoWindow.h"

#include "CMulberryApp.h"
#include "CPreferences.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CMailboxInfoWindow

IMPLEMENT_DYNCREATE(CMailboxInfoWindow, CView)

BEGIN_MESSAGE_MAP(CMailboxInfoWindow, CMailboxWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMailboxInfoWindow construction/destruction

CMultiDocTemplate* CMailboxInfoWindow::sMailboxDocTemplate = NULL;

// Default constructor
CMailboxInfoWindow::CMailboxInfoWindow()
{
	mSplitter = &mSplitterView;
}

// Default destructor
CMailboxInfoWindow::~CMailboxInfoWindow()
{
}

// Manually create document
CMailboxInfoWindow* CMailboxInfoWindow::ManualCreate(void)
{
	CDocument* aDoc = sMailboxDocTemplate->OpenDocumentFile(nil, false);
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CMailboxInfoWindow* pWnd = (CMailboxInfoWindow*) aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CMailboxInfoWindow)))
	{
		pWnd->mMailboxFrame = aFrame;
		return (CMailboxInfoWindow*) pWnd;
	}

	return NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CMailboxInfoWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxWindow::OnCreate(lpCreateStruct) == -1)
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
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mMailboxInfoView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mMailboxInfoView);
	
	// Create message view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mMessageView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	GetMailboxView()->SetPreview(&mMessageView);
	mMessageView.SetOwnerView(GetMailboxView());

	// Install the splitter items
	mSplitter->InstallViews(GetMailboxInfoView(), &mMessageView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(mSplitter);
	GetMailboxInfoView()->MakeToolbars(&mToolbarView);
	GetMailboxInfoView()->GetPreview()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set status
	GetMailboxInfoView()->SetOpen();
	
	return 0;
}

// Tidy up when window closed
void CMailboxInfoWindow::OnDestroy(void)
{
	// Close actual mailbox
	GetMailboxInfoView()->DoCloseMbox();
	mMessageView.DoClose();

	// Do inherited
	CMailboxWindow::OnDestroy();
}

// Resize sub-views
void CMailboxInfoWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	CRect rect(0, 0, cx, cy);
	mSplitterView.MoveWindow(rect);
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CView::OnSize(nType, cx, cy);
}

// Possibly do expunge
bool CMailboxInfoWindow::AttemptClose(void)
{
	// Look for dangling messages then process close actions
	return GetMailboxInfoView()->TestClose();
}

