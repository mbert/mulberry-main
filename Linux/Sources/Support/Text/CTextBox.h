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


// Header for CTextBox class

#ifndef __CTextBox__MULBERRY__
#define __CTextBox__MULBERRY__

#include "CMulberryCommon.h"

// Classes

class CTextBox
{
public:
		static void DrawText(JPainter* pDC, const char* theTxt, const JRect& box, EDrawStringAlignment align);

private:
		CTextBox() {}
		~CTextBox() {}
};
#endif
