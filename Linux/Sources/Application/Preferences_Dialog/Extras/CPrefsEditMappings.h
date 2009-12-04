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


// CPrefsEditMappings.h : header file
//

#ifndef __CPREFSEDITMAPPINGS__MULBERRY__
#define __CPREFSEDITMAPPINGS__MULBERRY__

#include "CDialogDirector.h"
#include "CMIMEMap.h"
#include "CTable.h"

class CEditMappingsTable : public CTable
{
public:
	CEditMappingsTable(JXScrollbarSet* scrollbarSet,
						 JXContainer* enclosure,
						 const HSizingOption hSizing,
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h);

	virtual	void	OnCreate();
	virtual	void	SetMap(CMIMEMapVector* theMap);


	virtual	void	NewMap();
	virtual	void	EditMap();
	virtual	void	DeleteMap();

	virtual void RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh = false);

protected:
	virtual	void	LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers);				// Clicked somewhere

private:
	CMIMEMapVector*	mItsMap;

	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,
								const JRect& inLocalRect);				// Draw the items
};

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditMappings dialog

class JXTextButton;

class CPrefsEditMappings : public CDialogDirector
{
// Construction
public:
	CPrefsEditMappings(JXDirector* supervisor);

	static bool PoseDialog(CMIMEMapVector* theMap);

protected:
// begin JXLayout

    JXTextButton* mOkBtn;
    JXTextButton* mCancelBtn;
    JXTextButton* mAddBtn;
    JXTextButton* mChangeBtn;
    JXTextButton* mDeleteBtn;

// end JXLayout

	CEditMappingsTable*	mItsTable;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void OnAddMap();
			void OnChangeMap();
			void OnDeleteMap();
};

#endif
