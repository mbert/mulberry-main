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


// Header for CPrefsDisplayServer class

#ifndef __CPREFSDISPLAYSERVER__MULBERRY__
#define __CPREFSDISPLAYSERVER__MULBERRY__

#include "CPrefsDisplayPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplayServer = 5040;
const	PaneIDT		paneid_DMSOpenColour = 'OCOL';
const	PaneIDT		paneid_DMSClosedColour = 'CCOL';
const	PaneIDT		paneid_DMSOpenBold = 'OBLD';
const	PaneIDT		paneid_DMSClosedBold = 'CBLD';
const	PaneIDT		paneid_DMSOpenItalic = 'OITL';
const	PaneIDT		paneid_DMSClosedItalic = 'CITL';
const	PaneIDT		paneid_DMSOpenStrike = 'OSTK';
const	PaneIDT		paneid_DMSClosedStrike = 'CSTK';
const	PaneIDT		paneid_DMSOpenUnderline = 'OUND';
const	PaneIDT		paneid_DMSClosedUnderline = 'CUND';
const	PaneIDT		paneid_DMSServerColour = 'SCOL';
const	PaneIDT		paneid_DMSUseServer = 'SBKG';
const	PaneIDT		paneid_DMSFavouriteColour = 'FCOL';
const	PaneIDT		paneid_DMSUseFavourite = 'FBKG';
const	PaneIDT		paneid_DMSHierarchyColour = 'HCOL';
const	PaneIDT		paneid_DMSUseHierarchy = 'HBKG';

// Messages
const	MessageT	msg_DMSUseServer = 'SBKG';
const	MessageT	msg_DMSUseFavourite = 'FBKG';
const	MessageT	msg_DMSUseHierarchy = 'HBKG';

// Resources
const	ResIDT		RidL_CPrefsDisplayServerBtns = 5040;

// Classes
class LCheckBox;
class LGAColorSwatchControl;

class	CPrefsDisplayServer : public CPrefsDisplayPanel,
							public LListener
{
private:
	SFullStyleItems			mOpen;
	SFullStyleItems			mClosed;
	LGAColorSwatchControl*	mServerColour;
	LCheckBox*				mUseServer;
	LGAColorSwatchControl*	mFavouriteColour;
	LCheckBox*				mUseFavourite;
	LGAColorSwatchControl*	mHierarchyColour;
	LCheckBox*				mUseHierarchy;

public:
	enum { class_ID = 'Dsrv' };

					CPrefsDisplayServer();
					CPrefsDisplayServer(LStream *inStream);
	virtual 		~CPrefsDisplayServer();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data
};

#endif
