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


// Header for C3PanePanel class

#ifndef __C3PANEPANEL__MULBERRY__
#define __C3PANEPANEL__MULBERRY__

#include "C3PaneWindowFwd.h"

// Constants
const	PaneIDT		paneid_3PaneView = 'VIEW';

// Messages

// Resources

// Classes
class C3PaneWindow;

class C3PanePanel : public LView
{
public:
					C3PanePanel();
					C3PanePanel(LStream *inStream);
	virtual 		~C3PanePanel();

	virtual bool	TestClose() = 0;
	virtual void	DoClose() = 0;

	virtual bool	HasFocus() const
		{ return false; }
	virtual void	Focus() {}

	virtual bool	IsSpecified() const
		{ return false; }

	virtual cdstring GetTitle() const
		{ return cdstring::null_str; }
	virtual ResIDT	GetIconID() const
		{ return 0; }

	virtual void	SetViewType(N3Pane::EViewType view);

protected:
	C3PaneWindow*		m3PaneWindow;
	N3Pane::EViewType	mViewType;

	virtual void	FinishCreateSelf(void);
};

#endif
