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


//	Stores data for a TableView in an Array

#ifndef _H_LTableArrayStorage
#define _H_LTableArrayStorage

#include <UNX_UTableHelpers.h>

#include <vector>

// ---------------------------------------------------------------------------

class	LTableArrayStorage : public LTableStorage {
public:
	typedef std::pair<void*, UInt32>			LTableStorageElement;
	typedef std::vector<LTableStorageElement>	LTableStorageArray;

						LTableArrayStorage(
								LTableView*			inTableView,
								UInt32				inDataSize);

						LTableArrayStorage(
								LTableView*			inTableView,
								LTableStorageArray*	inDataArray);

	virtual				~LTableArrayStorage();

	virtual void		SetCellData(
								const STableCell&	inCell,
								const void*			inDataPtr,
								UInt32				inDataSize);

	virtual void		GetCellData(
								const STableCell&	inCell,
								void*				outDataPtr,
								UInt32&				ioDataSize) const;

	void				GetCellDataByIndex(
								TableIndexT			inCellIndex,
								void*				outDataPtr,
								UInt32&				ioDataSize) const;

	virtual bool		FindCellData(
								STableCell&			outCell,
								const void*			inDataPtr,
								UInt32				inDataSize) const;

	virtual void		InsertRows(
								UInt32				inHowMany,
								TableIndexT			inAfterRow,
								const void*			inDataPtr,
								UInt32				inDataSize);

	virtual void		InsertCols(
								UInt32				inHowMany,
								TableIndexT			inAfterCol,
								const void*			inDataPtr,
								UInt32				inDataSize);

	virtual void		RemoveRows(
								UInt32				inHowMany,
								TableIndexT			inFromRow);

	virtual void		RemoveCols(
								UInt32				inHowMany,
								TableIndexT			inFromCol);

	virtual void		GetStorageSize(
								TableIndexT&		outRows,
								TableIndexT&		outCols);

protected:
	LTableStorageArray			*mDataArray;
	bool						mOwnsArray;
};

#endif
