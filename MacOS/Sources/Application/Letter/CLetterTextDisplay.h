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


#ifndef __CLetterTextDisplay__MULBERRY__
#define __CLetterTextDisplay__MULBERRY__

#include "CEditFormattedTextDisplay.h"

class CLetterWindow;

class CLetterTextDisplay : public CEditFormattedTextDisplay
{
public:
	enum { class_ID = 'LETD' };
	
	CLetterTextDisplay(LStream *inStream);
	virtual ~CLetterTextDisplay();

protected:
	CLetterWindow*		mLetterWnd;

	virtual void	FinishCreateSelf(void);				// Get details of sub-panes

	virtual void	DoDragReceive(DragReference	inDragRef);				// Get multiple text items

};

#endif
