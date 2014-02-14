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
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CScriptsTable.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CStreamOptionsMap.h"
#include "CWindowsMenu.h"

#include "cdfstream.h"

#include <LBevelButton.h>
#include <LTabsControl.h>
#include <UStandardDialogs.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CRulesWindow*	CRulesWindow::sRulesWindow = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesWindow::CRulesWindow()
{
	sRulesWindow = this;

	mType = CFilterItem::eLocal;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Constructor from stream
CRulesWindow::CRulesWindow(LStream *inStream)
		: CTableWindow(inStream)
{
	sRulesWindow = this;

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
		CRulesWindow* rules = (CRulesWindow*) LWindow::CreateWindow(paneid_RulesWindow, CMulberryApp::sApp);
		rules->ResetState();
		rules->Show();
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

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CRulesWindow::FinishCreateSelf()
{
	// Do inherited
	SetWindowInfo(eRulesColumnName, 16, false);
	LWindow::FinishCreateSelf();

	mLocalBtns = (LView*) FindPaneByID(paneid_RulesLocalBtns);
	mSIEVEBtns = (LView*) FindPaneByID(paneid_RulesSIEVEBtns);
	mShowTriggers = (LBevelButton*) FindPaneByID(paneid_RulesShowTriggers);

	// Deal with splitter
	mSplitter = (CSplitterView*) FindPaneByID(paneid_RulesSplitter);
	mSplitter->InstallViews((LView*) FindPaneByID(paneid_RulesSplit1), (LView*) FindPaneByID(paneid_RulesSplit2), false);

	mTitles = mRulesTitleTable = (CSimpleTitleTable*) FindPaneByID(paneid_RulesRulesTitles);
	mRulesTitleTable->SetTitleInfo(false, false, "UI::Titles::Rules", 1, 0);
	mTable = mRulesTable = (CRulesTable*) FindPaneByID(paneid_RulesRules);
	mScriptsTitleTable = (CSimpleTitleTable*) FindPaneByID(paneid_RulesScriptsTitles);
	mScriptsTitleTable->SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);
	mScriptsTable = (CScriptsTable*) FindPaneByID(paneid_RulesScripts);
	mRulesTable->SetScriptsTable(mScriptsTable);

	SetLatentSub(mRulesTable);

	// Set trigger visibility
	OnShowTriggers();

	// Init columns
	InitColumns();

	// Get text traits resource
	ResetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

    // Set type based on whether rules exist or not
    mType = (CPreferences::sPrefs->GetFilterManager()->GetFilters(CFilterItem::eLocal).size() != 0) ? CFilterItem::eLocal : CFilterItem::eSIEVE;
    OnTabs(mType + 1);

	// Set status
	SetOpen();

#ifdef NOTYET
	CContextMenuAttachment::AddUniqueContext(this, 9003, GetTable());
#endif

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CRulesWindowBtns);

} // CRulesWindow::FinishCreateSelf

// Stop save action if demo
Boolean CRulesWindow::ObeyCommand(CommandT inCommand, void *ioParam)
{
	Boolean cmdHandled = true;

	switch(inCommand)
	{
	case cmd_FileImport:
		OnImportRules();
		break;

	case cmd_FileExport:
		OnExportRules();
		break;

	default:
		cmdHandled = CTableWindow::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CRulesWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_FileImport:
	{
		outEnabled = true;
		LStr255 txt(STRx_Standards, str_ImportRules);
		::PLstrcpy(outName, txt);
		break;
	}
	case cmd_FileExport:
	{
		outEnabled = true;
		LStr255 txt(STRx_Standards, str_ExportRules);
		::PLstrcpy(outName, txt);
		break;
	}

	default:
		CTableWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Respond to clicks in the icon buttons
void CRulesWindow::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_RulesTabs:
		OnTabs(*(unsigned long*) ioParam);
		break;
	case msg_RulesShowTriggers:
		OnShowTriggers();
		break;
	}
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
	mRulesTitleTable->SetColWidth(newInfo.column_width, 1, 1);
	mRulesTable->SetColWidth(newInfo.column_width, 1, 1);

	// Scripts Name, width = 272
	newInfo.column_type = eRulesColumnName;
	newInfo.column_width = 272;
	mScriptsTitleTable->GetColumns().push_back(newInfo);
	mScriptsTitleTable->SetColWidth(newInfo.column_width, 1, 1);
	mScriptsTable->SetColWidth(newInfo.column_width, 1, 1);
}

void CRulesWindow::OnTabs(unsigned long index)
{
	mType = static_cast<CFilterItem::EType>(index - 1);
	mRulesTable->SetFilterType(mType);
	mScriptsTable->SetFilterType(mType);
	switch(mType)
	{
	case CFilterItem::eLocal:
	default:
		mLocalBtns->Show();
		mSIEVEBtns->Hide();
		break;
	case CFilterItem::eSIEVE:
		mSIEVEBtns->Show();
		mLocalBtns->Hide();
		break;
	}
	
	// Set trigger display
	OnShowTriggers();
}

