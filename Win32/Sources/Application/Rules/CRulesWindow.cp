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


// CRulesWindow.cp : implementation of the CRulesWindow class
//

#include "CRulesWindow.h"

#include "CContainerView.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CFontCache.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CStreamOptionsMap.h"

#include "cdfstream.h"

const int cWindowWidth = 300;
const int cWindowHeight = 200;

const int cTabsHeight = 32;
const int cHeaderHeight = 56;

const int cBtnTop = 4;
const int cBtnStart = 4;
const int cBtnOffset = 44;

const int cShowTriggersWidth = 90;
const int cShowTriggersHeight = 18;
const int cShowTriggersHOffset  = 16;
const int cShowTriggersVOffset  = cBtnTop;

const int cTitleHeight = 16;

/////////////////////////////////////////////////////////////////////////////
// CRulesWindow

IMPLEMENT_DYNCREATE(CRulesWindow, CView)

BEGIN_MESSAGE_MAP(CRulesWindow, CTableWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()

	ON_UPDATE_COMMAND_UI(IDM_FILE_IMPORT, OnUpdateFileImport)
	ON_COMMAND(IDM_FILE_IMPORT, OnFileImport)

	ON_UPDATE_COMMAND_UI(IDM_FILE_EXPORT, OnUpdateFileExport)
	ON_COMMAND(IDM_FILE_EXPORT, OnFileExport)

	ON_NOTIFY(TCN_SELCHANGE, IDC_RULES_TABS, OnSelChangeRulesTabs)
	ON_COMMAND(IDC_RULESSHOWTRIGGERSBTN, OnShowTriggers)

	ON_COMMAND(IDC_RULESNEWBTN, OnNewRules)
	ON_COMMAND(IDC_RULESNEWTARGETBTN, OnNewTrigger)
	ON_COMMAND(IDC_RULESNEWSCRIPTSBTN, OnNewScript)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRulesWindow construction/destruction

CRulesWindow* CRulesWindow::sRulesWindow = nil;
CMultiDocTemplate* CRulesWindow::sRulesDocTemplate = nil;

CRulesWindow::CRulesWindow()
{
	sRulesWindow = this;
	mFocusRules = true;
	mType = CFilterItem::eLocal;
}

CRulesWindow::~CRulesWindow()
{
	sRulesWindow = nil;
}

// Manually create document
CRulesWindow* CRulesWindow::ManualCreate(void)
{
	CDocument* aDoc = sRulesDocTemplate->OpenDocumentFile(nil, false);
	CString strDocName;
	sRulesDocTemplate->GetDocString(strDocName, CDocTemplate::docName);
	aDoc->SetTitle(strDocName);

	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDlgItem(AFX_IDW_PANE_FIRST);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(CRulesWindow)))
		return (CRulesWindow*) pWnd;

	return nil;
}

int CRulesWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTableWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Tabs
	mTabs.Create(WS_CHILD | WS_VISIBLE, CRect(0, 0, cWindowWidth, cTabsHeight), this, IDC_RULES_TABS);

	// Set tabs
	CString s;
	TC_ITEM tabs;
	tabs.mask = TCIF_TEXT;
	s.LoadString(IDS_RULES_TAB_LOCAL);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*) s);
	mTabs.InsertItem(0, &tabs);
	s.LoadString(IDS_RULES_TAB_SIEVE);
	tabs.pszText = const_cast<TCHAR*>((const TCHAR*) s);
	mTabs.InsertItem(1, &tabs);
	mTabs.SetFont(CMulberryApp::sAppFont);

	// Create header pane
	mHeader.CreateEx(WS_EX_DLGMODALFRAME, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, 0, cTabsHeight, cWindowWidth, cHeaderHeight + large_offset, GetSafeHwnd(), (HMENU)IDC_SERVERHEADER);

	// Toolbar containers
	mLocalBtns.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, cWindowWidth, cHeaderHeight + large_offset), &mHeader, IDC_STATIC);
	mSIEVEBtns.Create(_T(""), WS_CHILD, CRect(0, 0, cWindowWidth, cHeaderHeight + large_offset), &mHeader, IDC_STATIC);

	// Buttons
	int left = cBtnStart + small_offset;
	s.LoadString(IDS_RULES_NEW_BTN_TEXT);
	mNewRuleBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mHeader, IDC_RULESNEWBTN, IDC_STATIC, IDI_RULES_NEW_BTN);

	left += cBtnOffset + small_offset;
	s.LoadString(IDS_RULES_NEW_TARGETS_BTN_TEXT);
	mNewTargetBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mLocalBtns, IDC_RULESNEWTARGETBTN, IDC_STATIC, IDI_RULES_NEW_TARGET_BTN);

	s.LoadString(IDS_RULES_NEW_SCRIPTS_BTN_TEXT);
	mNewScriptBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mSIEVEBtns, IDC_RULESNEWSCRIPTSBTN, IDC_STATIC, IDI_RULES_NEW_SCRIPTS_BTN);

	left += cBtnOffset + small_offset;
	s.LoadString(IDS_RULES_EDIT_BTN_TEXT);
	mEditBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mHeader, IDC_RULESEDITBTN, IDC_STATIC, IDI_RULES_EDIT_BTN);

	left += cBtnOffset + small_offset;
	s.LoadString(IDS_RULES_DELETE_BTN_TEXT);
	mDeleteBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mHeader, IDC_RULESDELETEBTN, IDC_STATIC, IDI_RULES_DELETE_BTN);

	left += cBtnOffset + small_offset;
	s.LoadString(IDS_RULES_APPLY_BTN_TEXT);
	mApplyBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mLocalBtns, IDC_RULESAPPLYBTN, IDC_STATIC, IDI_RULES_APPLY_BTN);

	s.LoadString(IDS_RULES_GENERATE_BTN_TEXT);
	mGenerateBtn.Create(s, CRect(left, cBtnTop + small_offset, left + 32, cBtnTop + small_offset + 32), &mSIEVEBtns, IDC_RULESGENERATEBTN, IDC_STATIC, IDI_RULES_GENERATE_BTN);

	left += cBtnOffset + small_offset;
	s.LoadString(IDS_RULES_SHOWTRIGGERS_BTN_TEXT);
	mShowTriggers.Create(s, CRect(left, cBtnTop + small_offset, left + cShowTriggersWidth, cShowTriggersVOffset + small_offset + cShowTriggersHeight), &mHeader, IDC_RULESSHOWTRIGGERSBTN);
	mShowTriggers.SetFont(CMulberryApp::sAppFont);
	mShowTriggers.SetPushed(true);

	// Splitter
	CRect rect = CRect(0, cTabsHeight + cHeaderHeight + large_offset, cWindowWidth, cWindowHeight);
	mSplitterView.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Rules focus ring
	mRulesFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, cWindowWidth, cWindowHeight), this, IDC_STATIC);
	mRulesFocusRing.SetFocusBorder();
	UINT focus_indent = 3;

	// Rules Table
	mRulesTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL,
					CRect(focus_indent, focus_indent + cTitleHeight, cWindowWidth - focus_indent, cWindowHeight - focus_indent), &mRulesFocusRing, IDC_STATIC);
	mRulesFocusRing.AddAlignment(new CWndAlignment(&mRulesTable, CWndAlignment::eAlign_WidthHeight));
	mRulesTable.ResetFont(CFontCache::GetListFont());
	mRulesTable.SetColumnInfo(mColumnInfo);
//	mRulesTable.SetContextMenuID(IDR_POPUP_CONTEXT_ADDRMANAGER);
	mRulesTable.SetContextView(this);
	{
		SCROLLINFO scinfo;
		scinfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		scinfo.nMin = 0;
		scinfo.nMax = 0;
		scinfo.nPage = 0;
		scinfo.nPos = 0;
		mRulesTable.SetScrollInfo(SB_VERT, &scinfo);
	}

	// Get titles
	mRulesTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent, cWindowWidth - focus_indent, focus_indent + cTitleHeight), &mRulesFocusRing, IDC_STATIC);
	mRulesFocusRing.AddAlignment(new CWndAlignment(&mRulesTitles, CWndAlignment::eAlign_TopWidth));
	mRulesTitles.SetFont(CFontCache::GetListFont());
	mRulesTitles.SetTitleInfo(false, false, "UI::Titles::Rules", 1, 0);

	// Scripts focus ring
	mScriptsFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, cWindowWidth, cWindowHeight), this, IDC_STATIC);
	mScriptsFocusRing.SetFocusBorder();

	// Scripts Table
	mScriptsTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cTitleHeight, cWindowWidth - focus_indent, cWindowHeight - focus_indent), &mScriptsFocusRing, IDC_STATIC);
	mScriptsFocusRing.AddAlignment(new CWndAlignment(&mScriptsTable, CWndAlignment::eAlign_WidthHeight));
	mScriptsTable.ResetFont(CFontCache::GetListFont());
	mScriptsTable.SetColumnInfo(mColumnInfo);
