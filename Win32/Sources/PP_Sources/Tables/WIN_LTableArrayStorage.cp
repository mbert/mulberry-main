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

#include <WIN_LTableArrayStorage.h>



// ---------------------------------------------------------------------------
//	¥ LTableArrayStorage					Constructor				  [public]
// ---------------------------------------------------------------------------
//	Associates Storage with a particular TableView
//
//	Use inDataSize > 0 for cells which have the SAME size data
//	Use inDataSize == 0 for cells which have DIFFERENT size data

LTableArrayStorage::LTableArrayStorage(
	LTableView*	inTableView,
	UInt32		inDataSize)

	: LTableStorage(inTableView)
{
	//SignalIf_(mTableView == nil);

		// Allocate a new Array to store the data. The ArrayStorage
		// owns this Array, which it will delete in its destructor.

	mDataArray = new LTableStorageArray;

	mOwnsArray = true;
}


// ---------------------------------------------------------------------------
//	¥ LTableArrayStorage					Constructor				  [public]
// ---------------------------------------------------------------------------
//	Associates Storage with a particular TableView and specifies the Array
//	to use for storing the data for each cell
//
//	The Array will NOT be deleted when the Storage is deleted

LTableArrayStorage::LTableArrayStorage(
	LTableView*			inTableView,
	LTableStorageArray*	inDataArray)

	: LTableStorage(inTableView)
{
	//SignalIf_(inDataArray == nil);

	mDataArray = inDataArray;
	mOwnsArray = false;
}


// ---------------------------------------------------------------------------
//	¥ ~LTableArrayStorage					Destructor				  [public]
// ---------------------------------------------------------------------------

LTableArrayStorage::~LTableArrayStorage()
{
	if (mOwnsArray) {
		// Delete all assigned elements
		for(LTableStorageArray::iterator iter = mDataArray->begin(); iter != mDataArray->end(); iter++)
			std::free((*iter).first);
		delete mDataArray;
	}
}


// ---------------------------------------------------------------------------
//	¥ SetCellData													  [public]
// ---------------------------------------------------------------------------
//	Store data for a particular Cell

void
LTableArrayStorage::SetCellData(
	const STableCell&	inCell,
	const void*			inDataPtr,
	UInt32				inDataSize)
{
	TableIndexT	cellIndex;
	mTableView->CellToIndex(inCell, cellIndex);

	if (cellIndex > 0)
	{
		// Delete existing data
		std::free(mDataArray->at(cellIndex -1).first);
		
		// Create new data
		LTableStorageElement element(NULL, inDataSize);
		if (inDataSize > 0)
		{
			element.first = std::malloc(inDataSize);
			std::memcpy(element.first, inDataPtr, inDataSize);
		}

		mDataArray->at(cellIndex - 1) = element;
	}
}


// ---------------------------------------------------------------------------
//	¥ GetCellData													  [public]
// ---------------------------------------------------------------------------
//	Retrieve data for a particular Cell
//
//	If outDataPtr is nil, pass back the size of the Cell data
//
//	If outDataPtr is not nil, it must point to a buffer of at least
//	ioDataSize bytes. On output, ioDataSize is set to the minimum
//	of the Cell data size and the input value of ioDataSize and that
//	many bytes are copied to outDataPtr.

void
LTableArrayStorage::GetCellData(
	const STableCell&	inCell,
	void*				outDataPtr,
	UInt32&				ioDataSize) const
{
	TableIndexT	cellIndex;
	mTableView->CellToIndex(inCell, cellIndex);

	GetCellDataByIndex(cellIndex, outDataPtr, ioDataSize);
}


// ---------------------------------------------------------------------------
//	¥ GetCellDataByIndex											  [public]
// ---------------------------------------------------------------------------
//	Retrieve data for a particular Cell specified by index
//
//	If outDataPtr is nil, pass back the size of the Cell data
//
//	If outDataPtr is not nil, it must point to a buffer of at least
//	ioDataSize bytes. On output, ioDataSize is set to the minimum
//	of the Cell data size and the input value of ioDataSize and that
//	many bytes are copied to outDataPtr.

void
LTableArrayStorage::GetCellDataByIndex(
	TableIndexT		inCellIndex,
	void*			outDataPtr,
	UInt32&			ioDataSize) const
{
	if (outDataPtr == NULL) {
		ioDataSize = mDataArray->at(inCellIndex - 1).second;
	} else {
		ioDataSize = min(mDataArray->at(inCellIndex - 1).second, ioDataSize);
		::memcpy(outDataPtr, mDataArray->at(inCellIndex - 1).first, ioDataSize);
	}
}


// ---------------------------------------------------------------------------
//	¥ FindCellData													  [public]
// ---------------------------------------------------------------------------
//	Pass back the Cell containing the specified data. Returns whether
//	or not such a Cell was found.

// Functor to find matching cell data
class find_if_cell_data
{
public:
	find_if_cell_data(const LTableArrayStorage::LTableStorageElement& comp) : mComp(comp) {}

	bool operator() (const LTableArrayStorage::LTableStorageElement& elem)
	{
		return (elem.second == mComp.second) && (std::memcmp(elem.first, mComp.first, elem.second) == 0);
	}

private:
	const LTableArrayStorage::LTableStorageElement& mComp;
};

