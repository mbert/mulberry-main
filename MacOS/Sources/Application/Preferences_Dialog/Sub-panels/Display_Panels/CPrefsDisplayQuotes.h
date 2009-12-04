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


// Header for CPrefsDisplayQuotes class

#ifndef __CPREFSDISPLAYQUOTES__MULBERRY__
#define __CPREFSDISPLAYQUOTES__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayQuotes = 5043;
const	PaneIDT		paneid_DQQuotation1Color = '1COL';
const	PaneIDT		paneid_DQQuotationBold = 'QBLD';
const	PaneIDT		paneid_DQQuotationItalic = 'QITL';
const	PaneIDT		paneid_DQQuotationUnderline = 'QUND';
const	PaneIDT		paneid_DQQuotation2Color = '2COL';
const	PaneIDT		paneid_DQQuotation3Color = '3COL';
const	PaneIDT		paneid_DQQuotation4Color = '4COL';
const	PaneIDT		paneid_DQQuotation5Color = '5COL';
const	PaneIDT		paneid_DQUseQuotation = 'USEM';
const	PaneIDT		paneid_DQQuotes = 'QUOT';

// Mesages
const	MessageT	msg_DQUseQuotation = 'USEM';

// Resources
const	ResIDT		RidL_CPrefsDisplayQuotesBtns = 5043;

// Classes
class LCheckBox;
class LGAColorSwatchControl;
class CTextDisplay;

class	CPrefsDisplayQuotes : public CPrefsTabSubPanel,
							public LListener
{
private:
	LGAColorSwatchControl*	mQuotation1Color;
	LCheckBox* 				mQuotationBold;
	LCheckBox* 				mQuotationItalic;
	LCheckBox* 				mQuotationUnderline;
	LGAColorSwatchControl*	mQuotation2Color;
	LGAColorSwatchControl*	mQuotation3Color;
	LGAColorSwatchControl*	mQuotation4Color;
	LGAColorSwatchControl*	mQuotation5Color;
	LCheckBox* 				mUseQuotation;
	CTextDisplay*			mQuotes;

public:
	enum { class_ID = 'Dqts' };

					CPrefsDisplayQuotes();
					CPrefsDisplayQuotes(LStream *inStream);
	virtual 		~CPrefsDisplayQuotes();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
