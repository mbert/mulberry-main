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


#ifndef __CFORMATLIST__MULBERRY__
#define __CFORMATLIST__MULBERRY__

#include "CFormatElement.h"

#if __dest_os == __linux_os
class CFormattedTextDisplay;
//typedef CFormattedTextDisplay CDisplayFormatter;
#define CDisplayFormatter CFormattedTextDisplay
#elif __dest_os == __mac_os || __dest_os == __mac_os_x
#define CDisplayFormatter CFormattedTextDisplay
#endif

class CDisplayFormatter;

class CFormatList
{
public:
	CFormatList();
	~CFormatList();
	
	void addElement(CFormatElement*);
	void draw(CDisplayFormatter* display);
	CFormatElement* GetElement();

private:
	CFormatElement* mList;
};

#endif
