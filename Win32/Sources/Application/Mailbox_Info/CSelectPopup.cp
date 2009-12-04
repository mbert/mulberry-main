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


// Source for CSelectPopup class

#include "CSelectPopup.h"

#include "CMailboxInfoTable.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CSelectPopup::CSelectPopup()
{
}

// Default destructor
CSelectPopup::~CSelectPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CSelectPopup::SetupCurrentMenuItem(bool check)
{
	// Only when setting it up
	if (check)
	{
		CMenu* pPopup = GetPopupMenu();
		
		// Change name of labels
		for(short i = CMailboxInfoTable::eSelectBy_Label1; i <  CMailboxInfoTable::eSelectBy_Label1 + NMessage::eMaxLabels; i++)
			CUnicodeUtils::ModifyMenuUTF8(pPopup, i, MF_BYCOMMAND | MF_STRING, i, CPreferences::sPrefs->mLabels.GetValue()[i -  CMailboxInfoTable::eSelectBy_Label1]->name);
	}
}

