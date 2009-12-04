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


// Header for CACLStylePopup class

#ifndef __CACLSTYLEPOPUP__MULBERRY__
#define __CACLSTYLEPOPUP__MULBERRY__

#include "CIconButton.h"

#include "CACL.h"
#include "CPreferenceValue.h"

// Classes
class	CACLStylePopup : public CIconButton
{

	DECLARE_DYNCREATE(CACLStylePopup)

public:
					CACLStylePopup();
	virtual 		~CACLStylePopup();

	virtual void	SetPopupID(UINT popup_id)
						{ mPopupID = popup_id; }

	afx_msg	void OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere

	virtual void	Reset(bool mbox);						// Reset items
	virtual void	DoNewStyle(SACLRight rights);			// Add new style
	virtual void	DoDeleteStyle(void);					// Delete existing styles

private:
	UINT	mPopupID;
	bool	mMbox;
	CPreferenceValueMap<SACLStyleList>*	mList;

protected:
	DECLARE_MESSAGE_MAP()
};

#endif
