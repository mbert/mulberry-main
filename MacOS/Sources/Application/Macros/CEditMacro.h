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


// Header for CEditMacro class

#ifndef __CEDITMACRO__MULBERRY__
#define __CEDITMACRO__MULBERRY__

#include <LDialogBox.h>

#include "cdstring.h"


// Panes
const	PaneIDT		paneid_EditMacroDialog = 8011;
const	PaneIDT		paneid_EditMacroHeader = 'MHDR';
const	PaneIDT		paneid_EditMacroName = 'NAME';
const	PaneIDT		paneid_EditMacroRuler = 'RULE';
const	PaneIDT		paneid_EditMacroScroller = 'SCRL';
const	PaneIDT		paneid_EditMacroText = 'TEXT';

// Resources

// Messages

// Classes

class CEditMacro : public LDialogBox
{

public:
	enum { class_ID = 'EdMa' };

					CEditMacro();
					CEditMacro(LStream *inStream);
	virtual 		~CEditMacro();

	static bool PoseDialog(cdstring& name, cdstring& macro);

	virtual void	SetData(const cdstring& name, const cdstring& text);		// Set data
	virtual void	GetData(cdstring& name, cdstring& text);					// Get data
	virtual void	SetRuler();													// Set current wrap length

protected:
	virtual void	FinishCreateSelf(void);										// Do odds & ends
};

#endif
