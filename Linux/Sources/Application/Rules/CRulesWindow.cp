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


// Source for CRulesWindow class

#include "CRulesWindow.h"

#include "CCommands.h"
#include "CContextMenu.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CFocusBorder.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CRulesTable.h"
#include "CScriptsTable.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CStreamOptionsMap.h"
#include "CTableScrollbarSet.h"
#include "CWindowsMenu.h"

#include "cdfstream.h"

#include "HResourceMap.h"
#include "HButtonText.h"

#include <JXChooseSaveFile.h>
#include <JXDisplay.h>
#include "JXMultiImageButton.h"
#include <JXScrollbar.h>
#include <JXStaticText.h>
#include "JXTabs.h"
#include "JXTextPushButton.h"
#include <JXUpRect.h>
#include <JXWindow.h>
#include <jXGlobals.h>

const int cTitleHeight = 16;

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CRulesWindow

CRulesWindow* CRulesWindow::sRulesWindow = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesWindow::CRulesWindow(JXDirector* owner)
	: CTableWindow(owner)
{
	sRulesWindow = this;

	mRulesTitleTable = NULL;
	mRulesTable = NULL;
	mScriptsTitleTable = NULL;
	mScriptsTable = NULL;
	mType = CFilterItem::eLocal;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Default destructor
CRulesWindow::~CRulesWindow()
{
	// Set status
	SetClosing();

	// Remove from list
	CWindowsMenu::RemoveWindow(this);

	// Save state
	SaveDefaultState();

	sRulesWindow = nil;

	// Set status
	SetClosed();
}

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
		CRulesWindow * pWnd = new CRulesWindow(CMulberryApp::sApp);
		if (pWnd)
		{
			pWnd->OnCreate();
			pWnd->ResetState();
		}
	}
}

void CRulesWindow::DestroyRulesWindow()
{
	if (sRulesWindow)
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

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CRulesWindow::OnCreate()
{
	// Do inherited
	SetWindowInfo(eRulesColumnName, 16);

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, 64);
	heights.InsertElementAtIndex(2, 64);
	minHeights.InsertElementAtIndex(1, 32);
	minHeights.InsertElementAtIndex(2, 32);

// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,270, "Servers");
    assert( window != NULL );
    SetWindow(window);

    mTabs =
        new JXTabs(window,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,30, 340,40);
    assert( mTabs != NULL );

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,70, 340,56);
    assert( obj1 != NULL );

    mRuleBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 8,4, 32,32);
    assert( mRuleBtn != NULL );

    mTargetBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 50,4, 32,32);
    assert( mTargetBtn != NULL );

    mEditBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 92,4, 32,32);
    assert( mEditBtn != NULL );

    mDeleteBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 134,4, 32,32);
    assert( mDeleteBtn != NULL );

    mWriteBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 176,4, 32,32);
    assert( mWriteBtn != NULL );

    mScriptBtn =
        new HButtonText<JXMultiImageButton>(obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 50,4, 32,32);
    assert( mScriptBtn != NULL );

    mShowTriggers =
        new JXTextPushButton("Show Triggers", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 245,5, 88,21);
    assert( mShowTriggers != NULL );

    mSplitter =
        new CSplitterView(heights, 0, minHeights, window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,126, 340,144);
    assert( mSplitter != NULL );

