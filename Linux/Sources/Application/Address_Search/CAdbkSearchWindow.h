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
#include "CMbox.h"

#include "HPopupMenu.h"

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

class CAdbkSearchTitleTable;
class CAdbkServerPopup;
class CTextInputField;
template <class T> class HButtonText;
class JXMultiImageButton;
class JXStaticText;
class JXUpRect;

class	CAdbkSearchWindow : public CTableWindow
{
	friend class CAdbkSearchTable;

public:
	static CAdbkSearchWindow* sAdbkSearch;

		CAdbkSearchWindow(JXDirector* owner);
	virtual ~CAdbkSearchWindow();

	static CAdbkSearchWindow* ManualCreate();			// Manually create document
	static void DestroyWindow();

	CAdbkSearchTable* GetTable() const
		{ return (CAdbkSearchTable*) mTable; }
	CAdbkSearchTitleTable* GetTitles() const
		{ return (CAdbkSearchTitleTable*) mTitles; }
	void ResetTable()
		{ GetTable()->ResetTable();}				// Reset the table

protected:
// begin JXLayout

    JXUpRect*                        mHeader;
    HPopupMenu*                      mSourcePopup;
    JXStaticText*                    mFieldTitle1;
    JXStaticText*                    mServersTitle;
    CAdbkServerPopup*                mServerPopup;
    JXStaticText*                    mFieldTitle2;
    HPopupMenu*                      mFieldPopup;
    HPopupMenu*                      mMethodPopup;
    CTextInputField*                 mSearchText;
    HButtonText<JXMultiImageButton>* mSearchBtn;
    HButtonText<JXMultiImageButton>* mClearBtn;
    HButtonText<JXMultiImageButton>* mNewMessageBtn;

// end JXLayout

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void	Activate();						// Activate text item

			void	OnChangeSource(JIndex nID);

public:
	virtual void	InitColumns();						// Init columns and text

	virtual int		GetSortBy() {return 0;}				// No sorting
	virtual void	SetSortBy(int sort) {}					// No sorting

	virtual void	ResetState(bool force = false);			// Reset state from prefs

private:
	virtual void	SaveDefaultState();					// Save state in prefs

};

#endif
