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


// Header for CRulesWindow class

#ifndef __CRULESWINDOW__MULBERRY__
#define __CRULESWINDOW__MULBERRY__

#include "CTableWindow.h"

#include "CFilterItem.h"

// Classes
class CMessageList;
class CRulesTable;
class CScriptsTable;
class CSimpleTitleTable;
class CSplitterView;
template <class T> class HButtonText;
class JXMultiImageButton;
class JXTabs;
class JXTextPushButton;
class JXUpRect;

class CRulesWindow : public CTableWindow
{
	friend class CRulesTable;
	friend class CScriptsTable;

public:
	static CRulesWindow*		sRulesWindow;

			CRulesWindow(JXDirector* owner);
	virtual ~CRulesWindow();

	static void CreateRulesWindow();		// Create it or bring it to the front
	static void DestroyRulesWindow();		// Destroy the window

	static void MakeRule(const CMessageList& msgs);	// Make rule from example messages

	virtual CRulesTable* GetTable(void) const
						{ return (CRulesTable*) mTable; }
	virtual void	ResetTable(void);				// Reset the table

protected:
// begin JXLayout

    JXTabs*                          mTabs;
    HButtonText<JXMultiImageButton>* mRuleBtn;
    HButtonText<JXMultiImageButton>* mTargetBtn;
    HButtonText<JXMultiImageButton>* mEditBtn;
    HButtonText<JXMultiImageButton>* mDeleteBtn;
    HButtonText<JXMultiImageButton>* mWriteBtn;
    HButtonText<JXMultiImageButton>* mScriptBtn;
    JXTextPushButton*                mShowTriggers;
    CSplitterView*                   mSplitter;

// end JXLayout
	CSimpleTitleTable*	mRulesTitleTable;
	CRulesTable*		mRulesTable;
	CSimpleTitleTable*	mScriptsTitleTable;
	CScriptsTable*		mScriptsTable;

	CFilterItem::EType	mType;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

			void	OnTabs(JIndex index);

			void	OnUpdateFileImport(CCmdUI* pCmdUI);
			void	OnUpdateFileExport(CCmdUI* pCmdUI);

			void	OnFileImport();
			void	OnFileExport();
			void	OnNewRule();
			void	OnNewTarget();
			void	OnNewScript();
			void	OnEditEntry();
			void	OnDeleteEntry();
			void	OnWriteScript();
			void	OnShowTriggers();


public:
	virtual void	InitColumns(void);						// Init columns and text

	virtual void	ResetState(bool force = false);			// Reset state from prefs
	virtual void	ResetFont(const SFontInfo& list_traits);	// Reset text traits

private:
	virtual void	SaveDefaultState();					// Save state in prefs

};

#endif
