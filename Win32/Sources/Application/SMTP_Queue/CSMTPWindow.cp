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


	// Source for CSMTPWindow class

#include "CSMTPWindow.h"

#include "CFontCache.h"
#include "CMbox.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSMTPAccountManager.h"
#include "CSMTPSender.h"
#include "CSMTPTable.h"

const int cToolbarHeight = 56;
const int cWindowWidth = 128;
const int cWindowHeight = cToolbarHeight + 128;

/////////////////////////////////////////////////////////////////////////////
// CSMTPWindow

IMPLEMENT_DYNCREATE(CSMTPWindow, CView)

BEGIN_MESSAGE_MAP(CSMTPWindow, CMailboxWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSMTPWindow construction/destruction

CMultiDocTemplate* CSMTPWindow::sSMTPDocTemplate = NULL;
CSMTPWindow* CSMTPWindow::sSMTPWindow = NULL;

// Default constructor
CSMTPWindow::CSMTPWindow()
{
	mSplitter = &mSplitterView;
	sSMTPWindow = this;
}

// Default destructor
CSMTPWindow::~CSMTPWindow()
{
	sSMTPWindow = NULL;
}

// Open window
void CSMTPWindow::OpenSMTPWindow()
{

	// Create new or show existing
	if (sSMTPWindow)
	{
		FRAMEWORK_WINDOW_TO_TOP(sSMTPWindow)
	}
	else
	{
		// Create the mailbox list window
		CSMTPWindow* aWindow = ManualCreate();

		// This may throw and delete the window if problem ocurrs
		try
		{
			// Set first SMTP account
			CSMTPSenderList& senders = CSMTPAccountManager::sSMTPAccountManager->GetSMTPSenders();
			for(CSMTPSenderList::const_iterator iter = senders.begin(); iter != senders.end(); iter++)
			{
				// Exit for first enabled
				if (static_cast<CSMTPAccount*>((*iter)->GetAccount())->GetUseQueue())
				{
					aWindow->GetSMTPView()->SetSMTPSender(*iter);
					break;
				}
			}

			// Set status
			aWindow->GetSMTPView()->SetOpen();

			aWindow->GetParentFrame()->ShowWindow(SW_SHOW);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);

		}
	}
}

// Open window
void CSMTPWindow::CloseSMTPWindow()
{
	// Delete any existing window
	if (sSMTPWindow)
		FRAMEWORK_DELETE_WINDOW(sSMTPWindow)
}

// Manually create document
CSMTPWindow* CSMTPWindow::ManualCreate(void)
{
	CDocument* aDoc = sSMTPDocTemplate->OpenDocumentFile(nil, false);

	cdstring strDocName;
	strDocName.FromResource("Alerts::SMTP::Queues");
	aDoc->SetTitle(strDocName.win_str());

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CSMTPWindow* pWnd = (CSMTPWindow*) aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CSMTPWindow)))
	{
		pWnd->mMailboxFrame = aFrame;
		return (CSMTPWindow*) pWnd;
	}

	return nil;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CSMTPWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxWindow::OnCreate(lpCreateStruct) == -1)
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
	
	// Create mailbox view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mSMTPView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	SetTableView(&mSMTPView);

	// Create message view
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mMessageView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	GetMailboxView()->SetPreview(&mMessageView);
	mMessageView.SetOwnerView(GetMailboxView());

	// Install the splitter items
	mSplitter->InstallViews(GetSMTPView(), &mMessageView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetMailViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	mToolbarView.SetSibling(mSplitter);
	GetSMTPView()->MakeToolbars(&mToolbarView);
	GetSMTPView()->GetPreview()->MakeToolbars(&mToolbarView);
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	return 0;
}

// Tidy up when window closed
void CSMTPWindow::OnDestroy(void)
{
	// Close actual mailbox
	GetSMTPView()->DoClose();
	mMessageView.DoClose();

	// Do inherited
	CMailboxWindow::OnDestroy();
}

// Resize sub-views
void CSMTPWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its table view to fit
	CRect rect(0, 0, cx, cy);
	mSplitterView.MoveWindow(rect);
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CView::OnSize(nType, cx, cy);
}

void CSMTPWindow::UpdateTitle()
{
	// Set state of enabled button
	bool enable = false;
	enable = CConnectionManager::sConnectionManager.IsConnected() &&
				CSMTPAccountManager::sSMTPAccountManager->GetConnected();

	cdstring title(GetDocument()->GetTitle());
	
	cdstring new_title;
	new_title.FromResource("Alerts::SMTP::Queues");
	if (GetSMTPView()->GetSMTPSender())
	{
		cdstring temp;
		new_title += GetSMTPView()->GetSMTPSender()->GetAccountName();
		new_title += " - ";
		new_title += temp.FromResource(enable ? "Alerts::SMTP::QueuesEnabled" : "Alerts::SMTP::QueuesDisabled");
	}
	
	if (new_title != title)
		GetDocument()->SetTitle(new_title.win_str());
}
