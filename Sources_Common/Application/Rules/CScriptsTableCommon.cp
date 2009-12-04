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


// Source for CScriptsTable class

#include "CScriptsTable.h"

#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CGetStringDialog.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CResources.h"
#include "CRulesWindow.h"
#endif
#include "CStringUtils.h"
#include "CTargetsDialog.h"
#include "CUploadScriptDialog.h"

void CScriptsTable::DoSelectionChanged(void)
{
	CHierarchyTableDrag::DoSelectionChanged();

	UpdateButtons();
}

// Create a new script
void CScriptsTable::OnNewScript()
{
	if (mType == CFilterItem::eLocal)
	{
		// Create a new rule
		CTargetItem* rule = new CTargetItem;
		if (CTargetsDialog::PoseDialog(rule))
		{
			// Lock to prevent filter manager changes whilst running
			cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

			CPreferences::sPrefs->GetFilterManager()->GetTargets(CFilterItem::eLocal).push_back(rule);
			CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
			
			// Add new item to list
			TableIndexT	woRow = AddLastChildRow(0, &rule, sizeof(CFilterScript*), true, false);
			TableIndexT row = GetExposedIndex(woRow);
			if (row)
				RefreshRow(row - TABLE_ROW_ADJUST);
		}
		else
			delete rule;
	}
	else
	{
		// Get a new name for the mailbox (use old name as starter)
		cdstring script_name;
		if (CGetStringDialog::PoseDialog("Alerts::Rules::NEW_SCRIPT", script_name))
		{
			// Create new script
			CFilterScript* script = new CFilterScript;
			script->SetName(script_name);

			CPreferences::sPrefs->GetFilterManager()->GetScripts(CFilterItem::eSIEVE).push_back(script);
			CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
			
			// Add new item to list
			TableIndexT	woRow = AddLastChildRow(0, &script, sizeof(CFilterScript*), true, false);
			TableIndexT row = GetExposedIndex(woRow);
			if (row)
				RefreshRow(row - TABLE_ROW_ADJUST);
		}
	}
}

// Edit selected entries
void CScriptsTable::OnEditScript()
{
	// Edit each selected rule
	DoToSelection((DoToSelectionPP) &CScriptsTable::EditScript);
}

// Edit specified script
bool CScriptsTable::EditScript(TableIndexT row)
{
	bool done_edit = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;

	// Only if script
	if (!is_script)
		return false;

	// Get data item
	STableCell	woCell(woRow, 1);
	UInt32 dataSize = sizeof(CFilterScript*);
	GetCellData(woCell, &script, dataSize);

	if (mType == CFilterItem::eLocal)
	{
		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		// Create a new rule
		CTargetItem* rule = static_cast<CTargetItem*>(script);
		cdstring old_name = rule->GetName();
		if (CTargetsDialog::PoseDialog(rule))
		{
			// Replaced with new one
			CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
			RefreshRow(row);
			return true;
		}
		else
			return false;
	}
	else
	{
		// Get a new name for the mailbox (use old name as starter)
		cdstring script_name = script->GetName();
		if (CGetStringDialog::PoseDialog("Alerts::Rules::EDIT_SCRIPT", script_name))
		{
			// Rename script
			script->SetName(script_name);
			CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
			
			// Refresh
			RefreshRow(row);
			
			return true;
		}
		else
			return false;
	}
}

// Delete selected entries
void CScriptsTable::OnDeleteScript()
{
	// Check that this is what we want to do
	bool all_scripts = TestSelectionAnd((TestSelectionPP) &CScriptsTable::TestSelectionScript);
	bool all_rules = TestSelectionAnd((TestSelectionPP) &CScriptsTable::TestSelectionRule);

	if (CErrorHandler::PutCautionAlertRsrc(true,
										all_scripts ? "Alerts::Rules::DELETE_SCRIPT" :
											(all_rules ? "Alerts::Rules::DELETE_SCRIPTRULE" : "Alerts::Rules::DELETE_SCRIPTANDRULE")) == CErrorHandler::Ok)
	{
		// Delete each selected rule in reverse
		DoToSelection((DoToSelectionPP) &CScriptsTable::DeleteScripts, false);
		CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
	}
}

// Delete specified script
bool CScriptsTable::DeleteScripts(TableIndexT row)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;

	// Get data item
	CFilterScript* script = NULL;
	CFilterItem* filter = NULL;
	STableCell	woCell(woRow, 1);
	UInt32 dataSize = sizeof(void*);
	GetCellData(woCell, is_script ? (void*) &script : (void*) &filter, dataSize);

	// Get sibling index
	unsigned long index = static_cast<CNodeVectorTree*>(mCollapsableTree)->SiblingPosition(woRow) - 1;

	// Only if script
	if (is_script)
	{
		if (mType == CFilterItem::eLocal)
		{
			// Lock to prevent filter manager changes whilst running
			cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

			CTargetItemList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).begin() + index;
			CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).erase(iter);
		}
		else
		{
			CFilterScriptList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).begin() + index;
			CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).erase(iter);
		}
	}
	else
	{
		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		// Get script
		TableIndexT	parentRow = mCollapsableTree->GetParentIndex(woRow);
		STableCell	parentCell(parentRow, 1);
		dataSize = sizeof(CFilterScript*);
		GetCellData(parentCell, &script, dataSize);

		CFilterItems::iterator iter = script->GetFilters().begin() + index;
		script->GetFilters().erase(iter);
	}

	RemoveRows(1, woRow, true);
	return true;
}

// Generate selected entries
void CScriptsTable::OnGenerateScript()
{
	// Get script upload details from user
	CUploadScriptDialog::SUploadScript details;
	if (CUploadScriptDialog::PoseDialog(details))
	{
		// Edit each selected rule
		DoToSelection1((DoToSelection1PP) &CScriptsTable::GenerateScript, &details);
	}
}

// Generate specified script
bool CScriptsTable::GenerateScript(TableIndexT row, void* data)
{
	bool done_edit = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;

	// Only if script
	if (!is_script)
		return false;

	// Get data item
	STableCell	woCell(woRow, 1);
	UInt32 dataSize = sizeof(CFilterScript*);
	GetCellData(woCell, &script, dataSize);

	// Generate it
	CUploadScriptDialog::SUploadScript* details = reinterpret_cast<CUploadScriptDialog::SUploadScript*>(data);
	if (details->mFile)
		CPreferences::sPrefs->GetFilterManager()->WriteSIEVEScript(script);
	else
		CPreferences::sPrefs->GetFilterManager()->UploadSIEVEScript(script, details->mAccountIndex, details->mUpload, details->mActivate);

	return true;
}

// Test for selected item script
bool CScriptsTable::TestSelectionScript(TableIndexT row)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// This is a script
	return nestingLevel == 0;
}

// Test for selected item rule
bool CScriptsTable::TestSelectionRule(TableIndexT row)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// This is a rule
	return nestingLevel == 1;
}

bool CScriptsTable::TestCellSameScript(TableIndexT row, TableIndexT* parent)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	return (GetParentIndex(woRow) == *parent);
}
