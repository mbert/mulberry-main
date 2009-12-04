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


// Header for C3PaneViewPanel class

#ifndef __C3PANEVIEWPANEL__MULBERRY__
#define __C3PANEVIEWPANEL__MULBERRY__

#include "C3PanePanel.h"

// Constants

// Messages

// Resources

// Classes
class CBaseView;

class C3PaneViewPanel : public C3PanePanel
{
public:
					C3PaneViewPanel();
					C3PaneViewPanel(LStream *inStream);
	virtual 		~C3PaneViewPanel();

	virtual bool	TestClose();
	virtual void	DoClose();

	virtual bool	HasFocus() const;
	virtual void	Focus();

	virtual CBaseView*	GetBaseView() const = 0;

};

#endif
