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


// Header for CEventPreview class

#ifndef __CEVENTPREVIEW__MULBERRY__
#define __CEVENTPREVIEW__MULBERRY__

#include "CBaseView.h"

#include "CICalendarComponentExpanded.h"

// Constants
const	PaneIDT		paneid_EventPreviewTabs = 'TABS';
const	PaneIDT		paneid_EventPreviewDetails = 'DETA';
const	PaneIDT		paneid_EventPreviewEdit = 'EDIT';

// Messages
const	MessageT	msg_EventPreviewEdit = 'EDIT';

// Classes
class CTextDisplay;
class C3PaneWindow;
class LPushButton;

class CEventPreview : public CBaseView, public LListener
{
public:
	enum { class_ID = 'Epre' };

					CEventPreview();
					CEventPreview(LStream *inStream);
	virtual 		~CEventPreview();

	virtual void	MakeToolbars(CToolbarView* parent) {}

	virtual bool	HasFocus() const;
	virtual void	Focus();

	void SetComponent(const iCal::CICalendarComponentExpandedShared& comp);
	bool HasData() const;
	const cdstring& GetSummary() const
	{
		return mSummary;
	}

protected:
	CTextDisplay*	mDetails;
	LPushButton*	mEditBtn;
	cdstring		mSummary;
	iCal::CICalendarComponentExpandedShared	mComp;

	virtual void	FinishCreateSelf();					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);	// Respond to clicks in the icon buttons

			void	Edit();

			void	SetEvent(const iCal::CICalendarComponentExpandedShared& comp);
			void	SetToDo(const iCal::CICalendarComponentExpandedShared& comp);

public:
	virtual void	ResetState(bool force = false) {}			// Reset window state
	virtual void	SaveDefaultState() {}						// Save current state as default
};

#endif
