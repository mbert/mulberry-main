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


// Header for CAdbkManagerWindow class

#ifndef __CADBKMANAGERWINDOW__MULBERRY__
#define __CADBKMANAGERWINDOW__MULBERRY__

#include "CTableViewWindow.h"

#include "CAdbkManagerView.h"

// Constants
const	PaneIDT		paneid_AdbkManagerWindow = 9500;
const	PaneIDT		paneid_AdbkMgrToolbarView = 'TBar';

// Classes
class CToolbarView;

class CAdbkManagerWindow : public CTableViewWindow
{
public:
	static CAdbkManagerWindow* sAdbkManager;

	enum { class_ID = 'AmWi' };

					CAdbkManagerWindow();
					CAdbkManagerWindow(LStream *inStream);
	virtual 		~CAdbkManagerWindow();

	static void CreateAdbkManagerWindow();
	static void DestroyAdbkManagerWindow();

	CAdbkManagerView*	GetAdbkManagerView()
		{ return static_cast<CAdbkManagerView*>(GetTableView()); }

	virtual CToolbarView* GetToolbarView()
		{ return mToolbarView; }

protected:
	CToolbarView*	mToolbarView;

	virtual void	FinishCreateSelf(void);
};

#endif
