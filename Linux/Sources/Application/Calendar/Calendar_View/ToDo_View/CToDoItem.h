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

#ifndef H_CToDoItem
#define H_CToDoItem

#include <JXWidgetSet.h>
#include "CCommander.h"
#include "CBroadcaster.h"
#include "CContextMenu.h"

#include <vector>

#include "CICalendarVToDo.h"

class CCalendarTableBase;

class JXTextCheckbox;

// ===========================================================================
//	CToDoItem

class	CToDoItem : public CCommander,		// Commander must be first so it gets destroyed last
					public JXWidgetSet,
					public CBroadcaster,
					public CContextMenu
{
public:
	enum
	{
		eBroadcast_EditToDo = 'EdiT',			// param = NULL
		eBroadcast_SelectToDo = 'SelT'			// param = this
	};

	enum EType
	{
		eToDo = 0,
		eOverdue,
		eDueNow,
		eDueLater,
		eDone,
		eCancelled,
		eEmpty
	};

	static CToDoItem* Create(JXContainer* enclosure, const JRect& frame);

			CToDoItem(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual	~CToDoItem();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	void SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table);
	void SetDetails(EType type);

	void SelectToDo(bool select, bool silent = false);

	const iCal::CICalendarComponentExpandedShared& GetVToDo() const
		{ return mVToDo; }

	virtual void		HandleKeyPress(const int key, const JXKeyModifiers& modifiers);
	virtual void		Receive(JBroadcaster* sender, const Message& message);

protected:
	JXTextCheckbox*				mCompleted;

	EType									mType;
	iCal::CICalendarComponentExpandedShared	mVToDo;

	CCalendarTableBase*		mTable;
	bool					mIsSelected;
	bool					mIsCompleted;
	uint32_t				mColour;
	cdstring				mSummary;
	cdstring				mStatus;
	
	virtual void		OnCreate();

	virtual void 		HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void		Draw(JXWindowPainter& p, const JRect& rect);

	virtual void 		HandleFocusEvent();
	virtual void 		HandleUnfocusEvent();

			void		OnUpdateRealToDo(CCmdUI* pCmdUI);
			void		OnUpdateCompletedToDo(CCmdUI* pCmdUI);

			void		OnEditToDo();
			void		OnDuplicateToDo();
			void		OnDeleteToDo();
			void		OnChangeCompleted();
			void		OnCompleted();
			
	virtual void		SetupTagText();
};

typedef std::vector<CToDoItem*> CToDoItemList;

#endif
