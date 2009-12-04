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


// CPrefsSubTab

#ifndef __CPREFSSUBTAB__MULBERRY__
#define __CPREFSSUBTAB__MULBERRY__

#include "CTabs.h"

#include <vector>


class CPrefsTabSubPanel;
class LCommander;

class CPrefsSubTab : public CTabs
{
public:
	enum { class_ID = 'Ptab' };

					CPrefsSubTab(LStream *inStream);
	virtual 		~CPrefsSubTab();

	virtual void	AddPanel(unsigned long id);
	virtual CPrefsTabSubPanel*	GetPanel(unsigned long index)
		{ return mPanels.at(index); }
	virtual void	ChangePanel(unsigned long id, unsigned long index);

	virtual void	ToggleICDisplay(bool IC_on);				// Toggle display of IC
	virtual void	SetData(void* data);						// Set data
	virtual void	UpdateData(void* data);						// Force update of data
	virtual void	SetDisplayPanel(unsigned long index);		// Force update of display panel

protected:
	std::vector<CPrefsTabSubPanel*>		mPanels;
	CPrefsTabSubPanel* mCurrentPanel;
};

#endif
