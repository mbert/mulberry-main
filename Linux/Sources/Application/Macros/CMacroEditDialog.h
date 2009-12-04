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


// Header for CMacroEditDialog class

#ifndef __CMACROEDITDIALOG__MULBERRY__
#define __CMACROEDITDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CTextTable.h"
#include "CTextMacros.h"

// Classes
class JXStaticText;
class JXTextButton;

class CMacroEditTable : public CTextTable
{
public:
	CMacroEditTable(JXScrollbarSet* scrollbarSet,
					  JXContainer* enclosure,
					  const HSizingOption hSizing, 
					  const VSizingOption vSizing,
					  const JCoordinate x, const JCoordinate y,
					  const JCoordinate w, const JCoordinate h);
	virtual 		~CMacroEditTable();

	virtual void	OnCreate();					// Do odds & ends
			void	SetData(CTextMacros* macros);

protected:

	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual void	DrawCell(JPainter* pDC, const STableCell& inCell,
									const JRect& inLocalRect);					// Draw the string

private:
	CTextMacros*	mCopy;
};

class CMacroEditDialog : public CDialogDirector
{
public:
					CMacroEditDialog(JXDirector* supervisor);
	virtual 		~CMacroEditDialog();

	static  void	PoseDialog();

protected:
// begin JXLayout

    JXTextButton* mNewBtn;
    JXTextButton* mEditBtn;
    JXTextButton* mDeleteBtn;
    JXStaticText* mKey;
    JXTextButton* mChooseBtn;
    JXTextButton* mCancelBtn;
    JXTextButton* mOKBtn;

// end JXLayout
	CMacroEditTable*	mTable;
	CTextMacros			mCopy;
	unsigned char		mActualKey;
	CKeyModifiers		mActualMods;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	SetDetails();
			void	GetDetails();

private:

	void	OnNewMacro();
	void	OnEditMacro();
	void	OnDeleteMacro();
	void	OnGetKey();
};

#endif
