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


// Source for CMessageSpeechTableAction class

#include "CMessageSpeechTableAction.h"
#include "CMessageSpeechTable.h"


// __________________________________________________________________________________________________
// C L A S S __ CAddressTableAction
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSpeechTableAction::CMessageSpeechTableAction(CMessageSpeechTable* itsTable,
												CMessageSpeakVector original,
												CMessageSpeakVector changed) :
		LAction(STRx_RedoEdit, str_Clear, false)
{
	mItsTable = itsTable;
	mOriginal = original;
	mChanged = changed;
}

// Default destructor
CMessageSpeechTableAction::~CMessageSpeechTableAction()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

void CMessageSpeechTableAction::RedoSelf()
{
	// Replace with changed items
	*mItsTable->GetList() = mChanged;
	mItsTable->ResetTable();
}

void CMessageSpeechTableAction::UndoSelf()
{
	// Replace with original items
	*mItsTable->GetList() = mOriginal;
	mItsTable->ResetTable();
}