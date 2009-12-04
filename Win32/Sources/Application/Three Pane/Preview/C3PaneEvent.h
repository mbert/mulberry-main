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

#include "CEventPreview.h"
#include "CIconWnd.h"
#include "CToolbarButton.h"

#include "CICalendarComponentExpanded.h"

// Classes
class C3PaneEvent: public C3PaneViewPanel
{
public:
					C3PaneEvent();
	virtual 		~C3PaneEvent();

	virtual bool	TestClose();

	virtual bool	IsSpecified() const;

	virtual CBaseView*	GetBaseView() const;
	CEventPreview*	GetEventView()
		{ return &mEventView; }

	void SetComponent(const iCal::CICalendarComponentExpandedShared& comp);

	virtual cdstring	GetTitle() const;
			void		SetTitle(const cdstring& title);
	virtual UINT		GetIconID() const;

protected:
	CGrayBackground		mHeader;
	CIconWnd			mIcon;
	CStatic				mDescriptor;
	CToolbarButton	mZoom;
	CEventPreview	mEventView;

	// Generated message map functions
	//{{AFX_MSG()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnZoomPane(void);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
