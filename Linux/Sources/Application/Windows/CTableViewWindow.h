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


// Header for CTableViewWindow class

#ifndef __CTABLEVIEWWINDOW__MULBERRY__
#define __CTABLEVIEWWINDOW__MULBERRY__

#include "CWindow.h"

#include "CTableView.h"

// Classes
class CTableView;
class CTitleTableView;
class CToolbarView;
class CTable;

class CTableViewWindow : public CWindow
{
public:
					CTableViewWindow(JXDirector* owner);
	virtual 		~CTableViewWindow();

	void		SetTableView(CTableView* tblview)
		{ mTableView = tblview; }
	CTableView*	GetTableView()
		{ return mTableView; }
	CTable*		GetBaseTable()
		{ return mTableView->GetBaseTable(); }
	CTitleTableView*	GetBaseTitles()
		{ return mTableView->GetBaseTitles(); }

	virtual CToolbarView* GetToolbarView() = 0;

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

			void	ResetStandardSize();

protected:
	CTableView*			mTableView;

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual CCommander* GetTarget();

private:
			void	InitTableViewWindow();
};

#endif
