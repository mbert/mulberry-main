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


// Header for CPrefsDisplayStyles class

#ifndef __CPREFSDISPLAYSTYLES__MULBERRY__
#define __CPREFSDISPLAYSTYLES__MULBERRY__

#include "CPrefsDisplayPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayStyles = 5044;
const	PaneIDT		paneid_DSURLColor = 'UCOL';
const	PaneIDT		paneid_DSURLBold = 'UBLD';
const	PaneIDT		paneid_DSURLItalic = 'UITL';
const	PaneIDT		paneid_DSURLUnderline = 'UUND';
const	PaneIDT		paneid_DSURLSeenColor = 'VCOL';
const	PaneIDT		paneid_DSURLSeenBold = 'VBLD';
const	PaneIDT		paneid_DSURLSeenItalic = 'VITL';
const	PaneIDT		paneid_DSURLSeenUnderline = 'VUND';
const	PaneIDT		paneid_DSHeaderColor = 'HCOL';
const	PaneIDT		paneid_DSHeaderBold = 'HBLD';
const	PaneIDT		paneid_DSHeaderItalic = 'HITL';
const	PaneIDT		paneid_DSHeaderUnderline = 'HUND';
const	PaneIDT		paneid_DSTagColor = 'TCOL';
const	PaneIDT		paneid_DSTagBold = 'TBLD';
const	PaneIDT		paneid_DSTagItalic = 'TITL';
const	PaneIDT		paneid_DSTagUnderline = 'TUND';
const	PaneIDT		paneid_DSURLs = 'URLs';

// Mesages

// Resources

// Classes
class CTextDisplay;

struct SStyleTraits;

class	CPrefsDisplayStyles : public CPrefsDisplayPanel
{
private:
	SStyleItems		mURL;
	SStyleItems		mURLSeen;
	SStyleItems		mHeader;
	SStyleItems		mTag;
	CTextDisplay*	mURLs;

public:
	enum { class_ID = 'Dsty' };

					CPrefsDisplayStyles();
					CPrefsDisplayStyles(LStream *inStream);
	virtual 		~CPrefsDisplayStyles();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
