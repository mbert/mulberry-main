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

#ifndef H_CNewComponentDescription
#define H_CNewComponentDescription
#pragma once

#include "CNewComponentPanel.h"

#include "CCmdEdit.h"

// ===========================================================================
//	CNewComponentDescription

class CNewComponentDescription : public CNewComponentPanel
{
public:
						CNewComponentDescription();
	virtual				~CNewComponentDescription();

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:
// Dialog Data
	//{{AFX_DATA(CNewComponentDescription)
	enum { IDD = IDD_CALENDAR_NEW_DESCRIPTION };
	CCmdEdit			mLocation;
	CCmdEdit			mDescription;
	//}}AFX_DATA

	// Generated message map functions
	//{{AFX_MSG(CNewComponentDescription)
	virtual BOOL	OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
