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


// Header for CPrefsLetterStyled class

#ifndef __CPREFSLETTERSTYLED__MULBERRY__
#define __CPREFSLETTERSTYLED__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsLetterStyled = 5071;
const 	PaneIDT 	paneid_LSComposeAs = 'CTAS';
const 	PaneIDT 	paneid_LSEnrPlain = 'ENPL';
const 	PaneIDT 	paneid_LSEnrHTML = 'ENHT';
const 	PaneIDT 	paneid_LSHTMLPlain = 'HTPL';
const 	PaneIDT 	paneid_LSHTMLEnr = 'HTEN';
const 	PaneIDT 	paneid_LSFormatFlowed = 'FLOW';

// Mesages

// Resources

// Classes
class LCheckBox;
class LPopupButton;

class CPrefsLetterStyled : public CPrefsTabSubPanel
{
private:
	LPopupButton*	mComposeAs;
	LCheckBox*		mEnrichedPlain;
	LCheckBox*		mEnrichedHTML;
	LCheckBox*		mHTMLPlain;
	LCheckBox*		mHTMLEnriched;
	LCheckBox*		mFormatFlowed;

public:
	enum { class_ID = 'Lsty' };

					CPrefsLetterStyled();
					CPrefsLetterStyled(LStream *inStream);
	virtual 		~CPrefsLetterStyled();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
