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


// CTabs

#ifndef __CTABS__MULBERRY__
#define __CTABS__MULBERRY__

#include <LTabsControl.h>

// Panes
const	PaneIDT		paneid_Tabs = 'TABS';

// Messages
const	MessageT	msg_Tabs = 'TABS';

class LPlaceHolder;

class CTabs : public LTabsControl
{
public:
	enum { class_ID = 'Mtab' };

					CTabs(LStream *inStream);
	virtual 		~CTabs();

	virtual	void	RemoveTabButtonAt(ArrayIndexT inAtIndex);

protected:
	LPlaceHolder*		mPanelHost;			//	This is the view that panels get installed
													//		into when they are displayed in the tab
													//		panel, an API is provided for installing
													//		and removing panels														

	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual	LPlaceHolder* GetPanelHostView()
		{ return mPanelHost; }

	virtual	void	InstallPanel(LPane* inPanel,
									SInt16 inAlignment = -1,
									Boolean inRefresh = true);
	virtual	LPane*	RemovePanel();

};

#endif
