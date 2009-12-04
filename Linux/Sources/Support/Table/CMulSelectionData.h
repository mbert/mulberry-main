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


// Header for CTableDragAndDrop class

#ifndef __CMULSELECTIONDATA__MULBERRY__
#define __CMULSELECTIONDATA__MULBERRY__

#include <JXSelectionData.h>
#include "CTableDragAndDrop.h"

class CMulSelectionData : public JXSelectionData
{
public:
	CMulSelectionData(CTableDragAndDrop* table, const CTableDragAndDrop::CFlavorsList& flavors) 
		: JXSelectionData(table, "CMulSelectionData"), mTable(table),
			mDataFlavors(flavors), mDataType(0), mData(NULL), mDataLen(0){}
	
	virtual void AddTypes(const Atom selectionName) 
	{
		for(CTableDragAndDrop::CFlavorsList::const_iterator iter = mDataFlavors.begin(); iter != mDataFlavors.end(); iter++)
			AddType(*iter);
	}
	
	virtual JBoolean ConvertData(const Atom requestType, Atom* returnType,
										unsigned char** data, JSize* dataLength, JSize* bitsPerBlock) const 
	{
		if (mTable->RenderSelectionData(const_cast<CMulSelectionData*>(this), requestType))
		{
			*returnType = requestType;
			*data = mData;
			*dataLength = mDataLen;
			*bitsPerBlock = 8;
			//now set mData to NULL, since our caller will delete it
			mData = NULL;
			return kTrue;
		}
		else 
			return kFalse;
	}
	
	//This will be called by the various subtypes of CTableDragAndDrop
	//(inside of GetSelectionData, which they must override from JXWidget)
	//to pass us a pointer to the data.  We take ownership of data, so it must
	//be allocated off of the heap.
	void SetData(Atom type, unsigned char* data, unsigned long dataLen) 
		{
			mDataType = type;
			mData = data;
			mDataLen = dataLen;
		}

	//Here we delete mData.  If ConvertData was called then its caller has
	//taken ownership of mData, but we've set mData to NULL, so no damange is
	//done here.  If SetData was never called then mData is NULL, again no
	//damage.  If SetData was called, but ConvertData was never called, we'll
	//delete it when we are destroyed.
	virtual ~CMulSelectionData() 
		{
			delete mData;
		}
 protected:
	CTableDragAndDrop* mTable;
	CTableDragAndDrop::CFlavorsList mDataFlavors;
	Atom mDataType;
	mutable unsigned char* mData;
	unsigned long mDataLen;
};
#endif
