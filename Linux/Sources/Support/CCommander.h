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


// Header for CCommander class

#ifndef __CCOMMANDER__MULBERRY__
#define __CCOMMANDER__MULBERRY__

#include <JBroadcaster.h>

#include "cdstring.h"

class JXContainer;
class JXKeyModifiers;
class JXTextMenu;
class JXWidget;

class CCommander
{
public:
	struct SMenuCommandChoice
	{
		JXTextMenu* mMenu;
		JIndex 		mIndex;

		SMenuCommandChoice()
			{ mMenu = NULL; mIndex = 0; }
	};

	class CCmdUI
	{
	public:
		unsigned long		mCmd;
		bool 				mEnable;
		bool 				mCheck;
		cdstring			mText;
		JXWidget*			mOther;
		JXTextMenu*			mMenu;
		JIndex 				mMenuIndex;

		CCmdUI()
			{ mCmd = 0; mEnable = false; mCheck = false;
			  mOther = NULL; mMenu = NULL; mMenuIndex = 0; }

		void Enable(bool enable)
			{ mEnable = enable; }
		bool GetEnabled() const
			{ return mEnable; }

		void SetCheck(bool check)
			{ mCheck = check; }

		void SetText(const cdstring& txt);
	};

	static CCommander* sTarget;

	CCommander(CCommander* super = NULL);
	CCommander(JXContainer* superview);
	virtual ~CCommander();

	static CCommander* GetTarget()
		{ return sTarget; }
	static void SetTarget(CCommander* target)
		{ sTarget = target; if (target) target->MakeTarget(); }
	bool IsTarget() const
		{ return sTarget == this; }
	bool IsOnDuty() const;
	void MakeTarget();
	void AddSubTarget(CCommander* cmdr);
	CCommander* GetSubTarget() const;

	CCommander* GetSuperCommander() const
		{ return mSuperCommander; }
	void SetSuperCommander(CCommander* cmdr);

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

protected:
	typedef std::vector<CCommander*> CCommanders;
	
	CCommander* mSuperCommander;
	CCommanders	mSubCommanders;
	CCommander* mLastSubTarget;

	virtual bool ReceiveMenu(JBroadcaster* sender, const JBroadcaster::Message& message);

	void	AddSubCommander(CCommander* sub);
	void	RemoveSubCommander(CCommander* sub);

	// Updaters
	void OnUpdateAlways(CCmdUI* cmdui)
		{ cmdui->Enable(true); }
	void OnUpdateNever(CCmdUI* cmdui)
		{ cmdui->Enable(false); }
};

#endif
