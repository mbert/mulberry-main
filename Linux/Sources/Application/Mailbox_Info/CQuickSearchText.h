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


// CQuickSearchText.h : header file
//

#ifndef __CQUICKSEARCHTEXT__MULBERRY__
#define __CQUICKSEARCHTEXT__MULBERRY__

#include "CTextField.h"
#include "CBroadcaster.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchText window

class CQuickSearchText : public CTextInputField, public CBroadcaster
{
// Construction
public:
	enum
	{
		eBroadcast_Return = 'QSTr',
		eBroadcast_Tab = 'QSTt',
		eBroadcast_Key = 'QSTk'
	};

	CQuickSearchText(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual ~CQuickSearchText();

	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual void 		HandleFocusEvent();
	virtual void 		HandleUnfocusEvent();
	virtual void		HandleWindowFocusEvent();
	virtual void		HandleWindowUnfocusEvent();
	
private:
			void		SetFocus(bool focus);
};

/////////////////////////////////////////////////////////////////////////////

#endif
