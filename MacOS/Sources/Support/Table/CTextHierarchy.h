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


// CTextHierarchy.h
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

#ifndef __CTEXTHIERARCHY__MULBERRY__
#define __CTEXTHIERARCHY__MULBERRY__

#include <LTextHierTable.h>

class CTextHierarchy : public LTextHierTable,
						public LBroadcaster
{
public:
	enum { class_ID = 'THie' };
	
						CTextHierarchy(LStream*inStream);
	virtual				~CTextHierarchy();

	virtual void		SelectionChanged();

protected:
	MessageT			mSelectMessage;

	virtual void		HiliteCellActively(
								const STableCell	&inCell,
								Boolean				inHilite);
	virtual void		HiliteCellInactively(
								const STableCell	&inCell,
								Boolean				inHilite);
};

#endif
