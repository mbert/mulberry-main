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


// Header for CCommander class

#ifndef __CCOMMANDER__MULBERRY__
#define __CCOMMANDER__MULBERRY__

#include "CCommanderProtect.h"

class CCommander
{
public:
	static CCommander* sTarget;

	CCommander(CCommander* super = NULL);
	virtual ~CCommander();

	static BOOL ProcessCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

	static CCommander* GetTarget()
		{ return sTarget; }
	static void SetTarget(CCommander* target)
		{ sTarget = target; }
	bool IsTarget() const
		{ return sTarget == this; }
	bool IsOnDuty() const;

	CCommander* GetSuperCommander() const
		{ return mSuperCommander; }
	void SetSuperCommander(CCommander* cmdr);

	virtual BOOL	HandleCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command
	virtual	bool	HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags);			// Handle character

protected:
	typedef std::vector<CCommander*> CCommanders;
	
	CCommander* 		mSuperCommander;
	CCommanders			mSubCommanders;
	CCommanderProtect	mProtect;

	void	AddSubCommander(CCommander* sub);
	void	RemoveSubCommander(CCommander* sub);

};

#endif
