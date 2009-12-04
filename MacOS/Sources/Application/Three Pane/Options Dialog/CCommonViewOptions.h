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


// Header for CCommonViewOptions class

#ifndef __CCOMMONOPTIONS__MULBERRY__
#define __CCOMMONOPTIONS__MULBERRY__

#include "CTabPanel.h"

#include "CUserAction.h"

// Constants
const	PaneIDT		paneid_CommonViewListPreview = 'PRE1';
const	PaneIDT		paneid_CommonViewListFullView = 'FUL1';
const	PaneIDT		paneid_CommonViewItemsPreview = 'PRE2';
const	PaneIDT		paneid_CommonViewItemsFullView = 'FUL2';

// Messages

// Resources

// Classes
class CUserActionOptions;

class CCommonViewOptions : public CTabPanel
{
public:
					CCommonViewOptions(LStream *inStream);
	virtual 		~CCommonViewOptions();

	virtual void	SetData(void* data) {}
	virtual bool	UpdateData(void* data) { return true; }

	void	SetCommonViewData(const CUserAction& listPreview,
								const CUserAction& listFullView,
								const CUserAction& itemsPreview,
								const CUserAction& itemsFullView,
								bool is3pane);
	void	GetCommonViewData(CUserAction& listPreview,
								CUserAction& listFullView,
								CUserAction& itemsPreview,
								CUserAction& itemsFullView);

protected:
	CUserActionOptions*	mListPreview;
	CUserActionOptions*	mListFullView;
	CUserActionOptions*	mItemsPreview;
	CUserActionOptions*	mItemsFullView;

	virtual void	FinishCreateSelf(void);
};

#endif
