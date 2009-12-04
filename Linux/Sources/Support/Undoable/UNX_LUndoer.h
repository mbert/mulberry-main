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


//	Attachment for implementing Undo

#ifndef __UNX_LUNDOER__MULBERRY__
#define __UNX_LUNDOER__MULBERRY__

#include "jTypes.h"

class	LAction;

class JXTextMenu;

class	LUndoer {
public:
						LUndoer();
	virtual				~LUndoer();

	virtual void		PostAction(LAction* inAction);
	virtual void		ToggleAction(void);
	virtual void		FindUndoStatus(JXTextMenu* menu, JIndex item);
	
protected:
	LAction*			mAction;
};

#endif
