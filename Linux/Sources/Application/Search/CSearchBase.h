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


// CSearchBase.h

#ifndef __CSEARCHBASE__MULBERRY__
#define __CSEARCHBASE__MULBERRY__

#include "CListener.h"

#include "CFilterItem.h"

// Classes
class CSearchCriteria;
class CSearchCriteriaContainer;
class CSearchItem;
class CWindow;
class JXTextButton;
class JXWidget;

class CSearchBase : public CListener
{
public:
				CSearchBase(bool rules);
	virtual		~CSearchBase();

			void SelectNextCriteria(CSearchCriteria* previous);

protected:
	bool						mRules;
	CSearchCriteriaContainer*	mGroupItems;

	CFilterItem::EType		mFilterType;		// Used to toggle local/SIEVE switches

	virtual JXWidget* GetContainerWnd() = 0;
	virtual CWindow* GetParentView() = 0;
	virtual JXTextButton* GetFewerBtn() = 0;

	virtual void Resized(int dy) = 0;

	virtual void ListenTo_Message(long msg, void* param);	// Respond to changes

	void	OnMore();
	void	OnFewer();
	void	OnClear();

	void	MakeGroup();

	// message handlers
	void	InitCriteria(const CSearchItem* spec = NULL);
	void	AddCriteria(const CSearchItem* spec = NULL, bool use_or = true);
	void	RemoveCriteria(unsigned long num = 1);
	void	RemoveAllCriteria();

	CSearchItem*	ConstructSearch() const;
};

#endif
