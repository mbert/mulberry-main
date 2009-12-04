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


//	CTabber.cp

#include "CTabber.h"

#include "CCmdEdit.h"
#include "CCmdEditView.h"
#include "CTable.h"

CTabber::CTabber()
{
	mTabNext = NULL;
	mTabPrev = NULL;
	mTabViewNext = NULL;
	mTabViewPrev = NULL;
	mTabSelectAll = true;
}

CTabber::~CTabber()
{
}

// Handle tab
void CTabber::DoTab()
{
	// Look for explicit tab order
	if (mTabNext || mTabPrev || mTabViewNext || mTabViewPrev)
	{
		CView* view = NULL;
		CWnd* wnd = NULL;
		CTabber* tabber = this;
		do
		{
			// Get target window and view
			if (::GetKeyState(VK_SHIFT) < 0)
			{
				view = tabber->GetPrevView();
				wnd = tabber->GetPrevWnd();
			}
			else
			{
				view = tabber->GetNextView();
				wnd = tabber->GetNextWnd();
			}
			
			// See if window is active and visible
			if (wnd)
			{
				if (wnd->IsWindowEnabled() && wnd->IsWindowVisible())
					tabber = NULL;
				else
					// Redo tab with new target
					tabber = dynamic_cast<CTabber*>(wnd);
			}

		} while(tabber && (tabber != this));

		// Only bother if not self
		if (tabber)
			return;

		// Make the targets active
		if (view) view->GetParentFrame()->SetActiveView(view);
		if (wnd) wnd->SetFocus();
		
		// See if select all required
		tabber = dynamic_cast<CTabber*>(wnd);
		if (tabber && tabber->GetTabSelectAll())
		{
			// Now check wnd type
			CCmdEdit* edit = dynamic_cast<CCmdEdit*>(wnd);
			if (edit && edit->GetWindowTextLength())
				edit->SetSel(0, -1);
			
			CCmdEditView* editv = dynamic_cast<CCmdEditView*>(wnd);
			if (editv && editv->GetTextLength())
				editv->SetSelectionRange(0, -1);
			
			CTable* table = dynamic_cast<CTable*>(wnd);
			if (table && table->GetItemCount())
				table->SelectRow(0);
		}
	}
	else
	{
		// Do tabbing based on WS_TABSTOP style
		CWnd* me = dynamic_cast<CWnd*>(this);
		if (!me)
			return;
		CWnd* top_level = me->GetParentFrame();
		if (!top_level)
			return;
		CWnd* next_tab = top_level->GetNextDlgTabItem(me, ::GetKeyState(VK_SHIFT) < 0);
		if (next_tab)
			next_tab->SetFocus();
	}

}