// end JXLayout

	window->SetWMClass(cRulesWMClass, cMulberryWMClass);
	window->SetIcon(iconFromResource(IDR_RULESDOCSDI, window, 16, 0x00CCCCCC));
	window->SetTitle("Rules Manager");

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fMailboxes | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Deal with tabs
	cdstring s;
	mTabs->AppendCard(NULL, s.FromResource(IDS_RULES_TAB_LOCAL));
	mTabs->AppendCard(NULL, s.FromResource(IDS_RULES_TAB_SIEVE));

	// Deal with buttons
	s.FromResource(IDS_RULES_NEW_BTN_TEXT);
	mRuleBtn->SetText(s);
	mRuleBtn->SetImage(IDI_RULES_NEW_BTN);

	s.FromResource(IDS_RULES_NEW_TARGETS_BTN_TEXT);
	mTargetBtn->SetText(s);
	mTargetBtn->SetImage(IDI_RULES_NEW_TARGET_BTN);

	s.FromResource(IDS_RULES_EDIT_BTN_TEXT);
	mEditBtn->SetText(s);
	mEditBtn->SetImage(IDI_RULES_EDIT_BTN);

	s.FromResource(IDS_RULES_DELETE_BTN_TEXT);
	mDeleteBtn->SetText(s);
	mDeleteBtn->SetImage(IDI_RULES_DELETE_BTN);

	s.FromResource(IDS_RULES_GENERATE_BTN_TEXT);
	mWriteBtn->SetText(s);
	mWriteBtn->SetImage(IDI_RULES_GENERATE_BTN);
	mWriteBtn->Hide();

	s.FromResource(IDS_RULES_NEW_SCRIPTS_BTN_TEXT);
	mScriptBtn->SetText(s);
	mScriptBtn->SetImage(IDI_RULES_NEW_SCRIPTS_BTN);
	mScriptBtn->Hide();

	// Rules table
    CFocusBorder* focus1 =
        new CFocusBorder(window, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 300,135);
    CTableScrollbarSet* sbs1 =
        new CTableScrollbarSet(focus1, JXWidget::kHElastic, JXWidget::kVElastic,
        						0, 0, 300 - 2*cFocusBorderInset, 135 - 2*cFocusBorderInset);
	mRulesTable = new CRulesTable(sbs1, sbs1->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kVElastic, 0,cTitleHeight, 105, 100);
	mRulesTitleTable = new CSimpleTitleTable(sbs1, sbs1->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 105, cTitleHeight);
	mRulesTable->OnCreate();
	mRulesTitleTable->OnCreate();
	mRulesTitleTable->SetTitleInfo(false, false, "UI::Titles::Rules", 1, 0);

	// Scripts table
    CFocusBorder* focus2 =
        new CFocusBorder(window, JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 300,135);
    CTableScrollbarSet* sbs2 =
        new CTableScrollbarSet(focus2, JXWidget::kHElastic, JXWidget::kVElastic,
        						0, 0, 300 - 2*cFocusBorderInset, 135 - 2*cFocusBorderInset);
	mScriptsTable = new CScriptsTable(sbs2, sbs2->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kVElastic, 0,cTitleHeight, 105, 100);
	mScriptsTitleTable = new CSimpleTitleTable(sbs2, sbs2->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 105, cTitleHeight);
	mScriptsTable->OnCreate();
	mScriptsTitleTable->OnCreate();
	mScriptsTitleTable->SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);

	// Deal with splitter
	mSplitter->InstallViews(focus1, focus2, false);

	// Init columns
	PostCreate(mRulesTable, mRulesTitleTable);
	mRulesTable->SetScriptsTable(mScriptsTable);
	mRulesTable->ResetTable();
	mScriptsTable->ResetTable();

	// Set trigger visibility
	OnShowTriggers();

	// Set current width and height as minimum
	window->SetMinSize(window->GetBoundsGlobal().width(),
							window->GetBoundsGlobal().height());

	ListenTo(mTabs);
	ListenTo(mRuleBtn);
	ListenTo(mTargetBtn);
	ListenTo(mEditBtn);
	ListenTo(mDeleteBtn);
	ListenTo(mWriteBtn);
	ListenTo(mScriptBtn);
	ListenTo(mShowTriggers);

	// Set status
	SetOpen();

} // CRulesWindow::FinishCreateSelf

// Respond to clicks in the icon buttons
void CRulesWindow::Receive(JBroadcaster* sender, const Message& message)
{
	//Button Pushed
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mRuleBtn)
		{
			OnNewRule();
			return;
		}
		else if (sender == mTargetBtn)
		{
			OnNewTarget();
			return;
		}
		else if (sender == mScriptBtn)
		{
			OnNewScript();
			return;
		}
		else if (sender == mEditBtn)
		{
			OnEditEntry();
			return;
		}
		else if (sender == mDeleteBtn)
		{
			OnDeleteEntry();
			return;
		}
		else if (sender == mWriteBtn)
		{
			OnWriteScript();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mShowTriggers)
		{
			OnShowTriggers();
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged) && (sender == mTabs))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		OnTabs(index);
		return;
	}

	CTableWindow::Receive(sender, message);
}

bool CRulesWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnFileImport();
		return true;

	case CCommand::eFileExport:
		OnFileExport();
		return true;

	}

	return CTableWindow::ObeyCommand(cmd, menu);
}

void CRulesWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileImport:
		OnUpdateFileImport(cmdui);
		return;

	case CCommand::eFileExport:
		OnUpdateFileExport(cmdui);
		return;

	default:;
	}

	CTableWindow::UpdateCommand(cmd, cmdui);
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
	newInfo.column_width = mRulesTable->GetApertureWidth();
	mColumnInfo.push_back(newInfo);
	mRulesTable->SetColWidth(newInfo.column_width, 1, 1);
	mRulesTitleTable->SetColWidth(newInfo.column_width, 1, 1);

	// Scripts Name, width = 272
	newInfo.column_type = eRulesColumnName;
	newInfo.column_width = mScriptsTable->GetApertureWidth();
	mScriptsTable->SetColWidth(newInfo.column_width, 1, 1);
	mScriptsTitleTable->SetColWidth(newInfo.column_width, 1, 1);
}

