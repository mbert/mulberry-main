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


// Source for CAddressBookWindow class


#include "CAddressBookWindow.h"

#include "CAddressBook.h"
#include "CAddressBookDoc.h"
#include "CCommands.h"
#include "CLog.h"
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CWindowsMenu.h"

#include "HResourceMap.h"

#include <JXMenuBar.h>

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CAddressBookWindow

unsigned long CAddressBookWindow::s_nUntitledCount = 1;
cdmutexprotect<CAddressBookWindow::CAddressBookWindowList> CAddressBookWindow::sAddressBookWindows;

// CAddressBookWindow construction/destruction

// Default constructor
CAddressBookWindow::CAddressBookWindow(JXDirector* owner)
	: CAddressBookDoc(owner)
{
	mPreviewVisible = true;

	{
		cdmutexprotect<CAddressBookWindowList>::lock _lock(sAddressBookWindows);
		sAddressBookWindows->push_back(this);
	}

	// Add to list
	CWindowsMenu::AddWindow(this, false);
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

	// Remove from list
	CWindowsMenu::RemoveWindow(this);
}

// Manually create document
CAddressBookWindow* CAddressBookWindow::ManualCreate(CAddressBook* adbk, const char* fname)
{
	CAddressBookWindow * pWnd = new CAddressBookWindow(CMulberryApp::sApp);
	if (pWnd)
	{
		pWnd->OnCreate();

		// Give it to document
		pWnd->SetAddressBookWindow(pWnd);
	
		// Set lists
		pWnd->InitDoc(adbk);
		pWnd->SetAddressBook(adbk);
		pWnd->GetWindow()->SetTitle(adbk->GetName());
		pWnd->ResetState();
	}
	
	return pWnd;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cWindowWidth = 500;
const int cWindowHeight = 200;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cSplitterTop = cToolbarTop + cToolbarHeight;
const int cSplitterHeight = cWindowHeight - cSplitterTop;

void CAddressBookWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,200, "");
    assert( window != NULL );
    SetWindow(window);

// end JXLayout

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cSplitterHeight/2);
	heights.InsertElementAtIndex(2, cSplitterHeight/2);
	minHeights.InsertElementAtIndex(1, 16);
	minHeights.InsertElementAtIndex(2, 16);

	mSplitterView = new CSplitterView(heights, 1, minHeights, window,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);

	// Create server view
	mAddressBookView = new CAddressBookView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
	mAddressBookView->OnCreate();
	//SetTableView(mAddressBookView);

	mAddressView = new CAddressView(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 300);
	mAddressView->OnCreate();
	GetAddressBookView()->SetPreview(mAddressView);

	// Install the splitter items
	mSplitterView->InstallViews(GetAddressBookView(), mAddressView, true);

	// Hide preview if preference not set
	if (!CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview())
		ShowPreview(false);

	// Create toolbars for a view we own
	GetToolbarView()->SetSibling(mSplitterView);
	GetAddressBookView()->MakeToolbars(mToolbarView);
	GetAddressBookView()->GetPreview()->MakeToolbars(mToolbarView);
	GetToolbarView()->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());


	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Set current width and height as minimum
	GetWindow()->SetMinSize(cWindowWidth, cWindowHeight - 45 - 45);
	GetWindow()->SetWMClass(cAdbkWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_ADDRESSBOOKDOCSDI, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	GetAddressBookView()->SetOpen();
}

bool CAddressBookWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eWindowsShowPreview:
	case CCommand::eToolbarShowPreview:
		OnShowPreview();
		return true;
	default:;
	}

	return CAddressBookDoc::ObeyCommand(cmd, menu);
}

void CAddressBookWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eWindowsShowPreview:
		OnUpdateShowPreview(cmdui);
		return;

	case CCommand::eToolbarShowPreview:
		OnUpdateShowPreviewBtn(cmdui);
		return;

	default:;
	}

	CAddressBookDoc::UpdateCommand(cmd, cmdui);
}

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
		CAddressBookWindow* wnd = NULL;
		try
		{
			wnd = CAddressBookWindow::ManualCreate(adbk, NULL);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		
		return wnd != NULL;
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

void CAddressBookWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(mPreviewVisible ? IDS_HIDEPREVIEW : IDS_SHOWPREVIEW);
	pCmdUI->SetText(txt);
}

void CAddressBookWindow::OnUpdateShowPreviewBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mPreviewVisible);
}

// Tidy up when window closed
void CAddressBookWindow::OnShowPreview()
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
	mSplitterView->ShowView(true, show);
	GetAddressBookView()->SetUsePreview(mPreviewVisible);
	
	// Change preference which will control preview state for future windows
	if (CPreferences::sPrefs->Get3PaneOptions().GetValue().GetAddressViewOptions().GetShowPreview() != show)
	{
		CPreferences::sPrefs->Get3PaneOptions().Value().GetAddressViewOptions().SetShowPreview(show);
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
	}
}

// Reset window state
void CAddressBookWindow::ResetState(bool force)
{
	// Pass down to the view object which handles this
	GetAddressBookView()->ResetState(force);
}

// Save current state as default
void CAddressBookWindow::SaveDefaultState(void)
{
	// Pass down to the view object which handles this
	GetAddressBookView()->SaveDefaultState();
}
