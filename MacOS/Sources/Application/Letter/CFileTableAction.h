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


// Header for CFileTableAction classes

#ifndef __CFILETABLEACTION__MULBERRY__
#define __CFILETABLEACTION__MULBERRY__


#include "CAttachmentList.h"

// Resources

const ResIDT	STRx_RedoFileTableActions = 3002;
const ResIDT	STRx_UndoFileTableActions = 3003;
enum {
	str_FileTableEdit = 1,
	str_FileTableDelete,
	str_FileTableAdd
};

// Consts

// Classes

class CFileTable;

class CFileTableAction : public LAction {

protected:
	CFileTable*			mOwner;
	CAttachmentList		mItems;
	LArray				mItemPos;

public:

				CFileTableAction(CFileTable *owner,
									ResIDT inStringResID,
									SInt16 inStringIndex);
	virtual 	~CFileTableAction();

};

class CFileTableDeleteAction : public CFileTableAction {

public:
					CFileTableDeleteAction(CFileTable *owner);
	virtual 		~CFileTableDeleteAction();
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};
class CFileTableAddAction : public CFileTableAction {

public:
					CFileTableAddAction(CFileTable *owner, CAttachment* add);
	virtual 		~CFileTableAddAction();
	
	virtual void	Add(CAttachment* add);

protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

#endif
