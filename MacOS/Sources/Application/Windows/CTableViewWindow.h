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

#include "LWindow.h"
#include "CHelpTags.h"
#include "CWindowStatus.h"

#include "CTableView.h"

// Constants
const	PaneIDT		paneid_TableView = 'VIEW';

// Classes
class CTableView;
class CTitleTableView;
class CToolbarView;
class LTableView;

class CTableViewWindow : public LWindow,
							public CHelpTagWindow
{
public:
	static Point		sMouseUp;

					CTableViewWindow();
					CTableViewWindow(LStream *inStream);
	virtual 		~CTableViewWindow();

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	CTableView*	GetTableView()
		{ return mTableView; }
	LTableView*		GetBaseTable()
		{ return mTableView->GetBaseTable(); }
	CTitleTableView*	GetBaseTitles()
		{ return mTableView->GetBaseTitles(); }

	virtual CToolbarView* GetToolbarView() = 0;

	virtual void	ClickInContent(const EventRecord &inMacEvent);

	virtual void	ResetState(bool force = false);				// Reset window state
	virtual void	SaveState();								// Save current state in prefs
	virtual void	SaveDefaultState();							// Save current state as default

	virtual Boolean	CalcStandardBounds(Rect &outStdBounds) const;
	virtual void	ResetStandardSize();						// Take column width into account

			void	GetUserBounds(Rect& user_bounds) const
		{ user_bounds = mUserBounds; }

protected:
	CTableView*			mTableView;

	virtual void	FinishCreateSelf();

private:
			void	InitTableViewWindow();
};

#endif
