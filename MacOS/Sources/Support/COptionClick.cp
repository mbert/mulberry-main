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


// Source for COptionClick class

#include "COptionClick.h"


// __________________________________________________________________________________________________
// C L A S S __ C O P T I O N C L I C K
// __________________________________________________________________________________________________

bool COptionClick::sOptionKey = false;
bool COptionClick::sShiftKey = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
COptionClick::COptionClick()
		: LAttachment(msg_Click, true)
{
	sOptionKey = false;
	sShiftKey = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set value based on option key
void COptionClick::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	sOptionKey = ((((SMouseDownEvent*) ioParam)->macEvent.modifiers & optionKey) != 0);
	sShiftKey = ((((SMouseDownEvent*) ioParam)->macEvent.modifiers & shiftKey) != 0);
}

// __________________________________________________________________________________________________
// C L A S S __ C O P T I O N M E N U
// __________________________________________________________________________________________________

bool COptionMenu::sOptionKey = false;
bool COptionMenu::sShiftKey = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
COptionMenu::COptionMenu()
		: LAttachment(msg_Event, true)
{
	sOptionKey = false;
	sShiftKey = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set value based on option key
void COptionMenu::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	sOptionKey = ((((EventRecord*) ioParam)->modifiers & optionKey) != 0);
	sShiftKey = ((((EventRecord*) ioParam)->modifiers & optionKey) != 0);
}

