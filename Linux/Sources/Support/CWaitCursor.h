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


// CWaitCursor.h

// Unix class to duplicate Win32 CWaitCursor

#ifndef __CWAITCURSOR__MULBERRY__
#define __CWAITCURSOR__MULBERRY__

class CWaitCursor
{
public:
	CWaitCursor()
		{ StartBusyCursor(); }
	~CWaitCursor()
		{ StopBusyCursor(); }

	static void StartBusyCursor();
	static void StopBusyCursor();

private:
	static unsigned long sBusy;
};

#endif