//	mScriptsTable.SetContextMenuID(IDR_POPUP_CONTEXT_ADDRMANAGER);
	mScriptsTable.SetContextView(this);

	// Get titles
	mScriptsTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent, cWindowWidth - focus_indent, focus_indent + cTitleHeight), &mScriptsFocusRing, IDC_STATIC);
	mScriptsFocusRing.AddAlignment(new CWndAlignment(&mScriptsTitles, CWndAlignment::eAlign_TopWidth));
	mScriptsTitles.SetFont(CFontCache::GetListFont());
	mScriptsTitles.SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);

	PostCreate(&mRulesTable, &mRulesTitles);
	mRulesTable.SetScriptsTable(&mScriptsTable);

	// Install the splitter items
	mSplitterView.InstallViews(&mRulesFocusRing, &mScriptsFocusRing, false);
	mSplitterView.SetMinima(64, 64);

	mRulesTable.ResetTable();
	mScriptsTable.ResetTable();

	// Set status
	SetOpen();


	return 0;
}

// Resize sub-views
void CRulesWindow::OnSize(UINT nType, int cx, int cy)
{
	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	// Resize its tabs/header
	CRect rect(0, 0, cx, cTabsHeight);
	mTabs.MoveWindow(rect);

	rect = CRect(0, cTabsHeight, cx, cTabsHeight + cHeaderHeight + large_offset);
	mHeader.MoveWindow(rect);
	rect = CRect(0, 0, cx, cHeaderHeight + large_offset);
	mLocalBtns.MoveWindow(rect);
	mSIEVEBtns.MoveWindow(rect);

	// Right justify button
	rect = CRect(cx - cShowTriggersHOffset - cShowTriggersWidth, cShowTriggersVOffset + small_offset,
					cx - cShowTriggersHOffset, cShowTriggersVOffset + small_offset + cShowTriggersHeight);
	mShowTriggers.MoveWindow(rect);

	// Resize its splitter and offset to hide borders we don't want to see
	rect = CRect(0, cTabsHeight + cHeaderHeight + large_offset - 2, cx, cy);
	mSplitterView.MoveWindow(rect);

	CView::OnSize(nType, cx, cy);
}

BOOL CRulesWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Extend the framework's command route from the view to
	// the application-specific CMyShape that is currently selected
	// in the view. m_pActiveShape is NULL if no shape object
	// is currently selected in the view.
	if ((mFocusRules ? (CTable*) &mRulesTable : (CTable*) &mScriptsTable)->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Tell window to focus on this one
void CRulesWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to table
	(mFocusRules ? (CTable*) &mRulesTable : (CTable*) &mScriptsTable)->SetFocus();
}


#pragma mark ____________________________Statics

// Create it or bring it to the front
void CRulesWindow::CreateRulesWindow()
{
	// Create find & replace window or bring to front
	if (sRulesWindow)
	{
		FRAMEWORK_WINDOW_TO_TOP(sRulesWindow)
	}
	else
	{
		CRulesWindow* rules = CRulesWindow::ManualCreate();
		rules->ResetState();
		rules->GetParentFrame()->ShowWindow(SW_SHOW);
	}
}

void CRulesWindow::DestroyRulesWindow()
{
	FRAMEWORK_DELETE_WINDOW(sRulesWindow)
	sRulesWindow = NULL;
}

// Make rule from example messages
void CRulesWindow::MakeRule(const CMessageList& msgs)
{
	// Make sure rules window is visible
	CreateRulesWindow();
	
	if (sRulesWindow)
		sRulesWindow->GetTable()->MakeRule(msgs);
}

// Init columns and text
void CRulesWindow::InitColumns(void)
{
	SColumnInfo newInfo;

	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns

	// Name, width = 272
	newInfo.column_type = eRulesColumnName;
	newInfo.column_width = 272;
	mColumnInfo.push_back(newInfo);
	mRulesTable.SetColWidth(newInfo.column_width, 1, 1);
	mRulesTitles.SetColWidth(newInfo.column_width, 1, 1);

	// Name, width = 272
	mScriptsTable.SetColWidth(newInfo.column_width, 1, 1);
	mScriptsTitles.SetColWidth(newInfo.column_width, 1, 1);
}

