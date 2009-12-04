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


// Header for CGroupTableAction class

#ifndef __CGROUPTABLEACTION__MULBERRY__
#define __CGROUPTABLEACTION__MULBERRY__

#include "CGroupList.h"

// Resources

const ResIDT	STRx_RedoGroupActions = 9003;
const ResIDT	STRx_UndoGroupActions = 9004;
enum
{
	str_GroupNew = 4,
	str_GroupEdit,
	str_GroupDelete,
	str_GroupAddressDelete
};

// Classes

class CGroup;
class CGroupList;
class CGroupTable;

class CGroupTableNewAction : public LAction {

private:
	CGroupTable*			mItsTable;
	CGroupList*				mNewItems;
	bool					mWasDirty;

public:

					CGroupTableNewAction(CGroupTable* itsTable,
												CGroup* grp,
												bool wasDirty);
	virtual 		~CGroupTableNewAction();

	virtual	void	AddGroup(CGroup* new_grp);				// Add new item
	
	virtual void		GetDescription(
								Str255		outRedoString,
								Str255		outUndoString) const;
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableEditAction : public LAction {

private:
	CGroupTable*			mItsTable;
	CGroupList*				mOldItems;
	CGroupList*				mNewItems;
	bool					mWasDirty;

public:

					CGroupTableEditAction(CGroupTable* itsTable,
												bool wasDirty);
	virtual 		~CGroupTableEditAction();
	
	virtual	void	AddEdit(CGroup* old_grp, CGroup* new_grp);	// Add edited items

	virtual void		GetDescription(
								Str255		outRedoString,
								Str255		outUndoString) const;
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableDeleteAction : public LAction {

private:
	CGroupTable*			mItsTable;
	CGroupList*				mDeletedItems;
	bool					mWasDirty;

public:

					CGroupTableDeleteAction(CGroupTable* itsTable,
												CGroupList* deletedItems,
												bool wasDirty);
	virtual 		~CGroupTableDeleteAction();
	
	virtual void		GetDescription(
								Str255		outRedoString,
								Str255		outUndoString) const;
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableReplaceAction : public LAction {

private:
	CGroupTable*			mItsTable;
	CGroupList*				mOldItems;
	CGroupList*				mNewItems;
	bool					mWasDirty;

public:

					CGroupTableReplaceAction(CGroupTable* itsTable,
													CGroup* old_grp,
													CGroup* new_grp,
													bool wasDirty);
	virtual 		~CGroupTableReplaceAction();

	virtual	CGroup*	GetOldGroup(void)				// Get old group
						{ return mOldItems->front(); }
	virtual	CGroup*	GetNewGroup(void)				// Get new group
						{ return mNewItems->front(); }

	virtual void		GetDescription(
								Str255		outRedoString,
								Str255		outUndoString) const;
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

#endif