bool
LTableArrayStorage::FindCellData(
	STableCell&	outCell,
	const void*	inDataPtr,
	UInt32		inDataSize) const
{
	bool	found = false;

	LTableStorageElement comp(const_cast<void*>(inDataPtr), inDataSize);
	LTableStorageArray::const_iterator found_elem = std::find_if(mDataArray->begin(), mDataArray->end(), find_if_cell_data(comp));
	if (found_elem != mDataArray->end())
	{
		SInt32	dataIndex = found_elem - mDataArray->begin() + 1;
		mTableView->IndexToCell((TableIndexT) dataIndex, outCell);
		found = true;
	}

	return found;
}


// ---------------------------------------------------------------------------
//	¥ InsertRows													  [public]
// ---------------------------------------------------------------------------
//	Insert rows into an ArrayStorage.
//
//	inDataPtr points to the data for the new cells. Each new cell will
//		have the same data.

void
LTableArrayStorage::InsertRows(
	UInt32		inHowMany,
	TableIndexT	inAfterRow,
	const void*	inDataPtr,
	UInt32		inDataSize)
{
	STableCell	startCell(inAfterRow + 1, 1);
	UInt32		startIndex;
	mTableView->CellToIndex(startCell, startIndex);

	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	// Insert blank elements first - though all with the correct size
	LTableStorageElement element(NULL, inDataSize);
	mDataArray->insert(mDataArray->begin() + (startIndex - 1), inHowMany * cols, element);
	
	// Now create new data elements and store in each new array element
	for(LTableStorageArray::iterator iter = mDataArray->begin() + (startIndex - 1); iter != mDataArray->begin() + (startIndex - 1 + inHowMany * cols); iter++)
	{
		(*iter).first = std::malloc(inDataSize);
		std::memcpy((*iter).first, inDataPtr, inDataSize);
	}
}


// ---------------------------------------------------------------------------
//	¥ InsertCols													  [public]
// ---------------------------------------------------------------------------
//	Insert columns into an ArrayStorage.
//
//	inDataPtr points to the data for the new cells. Each new cell will
//		have the same data.

void
LTableArrayStorage::InsertCols(
	UInt32		inHowMany,
	TableIndexT	inAfterCol,
	const void*	inDataPtr,
	UInt32		inDataSize)
{
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	STableCell	theCell(0, inAfterCol + 1);

	for (theCell.row = 1; theCell.row <= rows; theCell.row++) {
		TableIndexT	startIndex;
		mTableView->CellToIndex(theCell, startIndex);

		// Insert blank elements first - though all with the correct size
		LTableStorageElement element(NULL, inDataSize);
		mDataArray->insert(mDataArray->begin() + (startIndex - 1), inHowMany, element);
		
		// Now create new data elements and store in each new array element
		for(LTableStorageArray::iterator iter = mDataArray->begin() + (startIndex - 1); iter != mDataArray->begin() + (startIndex - 1 + inHowMany); iter++)
		{
			(*iter).first = std::malloc(inDataSize);
			std::memcpy((*iter).first, inDataPtr, inDataSize);
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ RemoveRows													  [public]
// ---------------------------------------------------------------------------
//	Removes rows from an ArrayStorage

void
LTableArrayStorage::RemoveRows(
	UInt32		inHowMany,
	TableIndexT	inFromRow)
{
	STableCell	startCell(inFromRow, 1);
	UInt32		startIndex;
	mTableView->CellToIndex(startCell, startIndex);

	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	// Delete data first
	for(LTableStorageArray::iterator iter = mDataArray->begin() + (startIndex - 1); iter != mDataArray->begin() + (startIndex - 1 + inHowMany * cols); iter++)
		std::free((*iter).first);

	// Now remove elements from array
	mDataArray->erase(mDataArray->begin() + (startIndex - 1), mDataArray->begin() + (startIndex - 1 + inHowMany * cols));
}


// ---------------------------------------------------------------------------
//	¥ RemoveCols													  [public]
// ---------------------------------------------------------------------------
//	Removes columns from an ArrayStorage

void
LTableArrayStorage::RemoveCols(
	UInt32		inHowMany,
	TableIndexT	inFromCol)
{
	TableIndexT	rows, cols;					// Table size AFTER removal
	mTableView->GetTableSize(rows, cols);
	
	if (cols == 0) {						// All columns removed
		// Delete data first
		for(LTableStorageArray::iterator iter = mDataArray->begin(); iter != mDataArray->end(); iter++)
			std::free((*iter).first);
		mDataArray->clear();
		
	} else {
		STableCell	theCell(0, inFromCol);

		for (theCell.row = 1; theCell.row <= rows; theCell.row++) {
			TableIndexT	startIndex;
			mTableView->CellToIndex(theCell, startIndex);

			// Delete data first
			for(LTableStorageArray::iterator iter = mDataArray->begin() + (startIndex - 1); iter != mDataArray->begin() + (startIndex - 1 + inHowMany); iter++)
				std::free((*iter).first);

			// Now remove elements from array
			mDataArray->erase(mDataArray->begin() + (startIndex - 1), mDataArray->begin() + (startIndex - 1 + inHowMany));
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ GetStorageSize												  [public]
// ---------------------------------------------------------------------------
//	Pass back the number of rows and columns represented by the data
//	in an ArrayStorage

void
LTableArrayStorage::GetStorageSize(
	TableIndexT&	outRows,
	TableIndexT&	outCols)
{
		// An Array is one-dimensional. By default, we assume a
		// single column with each element being a separate row.

	outRows = mDataArray->size();
	outCols = 1;
	if (outRows == 0) {
		outCols = 0;
	}
}
