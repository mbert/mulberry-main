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


// Source for CCommander class

#include "CCommander.h"

#include "StValueChanger.h"

#include <algorithm>

CCommander* CCommander::sTarget;

CCommander::CCommander(CCommander* super)
{
	mSuperCommander = NULL;
	SetSuperCommander(super);
}

CCommander::~CCommander()
{
	// Remove it as the target
	if (IsTarget())
	{
		if (GetSuperCommander())
			SetTarget(GetSuperCommander());
		else
			SetTarget(NULL);
	}
	
	// Delete all sub-commanders
	while(mSubCommanders.size())
	{
		delete mSubCommanders.front();
		RemoveSubCommander(mSubCommanders.front());
	}

	SetSuperCommander(NULL);
}

bool CCommander::IsOnDuty() const
{
	CCommander* chain = sTarget;
	while(chain)
	{
		if (chain == this)
			return true;
		chain = chain->mSuperCommander;
	}
	
	return false;
}

void CCommander::SetSuperCommander(CCommander* cmdr)
{
	if (GetSuperCommander())
		GetSuperCommander()->RemoveSubCommander(this);

	mSuperCommander = cmdr;
	
	if (GetSuperCommander())
		GetSuperCommander()->AddSubCommander(this);
}

void CCommander::AddSubCommander(CCommander* sub)
{
	mSubCommanders.push_back(sub);
}

void CCommander::RemoveSubCommander(CCommander* sub)
{
	mSubCommanders.erase(std::remove(mSubCommanders.begin(), mSubCommanders.end(), sub), mSubCommanders.end());
}

// Special chain of command
BOOL CCommander::ProcessCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Get current target, if any
	if (GetTarget())
		// Pass command off to the target
		return dynamic_cast<CCmdTarget*>(GetTarget())->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	else
		return false;
}


// Special chain of command
BOOL CCommander::HandleCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;
	
	// Get current target, if any
	CCmdTarget* target = dynamic_cast<CCmdTarget*>(this);
	if (target && target->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// Try super commander if any
	else if (GetSuperCommander())
		return GetSuperCommander()->HandleCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	// No one wants it
	else
		return false;
}

// Special chain of command
bool CCommander::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mProtect, -1, WM_CHAR);
	if (_protect.AlreadyProcessing())
		return false;
	
	// Try super commander if any
	if (GetSuperCommander())
		return GetSuperCommander()->HandleChar(nChar, nRepCnt, nFlags);
	
	// No one wants it
	else
		return false;
}
