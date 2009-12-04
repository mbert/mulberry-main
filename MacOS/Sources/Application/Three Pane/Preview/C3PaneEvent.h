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


// Header for C3PaneEvent class

#ifndef __C3PANEEVENT__MULBERRY__
#define __C3PANEEVENT__MULBERRY__

#include "C3PaneViewPanel.h"

#include "CICalendarComponentExpanded.h"

// Constants
const	PaneIDT		paneid_3PaneEvent = 1738;
const	PaneIDT		paneid_3PaneEventDesc = 'DESC';
const	PaneIDT		paneid_3PaneEventZoom = 'ZOOM';

// Messages
const	MessageT	msg_3PaneEventZoom = 'ZOOM';

// Resources

// Classes
class CEventPreview;
class CStaticText;

class C3PaneEvent : public C3PaneViewPanel, public LListener
{
public:
	enum { class_ID = '3PNe' };

					C3PaneEvent();
					C3PaneEvent(LStream *inStream);
	virtual 		~C3PaneEvent();

	virtual bool	TestClose();

	virtual bool	IsSpecified() const;

	virtual CBaseView*	GetBaseView() const;
	CEventPreview*	GetEventView() const
		{ return mEventView; }

	void SetComponent(const iCal::CICalendarComponentExpandedShared& comp);

	virtual cdstring	GetTitle() const;
			void		SetTitle(const cdstring& title);
	virtual ResIDT		GetIconID() const;

protected:
	CStaticText*	mDescriptor;
	CEventPreview*	mEventView;

	virtual void	FinishCreateSelf(void);
	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

			void	OnZoom();
};

#endif