void CRulesWindow::OnUpdateFileImport(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(IDS_RULES_IMPORT);
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CRulesWindow::OnUpdateFileExport(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(IDS_RULES_EXPORT);
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CRulesWindow::OnFileImport()
{
	// Ask user whether to replace entire set of items, or only merge rules
	CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																	"ErrorDialog::Btn::RulesMergeRulesOnly",
																	"ErrorDialog::Btn::Cancel",
																	"ErrorDialog::Btn::RulesMergeAll",
																	NULL,
																	"ErrorDialog::Text::RulesMergeImportType");

	if (result == CErrorDialog::eBtn2)
		return;

	// Pick file to import
	CString filter = _T("Text File (*.txt) | *.txt; *.doc|Anyfile (*.*) | *.*||");
	CFileDialog dlgFile(true, NULL, NULL, OFN_FILEMUSTEXIST, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Get full path and create stream
		cdstring fpath(dlgFile.GetPathName());
		cdifstream fin(fpath, ios_base::in | ios_base::binary);
		if (fin.fail())
			return;

		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		// Create stream options map
		CStreamOptionsMap map;
		map.SetIStream(&fin);

		// Read it in
		map.ReadMap();

		// Import them based on entire replacement or merge option
		cdstring errs;
		if (result == CErrorDialog::eBtn1)
		{
			// Read in a new set of rules only
			CFilterItemList items;
			CPreferences::sPrefs->GetFilterManager()->ReadFromMap(&map, CPreferences::sPrefs->vers, mType, items);
			
			// Now merge the new ones with the existing set of rules
			CPreferences::sPrefs->GetFilterManager()->MergeRules(mType, items);

			// Now verify validity of rules
			for(CFilterItemList::const_iterator iter = items.begin(); iter != items.end(); iter++)
			{
				// Add name if it fails
				if (!(*iter)->CheckActions())
				{
					if (!errs.empty())
						errs += os_endl;
					errs += (*iter)->GetName();
				}
			}
		}
		else
		{
			// Read in entire set of local/sieve depending on current display
			CPreferences::sPrefs->GetFilterManager()->ReadFromMap(&map, CPreferences::sPrefs->vers,
																	(mType == CFilterItem::eLocal), (mType != CFilterItem::eLocal));

			// Now verify validity of all rules
			CPreferences::sPrefs->GetFilterManager()->CheckActions(mType, errs);
		}

		// Force reset of entire window
		ResetTable();
		
		// Alert user of invalid rules
		if (!errs.empty())
		{
			cdstring errtxt;
			errtxt.FromResource("Alerts::Rules::INVALIDIMPORTACTIONS");
			if (!errs.empty())
			{
				errtxt += os_endl2;
				errtxt += errs;
			}
			CErrorHandler::PutNoteAlert(errtxt);
		}
	}
}

void CRulesWindow::OnFileExport()
{
	// Check whether rules list has a selection and if so prompt user for type of export
	bool do_selection = false;
	if (mRulesTable.IsSelectionValid())
	{
		// Ask user whether to replace entire set of items, or only merge rules
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::RulesMergeSelection",
																		"ErrorDialog::Btn::Cancel",
																		"ErrorDialog::Btn::RulesMergeEntireSet",
																		NULL,
																		"ErrorDialog::Text::RulesMergeExportType");
		
		if (result == CErrorDialog::eBtn2)
			return;
		do_selection = (result == CErrorDialog::eBtn1);
	}

	// Pick file to export to
	CString saveAsName;
	CString filter = _T("Exported Rules (*.txt)|*.txt||");
	CFileDialog dlgFile(false, _T(".txt"), saveAsName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		// Get full path and create stream
		cdstring fpath(dlgFile.GetPathName());
		cdofstream fout(fpath, ios_base::in | ios_base::binary | ios_base::trunc);
		if (fout.fail())
			return;

		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		// Create stream options map
		CStreamOptionsMap map;
		map.SetOStream(&fout);

		// Export them based on selection or all
		if (do_selection)
		{
			// Add selection to list
			CFilterItemList items;
			items.set_delete_data(false);
			mRulesTable.DoToSelection1((CRulesTable::DoToSelection1PP) &CRulesTable::AddSelectionToList, &items);

			// Now write set to map
			CPreferences::sPrefs->GetFilterManager()->WriteToMap(&map, mType, items);
		}
		else
			// Write out just local/sieve depending on current display
			CPreferences::sPrefs->GetFilterManager()->WriteToMap(&map, false, (mType == CFilterItem::eLocal), (mType != CFilterItem::eLocal));
		
		// Now write it out
		map.WriteMap(false);
	}
}


