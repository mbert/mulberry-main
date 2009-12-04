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


// Header for C3PaneMainPanel class

#ifndef __C3PANEMAINPANEL__MULBERRY__
#define __C3PANEMAINPANEL__MULBERRY__

#include "C3PanePanel.h"

// Constants

// Messages

// Resources

// Classes
class C3PaneMainPanel : public C3PanePanel
{
public:
		
	enum { class_ID = '3PNP' };

					C3PaneMainPanel();
					C3PaneMainPanel(LStream *inStream);
	virtual 		~C3PaneMainPanel();

	virtual bool	TestClose();
	virtual void	DoClose();

			void	SetSubView(C3PanePanel* sub)
		{ mSubView = sub; }

protected:
	C3PanePanel*	mSubView;

	virtual void	FinishCreateSelf(void);
};

#endif