void CRulesWindow::OnTabs(JIndex index)
{
	mType = static_cast<CFilterItem::EType>(index - 1);
	mRulesTable->SetFilterType(mType);
	mScriptsTable->SetFilterType(mType);
	switch(mType)
	{
	case CFilterItem::eLocal:
	default:
		mScriptBtn->Hide();
		mWriteBtn->Hide();
		mTargetBtn->Show();
		mShowTriggers->Show();
		break;
	case CFilterItem::eSIEVE:
		mTargetBtn->Hide();
		mScriptBtn->Show();
		mWriteBtn->Show();
		mShowTriggers->Hide();
		break;
	}
	
	// Set trigger display
	OnShowTriggers();
}

void CRulesWindow::OnUpdateFileImport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(IDS_RULES_IMPORT);
	pCmdUI->SetText(txt);
}

void CRulesWindow::OnUpdateFileExport(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	cdstring txt;
	txt.FromResource(IDS_RULES_EXPORT);
	pCmdUI->SetText(txt);
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
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("File to import:", NULL, &fname))
	{
		// Get full path and create stream
		cdifstream fin(fname.GetCString(), ios_base::in | ios_base::binary);
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
	if (mRulesTable->IsSelectionValid())
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
	JString saveAsName;
	if (JXGetChooseSaveFile()->SaveFile("Rules File", NULL, saveAsName, &saveAsName))
	{
		// Get full path and create stream
		cdofstream fout(saveAsName.GetCString(), ios_base::in | ios_base::binary | ios_base::trunc);
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
			mRulesTable->DoToSelection1((CRulesTable::DoToSelection1PP) &CRulesTable::AddSelectionToList, &items);

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

void CRulesWindow::OnNewRule()
{
	mRulesTable->Focus();
	mRulesTable->OnNewRules();
}

void CRulesWindow::OnNewTarget()
{
	// Make sure triggers are visible
	if ((mType == CFilterItem::eLocal) && !mShowTriggers->IsChecked())
		mShowTriggers->SetState(kTrue);

	mScriptsTable->Focus();
	mScriptsTable->OnNewScript();
}

void CRulesWindow::OnNewScript()
{
	mScriptsTable->Focus();
	mScriptsTable->OnNewScript();
}

void CRulesWindow::OnEditEntry()
{
	if (mRulesTable->IsActive())
		mRulesTable->OnEditRules();
	else if (mScriptsTable->IsActive())
		mScriptsTable->OnEditScript();
}

void CRulesWindow::OnDeleteEntry()
{
	if (mRulesTable->IsActive())
		mRulesTable->OnDeleteRules();
	else if (mScriptsTable->IsActive())
		mScriptsTable->OnDeleteScript();
}

void CRulesWindow::OnWriteScript()
{
	if (mScriptsTable->IsActive())
		mScriptsTable->OnGenerateScript();
}

void CRulesWindow::OnShowTriggers()
{
	// Take triggers into account
	bool show_triggers = ((mType != CFilterItem::eLocal) || mShowTriggers->IsChecked());

	// Hide one part of the splitter
	mSplitter->ShowView(true, show_triggers);
}

void CRulesWindow::ResetTable(void)
{
	mRulesTable->ResetTable();
	mScriptsTable->ResetTable();
}

// Reset state from prefs
void CRulesWindow::ResetState(bool force)
{
	// Get default state
	CRulesWindowState* state = &CPreferences::sPrefs->mRulesWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mRulesWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Adjust size of tables
	//ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mRulesWindowDefault.GetValue()));

	// Adjust split pos
	mSplitter->SetRelativeSplitPos(state->GetSplitPos());

	// Set show triggers state
	mShowTriggers->SetState(JBoolean(state->GetShowTriggers()));

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetWindow()->ShowWindow(SW_SHOWMAXIMIZED);

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	// Always activate
	Activate();
}

// Save current state in prefs
void CRulesWindow::SaveDefaultState(void)
{
	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Add info to prefs
	CRulesWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo, mSplitter->GetRelativeSplitPos(), mShowTriggers->IsChecked(), false);
	if (CPreferences::sPrefs->mRulesWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesWindowDefault.SetDirty();

} // CRulesWindow::SaveDefaultState

// Reset text traits from prefs
void CRulesWindow::ResetFont(const SFontInfo& list_traits)
{
	mRulesTable->ResetFont(list_traits);
	mRulesTitleTable->ResetFont(list_traits);
	mRulesTitleTable->SyncTable(mRulesTable, true);
	mScriptsTable->ResetFont(list_traits);
	mScriptsTitleTable->ResetFont(list_traits);
	mScriptsTitleTable->SyncTable(mScriptsTable, true);
}
