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


// Header for CAdbkSearchWindow class

#ifndef __CADBKSEARCHWINDOW__MULBERRY__
#define __CADBKSEARCHWINDOW__MULBERRY__

#include "CTableWindow.h"

#include "CAdbkSearchTable.h"

// Constants
const	PaneIDT		paneid_AdbkSearchWindow = 9550;
const	PaneIDT		paneid_AdbkSearchHeader = 'AHDR';
const	PaneIDT		paneid_AdbkSearchMethod = 'METH';
const	PaneIDT		paneid_AdbkSearchServer = 'SERV';
const	PaneIDT		paneid_AdbkSearchField = 'ITEM';
const	PaneIDT		paneid_AdbkSearchMatch = 'MTCH';
const	PaneIDT		paneid_AdbkSearchPopup = 'TYPE';
const	PaneIDT		paneid_AdbkSearchBtn = 'SRCH';
const	PaneIDT		paneid_AdbkSearchClearBtn = 'DELE';
const	PaneIDT		paneid_AdbkSearchMessageBtn = 'MSGE';
const 	PaneIDT		paneid_AdbkSearchLDAPView = 'LDPA';
const 	PaneIDT		paneid_AdbkSearchAdbkView = 'ABPA';


// Resources
const	ResIDT		RidL_CAdbkSearchTableBtns = 9550;

// Messages
const	MessageT	msg_AdbkSearchMethod = 'METH';
const	MessageT	msg_AdbkSearchServer = 'SERV';
const	MessageT	msg_AdbkSearchBtn = 'SRCH';
const	MessageT	msg_AdbkSearchClearBtn = 'DELE';
const	MessageT	msg_AdbkSearchMessageBtn = 'MSGE';

enum
{
	menu_AddrSearchMethodMulberry = 1,
	menu_AddrSearchMethodLDAP
};

enum
{
	menu_AddrSearchMatchExactly = 1,
	menu_AddrSearchMatchAtStart,
	menu_AddrSearchMatchAtEnd,
	menu_AddrSearchMatchAnywhere
};

// Classes

class	CAdbkSearchWindow : public CTableWindow
{
public:
	static CAdbkSearchWindow*		sAdbkSearch;

	enum { class_ID = 'ASrc' };

					CAdbkSearchWindow();
					CAdbkSearchWindow(LStream *inStream);
	virtual 		~CAdbkSearchWindow();

	virtual CAdbkSearchTable* GetTable(void) const
						{ return (CAdbkSearchTable*) mTable; }
	virtual void	ResetTable(void)
		{ GetTable()->ResetTable(); ResetStandardSize();}				// Reset the table

protected:
	virtual void	FinishCreateSelf(void);
	virtual void	ActivateSelf(void);						// Activate text item

public:
	virtual void	InitColumns(void);						// Init columns and text

	virtual void	ResetState(bool force = false);			// Reset state from prefs

private:
	virtual void	SaveDefaultState(void);					// Save state in prefs

};

#endif
