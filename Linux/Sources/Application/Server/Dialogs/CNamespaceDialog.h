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


// Header for CNamespaceDialog class

#ifndef __CNAMESPACEDIALOG__MULBERRY__
#define __CNAMESPACEDIALOG__MULBERRY__

#include "CDialogDirector.h"
#include "CSimpleTitleTable.h"
#include "CTable.h"

#include "CMboxProtocol.h"

// Classes
class CTextInputDisplay;
class JXEngravedRect;
class JXStaticText;
class JXTextButton;
class JXTextCheckbox;
class JXUpRect;

class JXImage;
typedef std::vector<JXImage*> CIconArray;

class CNamespaceTable : public CTable
{
public:
	enum ENamespaceSelect
	{
		eNamespace_Personal,
		eNamespace_Shared,
		eNamespace_Public,
		eNamespace_All,
		eNamespace_None
	};

					CNamespaceTable(JXScrollbarSet* scrollbarSet,
						 JXContainer* enclosure,
						 const HSizingOption hSizing,
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h);
	virtual 		~CNamespaceTable();

			void	InitTable();							// Do odds & ends
			void	SetServerList(CMboxProtocol::SNamespace& servs, boolvector& servitems);		// Set namespaces
			void	ChangeSelection(ENamespaceSelect select);

protected:
	virtual void	DrawCell(JPainter* pDC, const STableCell& cell,
								const JRect& inLocalRect);				// Draw the items
	virtual	void	LClickCell(const STableCell& cell, 
			   					const JXKeyModifiers& modifiers);		// Clicked somewhere

private:
	CMboxProtocol::SNamespace*	mData;
	boolvector*					mDataOn;
	CIconArray					mIcons;
	cdstrvect					mTypeItems;
};

class	CNamespaceDialog : public CDialogDirector
{
public:
					CNamespaceDialog(JXDirector* supervisor);
	virtual 		~CNamespaceDialog();

	static  bool	PoseDialog(CMboxProtocol::SNamespace& server,
								boolvector& servitems,
								cdstrvect& items, bool& do_auto);

			void	SetItems(const cdstrvect& items,						// Set namespaces
								CMboxProtocol::SNamespace& servs,
								boolvector& servitems, bool do_auto);
			void	GetItems(cdstrvect& items, bool& do_auto);				// Get selected items
	
protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
// begin JXLayout

    JXStaticText*      mHelpText;
    JXEngravedRect*    mServer;
    JXStaticText*      mServerTitle;
    JXTextButton*      mPersonalBtn;
    JXTextButton*      mSharedBtn;
    JXTextButton*      mPublicBtn;
    JXTextButton*      mAllBtn;
    JXTextButton*      mNoneBtn;
    JXTextCheckbox*    mDoAuto;
    JXUpRect*          mUserItemsTitle;
    CTextInputDisplay* mUserItems;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mOKBtn;

// end JXLayout
	CSimpleTitleTable*	mTitles;
	CNamespaceTable* 	mTable;
};

#endif
