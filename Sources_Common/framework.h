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


// Header for common framework specific definitions

#ifndef __FRAMEWORK__MULBERRY__
#define __FRAMEWORK__MULBERRY__

// Frameworks
#define __powerplant	1
#define __mfc			2
#define __jx			3

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#define __framework		__powerplant
#elif __dest_os == __win32_os
#define __framework		__mfc
#elif __dest_os == __linux_os
#define __framework		__jx
#else
#error Unknown OS
#endif

// Powerplant defs
#if __framework == __powerplant

#define TABLE_START_INDEX		1
#define TABLE_ROW_ADJUST		0

#define PREFS_USE_LSTREAM		1

#define FRAMEWORK_DELETE_WINDOW(wnd)	\
	delete wnd;

#define FRAMEWORK_DELETE_DOC_WINDOW(wnd)	\
	delete (wnd)->GetDocument();

#define FRAMEWORK_REFRESH_WINDOW(wnd)	\
	(wnd)->Refresh();

#define FRAMEWORK_ENABLE_WINDOW(wnd)	\
	(wnd)->Enable();

#define FRAMEWORK_DISABLE_WINDOW(wnd)	\
	(wnd)->Disable();

#define FRAMEWORK_WINDOW_TO_TOP(wnd)	\
	{	\
		(wnd)->Show();	\
	  	(wnd)->Select();	\
	}

// MFC defs
#elif __framework == __mfc

//#define TABLE_START_INDEX		0
//#define TABLE_ROW_ADJUST		1
#define TABLE_START_INDEX		1
#define TABLE_ROW_ADJUST		0

#undef PREFS_USE_LSTREAM

#define FRAMEWORK_DELETE_WINDOW(wnd)	\
	(wnd)->GetDocument()->OnCloseDocument();

#define FRAMEWORK_DELETE_DOC_WINDOW(wnd)	\
	(wnd)->GetDocument()->OnCloseDocument();

#define FRAMEWORK_REFRESH_WINDOW(wnd)	\
	(wnd)->RedrawWindow(NULL, NULL, RDW_INVALIDATE);

#define FRAMEWORK_ENABLE_WINDOW(wnd)	\
	(wnd)->EnableWindow(true);

#define FRAMEWORK_DISABLE_WINDOW(wnd)	\
	(wnd)->EnableWindow(false);

#define FRAMEWORK_WINDOW_TO_TOP(wnd)	\
	{	\
		if ((wnd)->GetTopLevelParent()->IsIconic())	\
			(wnd)->GetTopLevelParent()->ShowWindow(SW_RESTORE);	\
		(wnd)->GetTopLevelParent()->BringWindowToTop();	\
	}

// JX defs
#elif __framework == __jx

#define TABLE_START_INDEX		1
#define TABLE_ROW_ADJUST		0

#define PREFS_USE_LSTREAM		1

#define FRAMEWORK_DELETE_WINDOW(wnd)	\
	(wnd)->Close();

#define FRAMEWORK_DELETE_DOC_WINDOW(wnd)	\
	(wnd)->Close();

#define FRAMEWORK_REFRESH_WINDOW(wnd)	\
	(wnd)->Refresh();

#define FRAMEWORK_ENABLE_WINDOW(wnd)	\
	(wnd)->Activate();

#define FRAMEWORK_DISABLE_WINDOW(wnd)	\
	(wnd)->Deactivate();

#define FRAMEWORK_WINDOW_TO_TOP(wnd)	\
	(wnd)->GetWindow()->Raise();

#endif

#endif
