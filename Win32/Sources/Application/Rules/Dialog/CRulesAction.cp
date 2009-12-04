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


// Source for CRulesAction class

#include "CRulesAction.h"

#include "CRulesDialog.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesAction::CRulesAction()
{
}

// Default destructor
CRulesAction::~CRulesAction()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
BOOL CRulesAction::Create(const CRect& rect, CRulesDialog* dlog, CWnd* pParentWnd, CView* parentView)
{
	mDlog = dlog;

	return CRulesAction::CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, rect, pParentWnd, IDC_STATIC);
}

CCriteriaBaseList& CRulesAction::GetList()
{
	return mDlog->GetActions();
}

void CRulesAction::SwitchWith(CCriteriaBase* other)
{
	CActionItem* this_action = GetActionItem();
	SetActionItem(static_cast<CRulesAction*>(other)->GetActionItem());
	static_cast<CRulesAction*>(other)->SetActionItem(this_action);
}
