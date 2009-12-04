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

#ifndef __C3PaneEvent__MULBERRY__
#define __C3PaneEvent__MULBERRY__

#include "C3PaneViewPanel.h"

#include "CICalendarComponentExpanded.h"

// Classes
class CEventPreview;
class CStaticText;
class CToolbarButton;
class JXImageWidget;

class C3PaneEvent: public C3PaneViewPanel
{
public:
					C3PaneEvent(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~C3PaneEvent();

	virtual void	OnCreate();

	virtual bool	TestClose();

	virtual bool	IsSpecified() const;

	virtual CBaseView*	GetBaseView() const;
	CEventPreview*	GetEventView()
		{ return mEventView; }

	void SetComponent(const iCal::CICalendarComponentExpandedShared& comp);

	virtual cdstring 		GetTitle() const;
			void			SetTitle(const cdstring& title);
	virtual unsigned int	GetIconID() const;

protected:
// begin JXLayout1

    CToolbarButton* mZoomBtn;
    JXImageWidget*  mIcon;
    CStaticText*    mDescriptor;
    JXWidgetSet*    mViewPane;

// end JXLayout1
	CEventPreview*	mEventView;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	OnZoom();
};

#endif
