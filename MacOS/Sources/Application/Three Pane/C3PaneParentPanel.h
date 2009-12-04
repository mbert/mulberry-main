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

// Constants

// Messages

// Resources

// Classes
class CBaseView;
class LIconControl;

class C3PaneParentPanel : public C3PanePanel, public CListener
{
public:
		
					C3PaneParentPanel();
					C3PaneParentPanel(LStream *inStream);
	virtual 		~C3PaneParentPanel();

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual void	ResetState() = 0;						// Reset state from prefs
	virtual void	SaveDefaultState() = 0;					// Save state in prefs

protected:
	LView*					mView;

	C3PanePanel*			mCurrent;

	virtual void	FinishCreateSelf(void);
	
			void	MakeToolbars(CBaseView* view);
};

#endif
