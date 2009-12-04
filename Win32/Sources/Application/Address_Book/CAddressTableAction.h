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


// Header for CAddressTableAction class

#ifndef __CADDRESSTABLEACTION__MULBERRY__
#define __CADDRESSTABLEACTION__MULBERRY__

#include <WIN_LAction.h>

// Classes

class CAdbkAddress;
class CAddressTable;
class CAddressList;

class CAddressTableNewAction : public LAction {

private:
	CAddressTable*			mItsTable;
	CAddressList*			mNewItems;
	bool					mWasDirty;

public:

					CAddressTableNewAction(CAddressTable* itsTable,
												CAdbkAddress* addr,
												bool wasDirty);
					CAddressTableNewAction(CAddressTable* itsTable,
												CAddressList* addrs,
												bool wasDirty);
	virtual 		~CAddressTableNewAction();
	
	virtual	void	AddAddresses(CAddressList* new_addrs);				// Add new items
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CAddressTableEditAction : public LAction {

private:
	CAddressTable*			mItsTable;
	CAddressList*			mOldItems;
	CAddressList*			mNewItems;
	bool					mWasDirty;

public:

					CAddressTableEditAction(CAddressTable* itsTable,
												bool wasDirty);
	virtual 		~CAddressTableEditAction();
	
	virtual	void	AddEdit(CAdbkAddress* old_addr, CAdbkAddress* new_addr);	// Add edited items

protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

class CAddressTableDeleteAction : public LAction {

private:
	CAddressTable*			mItsTable;
	CAddressList*			mDeletedItems;
	bool					mWasDirty;

public:

					CAddressTableDeleteAction(CAddressTable* itsTable,
												CAddressList* deletedItems,
												bool wasDirty);
	virtual 		~CAddressTableDeleteAction();
	
protected:
	virtual void	RedoSelf();
	virtual void	UndoSelf();

};

#endif
