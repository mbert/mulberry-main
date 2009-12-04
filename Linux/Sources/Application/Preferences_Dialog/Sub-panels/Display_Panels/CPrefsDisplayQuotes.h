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

#include "CPrefsDisplayPanel.h"

class JXColorButton;
class CTextInputDisplay;
class JXTextCheckbox;

class CPrefsDisplayQuotes : public CPrefsDisplayPanel
{
public:
	CPrefsDisplayQuotes(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();
	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    JXColorButton*     mQuotation1Colour;
    JXTextCheckbox*    mQuotationBold;
    JXTextCheckbox*    mQuotationItalic;
    JXTextCheckbox*    mQuotationUnderline;
    JXColorButton*     mQuotation2Colour;
    JXColorButton*     mQuotation3Colour;
    JXColorButton*     mQuotation4Colour;
    JXColorButton*     mQuotation5Colour;
    JXTextCheckbox*    mUseQuotationBtn;
    CTextInputDisplay* mQuotes;

// end JXLayout1

	SStyleItems		mQuotation;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnUseQuotation();
};

#endif