void CRulesWindow::OnShowTriggers()
{
	// Take triggers into account
	bool show_triggers = ((mType != CFilterItem::eLocal) || mShowTriggers->GetValue());

	// Hide one part of the splitter
	mSplitter->ShowView(true, show_triggers);
}

void CRulesWindow::OnImportRules()
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
	PPx::FSObject	fspec;
	if (PP_StandardDialogs::AskChooseOneFile(0, fspec, kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews))
	{
		// Get full path and create stream
		cdstring fpath(fspec.GetPath());
		cdifstream fin(fpath, std::ios_base::in | std::ios_base::binary);
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

void CRulesWindow::OnExportRules()
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
	PPx::CFString cfstr("");
	PPx::FSObject fspec;
	bool replacing;
	if (PP_StandardDialogs::AskSaveFile(cfstr, 'TEXT', fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))
	{
		// Fix for window manager bug after a replace operation
		Activate();

		// Get full path and create stream
		cdstring fpath(fspec.GetPath());
		cdofstream fout(fpath, std::ios_base::in | std::ios_base::binary | std::ios_base::trunc);
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

void CRulesWindow::ResetTable(void)
{
	mRulesTable->ResetTable();
	mScriptsTable->ResetTable();
	ResetStandardSize();
}

// Reset state from prefs
void CRulesWindow::ResetState(bool force)
{
	// Get default state
	CRulesWindowState* state = &CPreferences::sPrefs->mRulesWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mRulesWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// Adjust size of tables
	//ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mRulesWindowDefault.GetValue()));

	// Adjust split pos
	mSplitter->SetRelativeSplitPos(state->GetSplitPos());

	// Set show triggers state
	mShowTriggers->SetValue(state->GetShowTriggers());

	// Set zoom state
	if (state->GetState() == eWindowStateMax)
	{
		ResetStandardSize();
		DoSetZoom(true);
	}

	// Show/hide window based on state
	if (state->GetHide())
		Hide();
	else
		Show();

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CRulesWindow::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CRulesWindowState state(NULL, &mUserBounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo, mSplitter->GetRelativeSplitPos(), mShowTriggers->GetValue(), !IsVisible());
	if (CPreferences::sPrefs->mRulesWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesWindowDefault.SetDirty();

} // CRulesWindow::SaveDefaultState

// Take column width into account
void CRulesWindow::ResetStandardSize(void)
{
	TableIndexT	row_num;
	TableIndexT	col_num;
	SDimension16	std_size = {0, 0};

	mTable->GetTableSize(row_num, col_num);

	// Use the table's image width and height and vertical position
	SDimension32 tableSize;
	SPoint32 tablePos;
	mTable->GetImageSize(tableSize);
	mTable->GetFrameLocation(tablePos);
	std_size.width = tableSize.width;
	std_size.height = tableSize.height + tablePos.v;

	// Adjust for vertical scrollbar
	std_size.width += 14;
	if (std_size.width < mMinMaxSize.left)
		std_size.width = mMinMaxSize.left;

	// Adjust for horizontal scroll bar
	std_size.height += 15;
	if (std_size.height < mMinMaxSize.top)
		std_size.height = mMinMaxSize.top;

	SetStandardSize(std_size);

} // CRulesWindow::ResetStandardSize

// Reset text traits from prefs
void CRulesWindow::ResetTextTraits(const TextTraitsRecord& list_traits)
{
	ResetTextTraits(list_traits, mRulesTitleTable, mRulesTable);
	ResetTextTraits(list_traits, mScriptsTitleTable, mScriptsTable);
}

void CRulesWindow::ResetTextTraits(const TextTraitsRecord& list_traits,
									CTitleTable* titles, LTableView* tbl)
{
	// Get old header row height
	short old_height = titles->GetRowHeight(1);

	CTableDrag* tbl1 = dynamic_cast<CTableDrag*>(tbl);
	CHierarchyTableDrag* tbl2 = dynamic_cast<CHierarchyTableDrag*>(tbl);

	// Change fonts
	titles->SetTextTraits(list_traits);
	if (tbl1)
		tbl1->SetTextTraits(list_traits);
	if (tbl2)
		tbl2->SetTextTraits(list_traits);

	// Get new row height
	short diff_height = titles->GetRowHeight(1) - old_height;

	// Change panes if row height changed
	if (diff_height)
	{
		// Offset and resize table
		tbl->GetSuperView()->MoveBy(0, diff_height, false);
		tbl->GetSuperView()->ResizeFrameBy(0, -diff_height, true);

		// Do header and button
		titles->ResizeFrameBy(0, diff_height, true);
	}
}
