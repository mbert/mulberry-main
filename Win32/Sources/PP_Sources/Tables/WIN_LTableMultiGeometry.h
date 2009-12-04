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


//	Manages geometry of a TableView where rows and columns can have
//	different sizes

#ifndef _H_WIN_LTableMultiGeometry
#define _H_WIN_LTableMultiGeometry
#pragma once

#include <WIN_UTableHelpers.h>

#include <vector>

// ---------------------------------------------------------------------------

class	LTableMultiGeometry : public LTableGeometry {
public:
						LTableMultiGeometry(
								LTableView*			inTableView,
								UInt16				inColWidth,
								UInt16				inRowHeight);

	virtual				~LTableMultiGeometry();

	virtual void		GetImageCellBounds(
								const STableCell&	inCell,
								SInt32&				outLeft,
								SInt32&				outTop,
								SInt32&				outRight,
								SInt32&				outBottom) const;

	virtual TableIndexT	GetRowHitBy( const CPoint& inImagePt ) const;

	virtual TableIndexT	GetColHitBy( const CPoint& inImagePt ) const;

	virtual void		GetTableDimensions(
								UInt32&				outWidth,
								UInt32&				outHeight) const;

	virtual UInt16		GetRowHeight(
								TableIndexT			inRow) const;

	virtual void		SetRowHeight(
								UInt16				inHeight,
								TableIndexT			inFromRow,
								TableIndexT			inToRow);

	virtual UInt16		GetColWidth( TableIndexT inCol ) const;

	virtual void		SetColWidth(
								UInt16				inWidth,
								TableIndexT			inFromCol,
								TableIndexT			inToCol);

	virtual void		InsertRows(
								UInt32				inHowMany,
								TableIndexT			inAfterRow);

	virtual void		InsertCols(
								UInt32				inHowMany,
								TableIndexT			inAfterCol);

	virtual void		RemoveRows(
								UInt32				inHowMany,
								TableIndexT			inFromRow);

	virtual void		RemoveCols(
								UInt32				inHowMany,
								TableIndexT			inFromCol);

protected:
	std::vector<UInt16>		mRowHeights;
	std::vector<UInt16>		mColWidths;
	UInt16				mDefaultRowHeight;
	UInt16				mDefaultColWidth;
};

#endif
