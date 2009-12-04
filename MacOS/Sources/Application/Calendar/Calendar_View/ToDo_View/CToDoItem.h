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
#pragma once

#include <LView.h>
#include "CHelpTags.h"
#include <LCommander.h>
#include "CBroadcaster.h"
#include <LListener.h>

#include <vector>

#include "CICalendarVToDo.h"

class CCalendarTableBase;

class LCheckBox;

// ===========================================================================
//	CToDoItem

class	CToDoItem : public LView,
					public CHelpTagPane,
					public LCommander,
					public CBroadcaster,
					public LListener
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
	static CToDoItem* Create(LView* parent, const HIRect& frame, LCommander* inSuper);

						CToDoItem(const SPaneInfo &inPaneInfo,
									const SViewInfo &inViewInfo,
									LCommander* inSuper);
	virtual				~CToDoItem();

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	void SetDetails(iCal::CICalendarComponentExpandedShared& todo, CCalendarTableBase* table);
	void SetDetails(EType type);

	void SelectToDo(bool select, bool silent = false);

	const iCal::CICalendarComponentExpandedShared& GetVToDo() const
		{ return mVToDo; }

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons
	
	virtual void	ApplyForeAndBackColors() const;

protected:
	enum
	{
		eCheckbox_ID = 'CHCK'
	};

	LCheckBox*				mCompleted;

	EType									mType;
	iCal::CICalendarComponentExpandedShared	mVToDo;

	CCalendarTableBase*		mTable;
	bool					mIsSelected;
	bool					mIsCompleted;
	uint32_t				mColour;
	cdstring				mSummary;
	cdstring				mStatus;
	
	virtual void		FinishCreateSelf();

	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	virtual void		DrawSelf();

	virtual void		BeTarget();
	virtual void		DontBeTarget();

			void		OnEditToDo();
			void		OnDuplicateToDo();
			void		OnDeleteToDo();
			void		OnChangeCompleted();
			void		OnCompleted();
			
	virtual void		SetupTagText();
};

typedef std::vector<CToDoItem*> CToDoItemList;

#endif
