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


// Source for CRulesTable class

#include "CRulesTable.h"

#include "CAddressList.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMessage.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRulesDialog.h"
#include "CRulesWindow.h"
#include "CScriptsTable.h"
#include "CSMTPWindow.h"

#include <stdio.h>
#include <string.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// New address book
void CRulesTable::OnNewRules()
{
	// Create a new rule
	CFilterItem* rule = new CFilterItem(mType);
	bool trigger_change;
	if (CRulesDialog::PoseDialog(rule, trigger_change))
	{
		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		CPreferences::sPrefs->GetFilterManager()->AddRule(mType, rule);
		ResetTable();
		if (trigger_change)
			mScriptsTable->ResetTable();
	}
	else
		delete rule;
}

// Open address book
void CRulesTable::OnEditRules()
{
	// Edit each selected rule
	DoToSelection((DoToSelectionPP) &CRulesTable::EditRules);
}

// Edit a specified rule
bool CRulesTable::EditRules(TableIndexT row)
{
	// Edit a rule - NB locking is done in the dialog
	CFilterItem* rule = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row - TABLE_START_INDEX);
	bool trigger_change;
	if (CRulesDialog::PoseDialog(rule, trigger_change))
	{
		// Replaced with new one
		CPreferences::sPrefs->GetFilterManager()->ChangedRule(mType, rule);
		RefreshRow(row);
		
		// Refresh triggers/scripts if changed
		if (trigger_change)
			mScriptsTable->ResetTable();
		return true;
	}
	else
		return false;
}

// Delete address books
void CRulesTable::OnDeleteRules()
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Rules::REALLYDELETE") == CErrorHandler::Ok)
	{
		// Delete each selected rule in reverse
		DoToSelection((DoToSelectionPP) &CRulesTable::DeleteRules, false);
		ResetTable();
		mScriptsTable->ResetTable();
	}
}

// Delete specified rule
bool CRulesTable::DeleteRules(TableIndexT row)
{
	// Lock to prevent filter manager changes whilst running
	cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

	// Get its rule
	CFilterItem* rule = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row - TABLE_START_INDEX);

	// Remove it
	CPreferences::sPrefs->GetFilterManager()->RemoveRule(mType, rule);
	return true;
}

void CRulesTable::OnApplyRules()
{
	// Get list of all open mailboxes
	CMboxList selected;
	cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
	for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin();
			iter != CMailboxView::sMailboxViews->end(); iter++)
	{
		if ((!CSMTPWindow::sSMTPWindow || (*iter != CSMTPWindow::sSMTPWindow->GetMailboxView())) && (*iter)->GetMbox())
			selected.push_back((*iter)->GetMbox());
	}

	CPreferences::sPrefs->GetFilterManager()->ExecuteManual(selected);
}

void CRulesTable::MakeRule(const CMessageList& msgs)
{
	// Create a new rule
	CFilterItem* rule = new CFilterItem;
	CSearchItemList* list = new CSearchItemList;

	// Add criteria from each message
	for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		// Only if envelope exists
		const CEnvelope* env = (*iter)->GetEnvelope();
		if (!env)
			continue;

		// Use smart address test to determine whether to use From or To
		if ((*iter)->IsSmartFrom() && env->GetTo()->size())
		{
			// Add first To address
			list->push_back(new CSearchItem(CSearchItem::eTo, env->GetTo()->front()->GetFullAddress()));
		}
		else if (!(*iter)->IsSmartFrom() && env->GetFrom()->size())
		{
			// Add first From address
			list->push_back(new CSearchItem(CSearchItem::eFrom, env->GetFrom()->front()->GetFullAddress()));
		}
	
		// Add subject
		if (!env->GetSubject().empty())
			// Add first From address
			list->push_back(new CSearchItem(CSearchItem::eSubject, env->GetSubject()));
	}

	// Add items to rule
	if (list->size() > 1)
		rule->SetCriteria(new CSearchItem(CSearchItem::eOr, list));
	else if (list->size() == 1)
	{
		// Just add one from the list
		rule->SetCriteria(list->front());
		delete list;
	}

	// Now do rule edit dialog
	bool trigger_change;
	if (CRulesDialog::PoseDialog(rule, trigger_change))
	{
		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).push_back(rule);
		CPreferences::sPrefs->GetFilterManager()->ChangedFilters();
		ResetTable();
		if (trigger_change)
			mScriptsTable->ResetTable();
	}
	else
		delete rule;
}

// Add selected rules to list
bool CRulesTable::AddSelectionToList(TableIndexT row, CFilterItemList* list)
{
	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row - TABLE_START_INDEX);

	// Determine delete mailbox
	list->push_back(item);

	return true;
}

void CRulesTable::DoSelectionChanged(void)
{
	UpdateButtons();
}
