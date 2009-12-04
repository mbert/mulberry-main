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


// Header for CSearchCriteria class

#ifndef __CSEARCHCRITERIA__MULBERRY__
#define __CSEARCHCRITERIA__MULBERRY__

#include "CCriteriaBase.h"
#include "CBroadcaster.h"
#include "CListener.h"

#include "CFilterItem.h"
#include "CSearchItem.h"

// Classes
class CSearchCriteriaContainer;

class CSearchCriteria : public CCriteriaBase, public CBroadcaster, public CListener
{
public:
	// Messages for broadcast
	enum
	{
		eBroadcast_SearchCriteriaResized = 'rsiz'
	};

					CSearchCriteria(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CSearchCriteria();

	virtual void			OnCreate() = 0;

	virtual void	SetRules(bool rules)
	{
		mRules = rules;
	}

	virtual long			ShowOrAnd(bool show) = 0;
	virtual bool			IsOr() const = 0;
	virtual void			SetOr(bool use_or) = 0;

	virtual CSearchItem*	GetSearchItem() const = 0;
	virtual void			SetSearchItem(const CSearchItem* spec, bool negate = false) = 0;

protected:
	bool					  mRules;
	CSearchCriteriaContainer* mGroupItems;
	
	virtual CCriteriaBaseList& GetList();
	virtual void SwitchWith(CCriteriaBase* other);

	virtual void ListenTo_Message(long msg, void* param);	// Respond to list changes
	
			void MakeGroup(CFilterItem::EType type);
			void RemoveGroup();
};

#endif
