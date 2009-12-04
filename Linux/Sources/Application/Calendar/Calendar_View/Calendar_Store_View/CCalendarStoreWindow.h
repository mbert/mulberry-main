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


// Header for CCalendarStoreWindow class

#ifndef __CCalendarStoreWindow__MULBERRY__
#define __CCalendarStoreWindow__MULBERRY__

#include "CTableViewWindow.h"

#include "CCalendarStoreView.h"

// Classes
class CToolbarView;

class	CCalendarStoreWindow : public CTableViewWindow
{
public:
	static CCalendarStoreWindow* sCalendarStoreManager;

					CCalendarStoreWindow(JXDirector* owner);
	virtual 		~CCalendarStoreWindow();

	static void CreateCalendarStoreWindow();
	static void DestroyCalendarStoreWindow();

	virtual void	OnCreate();

	CCalendarStoreView*	GetCalendarStoreView()
		{ return static_cast<CCalendarStoreView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
// begin JXLayout


// end JXLayout
	CToolbarView*		mToolbarView;
};

#endif
