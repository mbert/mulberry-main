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


// CPrefsAddress.h : header file
//

#ifndef __CPREFSADDRESS__MULBERRY__
#define __CPREFSADDRESS__MULBERRY__

#include "CPrefsPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddress dialog

class CTabController;
class JXSecondaryRadioGroup;

class CPrefsAddress : public CPrefsPanel
{
// Construction
public:
	CPrefsAddress(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    JXSecondaryRadioGroup* mAddressChoiceGroup;
    CTabController*        mTabs;

// end JXLayout1
};

#endif
