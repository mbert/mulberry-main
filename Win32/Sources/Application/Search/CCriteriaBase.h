/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for CCriteriaBase class

#ifndef __CCRITERIABASE__MULBERRY__
#define __CCRITERIABASE__MULBERRY__

#include "CWndAligner.h"

#include "CGrayBackground.h"
#include "CIconButton.h"

// Constants

// Classes
class CCriteriaBase;
typedef std::vector<CCriteriaBase*> CCriteriaBaseList;

class CCriteriaBase : public CStatic, public CWndAligner
{
public:
					CCriteriaBase();
	virtual 		~CCriteriaBase();

	void			SetTop(bool top);
	void			SetBottom(bool bottom);

protected:
	CGrayBackground		mHeader;
	CIconButton			mMoveUp;
	CIconButton			mMoveDown;

	afx_msg int 	OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnSetMove(UINT nID);
	
	virtual CCriteriaBaseList& GetList() = 0;
	virtual void	SwitchWith(CCriteriaBase* other) = 0;

	DECLARE_MESSAGE_MAP()
};

#endif
