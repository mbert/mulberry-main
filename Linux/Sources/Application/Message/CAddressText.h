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


// CAddressText.h : header file
//

#ifndef __CADDRESSTEXT__MULBERRY__
#define __CADDRESSTEXT__MULBERRY__

#include "CTextDisplay.h"

/////////////////////////////////////////////////////////////////////////////
// CAddressText window

class CAddressText : public CTextDisplay
{
// Construction
public:
	CAddressText(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	CAddressText(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	CAddressText(JXScrollbarSet* sbs, JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	virtual ~CAddressText();

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
								
private:
			void	CaptureAddress();
};

/////////////////////////////////////////////////////////////////////////////

#endif
