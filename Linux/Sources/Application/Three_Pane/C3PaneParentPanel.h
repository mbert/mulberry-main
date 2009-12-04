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


// Header for C3PaneParentPanel class

#ifndef __C3PANEPARENTPANEL__MULBERRY__
#define __C3PANEPARENTPANEL__MULBERRY__

#include "C3PanePanel.h"
#include "CListener.h"

// Classes
class CBaseView;

class C3PaneParentPanel : public C3PanePanel, public CListener
{
public:
	static C3PaneParentPanel* sCurrentFocus;
		
					C3PaneParentPanel(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~C3PaneParentPanel();

	virtual void	OnCreate();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	ResetState() = 0;						// Reset state from prefs
	virtual void	SaveDefaultState() = 0;					// Save state in prefs

protected:
	JXWidgetSet*	mView;
	C3PanePanel*	mCurrent;

			void	MakeToolbars(CBaseView* view);
};

#endif
