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


// Header for CAddressPaneOptions class

#ifndef __CADDRESSPANEOPTIONS__MULBERRY__
#define __CADDRESSPANEOPTIONS__MULBERRY__

#include "CCommonViewOptions.h"

#include "C3PaneOptions.h"

// Classes
class CAddressViewOptions;
class CUserActionOptions;
class JXTextCheckbox;

class CAddressPaneOptions : public CCommonViewOptions
{
public:
	CAddressPaneOptions(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CCommonViewOptions(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual 		~CAddressPaneOptions() {}

	virtual void	OnCreate();					// Do odds & ends

	void	SetData(const CUserAction& listPreview,
					const CUserAction& listFullView,
					const CUserAction& itemsPreview,
					const CUserAction& itemsFullView,
					const CAddressViewOptions& options,
					bool is3pane);
	void	GetData(CUserAction& listPreview,
					CUserAction& listFullView,
					CUserAction& itemsPreview,
					CUserAction& itemsFullView,
					CAddressViewOptions& options);

protected:
// begin JXLayout1

    CUserActionOptions* mListPreview1;
    CUserActionOptions* mListFullView1;
    CUserActionOptions* mItemsPreview1;
    CUserActionOptions* mItemsFullView1;
    JXTextCheckbox*     mAddressSelect;

// end JXLayout1
};

#endif
