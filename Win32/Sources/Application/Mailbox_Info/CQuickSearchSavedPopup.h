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


// CQuickSearchSavedPopup.h : header file
//

#ifndef __CQuickSearchSavedPopup__MULBERRY__
#define __CQuickSearchSavedPopup__MULBERRY__

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchSavedPopup window

class CQuickSearchSavedPopup : public CPopupButton
{
// Construction
public:
	enum
	{
		eChoose = 0,
		eDivider,
		eFirst
	};

	CQuickSearchSavedPopup();
	virtual ~CQuickSearchSavedPopup();

	virtual void	SetupCurrentMenuItem(bool check);						// Check items before doing popup

	// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
	
private:
	void	SyncMenu();

};

/////////////////////////////////////////////////////////////////////////////

#endif
