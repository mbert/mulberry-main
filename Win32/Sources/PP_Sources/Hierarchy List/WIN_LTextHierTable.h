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


#ifndef __WIN_LTEXTHIERTABLE__MULBERRY__
#define __WIN_LTEXTHIERTABLE__MULBERRY__

#include <WIN_LHierarchyTable.h>
#include "cdstring.h"

// ---------------------------------------------------------------------------

class	LTextHierTable : public LHierarchyTable {
public:
						LTextHierTable();
	virtual				~LTextHierTable();

	virtual void		SetCellString(
								UInt32 					inWideOpenRow,
								const char*				inData)
		{ mData.insert(mData.begin() + (inWideOpenRow - 1), inData); }
	virtual const char* GetCellString(
								UInt32 					inWideOpenRow)
		{ return mData.at(inWideOpenRow - 1).c_str(); }

protected:
	cdstrvect	mData;

	virtual void	DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);				// Draw the items
};


#endif
