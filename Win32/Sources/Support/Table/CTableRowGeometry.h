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


//	CTableRowGeometry.h

#ifndef __CTABLEROWGEOMETRY__MULBERRY__
#define __CTABLEROWGEOMETRY__MULBERRY__

#include <WIN_LTableMultiGeometry.h>

// Classes

class CTableRowGeometry : public LTableMultiGeometry {

public:
						CTableRowGeometry(
								LTableView			*inTableView,
								UInt16				inColWidth,
								UInt16				inRowHeight);
	virtual				~CTableRowGeometry();

	virtual void		SetDefaultRowHeight(const UInt16 default_height)
							{ mDefaultRowHeight = default_height; }

	virtual void		GetImageCellBounds(
								const STableCell	&inCell,
								SInt32				&outLeft,
								SInt32				&outTop,
								SInt32				&outRight,
								SInt32				&outBottom) const;
								
	virtual TableIndexT	GetRowHitBy(
								const CPoint		&inImagePt) const;
								
	virtual void		GetTableDimensions(
								UInt32				&outWidth,
								UInt32				&outHeight) const;
	
	virtual UInt16		GetRowHeight(
								TableIndexT			inRow) const;

	virtual void		SetRowHeight(
								UInt16				inHeight,
								TableIndexT			inFromRow,
								TableIndexT			inToRow);

};

#endif
