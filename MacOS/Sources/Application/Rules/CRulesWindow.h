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

#include "CRulesTable.h"

// Constants
const	PaneIDT		paneid_RulesWindow = 1200;
const	PaneIDT		paneid_RulesHeader = 'AHDR';
const	PaneIDT		paneid_RulesLocalBtns = 'LBAR';
const	PaneIDT		paneid_RulesSIEVEBtns = 'SBAR';
const	PaneIDT		paneid_RulesShowTriggers = 'SHOW';
const	PaneIDT		paneid_RulesTabs = 'TABS';
const	PaneIDT		paneid_RulesSplitter = 'SPLT';
const	PaneIDT		paneid_RulesSplit1 = 'SPL1';
const	PaneIDT		paneid_RulesSplit2 = 'SPL2';
const	PaneIDT		paneid_RulesRules = 'LIST';
const	PaneIDT		paneid_RulesRulesTitles = 'TITL';
const	PaneIDT		paneid_RulesScripts = 'SCPT';
const	PaneIDT		paneid_RulesScriptsTitles = 'SCTL';

// Messages
const	MessageT	msg_RulesShowTriggers = 'SHOW';
const	MessageT	msg_RulesTabs = 'TABS';

// Resources
const	ResIDT		RidL_CRulesWindowBtns = 1200;

// Classes
class CRulesTable;
class CScriptsTable;
class CSimpleTitleTable;
class CSplitterView;
class LBevelButton;

class CRulesWindow : public CTableWindow,
						 	public LListener
{
private:
	enum {
		default_left = 300,
		default_top = 40,
		default_right = 556,
		default_bottom = 260
	};

public:
	static CRulesWindow*		sRulesWindow;

	enum { class_ID = 'RMan' };

					CRulesWindow();
					CRulesWindow(LStream *inStream);
	virtual 		~CRulesWindow();

	static void CreateRulesWindow();		// Create it or bring it to the front
	static void DestroyRulesWindow();		// Destroy the window

	static void MakeRule(const CMessageList& msgs);	// Make rule from example messages

	virtual CRulesTable* GetTable(void) const
						{ return (CRulesTable*) mTable; }
	virtual void	ResetTable(void);				// Reset the table

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

protected:
	LView*				mLocalBtns;
	LView*				mSIEVEBtns;
	LBevelButton*		mShowTriggers;
	CSplitterView*		mSplitter;
	CSimpleTitleTable*	mRulesTitleTable;
	CRulesTable*		mRulesTable;
	CSimpleTitleTable*	mScriptsTitleTable;
	CScriptsTable*		mScriptsTable;

	CFilterItem::EType	mType;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

			void	OnTabs(unsigned long index);
			void	OnShowTriggers();
			
			void	OnImportRules();
			void	OnExportRules();

public:
	virtual void	InitColumns(void);						// Init columns and text

	virtual void	ResetState(bool force = false);			// Reset state from prefs

	virtual void	ResetStandardSize(void);				// Take column width into account
	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits);	// Reset text traits

private:
	virtual void	SaveDefaultState(void);					// Save state in prefs

	virtual void	ResetTextTraits(const TextTraitsRecord& list_traits,
									CTitleTable* title,
									LTableView* tbl);	// Reset text traits

};

#endif
