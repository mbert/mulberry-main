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


// CTextHierarchy.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 18-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS
//
// Description:
// This class implements a textual hierarchy table with message send when clicked.
//
// History:
// 16-Nov-1997: Created initial header and implementation.
//

#include "CTextHierarchy.h"

#include <LTableMonoGeometry.h>
#include <LTableSingleSelector.h>
#include <LTableArrayStorage.h>

CTextHierarchy::CTextHierarchy(LStream	*inStream)
	: LTextHierTable(inStream)
{
	*inStream >> mSelectMessage;

	SetTableGeometry(new LTableMonoGeometry(this, 175, 16));
	SetTableSelector(new LTableSingleSelector(this));
	SetTableStorage(new LTableArrayStorage(this, (UInt32) 0));
	InsertCols(1, 0);
}

CTextHierarchy::~CTextHierarchy()
{
}

// Broadcast message when selected cells change

void CTextHierarchy::SelectionChanged()
{
	if (mSelectMessage != msg_Nothing)
		BroadcastMessage(mSelectMessage, (void*) this);
}

void CTextHierarchy::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	ApplyForeAndBackColors();
	LTextHierTable::HiliteCellActively(inCell, inHilite);
}

void CTextHierarchy::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	ApplyForeAndBackColors();
	LTextHierTable::HiliteCellInactively(inCell, inHilite);
}