void CRulesWindow::OnSelChangeRulesTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	mType = static_cast<CFilterItem::EType>(mTabs.GetCurSel());
	mRulesTable.SetFilterType(mType);
	mScriptsTable.SetFilterType(mType);
	switch(mType)
	{
	case CFilterItem::eLocal:
	default:
		mLocalBtns.ShowWindow(SW_SHOW);
		mSIEVEBtns.ShowWindow(SW_HIDE);
		mShowTriggers.ShowWindow(SW_SHOW);
		break;
	case CFilterItem::eSIEVE:
		mSIEVEBtns.ShowWindow(SW_SHOW);
		mLocalBtns.ShowWindow(SW_HIDE);
		mShowTriggers.ShowWindow(SW_HIDE);
		break;
	}
	
	// Set trigger display for new state
	SetTriggerSplit();

	*pResult = 0;
}

void CRulesWindow::OnShowTriggers() 
{
	// Toggle button
	SetTriggerSplit();
}

// Show/hide triggers panel
void CRulesWindow::SetTriggerSplit()
{
	// Take triggers into account
	bool show_triggers = ((mType != CFilterItem::eLocal) || mShowTriggers.IsPushed());

	// Hide one part of the splitter
	mSplitterView.ShowView(true, show_triggers);

	// Force focus to rules if hiding triggers
	if (!show_triggers)
		mRulesTable.SetFocus();
}

void CRulesWindow::OnNewRules() 
{
	// Pass down to rules table
	mRulesTable.SetFocus();
	mRulesTable.SendMessage(WM_COMMAND, IDC_RULESNEWBTN);
}

void CRulesWindow::OnNewTrigger() 
{
	// Make sure triggers are visible
	if ((mType == CFilterItem::eLocal) && !mShowTriggers.IsPushed())
		OnShowTriggers();

	// Pass down to triggers table
	mScriptsTable.SetFocus();
	mScriptsTable.SendMessage(WM_COMMAND, IDC_RULESNEWTARGETBTN);
}

void CRulesWindow::OnNewScript() 
{
	// Pass down to scripts table
	mScriptsTable.SetFocus();
	mScriptsTable.SendMessage(WM_COMMAND, IDC_RULESNEWSCRIPTSBTN);
}

#pragma mark ____________________________Window State

// Reset both tables
void CRulesWindow::ResetTable(void)
{
	mRulesTable.ResetTable();
	mScriptsTable.ResetTable();
}

// Reset both list fonts
void CRulesWindow::ResetFont(CFont* font)
{
	mRulesTable.ResetFont(font);
	mScriptsTable.ResetFont(font);
	
	// May need to manually adjust titles and position of table
	UInt16 old_height = mRulesTitles.GetRowHeight(1);
	mRulesTitles.ResetFont(font);
	SInt16 delta = mRulesTitles.GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(&mRulesTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(&mRulesTable, 0, -delta, false);
		::MoveWindowBy(&mRulesTable, 0, delta, false);
		
		mRulesTable.RedrawWindow();
		mRulesTitles.RedrawWindow();
	}
	
	// May need to manually adjust titles and position of table
	old_height = mScriptsTitles.GetRowHeight(1);
	mScriptsTitles.ResetFont(font);
	delta = mScriptsTitles.GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(&mScriptsTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(&mScriptsTable, 0, -delta, false);
		::MoveWindowBy(&mScriptsTable, 0, delta, false);
		
		mScriptsTable.RedrawWindow();
		mScriptsTitles.RedrawWindow();
	}
}

// Reset state from prefs
void CRulesWindow::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	CRulesWindowState* state = &CPreferences::sPrefs->mRulesWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mRulesWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		GetParentFrame()->SetWindowPos(nil, set_rect.left, set_rect.top,
										set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	//ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mRulesWindowDefault.GetValue()));

	// Adjust split pos
	mSplitterView.SetRelativeSplitPos(state->GetSplitPos());

	// Set show triggers state
	mShowTriggers.SetPushed(!state->GetShowTriggers());
	OnShowTriggers();

	if (force)
		OnSaveDefaultState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (state->GetState() == eWindowStateMax)
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	if (!force)
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		GetParentFrame()->RedrawWindow();
	}
	else
		GetParentFrame()->RedrawWindow();
}

// Save current state in prefs
void CRulesWindow::SaveState(void)
{
	// Just do the default action
	SaveDefaultState();
}

// Save current state in prefs
void CRulesWindow::SaveDefaultState(void)
{
	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	CRulesWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo,
							mSplitterView.GetRelativeSplitPos(), mShowTriggers.IsPushed(), !GetParentFrame()->IsWindowVisible());
	if (CPreferences::sPrefs->mRulesWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesWindowDefault.SetDirty();

}
