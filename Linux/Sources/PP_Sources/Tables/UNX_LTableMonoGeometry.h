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


//	Manages geometry of a TableView that has the same width and height for
//	every cell

#ifndef _H_UNX_LTableMonoGeometry
#define _H_UNX_LTableMonoGeometry

#include <UNX_UTableHelpers.h>

// ---------------------------------------------------------------------------

class	LTableMonoGeometry : public LTableGeometry {
public:
						LTableMonoGeometry(
								LTableView			*inTableView,
								UInt16				inColWidth,
								UInt16				inRowHeight);
	virtual				~LTableMonoGeometry() { }

	virtual void		GetImageCellBounds(
								const STableCell	&inCell,
								SInt32				&outLeft,
								SInt32				&outTop,
								SInt32				&outRight,
								SInt32				&outBottom) const;

	virtual TableIndexT	GetRowHitBy(
								const JPoint		&inImagePt) const;
	virtual TableIndexT	GetColHitBy(
								const JPoint		&inImagePt) const;

	virtual void		GetTableDimensions(
								UInt32				&outWidth,
								UInt32				&outHeight) const;

	virtual UInt16		GetRowHeight(
								TableIndexT			/* inRow */) const
							{
								return mRowHeight;
							}

	virtual void		SetRowHeight(
								UInt16				inHeight,
								TableIndexT			inFromRow,
								TableIndexT			inToRow);

	virtual UInt16		GetColWidth(
								TableIndexT			/* inCol */) const
							{
								return mColWidth;
							}

	virtual void		SetColWidth(
								UInt16				inWidth,
								TableIndexT			inFromCol,
								TableIndexT			inToCol);

protected:
	UInt16		mColWidth;
	UInt16		mRowHeight;
};

#endif
