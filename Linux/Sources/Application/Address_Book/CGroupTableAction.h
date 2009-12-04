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

#include "UNX_LAction.h"

#include "CGroupList.h"

// Classes

class CGroup;
class CGroupTable;

class CGroupTableNewAction : public LAction
{
private:
	CGroupTable*			mItsTable;
	CGroupList*				mNewItems;
	bool					mWasDirty;

public:
					CGroupTableNewAction(CGroupTable* itsTable,
												CGroup* grp,
												bool wasDirty);
					CGroupTableNewAction(CGroupTable* itsTable,
												CGroupList* grps,
												bool wasDirty);
	virtual 		~CGroupTableNewAction();

	virtual	void	AddGroup(CGroup* new_grp);				// Add new item
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableEditAction : public LAction
{
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

protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableDeleteAction : public LAction
{
private:
	CGroupTable*			mItsTable;
	CGroupList*				mDeletedItems;
	bool					mWasDirty;

public:
					CGroupTableDeleteAction(CGroupTable* itsTable,
												CGroupList* deletedItems,
												bool wasDirty);
	virtual 		~CGroupTableDeleteAction();
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CGroupTableReplaceAction : public LAction
{
private:
	CGroupTable*			mItsTable;
	CGroupList*				mOldItems;
	CGroupList*				mNewItems;
	bool					mWasDirty;

public:
					CGroupTableReplaceAction(CGroupTable* itsTable,
													CGroup* oold_grp,
													CGroup* new_grp,
													bool wasDirty);
	virtual 		~CGroupTableReplaceAction();

	virtual	CGroup*	GetOldGroup(void)				// Get old group
						{ return mOldItems->front(); }
	virtual	CGroup*	GetNewGroup(void)				// Get new group
						{ return mNewItems->front(); }

protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

#endif
